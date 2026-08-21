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

/** @brief 遮挡混合器接口占位。 @todo Phase2 实现基于深度的前后景混合。 */
class PHYSFX_API OcclusionBlender {
 public:
  /** @brief 执行遮挡混合。 @param frame 输入帧。 @param mask 遮挡掩码。 @return 输出帧。 */
  core::Frame blend(const core::Frame& frame, const core::OcclusionMask& mask) const;
};

}  // namespace physfx::compositing
