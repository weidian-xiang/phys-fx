/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/physics/TaichiFluidSimulator.h"

#include <algorithm>
#include <cmath>

namespace physfx::physics {

core::Status TaichiFluidSimulator::initialize(const SimConfig& config) {
  if (config.timeStepSeconds <= 0.0F || config.substeps <= 0) {
    return {core::StatusCode::kInvalidArgument, "Taichi 烟雾时间步或子步数无效"};
  }
  if (config.gridWidth < 8U || config.gridHeight < 8U || config.dissipation <= 0.0F ||
      config.dissipation > 1.0F) {
    return {core::StatusCode::kInvalidArgument, "Taichi 烟雾网格或耗散参数无效"};
  }
  config_ = config;
  width_ = config.gridWidth;
  height_ = config.gridHeight;
  density_.assign(static_cast<std::size_t>(width_) * height_, 0.0F);
  initialized_ = true;
  return core::Status::success();
}

core::Result<core::SimulationResult> TaichiFluidSimulator::simulate(
    const core::Frame& frame, const core::SceneContext& scene) {
  if (!initialized_)
    return core::Status{core::StatusCode::kInvalidArgument, "Taichi 模拟器尚未初始化"};
  (void)scene;
  // Deterministic semi-Lagrangian density solver. It mirrors the Taichi bridge
  // contract and remains usable in a zero-dependency build.
  const float dt = config_.timeStepSeconds / static_cast<float>(config_.substeps);
  const float emitterX = config_.emitterPosition.x;
  const float emitterY = config_.emitterPosition.y;
  for (int step = 0; step < config_.substeps; ++step) {
    const auto old = density_;
    for (std::uint32_t y = 0; y < height_; ++y) {
      for (std::uint32_t x = 0; x < width_; ++x) {
        const float gx = static_cast<float>(x) - config_.initialVelocity.x * dt * 0.02F;
        const float gy = static_cast<float>(y) - config_.initialVelocity.y * dt * 0.02F;
        const auto sx =
            static_cast<std::uint32_t>(std::clamp(gx, 0.0F, static_cast<float>(width_ - 1U)));
        const auto sy =
            static_cast<std::uint32_t>(std::clamp(gy, 0.0F, static_cast<float>(height_ - 1U)));
        const auto source = static_cast<std::size_t>(sy) * width_ + sx;
        const auto target = static_cast<std::size_t>(y) * width_ + x;
        const float rise = config_.buoyancy * dt * (1.0F - static_cast<float>(y) / height_);
        density_[target] =
            std::clamp(old[source] * config_.dissipation + rise * old[source], 0.0F, 1.0F);
      }
    }
    const float normX = frame.width == 0 ? 0.5F : emitterX / static_cast<float>(frame.width);
    const float normY = frame.height == 0 ? 0.75F : emitterY / static_cast<float>(frame.height);
    const auto ex = static_cast<std::uint32_t>(std::clamp(normX, 0.0F, 1.0F) * (width_ - 1U));
    const auto ey = static_cast<std::uint32_t>(std::clamp(normY, 0.0F, 1.0F) * (height_ - 1U));
    for (int radius = -2; radius <= 2; ++radius) {
      const int yy = static_cast<int>(ey) + radius;
      if (yy < 0 || yy >= static_cast<int>(height_)) continue;
      const auto index = static_cast<std::size_t>(yy) * width_ + ex;
      density_[index] = std::min(1.0F, density_[index] + 0.18F * (1.0F - std::abs(radius) / 3.0F));
    }
  }
  core::SimulationResult result{};
  result.stepCount = static_cast<std::size_t>(config_.substeps);
  result.densityWidth = width_;
  result.densityHeight = height_;
  result.density = density_;
  return result;
}

}  // namespace physfx::physics
