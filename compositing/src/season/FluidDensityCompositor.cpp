/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/compositing/FluidDensityCompositor.h"

#include <algorithm>
#include <cstdint>

namespace physfx::compositing {

core::Frame FluidDensityCompositor::compose(const core::Frame& frame, const core::SceneContext&,
                                            const core::SimulationResult& simulation) {
  core::Frame output = frame;
  if (frame.format != core::PixelFormat::kRgb8 || simulation.density.empty() ||
      simulation.densityWidth == 0 || simulation.densityHeight == 0 ||
      frame.pixels.size() < static_cast<std::size_t>(frame.width) * frame.height * 3U) {
    return output;
  }
  for (std::uint32_t y = 0; y < frame.height; ++y) {
    const auto sy = static_cast<std::uint32_t>(static_cast<double>(y) * simulation.densityHeight /
                                               std::max(1U, frame.height));
    for (std::uint32_t x = 0; x < frame.width; ++x) {
      const auto sx = static_cast<std::uint32_t>(static_cast<double>(x) * simulation.densityWidth /
                                                 std::max(1U, frame.width));
      const auto densityIndex = std::min<std::size_t>(
          simulation.density.size() - 1U,
          static_cast<std::size_t>(std::min(sy, simulation.densityHeight - 1U)) *
                  simulation.densityWidth +
              std::min(sx, simulation.densityWidth - 1U));
      const float alpha = std::clamp(simulation.density[densityIndex], 0.0F, 1.0F) * 0.45F;
      if (alpha <= 0.001F) continue;
      const auto offset = (static_cast<std::size_t>(y) * frame.width + x) * 3U;
      // Cool blue-white for splash/smoke; premultiplied-style alpha keeps edges stable.
      const float source[3] = {0.82F, 0.90F, 1.0F};
      for (std::size_t channel = 0; channel < 3U; ++channel) {
        output.pixels[offset + channel] = static_cast<std::uint8_t>(
            std::clamp(static_cast<int>(output.pixels[offset + channel] * (1.0F - alpha) +
                                        source[channel] * 255.0F * alpha),
                       0, 255));
      }
    }
  }
  return output;
}

}  // namespace physfx::compositing
