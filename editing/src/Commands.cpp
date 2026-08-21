/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <utility>

#include "physfx/editing/commands/AnimateParameter.h"
#include "physfx/editing/commands/ChangeMaterial.h"
#include "physfx/editing/commands/CopyEntity.h"
#include "physfx/editing/commands/DeleteEntity.h"
#include "physfx/editing/commands/EmptyCommand.h"
#include "physfx/editing/commands/MoveEntity.h"
#include "physfx/editing/commands/SelectEntity.h"
#include "physfx/editing/commands/SetSeason.h"
#include "physfx/editing/commands/SetTimeOfDay.h"
#include "physfx/editing/commands/SetWeather.h"

namespace physfx::editing::commands {

namespace {

core::Status missingEntity() { return {core::StatusCode::kNotFound, "实体不存在"}; }

}  // namespace

AnimateParameter::AnimateParameter(core::ParameterCurve curve) : curve_(std::move(curve)) {}
std::string_view AnimateParameter::name() const noexcept { return "animate_parameter"; }
core::Status AnimateParameter::execute(core::SemanticScene& scene) {
  const auto existing = std::find_if(scene.parameterCurves.begin(), scene.parameterCurves.end(),
                                      [&](const auto& item) { return item.parameter == curve_.parameter; });
  inserted_ = existing == scene.parameterCurves.end();
  if (inserted_) scene.parameterCurves.push_back(curve_);
  else {
    previous_ = *existing;
    *existing = curve_;
  }
  return core::Status::success();
}
core::Status AnimateParameter::undo(core::SemanticScene& scene) {
  const auto existing = std::find_if(scene.parameterCurves.begin(), scene.parameterCurves.end(),
                                      [&](const auto& item) { return item.parameter == curve_.parameter; });
  if (existing == scene.parameterCurves.end()) {
    return {core::StatusCode::kNotFound, "关键帧曲线不存在；请刷新工程状态后重试撤销"};
  }
  if (inserted_) scene.parameterCurves.erase(existing);
  else if (previous_) *existing = *previous_;
  else return {core::StatusCode::kInternalError, "关键帧命令尚未执行；请先执行命令再撤销"};
  return core::Status::success();
}
std::string AnimateParameter::serialize() const {
  std::string output = "{\"type\":\"animate_parameter\",\"parameter\":\"" + curve_.parameter +
                       "\",\"interpolation\":\"" + curve_.interpolation + "\",\"keyframes\":[";
  for (std::size_t index = 0; index < curve_.keyframes.size(); ++index) {
    if (index != 0U) output += ',';
    output += "{\"frame\":" + std::to_string(curve_.keyframes[index].frame) +
              ",\"value\":" + std::to_string(curve_.keyframes[index].value) + "}";
  }
  return output + "]}";
}

std::string_view EmptyCommand::name() const noexcept { return "empty"; }
core::Status EmptyCommand::execute(core::SemanticScene&) { return core::Status::success(); }
core::Status EmptyCommand::undo(core::SemanticScene&) { return core::Status::success(); }
std::string EmptyCommand::serialize() const { return R"({"type":"empty"})"; }

MoveEntity::MoveEntity(std::uint64_t entityId, core::Vec3 target)
    : entityId_(entityId), target_(target) {}
std::string_view MoveEntity::name() const noexcept { return "move_entity"; }
core::Status MoveEntity::execute(core::SemanticScene& scene) {
  auto* entity = scene.findEntity(entityId_);
  if (!entity) return missingEntity();
  if (entity->trajectory.empty()) entity->trajectory.push_back({});
  previous_ = entity->trajectory.back().position;
  entity->trajectory.back().position = target_;
  return core::Status::success();
}
core::Status MoveEntity::undo(core::SemanticScene& scene) {
  auto* entity = scene.findEntity(entityId_);
  if (!entity || !previous_ || entity->trajectory.empty()) return missingEntity();
  entity->trajectory.back().position = *previous_;
  return core::Status::success();
}
std::string MoveEntity::serialize() const {
  return "{\"type\":\"move_entity\",\"entity_id\":" + std::to_string(entityId_) +
         ",\"x\":" + std::to_string(target_.x) + ",\"y\":" + std::to_string(target_.y) +
         ",\"z\":" + std::to_string(target_.z) + "}";
}
std::vector<std::uint64_t> MoveEntity::affectedEntityIds() const { return {entityId_}; }

DeleteEntity::DeleteEntity(std::uint64_t entityId) : entityId_(entityId) {}
std::string_view DeleteEntity::name() const noexcept { return "delete_entity"; }
core::Status DeleteEntity::execute(core::SemanticScene& scene) {
  auto* entity = scene.findEntity(entityId_);
  if (!entity) return missingEntity();
  previous_ = entity->deleted;
  entity->deleted = true;
  return core::Status::success();
}
core::Status DeleteEntity::undo(core::SemanticScene& scene) {
  auto* entity = scene.findEntity(entityId_);
  if (!entity || !previous_) return missingEntity();
  entity->deleted = *previous_;
  return core::Status::success();
}
std::string DeleteEntity::serialize() const {
  return "{\"type\":\"delete_entity\",\"entity_id\":" + std::to_string(entityId_) + "}";
}
std::vector<std::uint64_t> DeleteEntity::affectedEntityIds() const { return {entityId_}; }

ChangeMaterial::ChangeMaterial(std::uint64_t entityId, core::MaterialProperties material)
    : entityId_(entityId), target_(std::move(material)) {}
std::string_view ChangeMaterial::name() const noexcept { return "change_material"; }
core::Status ChangeMaterial::execute(core::SemanticScene& scene) {
  auto* entity = scene.findEntity(entityId_);
  if (!entity) return missingEntity();
  previous_ = entity->material;
  entity->material = target_;
  return core::Status::success();
}
core::Status ChangeMaterial::undo(core::SemanticScene& scene) {
  auto* entity = scene.findEntity(entityId_);
  if (!entity || !previous_) return missingEntity();
  entity->material = *previous_;
  return core::Status::success();
}
std::string ChangeMaterial::serialize() const {
  return "{\"type\":\"change_material\",\"entity_id\":" + std::to_string(entityId_) +
         ",\"material\":\"" + target_.name + "\"}";
}
std::vector<std::uint64_t> ChangeMaterial::affectedEntityIds() const { return {entityId_}; }

SelectEntity::SelectEntity(std::uint64_t entityId) : entityId_(entityId) {}
std::string_view SelectEntity::name() const noexcept { return "select_entity"; }
core::Status SelectEntity::execute(core::SemanticScene& scene) {
  if (!scene.findEntity(entityId_)) return missingEntity();
  previous_ = scene.selectedEntityId;
  scene.selectedEntityId = entityId_;
  return core::Status::success();
}
core::Status SelectEntity::undo(core::SemanticScene& scene) {
  if (!previous_) return {core::StatusCode::kInternalError, "命令尚未执行"};
  scene.selectedEntityId = *previous_;
  return core::Status::success();
}
std::string SelectEntity::serialize() const {
  return "{\"type\":\"select_entity\",\"entity_id\":" + std::to_string(entityId_) + "}";
}
std::vector<std::uint64_t> SelectEntity::affectedEntityIds() const { return {entityId_}; }

CopyEntity::CopyEntity(std::uint64_t entityId) : entityId_(entityId) {}
std::string_view CopyEntity::name() const noexcept { return "copy_entity"; }
core::Status CopyEntity::execute(core::SemanticScene& scene) {
  const auto* source = scene.findEntity(entityId_);
  if (!source) return missingEntity();
  if (!copyId_) {
    std::uint64_t nextId = 1;
    for (const auto& entity : scene.entities) nextId = std::max(nextId, entity.id + 1);
    copyId_ = nextId;
  }
  if (scene.findEntity(*copyId_)) return {core::StatusCode::kInternalError, "复制实体编号冲突"};
  auto copy = *source;
  copy.id = *copyId_;
  copy.category += "_copy";
  scene.entities.push_back(std::move(copy));
  return core::Status::success();
}
core::Status CopyEntity::undo(core::SemanticScene& scene) {
  if (!copyId_) return {core::StatusCode::kInternalError, "命令尚未执行"};
  const auto before = scene.entities.size();
  scene.entities.erase(std::remove_if(scene.entities.begin(), scene.entities.end(),
                                      [&](const auto& entity) { return entity.id == *copyId_; }),
                       scene.entities.end());
  return scene.entities.size() == before ? missingEntity() : core::Status::success();
}
std::string CopyEntity::serialize() const {
  return "{\"type\":\"copy_entity\",\"entity_id\":" + std::to_string(entityId_) + "}";
}
std::vector<std::uint64_t> CopyEntity::affectedEntityIds() const {
  return copyId_ ? std::vector<std::uint64_t>{entityId_, *copyId_}
                 : std::vector<std::uint64_t>{entityId_};
}

SetWeather::SetWeather(core::Weather weather) : target_(weather) {}
std::string_view SetWeather::name() const noexcept { return "set_weather"; }
core::Status SetWeather::execute(core::SemanticScene& scene) {
  previous_ = scene.weather;
  scene.weather = target_;
  return core::Status::success();
}
core::Status SetWeather::undo(core::SemanticScene& scene) {
  if (!previous_) return {core::StatusCode::kInternalError, "命令尚未执行"};
  scene.weather = *previous_;
  return core::Status::success();
}
std::string SetWeather::serialize() const { return R"({"type":"set_weather"})"; }

SetTimeOfDay::SetTimeOfDay(core::TimeOfDay timeOfDay) : target_(timeOfDay) {}
std::string_view SetTimeOfDay::name() const noexcept { return "set_time_of_day"; }
core::Status SetTimeOfDay::execute(core::SemanticScene& scene) {
  previous_ = scene.timeOfDay;
  scene.timeOfDay = target_;
  return core::Status::success();
}
core::Status SetTimeOfDay::undo(core::SemanticScene& scene) {
  if (!previous_) return {core::StatusCode::kInternalError, "命令尚未执行"};
  scene.timeOfDay = *previous_;
  return core::Status::success();
}
std::string SetTimeOfDay::serialize() const { return R"({"type":"set_time_of_day"})"; }

SetSeason::SetSeason(core::Season season) : target_(season) {}
std::string_view SetSeason::name() const noexcept { return "set_season"; }
core::Status SetSeason::execute(core::SemanticScene& scene) {
  previous_ = scene.season;
  scene.season = target_;
  return core::Status::success();
}
core::Status SetSeason::undo(core::SemanticScene& scene) {
  if (!previous_) return {core::StatusCode::kInternalError, "命令尚未执行"};
  scene.season = *previous_;
  return core::Status::success();
}
std::string SetSeason::serialize() const {
  const char* value = "original";
  switch (target_) {
    case core::Season::kSpring:
      value = "spring";
      break;
    case core::Season::kSummer:
      value = "summer";
      break;
    case core::Season::kAutumn:
      value = "autumn";
      break;
    case core::Season::kWinter:
      value = "winter";
      break;
    case core::Season::kOriginal:
      break;
  }
  return std::string{"{\"type\":\"set_season\",\"season\":\""} + value + "\"}";
}

}  // namespace physfx::editing::commands
