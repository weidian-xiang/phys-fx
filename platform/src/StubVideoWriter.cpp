/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/platform/IVideoWriter.h"

namespace physfx::platform {

PlatformStatus StubVideoWriter::open(const std::filesystem::path&, std::uint32_t, std::uint32_t) {
  open_ = false;
  return core::Status::notImplemented("FFmpeg 视频写入尚未实现");
}

PlatformStatus StubVideoWriter::write(const core::Frame&) {
  return core::Status::notImplemented("FFmpeg 视频写入尚未实现");
}

void StubVideoWriter::close() noexcept { open_ = false; }
bool StubVideoWriter::isOpen() const noexcept { return open_; }

}  // namespace physfx::platform
