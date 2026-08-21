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

#include "physfx/core/Frame.h"
#include "physfx/core/SceneContext.h"

namespace physfx::plugins {

/** @brief 插件类别。 */
enum class PluginType { kEffectTemplate, kEditTemplate };

/** @brief 插件元数据。 @todo Phase2 增加 ABI 版本和能力声明。 */
struct PluginMetadata {
  std::string name{};
  std::string version{};
  PluginType type{PluginType::kEffectTemplate};
};

/**
 * @brief 特效模板插件生命周期与处理接口，是商业扩展边界。
 * @param frame 可被插件读取或生成的帧。
 * @param scene 当前场景上下文。
 * @return 插件处理是否成功。
 * @todo Phase2 增加动态库加载与沙箱能力。
 */
class PHYSFX_API IEffectPlugin {
 public:
  virtual ~IEffectPlugin() = default;
  /** @brief 获取插件元数据。 @return 插件元数据。 @todo Phase2 实现：增加 ABI 信息。 */
  [[nodiscard]] virtual PluginMetadata metadata() const = 0;
  /** @brief 加载插件。 @return 是否加载成功。 @todo Phase2 实现：加载动态资源。 */
  virtual bool onLoad() = 0;
  /** @brief 卸载插件。 @return 无。 @todo Phase2 实现：释放动态资源。 */
  virtual void onUnload() = 0;
  /**
   * @brief 处理当前帧。
   * @param frame 可被插件修改的视频帧。
   * @param scene 当前场景上下文。
   * @return 插件处理是否成功。
   * @todo Phase2 实现：支持模板参数和资源绑定。
   */
  virtual bool process(core::Frame& frame, const core::SceneContext& scene) = 0;
};

}  // namespace physfx::plugins
