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

#include <memory>
#include <string>
#include <vector>

#include "physfx/plugins/IEffectPlugin.h"

namespace physfx::plugins {

/**
 * @brief 插件注册表。
 * @todo Phase2 支持从目录发现、加载和卸载动态插件。
 */
class PHYSFX_API PluginRegistry {
 public:
  /** @brief 注册插件。 @param plugin 插件实例。 @return 是否注册成功。 */
  bool registerPlugin(std::shared_ptr<IEffectPlugin> plugin);
  /** @brief 按名称查找插件。 @param name 插件名称。 @return 插件实例或空指针。 */
  [[nodiscard]] std::shared_ptr<IEffectPlugin> find(const std::string& name) const;
  /** @brief 按类型列出插件。 @param type 插件类型。 @return 插件列表。 */
  [[nodiscard]] std::vector<std::shared_ptr<IEffectPlugin>> findByType(PluginType type) const;
  /** @brief 返回当前插件数量。 @return 插件数量。 */
  [[nodiscard]] std::size_t size() const noexcept;

 private:
  std::vector<std::shared_ptr<IEffectPlugin>> plugins_{};
};

}  // namespace physfx::plugins
