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

#include "physfx/core/Frame.h"
#include "physfx/core/SceneContext.h"

namespace physfx::compositing {

/** @brief 色彩匹配器接口占位。 @todo Phase2 实现曝光、白平衡和色调匹配。 */
class ColorMatcher {
 public:
  /** @brief 匹配帧色彩。 @param frame 输入帧。 @param scene 场景光照。 @return 匹配后的帧。 */
  core::Frame match(const core::Frame& frame, const core::SceneContext& scene) const;
};

}  // namespace physfx::compositing
