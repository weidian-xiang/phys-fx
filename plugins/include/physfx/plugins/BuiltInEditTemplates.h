/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <vector>

#include "physfx/plugins/IEditTemplatePlugin.h"
#include "physfx/plugins/PluginRegistry.h"

namespace physfx::plugins {

/** @brief 一键编辑模板的轻量描述插件。 */
class PHYSFX_API BuiltInEditTemplate final : public IEditTemplatePlugin {
 public:
  BuiltInEditTemplate(std::string id, std::string displayName, std::vector<std::string> commands);
  [[nodiscard]] PluginMetadata metadata() const override;
  bool onLoad() override;
  void onUnload() override;
  bool process(core::Frame&, const core::SceneContext&) override;
  [[nodiscard]] std::vector<std::string> commandDescriptors() const override;

 private:
  std::string id_;
  std::string displayName_;
  std::vector<std::string> commands_;
};

/** @brief 注册 remove/move/smoke 三个编辑模板。 */
PHYSFX_API bool registerBuiltInEditTemplates(PluginRegistry& registry);

/** @brief 注册 Phase 5 模板三件套，不改变 Phase 4 注册函数的兼容数量。 */
PHYSFX_API bool registerPhase5EditTemplates(PluginRegistry& registry);

}  // namespace physfx::plugins
