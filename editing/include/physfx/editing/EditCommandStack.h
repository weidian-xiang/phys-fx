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

#include <cstddef>
#include <memory>
#include <vector>

#include "physfx/editing/IEditCommand.h"

namespace physfx::editing {

/** @brief 管理编辑命令执行、撤销和重做历史。 */
class EditCommandStack {
 public:
  /** @brief 执行并记录命令。 @param command 命令所有权。 @param scene 场景。 @return 状态。 */
  core::Status execute(std::unique_ptr<IEditCommand> command, core::SemanticScene& scene);
  /** @brief 在完整场景上下文上执行命令。 */
  core::Status execute(std::unique_ptr<IEditCommand> command, core::SceneContext& context);
  /** @brief 撤销最近命令。 @param scene 场景。 @return 状态。 */
  core::Status undo(core::SemanticScene& scene);
  /** @brief 在完整场景上下文上撤销命令。 */
  core::Status undo(core::SceneContext& context);
  /** @brief 重做最近撤销命令。 @param scene 场景。 @return 状态。 */
  core::Status redo(core::SemanticScene& scene);
  /** @brief 在完整场景上下文上重做命令。 */
  core::Status redo(core::SceneContext& context);
  /** @brief 返回已执行命令数。 @return 命令数。 */
  [[nodiscard]] std::size_t size() const noexcept;

 private:
  std::vector<std::unique_ptr<IEditCommand>> undoStack_{};
  std::vector<std::unique_ptr<IEditCommand>> redoStack_{};
};

}  // namespace physfx::editing
