/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cassert>
#include <memory>

#include "physfx/editing/EditCommandStack.h"
#include "physfx/editing/CommandFactory.h"
#include "physfx/editing/commands/CopyEntity.h"
#include "physfx/editing/commands/DeleteEntity.h"
#include "physfx/editing/commands/EmptyCommand.h"
#include "physfx/editing/commands/MoveEntity.h"
#include "physfx/editing/commands/SelectEntity.h"
#include "physfx/editing/commands/SetWeather.h"

int main() {
  physfx::core::SemanticScene scene;
  physfx::editing::EditCommandStack stack;
  assert(stack.execute(std::make_unique<physfx::editing::commands::EmptyCommand>(), scene).ok());
  assert(stack
             .execute(std::make_unique<physfx::editing::commands::SetWeather>(
                          physfx::core::Weather::kRain),
                      scene)
             .ok());
  assert(scene.weather == physfx::core::Weather::kRain);
  assert(stack.undo(scene).ok());
  assert(scene.weather == physfx::core::Weather::kOriginal);
  assert(stack.redo(scene).ok());
  scene.entities.push_back({1, "person"});
  assert(stack.execute(std::make_unique<physfx::editing::commands::SelectEntity>(1), scene).ok());
  assert(scene.selectedEntityId == 1);
  assert(stack.execute(std::make_unique<physfx::editing::commands::MoveEntity>(
                          1, physfx::core::Vec3{10.0F, 20.0F, 0.0F}),
                      scene)
             .ok());
  assert(scene.findEntity(1)->trajectory.back().position.x == 10.0F);
  assert(stack.undo(scene).ok());
  assert(stack.redo(scene).ok());
  auto parsed = physfx::editing::deserializeCommand(
      R"({"type":"delete_entity","entity_id":1})");
  assert(parsed.ok());
  assert(stack.execute(std::move(parsed).value(), scene).ok());
  assert(scene.findEntity(1)->deleted);
  assert(stack.undo(scene).ok());
  auto invalid = physfx::editing::deserializeCommand(R"({"type":"unknown"})");
  assert(!invalid.ok());
  assert(stack.execute(std::make_unique<physfx::editing::commands::CopyEntity>(1), scene).ok());
  assert(scene.entities.size() == 2);
  assert(stack.undo(scene).ok());
  assert(scene.entities.size() == 1);
  return 0;
}
