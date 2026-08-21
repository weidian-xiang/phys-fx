/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "EditorModel.h"

#include <algorithm>
#include <utility>

namespace physfx::gui {

void Timeline::scrub(std::uint64_t frame) noexcept {
  currentFrame_ = totalFrames_ == 0 ? frame : std::min(frame, totalFrames_ - 1U);
}

void Timeline::stepForward() noexcept { scrub(currentFrame_ + 1U); }

void Timeline::stepBackward() noexcept { scrub(currentFrame_ == 0 ? 0 : currentFrame_ - 1U); }

void PreviewWindow::click(std::uint32_t x, std::uint32_t y) noexcept {
  if (width_ == 0 || height_ == 0) {
    selected_ = false;
    return;
  }
  selectedX_ = std::min(x, width_ - 1U);
  selectedY_ = std::min(y, height_ - 1U);
  selected_ = true;
}

void CommandStackPanel::record(std::string serializedCommand) {
  if (cursor_ < history_.size()) history_.resize(cursor_);
  history_.push_back(std::move(serializedCommand));
  cursor_ = history_.size();
}

void CommandStackPanel::undo() noexcept {
  if (cursor_ > 0) --cursor_;
}

void CommandStackPanel::redo() noexcept {
  if (cursor_ < history_.size()) ++cursor_;
}

Node& NodeGraph::addNode(std::string id, std::string type, std::string label) {
  project_.nodes.push_back({std::move(id), std::move(type), std::move(label), {}});
  return project_.nodes.back();
}

bool NodeGraph::addCommand(std::string_view nodeId, std::string commandJson) {
  for (auto& node : project_.nodes) {
    if (node.id == nodeId) {
      node.commandJson.push_back(std::move(commandJson));
      return true;
    }
  }
  return false;
}

}  // namespace physfx::gui
