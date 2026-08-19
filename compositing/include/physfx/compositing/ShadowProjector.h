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

#include "physfx/core/SceneContext.h"

namespace physfx::compositing {

/** @brief 阴影投射器接口占位。 @todo Phase2 实现软阴影和地面接触阴影。 */
class ShadowProjector {
 public:
  /** @brief 检查投影条件。 @param scene 场景上下文。 @return 当前是否可投影。 */
  [[nodiscard]] bool canProject(const core::SceneContext& scene) const noexcept;
};

}  // namespace physfx::compositing
