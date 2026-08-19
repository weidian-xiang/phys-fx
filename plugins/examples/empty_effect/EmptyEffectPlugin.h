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

#include "physfx/plugins/IEffectPlugin.h"

namespace physfx::plugins::examples {

/** @brief 不改变输入的示例空插件。 @todo Phase2 替换为真实模板示例。 */
class EmptyEffectPlugin final : public IEffectPlugin {
 public:
  [[nodiscard]] PluginMetadata metadata() const override;
  bool onLoad() override;
  void onUnload() override;
  bool process(core::Frame& frame, const core::SceneContext& scene) override;
};

}  // namespace physfx::plugins::examples
