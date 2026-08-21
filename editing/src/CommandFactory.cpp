/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/editing/CommandFactory.h"

#include <cmath>
#include <limits>
#include <optional>
#include <string>
#include <utility>

#include "physfx/core/Json.h"
#include "physfx/editing/commands/AnimateParameter.h"
#include "physfx/editing/commands/ChangeMaterial.h"
#include "physfx/editing/commands/CopyEntity.h"
#include "physfx/editing/commands/DeleteEntity.h"
#include "physfx/editing/commands/EmptyCommand.h"
#include "physfx/editing/commands/MoveEntity.h"
#include "physfx/editing/commands/SelectEntity.h"
#include "physfx/editing/commands/SetSeason.h"

namespace physfx::editing {
namespace {

std::optional<std::uint64_t> number(const core::JsonValue& object, std::string_view key) {
  const auto* value = object.find(key);
  if (value == nullptr || value->number() == nullptr) return std::nullopt;
  const double number = *value->number();
  if (number < 0.0 || number > static_cast<double>(std::numeric_limits<std::uint64_t>::max()) ||
      std::floor(number) != number) {
    return std::nullopt;
  }
  return static_cast<std::uint64_t>(number);
}

std::optional<float> decimal(const core::JsonValue& object, std::string_view key) {
  const auto* value = object.find(key);
  if (value == nullptr || value->number() == nullptr ||
      *value->number() < -static_cast<double>(std::numeric_limits<float>::max()) ||
      *value->number() > static_cast<double>(std::numeric_limits<float>::max())) {
    return std::nullopt;
  }
  return static_cast<float>(*value->number());
}

std::string stringValue(const core::JsonValue& object, std::string_view key) {
  const auto* value = object.find(key);
  return value == nullptr || value->string() == nullptr ? std::string{} : *value->string();
}

std::optional<core::Season> seasonValue(const core::JsonValue& object) {
  const auto value = stringValue(object, "season");
  if (value == "spring") return core::Season::kSpring;
  if (value == "summer") return core::Season::kSummer;
  if (value == "autumn" || value == "fall") return core::Season::kAutumn;
  if (value == "winter") return core::Season::kWinter;
  if (value == "original") return core::Season::kOriginal;
  return std::nullopt;
}

}  // namespace

core::Result<std::unique_ptr<IEditCommand>> deserializeCommand(std::string_view json) {
  auto parsed = core::parseJson(json, {.maxBytes = 64U * 1024U, .maxDepth = 16U});
  if (!parsed.ok()) return parsed.status();
  const auto object = std::move(parsed).value();
  if (object.object() == nullptr) {
    return core::Status{core::StatusCode::kInvalidArgument,
                        "编辑命令必须是 JSON 对象；请检查 type 与参数"};
  }
  const auto commandType = stringValue(object, "type");
  auto boxed = [](auto command) -> std::unique_ptr<IEditCommand> { return std::move(command); };
  if (commandType == "empty") return boxed(std::make_unique<commands::EmptyCommand>());
  const auto entityId = number(object, "entity_id");
  if (commandType == "select_entity" && entityId) {
    return boxed(std::make_unique<commands::SelectEntity>(*entityId));
  }
  if (commandType == "delete_entity" && entityId) {
    return boxed(std::make_unique<commands::DeleteEntity>(*entityId));
  }
  if (commandType == "move_entity" && entityId) {
    const auto x = decimal(object, "x");
    const auto y = decimal(object, "y");
    if (x && y)
      return boxed(std::make_unique<commands::MoveEntity>(*entityId, core::Vec3{*x, *y, 0.0F}));
  }
  if (commandType == "copy_entity" && entityId) {
    return boxed(std::make_unique<commands::CopyEntity>(*entityId));
  }
  if (commandType == "change_material" && entityId) {
    core::MaterialProperties material{};
    material.name = stringValue(object, "material");
    if (material.name.empty())
      return core::Status{core::StatusCode::kInvalidArgument, "外观命令缺少材质字段"};
    if (material.name == "red") material.baseColor = {1.0F, 0.15F, 0.1F};
    if (material.name == "blue") material.baseColor = {0.1F, 0.35F, 1.0F};
    if (material.name == "green") material.baseColor = {0.1F, 0.9F, 0.2F};
    return boxed(std::make_unique<commands::ChangeMaterial>(*entityId, std::move(material)));
  }
  if (commandType == "set_season") {
    const auto season = seasonValue(object);
    if (season) return boxed(std::make_unique<commands::SetSeason>(*season));
  }
  if (commandType == "animate_parameter") {
    core::ParameterCurve curve{};
    curve.parameter = stringValue(object, "parameter");
    curve.interpolation = stringValue(object, "interpolation");
    const auto* keyframes = object.find("keyframes");
    if (curve.interpolation.empty()) curve.interpolation = "linear";
    if (!curve.parameter.empty() && (curve.interpolation == "linear" || curve.interpolation == "smooth") &&
        keyframes != nullptr && keyframes->array() != nullptr && keyframes->array()->size() >= 2U &&
        keyframes->array()->size() <= 1000U) {
      bool valid = true;
      for (const auto& keyframe : *keyframes->array()) {
        const auto frame = number(keyframe, "frame");
        const auto value = decimal(keyframe, "value");
        if (!frame || !value || (!curve.keyframes.empty() && frame <= curve.keyframes.back().frame)) {
          valid = false;
          break;
        }
        curve.keyframes.push_back({*frame, *value});
      }
      if (valid) return boxed(std::make_unique<commands::AnimateParameter>(std::move(curve)));
    }
  }
  return core::Status{core::StatusCode::kInvalidArgument,
                      "编辑命令无效或缺少字段；请核对 type、entity_id 与必填参数"};
}

}  // namespace physfx::editing
