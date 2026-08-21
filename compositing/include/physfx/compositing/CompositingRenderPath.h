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

#include <memory>

#include "physfx/compositing/ICompositor.h"
#include "physfx/core/IRenderPath.h"

namespace physfx::compositing {

/** @brief 将物理合成器适配到统一渲染出口。 */
class PHYSFX_API CompositingRenderPath final : public core::IRenderPath {
 public:
  /** @brief 创建物理合成路径。 @param compositor 合成器实现。 */
  explicit CompositingRenderPath(std::unique_ptr<ICompositor> compositor);
  [[nodiscard]] std::string_view name() const noexcept override;
  core::Result<core::Frame> render(const core::Frame& frame, const core::SceneContext& scene,
                                   const core::SimulationResult& simulation) override;

 private:
  std::unique_ptr<ICompositor> compositor_;
};

}  // namespace physfx::compositing
