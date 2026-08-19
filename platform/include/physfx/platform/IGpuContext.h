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

#include "physfx/platform/PlatformStatus.h"

namespace physfx::platform {

/**
 * @brief OpenGL/Vulkan GPU 上下文抽象接口。
 * @return 平台状态码。
 * @todo Phase2 接入 OpenGL 4.5+ / Vulkan 后端。
 */
class IGpuContext {
 public:
  virtual ~IGpuContext() = default;
  /** @brief 初始化 GPU 上下文。 @return 平台状态码。 @todo Phase2 实现：创建图形上下文。 */
  virtual PlatformStatus initialize() = 0;
  /** @brief 关闭 GPU 上下文。 @return 无。 @todo Phase2 实现：释放图形资源。 */
  virtual void shutdown() noexcept = 0;
  /** @brief 查询上下文状态。 @return 上下文是否可用。 @todo Phase2 实现：检查设备状态。 */
  [[nodiscard]] virtual bool isReady() const noexcept = 0;
};

/** @brief Phase 1 GPU 上下文桩。 @todo Phase2 替换为真实图形后端。 */
class StubGpuContext final : public IGpuContext {
 public:
  PlatformStatus initialize() override;
  void shutdown() noexcept override;
  [[nodiscard]] bool isReady() const noexcept override;

 private:
  bool ready_{false};
};

}  // namespace physfx::platform
