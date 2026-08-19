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
#include "physfx/editing/commands/EmptyCommand.h"
#include "physfx/editing/commands/SetWeather.h"

int main() {
  physfx::core::SemanticScene scene;
  physfx::editing::EditCommandStack stack;
  assert(stack.execute(std::make_unique<physfx::editing::commands::EmptyCommand>(), scene).ok());
  assert(stack.execute(std::make_unique<physfx::editing::commands::SetWeather>(
                           physfx::core::Weather::kRain),
                       scene)
             .ok());
  assert(scene.weather == physfx::core::Weather::kRain);
  assert(stack.undo(scene).ok());
  assert(scene.weather == physfx::core::Weather::kOriginal);
  assert(stack.redo(scene).ok());
  return 0;
}
