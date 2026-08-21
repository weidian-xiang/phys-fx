/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <optional>

#include "physfx/editing/IEditCommand.h"

namespace physfx::editing::commands {

/** @brief 将线性或平滑数值关键帧曲线写入语义场景。 */
class PHYSFX_API AnimateParameter final : public IEditCommand {
 public:
  explicit AnimateParameter(core::ParameterCurve curve);
  [[nodiscard]] std::string_view name() const noexcept override;
  core::Status execute(core::SemanticScene& scene) override;
  core::Status undo(core::SemanticScene& scene) override;
  [[nodiscard]] std::string serialize() const override;

 private:
  core::ParameterCurve curve_{};
  std::optional<core::ParameterCurve> previous_{};
  bool inserted_{false};
};

}  // namespace physfx::editing::commands
