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

#include "physfx/compositing/ICompositor.h"

namespace physfx::compositing {

/** @brief Phase 1 直通合成器，原样返回输入帧。 @todo Phase2 替换为真实渲染器。 */
class PassthroughCompositor final : public ICompositor {
 public:
  core::Frame compose(const core::Frame& frame,
                      const core::SceneContext& scene,
                      const core::SimulationResult& simulation) override;
};

}  // namespace physfx::compositing
