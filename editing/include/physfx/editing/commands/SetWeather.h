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

#include <optional>

#include "physfx/editing/IEditCommand.h"

namespace physfx::editing::commands {

/** @brief 设置场景天气的命令骨架。 */
class PHYSFX_API SetWeather final : public IEditCommand {
 public:
  /** @brief 创建天气命令。 @param weather 目标天气。 */
  explicit SetWeather(core::Weather weather);
  [[nodiscard]] std::string_view name() const noexcept override;
  core::Status execute(core::SemanticScene& scene) override;
  core::Status undo(core::SemanticScene& scene) override;
  [[nodiscard]] std::string serialize() const override;

 private:
  core::Weather target_{core::Weather::kOriginal};
  std::optional<core::Weather> previous_{};
};

}  // namespace physfx::editing::commands
