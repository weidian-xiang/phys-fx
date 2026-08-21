/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/neural_render/InpaintingRenderer.h"

#include <algorithm>
#include <array>
#include <cstddef>

#include "physfx/core/SemanticScene.h"

namespace physfx::neural_render {

namespace {

bool masked(const core::EntityMask& mask, std::uint32_t x, std::uint32_t y) {
  if (x >= mask.width || y >= mask.height) return false;
  const auto offset = static_cast<std::size_t>(y) * mask.width + x;
  return offset < mask.values.size() && mask.values[offset] != 0;
}

}  // namespace

std::string_view InpaintingRenderer::name() const noexcept { return "inpainting_cpu_fallback"; }

core::Result<core::Frame> InpaintingRenderer::render(const core::Frame& frame,
                                                     const core::SceneContext& scene,
                                                     const core::SimulationResult&) {
  core::Frame output = frame;
  if (frame.format != core::PixelFormat::kRgb8 || !scene.semanticScene) return output;
  const auto expected = static_cast<std::size_t>(frame.width) * frame.height * 3U;
  if (frame.pixels.size() < expected) return output;
  for (const auto& entity : scene.semanticScene->entities) {
    if (!entity.deleted) continue;
    for (const auto& mask : entity.maskTimeline) {
      if (mask.frameIndex != frame.index) continue;
      for (std::uint32_t y = 0; y < frame.height; ++y) {
        for (std::uint32_t x = 0; x < frame.width; ++x) {
          if (!masked(mask, x, y)) continue;
          std::array<int, 3> sum{};
          int samples = 0;
          for (const auto& delta :
               std::array<std::pair<int, int>, 4>{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}}) {
            const int nx = static_cast<int>(x) + delta.first;
            const int ny = static_cast<int>(y) + delta.second;
            if (nx < 0 || ny < 0 || nx >= static_cast<int>(frame.width) ||
                ny >= static_cast<int>(frame.height) ||
                masked(mask, static_cast<std::uint32_t>(nx), static_cast<std::uint32_t>(ny))) {
              continue;
            }
            const auto offset =
                (static_cast<std::size_t>(ny) * frame.width + static_cast<std::uint32_t>(nx)) * 3U;
            for (std::size_t channel = 0; channel < 3; ++channel)
              sum[channel] += frame.pixels[offset + channel];
            ++samples;
          }
          const auto outputOffset = (static_cast<std::size_t>(y) * frame.width + x) * 3U;
          if (samples == 0) {
            output.pixels[outputOffset] = 0;
            output.pixels[outputOffset + 1U] = 0;
            output.pixels[outputOffset + 2U] = 0;
          } else {
            for (std::size_t channel = 0; channel < 3; ++channel)
              output.pixels[outputOffset + channel] =
                  static_cast<std::uint8_t>(sum[channel] / samples);
          }
        }
      }
    }
  }
  return output;
}

}  // namespace physfx::neural_render
