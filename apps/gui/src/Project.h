/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "physfx/core/Status.h"

namespace physfx::gui {

/** @brief GUI 节点的无 UI 数据模型；可直接保存为 .physfxproj。 */
struct Node {
  std::string id{};
  std::string type{};
  std::string label{};
  std::vector<std::string> commandJson{};
};

struct Keyframe {
  std::uint64_t frame{0};
  float value{0.0F};
};

struct ParameterCurve {
  std::string parameter{"intensity"};
  std::string interpolation{"linear"};
  std::vector<Keyframe> keyframes{{0U, 0.0F}, {299U, 1.0F}};

  [[nodiscard]] float evaluate(std::uint64_t frame) const noexcept;
};

struct Project {
  std::string input{};
  std::string output{};
  std::vector<Node> nodes{};
  std::vector<ParameterCurve> curves{};
};

/** @brief 线性节点图编译器，输出与 CLI 编辑脚本相同的 JSON 结构。 */
class ProjectCompiler {
 public:
  [[nodiscard]] static std::string compileEditScript(const Project& project);
  [[nodiscard]] static std::string serialize(const Project& project);
  [[nodiscard]] static core::Result<Project> parse(std::string_view json);
  [[nodiscard]] static bool save(const Project& project, const std::string& path);
  [[nodiscard]] static bool load(const std::string& path, Project& project);
};

}  // namespace physfx::gui
