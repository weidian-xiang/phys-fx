/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/compositing/SeasonCompositor.h"

#include <algorithm>
#include <cstdint>

#include "physfx/core/SemanticScene.h"

namespace physfx::compositing {

core::Frame SeasonCompositor::compose(const core::Frame& frame, const core::SceneContext& scene,
                                      const core::SimulationResult&) {
  core::Frame output = frame;
  if (frame.format != core::PixelFormat::kRgb8 ||
      frame.pixels.size() < static_cast<std::size_t>(frame.width) * frame.height * 3U ||
      !scene.semanticScene) {
    return output;
  }
  float red = 1.0F;
  float green = 1.0F;
  float blue = 1.0F;
  switch (scene.semanticScene->season) {
    case core::Season::kWinter:
      red = 0.82F;
      green = 0.92F;
      blue = 1.10F;
      break;
    case core::Season::kAutumn:
      red = 1.12F;
      green = 0.84F;
      blue = 0.62F;
      break;
    case core::Season::kSpring:
      red = 0.94F;
      green = 1.08F;
      blue = 0.88F;
      break;
    case core::Season::kSummer:
      red = 1.06F;
      green = 1.04F;
      blue = 0.92F;
      break;
    case core::Season::kOriginal:
      return output;
  }
  for (std::uint32_t y = 0; y < frame.height; ++y) {
    const bool skyBand = y < (frame.height * 2U / 3U);
    for (std::uint32_t x = 0; x < frame.width; ++x) {
      const auto offset = (static_cast<std::size_t>(y) * frame.width + x) * 3U;
      const float skyFactor = skyBand ? 1.0F : 0.82F;
      output.pixels[offset] = static_cast<std::uint8_t>(
          std::clamp(static_cast<int>(output.pixels[offset] * red * skyFactor), 0, 255));
      output.pixels[offset + 1U] = static_cast<std::uint8_t>(
          std::clamp(static_cast<int>(output.pixels[offset + 1U] * green * skyFactor), 0, 255));
      output.pixels[offset + 2U] = static_cast<std::uint8_t>(
          std::clamp(static_cast<int>(output.pixels[offset + 2U] * blue * skyFactor), 0, 255));
    }
  }
  return output;
}

}  // namespace physfx::compositing
