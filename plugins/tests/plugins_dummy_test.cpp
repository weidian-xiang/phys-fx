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

#include "../examples/empty_effect/EmptyEffectPlugin.h"
#include "physfx/plugins/BuiltInEffectTemplates.h"
#include "physfx/plugins/BuiltInEditTemplates.h"
#include "physfx/plugins/PluginRegistry.h"

int main() {
  physfx::plugins::PluginRegistry registry;
  auto plugin = std::make_shared<physfx::plugins::examples::EmptyEffectPlugin>();
  assert(registry.registerPlugin(plugin));
  assert(registry.find("empty_effect") != nullptr);
  assert(registry.findByType(physfx::plugins::PluginType::kEffectTemplate).size() == 1);
  assert(physfx::plugins::registerBuiltInEffectTemplates(registry));
  assert(registry.findByType(physfx::plugins::PluginType::kEffectTemplate).size() == 3);
  const auto sparks =
      std::dynamic_pointer_cast<physfx::plugins::BuiltInEffectTemplate>(registry.find("sparks"));
  assert(sparks && sparks->displayName() == "火花");
  assert(sparks->parameters().emissionRate == 120.0F);
  assert(physfx::plugins::registerBuiltInEditTemplates(registry));
  assert(registry.findByType(physfx::plugins::PluginType::kEditTemplate).size() == 3);
  return 0;
}
