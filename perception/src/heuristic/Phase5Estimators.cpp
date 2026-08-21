/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/perception/Phase5Estimators.h"

#include <algorithm>
#include <cmath>

namespace physfx::perception {
namespace {

bool rgbFrame(const core::Frame& frame) {
  return frame.format == core::PixelFormat::kRgb8 &&
         frame.pixels.size() >= static_cast<std::size_t>(frame.width) * frame.height * 3U;
}

float luminance(const core::Frame& frame, std::uint32_t x, std::uint32_t y) {
  const auto offset = (static_cast<std::size_t>(y) * frame.width + x) * 3U;
  return (0.2126F * frame.pixels[offset] + 0.7152F * frame.pixels[offset + 1U] +
          0.0722F * frame.pixels[offset + 2U]) /
         255.0F;
}

}  // namespace

core::DepthMap DepthAnythingEstimator::estimate(const core::Frame& frame) {
  core::DepthMap depth{.width = frame.width, .height = frame.height};
  depth.values.resize(static_cast<std::size_t>(frame.width) * frame.height, 0.5F);
  if (!rgbFrame(frame) || frame.width == 0 || frame.height == 0) return depth;
  // Relative inverse luminance is monotonic for the common photographic case and
  // gives downstream ground/occlusion code a useful ordering without model weights.
  for (std::uint32_t y = 0; y < frame.height; ++y) {
    for (std::uint32_t x = 0; x < frame.width; ++x) {
      const float luma = luminance(frame, x, y);
      const float verticalPrior = static_cast<float>(y) / std::max(1U, frame.height - 1U);
      depth.values[static_cast<std::size_t>(y) * frame.width + x] =
          std::clamp(0.15F + 0.65F * (1.0F - luma) + 0.20F * verticalPrior, 0.0F, 1.0F);
    }
  }
  return depth;
}

core::OcclusionMask SkySegmenter::segment(const core::Frame& frame) {
  core::OcclusionMask mask{.width = frame.width, .height = frame.height};
  mask.values.assign(static_cast<std::size_t>(frame.width) * frame.height, 0U);
  if (!rgbFrame(frame) || frame.width == 0 || frame.height == 0) return mask;
  const auto horizon = std::max(1U, frame.height * 2U / 3U);
  for (std::uint32_t y = 0; y < horizon; ++y) {
    const float rowRatio = static_cast<float>(y) / static_cast<float>(horizon);
    for (std::uint32_t x = 0; x < frame.width; ++x) {
      const auto offset = (static_cast<std::size_t>(y) * frame.width + x) * 3U;
      const float r = frame.pixels[offset] / 255.0F;
      const float g = frame.pixels[offset + 1U] / 255.0F;
      const float b = frame.pixels[offset + 2U] / 255.0F;
      const float blueBias = b - (r + g) * 0.25F;
      // Include neutral overcast skies while avoiding dark structures at the top.
      const bool sky = (blueBias > 0.04F || (r + g + b > 1.55F && rowRatio < 0.45F)) &&
                       luminance(frame, x, y) > 0.18F;
      mask.values[static_cast<std::size_t>(y) * frame.width + x] = sky ? 1U : 0U;
    }
  }
  return mask;
}

core::LightParams HeuristicLightEstimator::estimate(const core::Frame& frame) {
  core::LightParams result{};
  if (!rgbFrame(frame) || frame.width < 2 || frame.height < 2) return result;
  float brightest = -1.0F;
  std::uint32_t brightX = frame.width / 2U;
  std::uint32_t brightY = frame.height / 2U;
  double average = 0.0;
  for (std::uint32_t y = 0; y < frame.height; ++y) {
    for (std::uint32_t x = 0; x < frame.width; ++x) {
      const float value = luminance(frame, x, y);
      average += value;
      if (value > brightest) {
        brightest = value;
        brightX = x;
        brightY = y;
      }
    }
  }
  result.ambientIntensity = static_cast<float>(average / (frame.width * frame.height));
  const float dx = static_cast<float>(frame.width / 2U) - static_cast<float>(brightX);
  const float dy = static_cast<float>(frame.height / 2U) - static_cast<float>(brightY);
  const float length = std::sqrt(dx * dx + dy * dy);
  if (length > 0.001F) result.dominantDirection = {dx / length, dy / length, 0.0F};
  return result;
}

}  // namespace physfx::perception
