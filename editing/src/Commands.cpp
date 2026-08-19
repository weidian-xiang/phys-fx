/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <utility>

#include "physfx/editing/commands/ChangeMaterial.h"
#include "physfx/editing/commands/DeleteEntity.h"
#include "physfx/editing/commands/EmptyCommand.h"
#include "physfx/editing/commands/MoveEntity.h"
#include "physfx/editing/commands/SetTimeOfDay.h"
#include "physfx/editing/commands/SetWeather.h"

namespace physfx::editing::commands {

namespace {

core::Status missingEntity() { return {core::StatusCode::kNotFound, "实体不存在"}; }

}  // namespace

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
std::string MoveEntity::serialize() const { return R"({"type":"move_entity"})"; }

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
std::string DeleteEntity::serialize() const { return R"({"type":"delete_entity"})"; }

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
std::string ChangeMaterial::serialize() const { return R"({"type":"change_material"})"; }

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

}  // namespace physfx::editing::commands
