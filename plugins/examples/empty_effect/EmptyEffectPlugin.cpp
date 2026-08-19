/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "EmptyEffectPlugin.h"

namespace physfx::plugins::examples {

PluginMetadata EmptyEffectPlugin::metadata() const {
  return {"empty_effect", "0.1.0-dev", PluginType::kEffectTemplate};
}
bool EmptyEffectPlugin::onLoad() { return true; }
void EmptyEffectPlugin::onUnload() {}
bool EmptyEffectPlugin::process(core::Frame&, const core::SceneContext&) { return true; }

}  // namespace physfx::plugins::examples
