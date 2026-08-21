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
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "Project.h"

namespace physfx::gui {

class Timeline {
 public:
  void setTotalFrames(std::uint64_t frames) noexcept { totalFrames_ = frames; }
  void scrub(std::uint64_t frame) noexcept;
  void stepForward() noexcept;
  void stepBackward() noexcept;
  void play() noexcept { playing_ = true; }
  void pause() noexcept { playing_ = false; }
  [[nodiscard]] std::uint64_t currentFrame() const noexcept { return currentFrame_; }
  [[nodiscard]] bool playing() const noexcept { return playing_; }

 private:
  std::uint64_t totalFrames_{0};
  std::uint64_t currentFrame_{0};
  bool playing_{false};
};

class PreviewWindow {
 public:
  void setFrameSize(std::uint32_t width, std::uint32_t height) noexcept {
    width_ = width;
    height_ = height;
  }
  void click(std::uint32_t x, std::uint32_t y) noexcept;
  [[nodiscard]] bool hasSelection() const noexcept { return selected_; }
  [[nodiscard]] std::uint32_t selectedX() const noexcept { return selectedX_; }
  [[nodiscard]] std::uint32_t selectedY() const noexcept { return selectedY_; }

 private:
  std::uint32_t width_{0};
  std::uint32_t height_{0};
  std::uint32_t selectedX_{0};
  std::uint32_t selectedY_{0};
  bool selected_{false};
};

class CommandStackPanel {
 public:
  void record(std::string serializedCommand);
  void undo() noexcept;
  void redo() noexcept;
  [[nodiscard]] const std::vector<std::string>& history() const noexcept { return history_; }
  [[nodiscard]] std::size_t cursor() const noexcept { return cursor_; }

 private:
  std::vector<std::string> history_{};
  std::size_t cursor_{0};
};

enum class PreviewQuality { kQuickHalfResolution, kFullResolution };

/** @brief 参数改变后的预览分层状态机；快速帧不阻塞后台全量渲染。 */
class PreviewScheduler {
 public:
  void markDirty() noexcept {
    dirty_ = true;
    quality_ = PreviewQuality::kQuickHalfResolution;
  }
  void confirmFullRender() noexcept {
    dirty_ = false;
    quality_ = PreviewQuality::kFullResolution;
  }
  [[nodiscard]] bool dirty() const noexcept { return dirty_; }
  [[nodiscard]] PreviewQuality quality() const noexcept { return quality_; }

 private:
  bool dirty_{false};
  PreviewQuality quality_{PreviewQuality::kFullResolution};
};

class NodeGraph {
 public:
  Node& addNode(std::string id, std::string type, std::string label);
  bool addCommand(std::string_view nodeId, std::string commandJson);
  [[nodiscard]] const Project& project() const noexcept { return project_; }
  [[nodiscard]] std::string compile() const { return ProjectCompiler::compileEditScript(project_); }

 private:
  Project project_{};
};

}  // namespace physfx::gui
