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

#include "physfx/platform/IVideoWriter.h"

namespace physfx::platform {

/** @brief 基于 FFmpeg C API 的 H.264/MP4 RGB24 视频写入器。 */
class PHYSFX_API FFmpegVideoWriter final : public IVideoWriter {
 public:
  FFmpegVideoWriter();
  ~FFmpegVideoWriter() override;
  FFmpegVideoWriter(FFmpegVideoWriter&&) noexcept;
  FFmpegVideoWriter& operator=(FFmpegVideoWriter&&) noexcept;
  FFmpegVideoWriter(const FFmpegVideoWriter&) = delete;
  FFmpegVideoWriter& operator=(const FFmpegVideoWriter&) = delete;

  PlatformStatus open(const std::filesystem::path& path, std::uint32_t width,
                      std::uint32_t height) override;
  PlatformStatus write(const core::Frame& frame) override;
  void close() noexcept override;
  [[nodiscard]] bool isOpen() const noexcept override;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace physfx::platform
