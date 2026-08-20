/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/pipeline/StageFactory.h"

#include <memory>
#include <utility>

#include "physfx/compositing/CompositingRenderPath.h"
#include "physfx/compositing/LayeredCompositor.h"
#include "physfx/compositing/PassthroughCompositor.h"
#include "physfx/compositing/SpriteCompositor.h"
#include "physfx/editing/EditCommandStack.h"
#include "physfx/neural_render/INeuralRenderer.h"
#include "physfx/neural_render/InpaintingRenderer.h"
#include "physfx/perception/StubEstimators.h"
#include "physfx/physics/IPhysicsSimulator.h"
#include "physfx/physics/SimpleParticleSystem.h"
#include "physfx/physics/TaichiFluidSimulator.h"
#include "physfx/platform/VideoIoFactory.h"
#include "physfx/plugins/BuiltInEffectTemplates.h"
#include "physfx/plugins/BuiltInEditTemplates.h"
#include "physfx/plugins/PluginRegistry.h"
#include "physfx/semantics/OnnxSegmenter.h"
#include "physfx/semantics/OnnxTracker.h"
#include "physfx/semantics/StubSemantics.h"
#include "physfx/semantics/TraditionalSemantics.h"

namespace physfx::pipeline {

std::unique_ptr<Pipeline> StageFactory::createDefaultPipeline() {
  return createPipeline(core::Config{});
}

std::unique_ptr<Pipeline> StageFactory::createPipeline(const core::Config& config) {
  PipelineDependencies dependencies{};
  dependencies.lightEstimator = std::make_unique<perception::StubLightEstimator>();
  dependencies.depthEstimator = std::make_unique<perception::StubDepthEstimator>();
  dependencies.cameraTracker = std::make_unique<perception::StubCameraTracker>();
  dependencies.groundPlaneDetector = std::make_unique<perception::StubGroundPlaneDetector>();
  dependencies.occlusionEstimator = std::make_unique<perception::StubOcclusionEstimator>();
  if (config.semanticsBackend == "onnx") {
    dependencies.segmenter = std::make_unique<semantics::OnnxSegmenter>(config.segmenterModelPath);
    dependencies.tracker = std::make_unique<semantics::OnnxTracker>(config.trackerModelPath);
  } else if (config.semanticsBackend == "traditional") {
    dependencies.segmenter =
        std::make_unique<semantics::ColorPromptSegmenter>(config.promptX, config.promptY);
    dependencies.tracker = std::make_unique<semantics::CentroidTracker>();
  } else {
    dependencies.segmenter = std::make_unique<semantics::StubSegmenter>();
    dependencies.tracker = std::make_unique<semantics::StubTracker>();
  }
  dependencies.attributeEstimator = std::make_unique<semantics::StubEntityAttributeEstimator>();
  dependencies.editCommandStack = std::make_unique<editing::EditCommandStack>();
  if (config.physicsBackend == "taichi_smoke") {
    dependencies.physicsSimulator = std::make_unique<physics::TaichiFluidSimulator>();
  } else if (config.physicsBackend == "simple_particles") {
    dependencies.physicsSimulator = std::make_unique<physics::SimpleParticleSystem>();
    dependencies.physicsConfig.particlePreset = config.particlePreset == "smoke"
                                                    ? physics::ParticlePreset::kSmoke
                                                    : physics::ParticlePreset::kSparks;
    dependencies.physicsConfig.boundEntityId = config.boundEntityId;
    dependencies.physicsConfig.emitterPosition = config.editTarget;
    plugins::PluginRegistry templateRegistry;
    plugins::registerBuiltInEditTemplates(templateRegistry);
    if (plugins::registerBuiltInEffectTemplates(templateRegistry)) {
      const auto selected = std::dynamic_pointer_cast<plugins::BuiltInEffectTemplate>(
          templateRegistry.find(config.particlePreset));
      if (selected) {
        dependencies.physicsConfig.emissionRate = selected->parameters().emissionRate;
        dependencies.physicsConfig.maxParticles = selected->parameters().maxParticles;
      }
    }
  } else {
    dependencies.physicsSimulator = std::make_unique<physics::MockSimulator>();
  }
  if (config.editOperation == "remove") {
    dependencies.renderPath = std::make_unique<neural_render::InpaintingRenderer>();
  } else if (config.editOperation == "move" || config.editOperation == "copy" ||
             config.editOperation == "appearance") {
    dependencies.renderPath = std::make_unique<compositing::CompositingRenderPath>(
        std::make_unique<compositing::LayeredCompositor>());
  } else if (config.renderPath == core::RenderPathKind::kNeural) {
    dependencies.renderPath = std::make_unique<neural_render::PassthroughNeuralRenderer>();
  } else {
    std::unique_ptr<compositing::ICompositor> compositor;
    if (config.compositorBackend == "sprite") {
      compositor = std::make_unique<compositing::SpriteCompositor>();
    } else {
      compositor = std::make_unique<compositing::PassthroughCompositor>();
    }
    dependencies.renderPath =
        std::make_unique<compositing::CompositingRenderPath>(std::move(compositor));
  }
  dependencies.videoReader = platform::createVideoReader();
  dependencies.videoWriter = platform::createVideoWriter();
  return std::make_unique<Pipeline>(std::move(dependencies));
}

}  // namespace physfx::pipeline
