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

#include <cstdint>
#include <optional>

#include "physfx/editing/IEditCommand.h"

namespace physfx::editing::commands {

/** @brief 删除实体的命令骨架。 */
class DeleteEntity final : public IEditCommand {
 public:
  /** @brief 创建删除命令。 @param entityId 实体编号。 */
  explicit DeleteEntity(std::uint64_t entityId);
  [[nodiscard]] std::string_view name() const noexcept override;
  core::Status execute(core::SemanticScene& scene) override;
  core::Status undo(core::SemanticScene& scene) override;
  [[nodiscard]] std::string serialize() const override;
  [[nodiscard]] std::vector<std::uint64_t> affectedEntityIds() const override;

 private:
  std::uint64_t entityId_{0};
  std::optional<bool> previous_{};
};

}  // namespace physfx::editing::commands
