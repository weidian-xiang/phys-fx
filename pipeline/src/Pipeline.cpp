/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/pipeline/Pipeline.h"

#include <chrono>
#include <memory>
#include <sstream>
#include <string_view>
#include <utility>

#include "physfx/core/Log.h"
#include "physfx/core/SemanticScene.h"
#include "physfx/editing/commands/EmptyCommand.h"

namespace physfx::pipeline {

namespace {

using Clock = std::chrono::steady_clock;

void logStage(std::string_view stage, const Clock::time_point startedAt, std::string_view status) {
  const auto elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - startedAt);
  std::ostringstream message;
  message << "stage=" << stage << " status=" << status
          << " elapsed_ms=" << static_cast<double>(elapsed.count()) / 1000.0;
  core::Logger::info(message.str());
}

}  // namespace

Pipeline::Pipeline(PipelineDependencies dependencies) : dependencies_(std::move(dependencies)) {}

bool Pipeline::run(const core::Config& config) {
  const bool useVideoInput = !config.inputPath.empty();
  if (useVideoInput) {
    if (!dependencies_.videoReader) {
      core::Logger::error("video_io status=error reason=reader_missing");
      return false;
    }
    const auto status = dependencies_.videoReader->open(config.inputPath);
    if (!status.ok()) {
      core::Logger::error("video_io status=error reason=open_input_failed message=" +
                          status.message);
      return false;
    }
  }
  if (config.physicsEnabled &&
      (!dependencies_.physicsSimulator ||
       !dependencies_.physicsSimulator->initialize(dependencies_.physicsConfig).ok())) {
    core::Logger::error("stage=physics status=error reason=initialize_failed");
    return false;
  }

  const std::size_t syntheticFrameCount = config.frameCount == 0 ? 1 : config.frameCount;
  std::size_t index = 0;
  bool wroteAnyFrame = false;
  while ((useVideoInput && (config.frameCount == 0 || index < config.frameCount)) ||
         (!useVideoInput && index < syntheticFrameCount)) {
    PipelineContext context{};
    if (useVideoInput) {
      const auto status = dependencies_.videoReader->read(context.frame);
      if (status.code == core::StatusCode::kNotFound) {
        break;
      }
      if (!status.ok()) {
        core::Logger::error("video_io status=error reason=read_failed message=" + status.message);
        return false;
      }
    } else {
      context.frame.index = index;
      context.frame.timestampSeconds = static_cast<double>(index) / 30.0;
    }

    auto startedAt = Clock::now();
    if (config.perceptionEnabled) {
      if (!dependencies_.lightEstimator || !dependencies_.depthEstimator ||
          !dependencies_.cameraTracker || !dependencies_.groundPlaneDetector ||
          !dependencies_.occlusionEstimator) {
        core::Logger::error("stage=perception status=error reason=dependency_missing");
        return false;
      }
      context.scene.light = dependencies_.lightEstimator->estimate(context.frame);
      context.scene.depth = dependencies_.depthEstimator->estimate(context.frame);
      context.scene.camera.poses.push_back(dependencies_.cameraTracker->track(context.frame));
      context.scene.ground = dependencies_.groundPlaneDetector->detect(context.scene.depth);
      context.scene.occlusion =
          dependencies_.occlusionEstimator->estimate(context.frame, context.scene.depth);
      logStage("perception", startedAt, "ok");
    } else {
      logStage("perception", startedAt, "disabled");
    }

    startedAt = Clock::now();
    context.scene.semanticScene = std::make_shared<core::SemanticScene>();
    if (config.semanticsEnabled) {
      if (!dependencies_.segmenter || !dependencies_.tracker || !dependencies_.attributeEstimator) {
        core::Logger::error("stage=semantics status=error reason=dependency_missing");
        return false;
      }
      auto segmented = dependencies_.segmenter->segment(context.frame);
      if (!segmented.ok()) {
        core::Logger::error("stage=semantics status=error reason=segment_failed");
        return false;
      }
      auto tracked = dependencies_.tracker->track(context.frame, segmented.value());
      if (!tracked.ok()) {
        core::Logger::error("stage=semantics status=error reason=track_failed");
        return false;
      }
      context.scene.semanticScene->entities = std::move(tracked).value();
      for (auto& entity : context.scene.semanticScene->entities) {
        if (!dependencies_.attributeEstimator->estimate(context.frame, entity).ok()) {
          core::Logger::error("stage=semantics status=error reason=attribute_failed");
          return false;
        }
      }
      logStage("semantics", startedAt, "ok");
    } else {
      logStage("semantics", startedAt, "disabled");
    }

    startedAt = Clock::now();
    if (config.editingEnabled) {
      if (!dependencies_.editCommandStack ||
          !dependencies_.editCommandStack
               ->execute(std::make_unique<editing::commands::EmptyCommand>(),
                         *context.scene.semanticScene)
               .ok()) {
        core::Logger::error("stage=editing status=error reason=command_failed");
        return false;
      }
      logStage("editing", startedAt, "ok");
    } else {
      logStage("editing", startedAt, "disabled");
    }

    startedAt = Clock::now();
    if (config.physicsEnabled) {
      auto simulation = dependencies_.physicsSimulator->simulate(context.frame, context.scene);
      if (!simulation.ok()) {
        core::Logger::error("stage=physics status=error reason=simulate_failed");
        return false;
      }
      context.simulation = std::move(simulation).value();
      logStage("physics", startedAt, "ok");
    } else {
      logStage("physics", startedAt, "disabled");
    }

    startedAt = Clock::now();
    if (config.renderEnabled) {
      if (!dependencies_.renderPath) {
        core::Logger::error("stage=render status=error reason=dependency_missing");
        return false;
      }
      auto rendered =
          dependencies_.renderPath->render(context.frame, context.scene, context.simulation);
      if (!rendered.ok()) {
        core::Logger::error("stage=render status=error reason=render_failed");
        return false;
      }
      context.frame = std::move(rendered).value();
      logStage("render", startedAt, "ok");
    } else {
      logStage("render", startedAt, "disabled");
    }

    if (!config.outputPath.empty()) {
      if (!dependencies_.videoWriter) {
        core::Logger::error("video_io status=error reason=writer_missing");
        return false;
      }
      if (!dependencies_.videoWriter->isOpen()) {
        const auto status = dependencies_.videoWriter->open(config.outputPath, context.frame.width,
                                                            context.frame.height);
        if (!status.ok()) {
          core::Logger::error("video_io status=error reason=open_output_failed message=" +
                              status.message);
          return false;
        }
      }
      const auto status = dependencies_.videoWriter->write(context.frame);
      if (!status.ok()) {
        core::Logger::error("video_io status=error reason=write_failed message=" + status.message);
        return false;
      }
      wroteAnyFrame = true;
    }
    ++index;
  }
  if (dependencies_.videoReader) dependencies_.videoReader->close();
  if (dependencies_.videoWriter) dependencies_.videoWriter->close();
  if (useVideoInput && index == 0) {
    core::Logger::error("video_io status=error reason=no_frames");
    return false;
  }
  if (!config.outputPath.empty() && !wroteAnyFrame) {
    core::Logger::error("video_io status=error reason=no_output_frames");
    return false;
  }
  return true;
}

}  // namespace physfx::pipeline
