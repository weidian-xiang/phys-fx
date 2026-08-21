/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "physfx/compositing/ICompositor.h"

namespace physfx::compositing {

/** @brief 换季节模板的轻量天空/全局色调合成阶段。 */
class SeasonCompositor final : public ICompositor {
 public:
  core::Frame compose(const core::Frame& frame, const core::SceneContext& scene,
                      const core::SimulationResult& simulation) override;
};

}  // namespace physfx::compositing
