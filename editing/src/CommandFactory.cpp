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

#include <charconv>
#include <optional>
#include <string>
#include <utility>

#include "physfx/editing/commands/ChangeMaterial.h"
#include "physfx/editing/commands/CopyEntity.h"
#include "physfx/editing/commands/DeleteEntity.h"
#include "physfx/editing/commands/EmptyCommand.h"
#include "physfx/editing/commands/MoveEntity.h"
#include "physfx/editing/commands/SelectEntity.h"
#include "physfx/editing/commands/SetSeason.h"

namespace physfx::editing {
namespace {

std::optional<std::uint64_t> number(std::string_view json, std::string_view key) {
  const auto keyPosition = json.find("\"" + std::string(key) + "\"");
  if (keyPosition == std::string_view::npos) return std::nullopt;
  const auto colon = json.find(':', keyPosition);
  if (colon == std::string_view::npos) return std::nullopt;
  const auto first = json.find_first_of("0123456789", colon);
  if (first == std::string_view::npos) return std::nullopt;
  std::uint64_t value = 0;
  const auto result = std::from_chars(json.data() + first, json.data() + json.size(), value);
  return result.ec == std::errc{} ? std::optional<std::uint64_t>{value} : std::nullopt;
}

std::optional<float> decimal(std::string_view json, std::string_view key) {
  const auto keyPosition = json.find("\"" + std::string(key) + "\"");
  if (keyPosition == std::string_view::npos) return std::nullopt;
  const auto colon = json.find(':', keyPosition);
  if (colon == std::string_view::npos) return std::nullopt;
  const auto first = json.find_first_of("-0123456789", colon);
  if (first == std::string_view::npos) return std::nullopt;
  const auto end = json.find_first_of(",}", first);
  try {
    return std::stof(std::string(json.substr(first, end - first)));
  } catch (...) {
    return std::nullopt;
  }
}

std::string type(std::string_view json) {
  const auto position = json.find("\"type\"");
  if (position == std::string_view::npos) return {};
  const auto first = json.find('"', json.find(':', position) + 1);
  const auto second =
      first == std::string_view::npos ? std::string_view::npos : json.find('"', first + 1);
  return first == std::string_view::npos || second == std::string_view::npos
             ? std::string{}
             : std::string(json.substr(first + 1, second - first - 1));
}

std::string stringValue(std::string_view json, std::string_view key) {
  const auto keyPosition = json.find("\"" + std::string(key) + "\"");
  if (keyPosition == std::string_view::npos) return {};
  const auto colon = json.find(':', keyPosition);
  const auto first = json.find('"', colon == std::string_view::npos ? keyPosition : colon + 1);
  const auto second =
      first == std::string_view::npos ? std::string_view::npos : json.find('"', first + 1);
  return first == std::string_view::npos || second == std::string_view::npos
             ? std::string{}
             : std::string(json.substr(first + 1, second - first - 1));
}

std::optional<core::Season> seasonValue(std::string_view json) {
  const auto value = stringValue(json, "season");
  if (value == "spring") return core::Season::kSpring;
  if (value == "summer") return core::Season::kSummer;
  if (value == "autumn" || value == "fall") return core::Season::kAutumn;
  if (value == "winter") return core::Season::kWinter;
  if (value == "original") return core::Season::kOriginal;
  return std::nullopt;
}

}  // namespace

core::Result<std::unique_ptr<IEditCommand>> deserializeCommand(std::string_view json) {
  const auto commandType = type(json);
  auto boxed = [](auto command) -> std::unique_ptr<IEditCommand> { return std::move(command); };
  if (commandType == "empty") return boxed(std::make_unique<commands::EmptyCommand>());
  const auto entityId = number(json, "entity_id");
  if (commandType == "select_entity" && entityId) {
    return boxed(std::make_unique<commands::SelectEntity>(*entityId));
  }
  if (commandType == "delete_entity" && entityId) {
    return boxed(std::make_unique<commands::DeleteEntity>(*entityId));
  }
  if (commandType == "move_entity" && entityId) {
    const auto x = decimal(json, "x");
    const auto y = decimal(json, "y");
    if (x && y)
      return boxed(std::make_unique<commands::MoveEntity>(*entityId, core::Vec3{*x, *y, 0.0F}));
  }
  if (commandType == "copy_entity" && entityId) {
    return boxed(std::make_unique<commands::CopyEntity>(*entityId));
  }
  if (commandType == "change_material" && entityId) {
    core::MaterialProperties material{};
    material.name = stringValue(json, "material");
    if (material.name.empty())
      return core::Status{core::StatusCode::kInvalidArgument, "外观命令缺少材质字段"};
    if (material.name == "red") material.baseColor = {1.0F, 0.15F, 0.1F};
    if (material.name == "blue") material.baseColor = {0.1F, 0.35F, 1.0F};
    if (material.name == "green") material.baseColor = {0.1F, 0.9F, 0.2F};
    return boxed(std::make_unique<commands::ChangeMaterial>(*entityId, std::move(material)));
  }
  if (commandType == "set_season") {
    const auto season = seasonValue(json);
    if (season) return boxed(std::make_unique<commands::SetSeason>(*season));
  }
  return core::Status{core::StatusCode::kInvalidArgument, "编辑命令 JSON 无效或缺少字段"};
}

}  // namespace physfx::editing
