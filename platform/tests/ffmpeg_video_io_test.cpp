/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>

#include "physfx/platform/FFmpegVideoReader.h"
#include "physfx/platform/FFmpegVideoWriter.h"

int main() {
  const auto directory = std::filesystem::temp_directory_path();
  const auto videoPath = directory / "physfx_ffmpeg_roundtrip.mp4";
  const auto damagedPath = directory / "physfx_ffmpeg_damaged.mp4";

  physfx::platform::FFmpegVideoWriter writer;
  assert(writer.open(videoPath, 16, 16).ok());
  for (std::uint64_t index = 0; index < 10; ++index) {
    physfx::core::Frame frame{};
    frame.index = index;
    frame.timestampSeconds = static_cast<double>(index) / 30.0;
    frame.width = 16;
    frame.height = 16;
    frame.format = physfx::core::PixelFormat::kRgb8;
    frame.pixels.resize(16U * 16U * 3U, static_cast<std::uint8_t>(index * 20U));
    assert(writer.write(frame).ok());
  }
  writer.close();

  physfx::platform::FFmpegVideoReader reader;
  assert(reader.open(videoPath).ok());
  std::size_t frameCount = 0;
  while (true) {
    physfx::core::Frame frame{};
    const auto status = reader.read(frame);
    if (status.code == physfx::core::StatusCode::kNotFound) break;
    assert(status.ok());
    assert(frame.width == 16 && frame.height == 16);
    assert(frame.format == physfx::core::PixelFormat::kRgb8);
    assert(frame.frameRate > 0.0);
    ++frameCount;
  }
  reader.close();
  assert(frameCount == 10);

  {
    std::ofstream damaged(damagedPath, std::ios::binary);
    damaged << "not-a-video";
  }
  assert(!reader.open(damagedPath).ok());
  std::filesystem::remove(videoPath);
  std::filesystem::remove(damagedPath);
  return 0;
}
