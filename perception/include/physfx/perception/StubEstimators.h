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

#include "physfx/perception/ICameraTracker.h"
#include "physfx/perception/IDepthEstimator.h"
#include "physfx/perception/IGroundPlaneDetector.h"
#include "physfx/perception/ILightEstimator.h"
#include "physfx/perception/IOcclusionEstimator.h"

namespace physfx::perception {

/** @brief Phase 1 光照估计桩。 @todo Phase2 替换为真实模型。 */
class StubLightEstimator final : public ILightEstimator {
 public:
  core::LightParams estimate(const core::Frame& frame) override;
};

/** @brief Phase 1 深度估计桩。 @todo Phase2 替换为真实模型。 */
class StubDepthEstimator final : public IDepthEstimator {
 public:
  core::DepthMap estimate(const core::Frame& frame) override;
};

/** @brief Phase 1 相机跟踪桩。 @todo Phase2 替换为真实跟踪器。 */
class StubCameraTracker final : public ICameraTracker {
 public:
  core::CameraPose track(const core::Frame& frame) override;
};

/** @brief Phase 1 地面平面检测桩。 @todo Phase2 替换为真实检测器。 */
class StubGroundPlaneDetector final : public IGroundPlaneDetector {
 public:
  core::Plane detect(const core::DepthMap& depth) override;
};

/** @brief Phase 1 遮挡估计桩。 @todo Phase2 替换为真实估计器。 */
class StubOcclusionEstimator final : public IOcclusionEstimator {
 public:
  core::OcclusionMask estimate(const core::Frame& frame, const core::DepthMap& depth) override;
};

}  // namespace physfx::perception
