/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/compositing/LayeredCompositor.h"

#include <algorithm>
#include <array>
#include <cmath>

#include "physfx/core/SemanticScene.h"

namespace physfx::compositing {

core::Frame LayeredCompositor::compose(const core::Frame& frame, const core::SceneContext& scene,
                                       const core::SimulationResult&) {
  core::Frame output = frame;
  if (!scene.semanticScene || frame.format != core::PixelFormat::kRgb8) return output;
  if (frame.pixels.size() < static_cast<std::size_t>(frame.width) * frame.height * 3U)
    return output;
  for (const auto& entity : scene.semanticScene->entities) {
    if (entity.deleted || entity.maskTimeline.empty() || entity.trajectory.empty()) continue;
    const auto& mask = entity.maskTimeline.back();
    const auto& target = entity.trajectory.back().position;
    if (entity.material.name != "default") {
      for (std::uint32_t y = 0; y < mask.height; ++y) {
        for (std::uint32_t x = 0; x < mask.width; ++x) {
          const auto index = static_cast<std::size_t>(y) * mask.width + x;
          if (index >= mask.values.size() || mask.values[index] == 0) continue;
          const auto offset = index * 3U;
          for (std::size_t channel = 0; channel < 3; ++channel) {
            const float factor = channel == 0   ? entity.material.baseColor.x
                                 : channel == 1 ? entity.material.baseColor.y
                                                : entity.material.baseColor.z;
            output.pixels[offset + channel] = static_cast<std::uint8_t>(std::clamp(
                static_cast<int>(std::lround(output.pixels[offset + channel] * factor)), 0, 255));
          }
        }
      }
    }
    float centerX = 0.0F;
    float centerY = 0.0F;
    std::size_t count = 0;
    for (std::uint32_t y = 0; y < mask.height; ++y) {
      for (std::uint32_t x = 0; x < mask.width; ++x) {
        const auto sourceIndex = static_cast<std::size_t>(y) * mask.width + x;
        if (sourceIndex < mask.values.size() && mask.values[sourceIndex] != 0) {
          centerX += static_cast<float>(x);
          centerY += static_cast<float>(y);
          ++count;
        }
      }
    }
    if (count == 0) continue;
    centerX /= static_cast<float>(count);
    centerY /= static_cast<float>(count);
    const int shiftX = static_cast<int>(std::lround(target.x - centerX));
    const int shiftY = static_cast<int>(std::lround(target.y - centerY));
    if (shiftX == 0 && shiftY == 0) continue;
    // 先用邻域均值清理原位置，避免移动后留下完整的旧图层。
    for (std::uint32_t y = 0; y < mask.height; ++y) {
      for (std::uint32_t x = 0; x < mask.width; ++x) {
        const auto sourceIndex = static_cast<std::size_t>(y) * mask.width + x;
        if (sourceIndex >= mask.values.size() || mask.values[sourceIndex] == 0) continue;
        const auto sourceOffset = sourceIndex * 3U;
        std::array<int, 3> sum{};
        int samples = 0;
        for (const auto& delta :
             std::array<std::pair<int, int>, 4>{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}}) {
          const int nx = static_cast<int>(x) + delta.first;
          const int ny = static_cast<int>(y) + delta.second;
          if (nx < 0 || ny < 0 || nx >= static_cast<int>(frame.width) ||
              ny >= static_cast<int>(frame.height))
            continue;
          const auto neighbourIndex =
              static_cast<std::size_t>(ny) * frame.width + static_cast<std::uint32_t>(nx);
          if (neighbourIndex < mask.values.size() && mask.values[neighbourIndex] != 0) continue;
          const auto neighbourOffset = neighbourIndex * 3U;
          for (std::size_t channel = 0; channel < 3; ++channel)
            sum[channel] += frame.pixels[neighbourOffset + channel];
          ++samples;
        }
        if (samples > 0) {
          for (std::size_t channel = 0; channel < 3; ++channel)
            output.pixels[sourceOffset + channel] =
                static_cast<std::uint8_t>(sum[channel] / samples);
        }
        const int destinationX = static_cast<int>(x) + shiftX;
        const int destinationY = static_cast<int>(y) + shiftY;
        if (destinationX < 0 || destinationY < 0 || destinationX >= static_cast<int>(frame.width) ||
            destinationY >= static_cast<int>(frame.height))
          continue;
        const auto destinationOffset = (static_cast<std::size_t>(destinationY) * frame.width +
                                        static_cast<std::uint32_t>(destinationX)) *
                                       3U;
        for (std::size_t channel = 0; channel < 3; ++channel)
          output.pixels[destinationOffset + channel] = frame.pixels[sourceOffset + channel];
      }
    }
  }
  return output;
}

}  // namespace physfx::compositing
