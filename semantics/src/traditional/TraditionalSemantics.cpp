/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/semantics/TraditionalSemantics.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <queue>
#include <utility>

namespace physfx::semantics {

ColorPromptSegmenter::ColorPromptSegmenter(std::uint32_t x, std::uint32_t y, float tolerance)
    : x_(x), y_(y), tolerance_(std::max(tolerance, 0.0F)) {}

core::Result<std::vector<core::Entity>> ColorPromptSegmenter::segment(const core::Frame& frame) {
  const std::size_t pixelCount = static_cast<std::size_t>(frame.width) * frame.height;
  if (frame.format != core::PixelFormat::kRgb8 || frame.width == 0 || frame.height == 0 ||
      frame.pixels.size() < pixelCount * 3U) {
    return core::Status{core::StatusCode::kInvalidArgument, "传统分割器需要非空 RGB8 视频帧"};
  }
  const std::uint32_t seedX = std::min(x_, frame.width - 1U);
  const std::uint32_t seedY = std::min(y_, frame.height - 1U);
  const std::size_t seedOffset = (static_cast<std::size_t>(seedY) * frame.width + seedX) * 3U;
  const std::array<int, 3> seed = {frame.pixels[seedOffset], frame.pixels[seedOffset + 1U],
                                   frame.pixels[seedOffset + 2U]};
  const float toleranceSquared = tolerance_ * tolerance_;

  core::EntityMask mask{frame.index, frame.width, frame.height,
                        std::vector<std::uint8_t>(pixelCount, 0)};
  std::vector<std::uint8_t> visited(pixelCount, 0);
  std::queue<std::size_t> pending;
  const std::size_t seedIndex = static_cast<std::size_t>(seedY) * frame.width + seedX;
  pending.push(seedIndex);
  visited[seedIndex] = 1;
  while (!pending.empty()) {
    const std::size_t index = pending.front();
    pending.pop();
    const std::size_t offset = index * 3U;
    float distanceSquared = 0.0F;
    for (std::size_t channel = 0; channel < 3; ++channel) {
      const float difference = static_cast<float>(frame.pixels[offset + channel]) - seed[channel];
      distanceSquared += difference * difference;
    }
    if (distanceSquared > toleranceSquared) continue;
    mask.values[index] = 255;
    const std::uint32_t x = static_cast<std::uint32_t>(index % frame.width);
    const std::uint32_t y = static_cast<std::uint32_t>(index / frame.width);
    const std::array<std::size_t, 4> neighbors = {
        y > 0 ? index - frame.width : index,
        y + 1U < frame.height ? index + frame.width : index,
        x > 0 ? index - 1U : index,
        x + 1U < frame.width ? index + 1U : index,
    };
    for (const auto neighbor : neighbors) {
      if (!visited[neighbor]) {
        visited[neighbor] = 1;
        pending.push(neighbor);
      }
    }
  }
  core::Entity entity{};
  entity.id = 1;
  entity.category = "prompt_region";
  entity.maskTimeline.push_back(std::move(mask));
  return std::vector<core::Entity>{std::move(entity)};
}

core::Result<std::vector<core::Entity>> CentroidTracker::track(
    const core::Frame& frame, const std::vector<core::Entity>& entities) {
  auto tracked = entities;
  for (auto& entity : tracked) {
    const auto previous = std::find_if(previous_.begin(), previous_.end(),
                                       [&](const auto& item) { return item.id == entity.id; });
    const auto currentMasks = entity.maskTimeline;
    if (previous != previous_.end()) {
      entity.trajectory = previous->trajectory;
      entity.maskTimeline = previous->maskTimeline;
      entity.maskTimeline.insert(entity.maskTimeline.end(), currentMasks.begin(),
                                 currentMasks.end());
    }
    core::Vec3 center{};
    std::size_t count = 0;
    if (!currentMasks.empty()) {
      const auto& mask = currentMasks.back();
      for (std::uint32_t y = 0; y < mask.height; ++y) {
        for (std::uint32_t x = 0; x < mask.width; ++x) {
          const auto offset = static_cast<std::size_t>(y) * mask.width + x;
          if (offset < mask.values.size() && mask.values[offset] != 0) {
            center.x += static_cast<float>(x);
            center.y += static_cast<float>(y);
            ++count;
          }
        }
      }
    }
    if (count > 0) {
      center.x /= static_cast<float>(count);
      center.y /= static_cast<float>(count);
    }
    entity.trajectory.push_back({frame.index, center});
  }
  previous_ = tracked;
  return tracked;
}

}  // namespace physfx::semantics
