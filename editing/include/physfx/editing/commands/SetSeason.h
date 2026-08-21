/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <optional>

#include "physfx/editing/IEditCommand.h"

namespace physfx::editing::commands {

/** @brief 设置全局季节预设；换季节模板将其与天空和色彩阶段组合。 */
class SetSeason final : public IEditCommand {
 public:
  explicit SetSeason(core::Season season);
  [[nodiscard]] std::string_view name() const noexcept override;
  core::Status execute(core::SemanticScene& scene) override;
  core::Status undo(core::SemanticScene& scene) override;
  [[nodiscard]] std::string serialize() const override;

 private:
  core::Season target_{core::Season::kOriginal};
  std::optional<core::Season> previous_{};
};

}  // namespace physfx::editing::commands
