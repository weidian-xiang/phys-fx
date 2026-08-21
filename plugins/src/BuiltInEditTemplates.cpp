/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/plugins/BuiltInEditTemplates.h"

#include <memory>
#include <utility>

namespace physfx::plugins {

BuiltInEditTemplate::BuiltInEditTemplate(std::string id, std::string displayName,
                                         std::vector<std::string> commands)
    : id_(std::move(id)), displayName_(std::move(displayName)), commands_(std::move(commands)) {}

PluginMetadata BuiltInEditTemplate::metadata() const {
  return {id_, "0.4.0", PluginType::kEditTemplate};
}
bool BuiltInEditTemplate::onLoad() { return true; }
void BuiltInEditTemplate::onUnload() {}
bool BuiltInEditTemplate::process(core::Frame&, const core::SceneContext&) { return true; }
std::vector<std::string> BuiltInEditTemplate::commandDescriptors() const { return commands_; }

bool registerBuiltInEditTemplates(PluginRegistry& registry) {
  return registry.registerPlugin(std::make_shared<BuiltInEditTemplate>(
             "remove", "一键去物", std::vector<std::string>{"select_entity", "delete_entity"})) &&
         registry.registerPlugin(std::make_shared<BuiltInEditTemplate>(
             "move", "一键移动", std::vector<std::string>{"select_entity", "move_entity"})) &&
         registry.registerPlugin(std::make_shared<BuiltInEditTemplate>(
             "smoke", "一键烟雾", std::vector<std::string>{"taichi_smoke", "occlusion_composite"}));
}

bool registerPhase5EditTemplates(PluginRegistry& registry) {
  return registry.registerPlugin(std::make_shared<BuiltInEditTemplate>(
             "splash", "泼水",
             std::vector<std::string>{"depth_estimate", "taichi_splash", "occlusion_composite"})) &&
         registry.registerPlugin(std::make_shared<BuiltInEditTemplate>(
             "explode", "爆炸",
             std::vector<std::string>{"particle_fire", "particle_fragments", "taichi_smoke",
                                      "layered_composite"})) &&
         registry.registerPlugin(std::make_shared<BuiltInEditTemplate>(
             "season", "换季节",
             std::vector<std::string>{"sky_segment", "set_season", "color_lighting_transform"}));
}

}  // namespace physfx::plugins
