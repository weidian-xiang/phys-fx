/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "physfx/core/Frame.h"
#include "physfx/core/SceneContext.h"

namespace physfx::perception {

/** @brief 天空二分类接口；输出值 1 表示天空像素。 */
class ISkySegmenter {
 public:
  virtual ~ISkySegmenter() = default;
  virtual core::OcclusionMask segment(const core::Frame& frame) = 0;
};

}  // namespace physfx::perception
