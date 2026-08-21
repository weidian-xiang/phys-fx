/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "physfx/compositing/ICompositor.h"

namespace physfx::compositing {

/** @brief 将烟雾/泼水密度场以屏幕空间精灵方式合成到 RGB 帧。 */
class PHYSFX_API FluidDensityCompositor final : public ICompositor {
 public:
  core::Frame compose(const core::Frame& frame, const core::SceneContext& scene,
                      const core::SimulationResult& simulation) override;
};

}  // namespace physfx::compositing
