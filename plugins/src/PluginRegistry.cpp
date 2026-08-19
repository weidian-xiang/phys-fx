/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/plugins/PluginRegistry.h"

#include <utility>

namespace physfx::plugins {

bool PluginRegistry::registerPlugin(std::shared_ptr<IEffectPlugin> plugin) {
  if (!plugin || !plugin->onLoad()) {
    return false;
  }
  plugins_.push_back(std::move(plugin));
  return true;
}

std::shared_ptr<IEffectPlugin> PluginRegistry::find(const std::string& name) const {
  for (const auto& plugin : plugins_) {
    if (plugin->metadata().name == name) {
      return plugin;
    }
  }
  return {};
}

std::vector<std::shared_ptr<IEffectPlugin>> PluginRegistry::findByType(PluginType type) const {
  std::vector<std::shared_ptr<IEffectPlugin>> matches;
  for (const auto& plugin : plugins_) {
    if (plugin->metadata().type == type) {
      matches.push_back(plugin);
    }
  }
  return matches;
}

std::size_t PluginRegistry::size() const noexcept { return plugins_.size(); }

}  // namespace physfx::plugins
