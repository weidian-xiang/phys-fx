/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/compositing/SpriteCompositor.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

#include "physfx/core/SemanticScene.h"

namespace physfx::compositing {

SpriteCompositor::SpriteCompositor(float gain, float warmth)
    : gain_(std::max(gain, 0.0F)), warmth_(std::clamp(warmth, -1.0F, 1.0F)) {}

bool SpriteCompositor::occluded(const core::SceneContext& scene, std::uint64_t frameIndex,
                                std::uint32_t x, std::uint32_t y) const {
  if (!scene.semanticScene) {
    return false;
  }
  for (const auto& entity : scene.semanticScene->entities) {
    if (entity.deleted) {
      continue;
    }
    for (const auto& mask : entity.maskTimeline) {
      const std::size_t offset = static_cast<std::size_t>(y) * mask.width + x;
      if (mask.frameIndex == frameIndex && x < mask.width && y < mask.height &&
          offset < mask.values.size() && mask.values[offset] != 0) {
        return true;
      }
    }
  }
  return false;
}

core::Frame SpriteCompositor::compose(const core::Frame& frame, const core::SceneContext& scene,
                                      const core::SimulationResult& simulation) {
  core::Frame output = frame;
  const std::size_t expected = static_cast<std::size_t>(frame.width) * frame.height * 3U;
  if (frame.format != core::PixelFormat::kRgb8 || frame.pixels.size() < expected) {
    return output;
  }
  for (const auto& particle : simulation.particles.particles) {
    const float radius = std::max(particle.sizePixels, 0.5F);
    const int minimumX = std::max(0, static_cast<int>(std::floor(particle.position.x - radius)));
    const int maximumX = std::min(static_cast<int>(frame.width) - 1,
                                  static_cast<int>(std::ceil(particle.position.x + radius)));
    const int minimumY = std::max(0, static_cast<int>(std::floor(particle.position.y - radius)));
    const int maximumY = std::min(static_cast<int>(frame.height) - 1,
                                  static_cast<int>(std::ceil(particle.position.y + radius)));
    std::array<float, 3> color = {particle.color.x * gain_ * (1.0F + 0.25F * warmth_),
                                  particle.color.y * gain_,
                                  particle.color.z * gain_ * (1.0F - 0.25F * warmth_)};
    for (int y = minimumY; y <= maximumY; ++y) {
      for (int x = minimumX; x <= maximumX; ++x) {
        const float dx = (static_cast<float>(x) + 0.5F) - particle.position.x;
        const float dy = (static_cast<float>(y) + 0.5F) - particle.position.y;
        const float distance = std::sqrt(dx * dx + dy * dy);
        if (distance > radius || occluded(scene, frame.index, static_cast<std::uint32_t>(x),
                                          static_cast<std::uint32_t>(y))) {
          continue;
        }
        const float alpha = std::clamp(particle.opacity * (1.0F - distance / radius), 0.0F, 1.0F);
        const std::size_t offset =
            (static_cast<std::size_t>(y) * frame.width + static_cast<std::uint32_t>(x)) * 3U;
        for (std::size_t channel = 0; channel < 3; ++channel) {
          const float source = std::clamp(color[channel], 0.0F, 1.0F) * 255.0F;
          const float destination = static_cast<float>(output.pixels[offset + channel]);
          const float mixed = particle.additive ? destination + source * alpha
                                                : destination * (1.0F - alpha) + source * alpha;
          output.pixels[offset + channel] =
              static_cast<std::uint8_t>(std::clamp(std::lround(mixed), 0L, 255L));
        }
      }
    }
  }
  return output;
}

}  // namespace physfx::compositing
