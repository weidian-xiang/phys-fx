/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/editing/EditCommandStack.h"

#include <utility>

namespace physfx::editing {

core::Status EditCommandStack::execute(std::unique_ptr<IEditCommand> command,
                                       core::SemanticScene& scene) {
  if (!command) {
    return {core::StatusCode::kInvalidArgument, "编辑命令不能为空"};
  }
  auto status = command->execute(scene);
  if (status.ok()) {
    undoStack_.push_back(std::move(command));
    redoStack_.clear();
  }
  return status;
}

core::Status EditCommandStack::undo(core::SemanticScene& scene) {
  if (undoStack_.empty()) {
    return {core::StatusCode::kNotFound, "没有可撤销的命令"};
  }
  auto command = std::move(undoStack_.back());
  undoStack_.pop_back();
  auto status = command->undo(scene);
  if (status.ok()) {
    redoStack_.push_back(std::move(command));
  }
  return status;
}

core::Status EditCommandStack::redo(core::SemanticScene& scene) {
  if (redoStack_.empty()) {
    return {core::StatusCode::kNotFound, "没有可重做的命令"};
  }
  auto command = std::move(redoStack_.back());
  redoStack_.pop_back();
  auto status = command->execute(scene);
  if (status.ok()) {
    undoStack_.push_back(std::move(command));
  }
  return status;
}

std::size_t EditCommandStack::size() const noexcept { return undoStack_.size(); }

}  // namespace physfx::editing
