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

#include "physfx/plugins/IEffectPlugin.h"

namespace physfx::plugins {

/** @brief 编辑模板插件标记接口；模板通过命令描述表达，不依赖 editing 实现。 */
class IEditTemplatePlugin : public IEffectPlugin {
 public:
  ~IEditTemplatePlugin() override = default;
  /** @brief 返回序列化编辑命令。 @return 命令描述列表。 */
  [[nodiscard]] virtual std::vector<std::string> commandDescriptors() const = 0;
};

}  // namespace physfx::plugins
