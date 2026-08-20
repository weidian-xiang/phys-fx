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

#include "physfx/platform/IVideoReader.h"
#include "physfx/platform/IVideoWriter.h"

namespace physfx::platform {

/** @brief 按构建能力创建视频读取器；无 FFmpeg 时返回桩。 */
[[nodiscard]] std::unique_ptr<IVideoReader> createVideoReader();
/** @brief 按构建能力创建视频写入器；无 FFmpeg 时返回桩。 */
[[nodiscard]] std::unique_ptr<IVideoWriter> createVideoWriter();

}  // namespace physfx::platform
