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

#include <cstddef>
#include <string>

#include "physfx/plugins/PluginRegistry.h"

namespace physfx::plugins {

/** @brief 不依赖具体物理模块的特效模板参数契约。 */
struct EffectTemplateParameters {
  std::string simulator{};
  std::string preset{};
  std::string blend{};
  float emissionRate{0.0F};
  std::size_t maxParticles{0};
};

/** @brief 内置火花/烟尘模板插件。 */
class BuiltInEffectTemplate final : public IEffectPlugin {
 public:
  BuiltInEffectTemplate(std::string id, std::string displayName,
                        EffectTemplateParameters parameters);

  [[nodiscard]] PluginMetadata metadata() const override;
  bool onLoad() override;
  void onUnload() override;
  bool process(core::Frame& frame, const core::SceneContext& scene) override;
  [[nodiscard]] const std::string& displayName() const noexcept;
  [[nodiscard]] const EffectTemplateParameters& parameters() const noexcept;

 private:
  std::string id_{};
  std::string displayName_{};
  EffectTemplateParameters parameters_{};
};

/** @brief 向注册表注册火花与烟尘两个真实模板。 @return 两个模板均注册成功时为 true。 */
bool registerBuiltInEffectTemplates(PluginRegistry& registry);

}  // namespace physfx::plugins
