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

#include <filesystem>

#include "physfx/core/Frame.h"
#include "physfx/platform/PlatformStatus.h"

namespace physfx::platform {

/**
 * @brief 视频读取器抽象接口。
 * @param path 输入视频路径。
 * @return 平台状态码。
 * @todo Phase2 接入 FFmpeg 解码和 OpenCV 帧转换。
 */
class IVideoReader {
 public:
  virtual ~IVideoReader() = default;
  /** @brief 打开视频。 @param path 输入路径。 @return 平台状态码。 @todo Phase2 实现：FFmpeg 解码。 */
  virtual PlatformStatus open(const std::filesystem::path& path) = 0;
  /** @brief 读取一帧。 @param frame 输出帧。 @return 平台状态码。 @todo Phase2 实现：像素转换。 */
  virtual PlatformStatus read(core::Frame& frame) = 0;
  /** @brief 关闭视频。 @return 无。 @todo Phase2 实现：释放解码资源。 */
  virtual void close() noexcept = 0;
  /** @brief 查询打开状态。 @return 是否已打开。 @todo Phase2 实现：同步后端状态。 */
  [[nodiscard]] virtual bool isOpen() const noexcept = 0;
};

/** @brief Phase 1 视频读取桩。 @todo Phase2 替换为 FFmpeg 实现。 */
class StubVideoReader final : public IVideoReader {
 public:
  PlatformStatus open(const std::filesystem::path& path) override;
  PlatformStatus read(core::Frame& frame) override;
  void close() noexcept override;
  [[nodiscard]] bool isOpen() const noexcept override;

 private:
  bool open_{false};
};

}  // namespace physfx::platform
