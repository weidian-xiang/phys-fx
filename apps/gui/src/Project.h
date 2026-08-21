/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace physfx::gui {

/** @brief GUI 节点的无 UI 数据模型；可直接保存为 .physfxproj。 */
struct Node {
  std::string id{};
  std::string type{};
  std::string label{};
  std::vector<std::string> commandJson{};
};

struct Project {
  std::string input{};
  std::string output{};
  std::vector<Node> nodes{};
};

/** @brief 线性节点图编译器，输出与 CLI 编辑脚本相同的 JSON 结构。 */
class ProjectCompiler {
 public:
  [[nodiscard]] static std::string compileEditScript(const Project& project);
  [[nodiscard]] static std::string serialize(const Project& project);
  [[nodiscard]] static bool save(const Project& project, const std::string& path);
  [[nodiscard]] static bool load(const std::string& path, Project& project);
};

}  // namespace physfx::gui
