/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>

#include "physfx/semantics/TraditionalSemantics.h"

namespace {

constexpr std::uint32_t kWidth = 1280;
constexpr std::uint32_t kHeight = 720;
constexpr std::uint32_t kObjectWidth = 160;
constexpr std::uint32_t kObjectHeight = 120;

float iou(const std::vector<std::uint8_t>& mask, std::uint32_t left, std::uint32_t top) {
  std::size_t intersection = 0;
  std::size_t unionCount = 0;
  for (std::uint32_t y = 0; y < kHeight; ++y) {
    for (std::uint32_t x = 0; x < kWidth; ++x) {
      const bool expected =
          x >= left && x < left + kObjectWidth && y >= top && y < top + kObjectHeight;
      const bool actual = mask[static_cast<std::size_t>(y) * kWidth + x] != 0;
      intersection += expected && actual ? 1U : 0U;
      unionCount += expected || actual ? 1U : 0U;
    }
  }
  return static_cast<float>(intersection) / static_cast<float>(unionCount);
}

}  // namespace

int main() {
  const std::array<std::array<std::uint8_t, 3>, 3> materials{
      {{220, 45, 45}, {45, 210, 80}, {50, 100, 230}}};
  double maxMilliseconds = 0.0;
  float minimumIou = std::numeric_limits<float>::max();

  for (const auto& color : materials) {
    physfx::semantics::CentroidTracker tracker;
    for (std::uint64_t frameIndex = 0; frameIndex < 300; ++frameIndex) {
      const auto horizontal = frameIndex <= 150U ? frameIndex : 300U - frameIndex;
      const std::uint32_t left = 80U + static_cast<std::uint32_t>(horizontal);
      const std::uint32_t top = 260U + static_cast<std::uint32_t>((frameIndex / 30U) % 3U);
      physfx::core::Frame frame{};
      frame.index = frameIndex;
      frame.timestampSeconds = static_cast<double>(frameIndex) / 30.0;
      frame.width = kWidth;
      frame.height = kHeight;
      frame.format = physfx::core::PixelFormat::kRgb8;
      frame.pixels.assign(static_cast<std::size_t>(kWidth) * kHeight * 3U, 24U);
      for (std::uint32_t y = top; y < top + kObjectHeight; ++y) {
        for (std::uint32_t x = left; x < left + kObjectWidth; ++x) {
          const auto offset = (static_cast<std::size_t>(y) * kWidth + x) * 3U;
          std::copy(color.begin(), color.end(), frame.pixels.begin() + offset);
        }
      }

      const auto started = std::chrono::steady_clock::now();
      physfx::semantics::ColorPromptSegmenter segmenter(left + kObjectWidth / 2U,
                                                        top + kObjectHeight / 2U, 30.0F);
      auto segmented = segmenter.segment(frame);
      assert(segmented.ok() && segmented.value().size() == 1U);
      auto entities = std::move(segmented).value();
      entities.front().id = frameIndex + 100U;
      auto tracked = tracker.track(frame, entities);
      const auto elapsed =
          std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started)
              .count();
      maxMilliseconds = std::max(maxMilliseconds, elapsed);
      if (!tracked.ok() || tracked.value().front().id != 100U) {
        std::cerr << "关联失败 frame=" << frameIndex
                  << " id=" << (tracked.ok() ? tracked.value().front().id : 0U) << '\n';
        return 1;
      }
      assert(tracked.value().front().trajectory.size() == frameIndex + 1U);
      const float frameIou = iou(tracked.value().front().maskTimeline.back().values, left, top);
      minimumIou = std::min(minimumIou, frameIou);
      assert(frameIou > 0.7F);
      assert(elapsed < 500.0);
    }
  }

  std::cout << "FALLBACK_ACCEPTANCE=PASS materials=3 frames=900 min_iou=" << minimumIou
            << " max_ms=" << maxMilliseconds << '\n';
  return 0;
}
