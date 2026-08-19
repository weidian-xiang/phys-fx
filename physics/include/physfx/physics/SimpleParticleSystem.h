/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <vector>

#include "physfx/physics/IPhysicsSimulator.h"

namespace physfx::physics {

/** @brief 确定性的纯 CPU 粒子系统，支持火花和烟尘预设。 */
class SimpleParticleSystem final : public IPhysicsSimulator {
 public:
  core::Status initialize(const SimConfig& config) override;
  core::Result<core::SimulationResult> simulate(const core::Frame& frame,
                                                const core::SceneContext& scene) override;

 private:
  struct ActiveParticle {
    core::Particle particle{};
    float initialLifetime{1.0F};
  };

  [[nodiscard]] core::Vec3 emitterFor(const core::Frame& frame,
                                      const core::SceneContext& scene) const;
  [[nodiscard]] float randomSigned() noexcept;

  SimConfig config_{};
  std::vector<ActiveParticle> active_{};
  float emissionCarry_{0.0F};
  std::uint32_t randomState_{0x75243808U};
  bool initialized_{false};
};

}  // namespace physfx::physics
