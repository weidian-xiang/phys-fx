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

#include "physfx/neural_render/INeuralRenderer.h"

namespace physfx::neural_render {

/**
 * @brief 掩码区域的确定性 CPU 修复适配器。
 *
 * 这是无模型环境的工程基线：使用邻域颜色传播保持接口可运行，不宣称神经修复质量。
 * 后续 ProPainter/E2FGVI ONNX 后端应保持相同渲染出口。
 */
class InpaintingRenderer final : public INeuralRenderer {
 public:
  [[nodiscard]] std::string_view name() const noexcept override;
  core::Result<core::Frame> render(const core::Frame& frame, const core::SceneContext& scene,
                                   const core::SimulationResult& simulation) override;
};

}  // namespace physfx::neural_render
