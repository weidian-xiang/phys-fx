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

#include <cstdint>
#include <vector>

namespace physfx::core {

/** @brief 视频帧的颜色格式。 @todo Phase2 支持更多 FFmpeg 像素格式。 */
enum class PixelFormat { kUnknown, kRgb8, kRgba8 };

/**
 * @brief 流水线中传递的视频帧。
 * @todo Phase2 接入 FFmpeg/OpenCV 的零拷贝帧存储。
 */
struct Frame {
  /** @brief 帧序号。 */
  std::uint64_t index{0};
  /** @brief 时间戳（秒）。 */
  double timestampSeconds{0.0};
  /** @brief 图像宽度。 */
  std::uint32_t width{0};
  /** @brief 图像高度。 */
  std::uint32_t height{0};
  /** @brief 像素格式。 */
  PixelFormat format{PixelFormat::kUnknown};
  /** @brief 交给平台层的原始像素缓冲。 */
  std::vector<std::uint8_t> pixels{};
};

}  // namespace physfx::core
