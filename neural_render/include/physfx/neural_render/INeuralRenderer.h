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

#include "physfx/core/IRenderPath.h"

namespace physfx::neural_render {

/** @brief NeRF、3DGS 与生成式视频重渲染的统一抽象。 */
class INeuralRenderer : public core::IRenderPath {
 public:
  ~INeuralRenderer() override = default;
};

/** @brief 原样返回输入帧的神经渲染桩。 */
class PassthroughNeuralRenderer final : public INeuralRenderer {
 public:
  [[nodiscard]] std::string_view name() const noexcept override;
  core::Result<core::Frame> render(const core::Frame& frame,
                                   const core::SceneContext& scene,
                                   const core::SimulationResult& simulation) override;
};

}  // namespace physfx::neural_render
