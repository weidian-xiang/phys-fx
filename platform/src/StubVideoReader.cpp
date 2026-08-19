/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/platform/IVideoReader.h"

namespace physfx::platform {

PlatformStatus StubVideoReader::open(const std::filesystem::path&) {
  open_ = false;
  return core::Status::notImplemented("FFmpeg 视频读取尚未实现");
}

PlatformStatus StubVideoReader::read(core::Frame&) {
  return core::Status::notImplemented("FFmpeg 视频读取尚未实现");
}

void StubVideoReader::close() noexcept { open_ = false; }
bool StubVideoReader::isOpen() const noexcept { return open_; }

}  // namespace physfx::platform
