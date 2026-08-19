/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/perception/StubEstimators.h"

namespace physfx::perception {

core::LightParams StubLightEstimator::estimate(const core::Frame&) { return {}; }

core::DepthMap StubDepthEstimator::estimate(const core::Frame& frame) {
  return {.width = frame.width, .height = frame.height, .values = {}};
}

core::CameraPose StubCameraTracker::track(const core::Frame& frame) {
  return {.transform = {}, .timestampSeconds = frame.timestampSeconds};
}

core::Plane StubGroundPlaneDetector::detect(const core::DepthMap&) { return {}; }

core::OcclusionMask StubOcclusionEstimator::estimate(const core::Frame& frame,
                                                      const core::DepthMap&) {
  return {.width = frame.width, .height = frame.height, .values = {}};
}

}  // namespace physfx::perception
