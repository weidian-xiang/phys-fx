/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/platform/VideoIoFactory.h"

#include <memory>

#if PHYSFX_FFMPEG_AVAILABLE
#include "physfx/platform/FFmpegVideoReader.h"
#include "physfx/platform/FFmpegVideoWriter.h"
#endif

namespace physfx::platform {

std::unique_ptr<IVideoReader> createVideoReader() {
#if PHYSFX_FFMPEG_AVAILABLE
  return std::make_unique<FFmpegVideoReader>();
#else
  return std::make_unique<StubVideoReader>();
#endif
}

std::unique_ptr<IVideoWriter> createVideoWriter() {
#if PHYSFX_FFMPEG_AVAILABLE
  return std::make_unique<FFmpegVideoWriter>();
#else
  return std::make_unique<StubVideoWriter>();
#endif
}

}  // namespace physfx::platform
