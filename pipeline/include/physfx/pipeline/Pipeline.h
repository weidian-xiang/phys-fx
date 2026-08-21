/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <memory>

#include "physfx/core/Config.h"
#include "physfx/core/IRenderPath.h"
#include "physfx/editing/EditCommandStack.h"
#include "physfx/perception/ICameraTracker.h"
#include "physfx/perception/IDepthEstimator.h"
#include "physfx/perception/IGroundPlaneDetector.h"
#include "physfx/perception/ILightEstimator.h"
#include "physfx/perception/IOcclusionEstimator.h"
#include "physfx/physics/IPhysicsSimulator.h"
#include "physfx/pipeline/Stage.h"
#include "physfx/platform/IVideoReader.h"
#include "physfx/platform/IVideoWriter.h"
#include "physfx/semantics/IEntityAttributeEstimator.h"
#include "physfx/semantics/ISegmenter.h"
#include "physfx/semantics/ITracker.h"

namespace physfx::pipeline {

/** @brief 管线依赖集合，用于依赖注入和替换真实实现。 @todo Phase2 从配置文件构造。 */
struct PipelineDependencies {
  std::unique_ptr<perception::ILightEstimator> lightEstimator;
  std::unique_ptr<perception::IDepthEstimator> depthEstimator;
  std::unique_ptr<perception::ICameraTracker> cameraTracker;
  std::unique_ptr<perception::IGroundPlaneDetector> groundPlaneDetector;
  std::unique_ptr<perception::IOcclusionEstimator> occlusionEstimator;
  std::unique_ptr<semantics::ISegmenter> segmenter;
  std::unique_ptr<semantics::ITracker> tracker;
  std::unique_ptr<semantics::IEntityAttributeEstimator> attributeEstimator;
  std::unique_ptr<editing::EditCommandStack> editCommandStack;
  std::unique_ptr<physics::IPhysicsSimulator> physicsSimulator;
  physics::SimConfig physicsConfig{};
  std::unique_ptr<core::IRenderPath> renderPath;
  std::unique_ptr<platform::IVideoReader> videoReader;
  std::unique_ptr<platform::IVideoWriter> videoWriter;
};

/**
 * @brief 编排 perception → semantics → editing → physics → render 的逐帧管线。
 * @todo Phase3 支持多输入、多输出和可配置 DAG。
 */
class PHYSFX_API Pipeline {
 public:
  /** @brief 使用注入的模块创建管线。 @param dependencies 模块依赖集合。 */
  explicit Pipeline(PipelineDependencies dependencies);

  /**
   * @brief 运行空管线。
   * @param config 运行配置。
   * @return 是否成功完成所有帧。
   */
  bool run(const core::Config& config);

 private:
  PipelineDependencies dependencies_;
};

}  // namespace physfx::pipeline
