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

#include "physfx/semantics/TraditionalSemantics.h"

int main() {
  physfx::core::Frame frame{};
  frame.width = 4;
  frame.height = 4;
  frame.format = physfx::core::PixelFormat::kRgb8;
  frame.pixels.assign(4U * 4U * 3U, 0);
  for (std::uint32_t y = 1; y <= 2; ++y) {
    for (std::uint32_t x = 1; x <= 2; ++x) {
      const auto offset = (static_cast<std::size_t>(y) * 4U + x) * 3U;
      frame.pixels[offset] = 255;
    }
  }
  physfx::semantics::ColorPromptSegmenter segmenter(1, 1, 10.0F);
  auto segmented = segmenter.segment(frame);
  assert(segmented.ok());
  const auto& values = segmented.value().front().maskTimeline.front().values;
  assert(values[0] == 0 && values[5] == 255 && values[10] == 255);

  physfx::semantics::CentroidTracker tracker;
  auto first = tracker.track(frame, segmented.value());
  assert(first.ok() && first.value().front().trajectory.size() == 1);
  frame.index = 1;
  auto second = tracker.track(frame, segmented.value());
  assert(second.ok() && second.value().front().trajectory.size() == 2);
  return 0;
}
