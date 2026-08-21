/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Project.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <utility>

#include "physfx/core/Json.h"

namespace physfx::gui {
namespace {

core::Status invalidProject(std::string message) {
  return {core::StatusCode::kInvalidArgument,
          std::move(message) + "；请参考 docs/gui.md 修正 .physfxproj"};
}

const std::string* stringField(const core::JsonValue& object, std::string_view key) {
  const auto* value = object.find(key);
  return value == nullptr ? nullptr : value->string();
}

core::JsonValue keyframeJson(const Keyframe& keyframe) {
  return core::JsonValue{
      core::JsonValue::Object{{"frame", core::JsonValue{static_cast<double>(keyframe.frame)}},
                              {"value", core::JsonValue{static_cast<double>(keyframe.value)}}}};
}

}  // namespace

float ParameterCurve::evaluate(std::uint64_t frame) const noexcept {
  if (keyframes.empty()) return 0.0F;
  if (frame <= keyframes.front().frame) return keyframes.front().value;
  if (frame >= keyframes.back().frame) return keyframes.back().value;
  const auto upper = std::upper_bound(
      keyframes.begin(), keyframes.end(), frame,
      [](std::uint64_t candidate, const Keyframe& item) { return candidate < item.frame; });
  const auto& right = *upper;
  const auto& left = *std::prev(upper);
  float ratio =
      static_cast<float>(frame - left.frame) / static_cast<float>(right.frame - left.frame);
  if (interpolation == "smooth") ratio = ratio * ratio * (3.0F - 2.0F * ratio);
  return left.value + (right.value - left.value) * ratio;
}

std::string ProjectCompiler::compileEditScript(const Project& project) {
  core::JsonValue::Array commands;
  for (const auto& node : project.nodes) {
    for (const auto& command : node.commandJson) {
      auto parsed = core::parseJson(command, {.maxBytes = 64U * 1024U, .maxDepth = 16U});
      if (parsed.ok() && parsed.value().object() != nullptr)
        commands.push_back(std::move(parsed).value());
    }
  }
  for (const auto& curve : project.curves) {
    core::JsonValue::Array keyframes;
    for (const auto& keyframe : curve.keyframes) keyframes.push_back(keyframeJson(keyframe));
    commands.emplace_back(
        core::JsonValue::Object{{"interpolation", core::JsonValue{curve.interpolation}},
                                {"keyframes", core::JsonValue{std::move(keyframes)}},
                                {"parameter", core::JsonValue{curve.parameter}},
                                {"type", core::JsonValue{std::string{"animate_parameter"}}}});
  }
  return std::string{"{\"version\":1,\"commands\":"} +
         core::serializeJson(core::JsonValue{std::move(commands)}) + "}";
}

std::string ProjectCompiler::serialize(const Project& project) {
  core::JsonValue::Array nodes;
  for (const auto& node : project.nodes) {
    core::JsonValue::Array commands;
    for (const auto& command : node.commandJson) {
      auto parsed = core::parseJson(command, {.maxBytes = 64U * 1024U, .maxDepth = 16U});
      if (parsed.ok() && parsed.value().object() != nullptr)
        commands.push_back(std::move(parsed).value());
    }
    nodes.emplace_back(core::JsonValue::Object{{"commands", core::JsonValue{std::move(commands)}},
                                               {"id", core::JsonValue{node.id}},
                                               {"label", core::JsonValue{node.label}},
                                               {"type", core::JsonValue{node.type}}});
  }
  core::JsonValue::Array curves;
  for (const auto& curve : project.curves) {
    core::JsonValue::Array keyframes;
    for (const auto& keyframe : curve.keyframes) keyframes.push_back(keyframeJson(keyframe));
    curves.emplace_back(
        core::JsonValue::Object{{"interpolation", core::JsonValue{curve.interpolation}},
                                {"keyframes", core::JsonValue{std::move(keyframes)}},
                                {"parameter", core::JsonValue{curve.parameter}}});
  }
  return core::serializeJson(
      core::JsonValue{core::JsonValue::Object{{"curves", core::JsonValue{std::move(curves)}},
                                              {"input", core::JsonValue{project.input}},
                                              {"nodes", core::JsonValue{std::move(nodes)}},
                                              {"output", core::JsonValue{project.output}},
                                              {"version", core::JsonValue{1.0}}}});
}

core::Result<Project> ProjectCompiler::parse(std::string_view json) {
  auto parsed = core::parseJson(json);
  if (!parsed.ok()) return parsed.status();
  const auto root = std::move(parsed).value();
  const auto* version = root.find("version");
  const auto* nodes = root.find("nodes");
  if (root.object() == nullptr || version == nullptr || version->number() == nullptr ||
      *version->number() != 1.0 || nodes == nullptr || nodes->array() == nullptr) {
    return invalidProject("工程根节点必须包含 version=1 和 nodes 数组");
  }
  if (nodes->array()->size() > 1024U) return invalidProject("工程节点超过 1024 个");
  Project project{};
  if (const auto* input = root.find("input"); input != nullptr && input->string() != nullptr)
    project.input = *input->string();
  if (const auto* output = root.find("output"); output != nullptr && output->string() != nullptr)
    project.output = *output->string();
  for (const auto& value : *nodes->array()) {
    const auto* id = stringField(value, "id");
    const auto* type = stringField(value, "type");
    const auto* label = stringField(value, "label");
    const auto* commands = value.find("commands");
    if (value.object() == nullptr || id == nullptr || id->empty() || type == nullptr ||
        type->empty() || label == nullptr || commands == nullptr || commands->array() == nullptr) {
      return invalidProject("节点必须包含非空 id/type、label 和 commands 数组");
    }
    if (commands->array()->size() > 10000U) return invalidProject("单节点命令超过 10000 条");
    Node node{*id, *type, *label, {}};
    for (const auto& command : *commands->array()) {
      const auto* commandType = command.find("type");
      if (command.object() == nullptr || commandType == nullptr ||
          commandType->string() == nullptr) {
        return invalidProject("节点命令必须是包含字符串 type 的对象");
      }
      node.commandJson.push_back(core::serializeJson(command));
    }
    project.nodes.push_back(std::move(node));
  }
  const auto* curves = root.find("curves");
  if (curves == nullptr) return project;
  if (curves->array() == nullptr || curves->array()->size() > 256U)
    return invalidProject("curves 必须是最多 256 项的数组");
  for (const auto& value : *curves->array()) {
    const auto* parameter = stringField(value, "parameter");
    const auto* interpolation = stringField(value, "interpolation");
    const auto* keyframes = value.find("keyframes");
    if (parameter == nullptr || parameter->empty() || interpolation == nullptr ||
        (*interpolation != "linear" && *interpolation != "smooth") || keyframes == nullptr ||
        keyframes->array() == nullptr || keyframes->array()->size() < 2U ||
        keyframes->array()->size() > 1000U) {
      return invalidProject("曲线需要 parameter、linear/smooth 插值和 2-1000 个关键帧");
    }
    ParameterCurve curve{*parameter, *interpolation, {}};
    for (const auto& item : *keyframes->array()) {
      const auto* frame = item.find("frame");
      const auto* number = item.find("value");
      if (frame == nullptr || frame->number() == nullptr || number == nullptr ||
          number->number() == nullptr || *frame->number() < 0.0 ||
          std::floor(*frame->number()) != *frame->number() ||
          *frame->number() > static_cast<double>(std::numeric_limits<std::uint64_t>::max()) ||
          *number->number() < -static_cast<double>(std::numeric_limits<float>::max()) ||
          *number->number() > static_cast<double>(std::numeric_limits<float>::max())) {
        return invalidProject("关键帧 frame 必须是非负整数，value 必须是有限数值");
      }
      const auto frameValue = static_cast<std::uint64_t>(*frame->number());
      if (!curve.keyframes.empty() && frameValue <= curve.keyframes.back().frame)
        return invalidProject("关键帧必须按 frame 严格递增");
      curve.keyframes.push_back({frameValue, static_cast<float>(*number->number())});
    }
    project.curves.push_back(std::move(curve));
  }
  return project;
}

bool ProjectCompiler::save(const Project& project, const std::string& path) {
  std::ofstream stream(path, std::ios::binary);
  if (!stream) return false;
  stream << serialize(project);
  return static_cast<bool>(stream);
}

bool ProjectCompiler::load(const std::string& path, Project& project) {
  std::error_code error;
  const auto bytes = std::filesystem::file_size(path, error);
  if (error || bytes > 1024U * 1024U) return false;
  std::ifstream stream(path, std::ios::binary);
  if (!stream) return false;
  const std::string text((std::istreambuf_iterator<char>(stream)),
                         std::istreambuf_iterator<char>());
  auto parsed = parse(text);
  if (!parsed.ok()) return false;
  project = std::move(parsed).value();
  return true;
}

}  // namespace physfx::gui
