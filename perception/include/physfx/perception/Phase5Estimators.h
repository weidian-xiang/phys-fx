/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "physfx/perception/IDepthEstimator.h"
#include "physfx/perception/ILightEstimator.h"
#include "physfx/perception/ISkySegmenter.h"

namespace physfx::perception {

/**
 * @brief Depth-Anything 兼容的轻量 CPU 前处理器。
 *
 * 当 ONNX 权重不可用时仍提供确定性的单目相对深度，保证编辑器交互可用；
 * 接入真实权重时可用同一接口替换而无需修改管线。
 */
class DepthAnythingEstimator final : public IDepthEstimator {
 public:
  core::DepthMap estimate(const core::Frame& frame) override;
};

/** @brief 天空分割的无权重回退，实现顶部先验与颜色/梯度判别。 */
class SkySegmenter final : public ISkySegmenter {
 public:
  core::OcclusionMask segment(const core::Frame& frame) override;
};

/** @brief 基于亮度梯度的主光方向估计，输出稳定的屏幕空间方向。 */
class HeuristicLightEstimator final : public ILightEstimator {
 public:
  core::LightParams estimate(const core::Frame& frame) override;
};

}  // namespace physfx::perception
