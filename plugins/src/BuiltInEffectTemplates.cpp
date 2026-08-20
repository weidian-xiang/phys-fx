/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/plugins/BuiltInEffectTemplates.h"

#include <memory>
#include <utility>

namespace physfx::plugins {

BuiltInEffectTemplate::BuiltInEffectTemplate(std::string id, std::string displayName,
                                             EffectTemplateParameters parameters)
    : id_(std::move(id)),
      displayName_(std::move(displayName)),
      parameters_(std::move(parameters)) {}

PluginMetadata BuiltInEffectTemplate::metadata() const {
  return {id_, "0.3.0", PluginType::kEffectTemplate};
}

bool BuiltInEffectTemplate::onLoad() { return true; }

void BuiltInEffectTemplate::onUnload() {}

bool BuiltInEffectTemplate::process(core::Frame&, const core::SceneContext&) { return true; }

const std::string& BuiltInEffectTemplate::displayName() const noexcept { return displayName_; }

const EffectTemplateParameters& BuiltInEffectTemplate::parameters() const noexcept {
  return parameters_;
}

bool registerBuiltInEffectTemplates(PluginRegistry& registry) {
  const bool sparks = registry.registerPlugin(std::make_shared<BuiltInEffectTemplate>(
      "sparks", "火花",
      EffectTemplateParameters{"simple_particles", "sparks", "additive", 120.0F, 10'000}));
  const bool smoke = registry.registerPlugin(std::make_shared<BuiltInEffectTemplate>(
      "smoke", "烟尘",
      EffectTemplateParameters{"simple_particles", "smoke", "alpha", 80.0F, 10'000}));
  return sparks && smoke;
}

}  // namespace physfx::plugins
