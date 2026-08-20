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

#include <cstddef>
#include <cstdint>

#include "physfx/core/Math.h"

namespace physfx::physics {

/** @brief 内置 CPU 粒子预设。 */
enum class ParticlePreset { kSparks, kSmoke };

/** @brief 模拟器通用配置，新增字段均有默认值以保持早期调用兼容。 */
struct SimConfig {
  float timeStepSeconds{1.0F / 60.0F};
  int substeps{1};
  ParticlePreset particlePreset{ParticlePreset::kSparks};
  core::Vec3 emitterPosition{320.0F, 240.0F, 0.0F};
  core::Vec3 initialVelocity{0.0F, -80.0F, 0.0F};
  core::Vec3 gravity{0.0F, 120.0F, 0.0F};
  float emissionRate{120.0F};
  float velocityDecay{0.98F};
  float lifetimeSeconds{1.0F};
  float startSizePixels{3.0F};
  float endSizePixels{1.0F};
  std::size_t maxParticles{10'000};
  std::uint64_t boundEntityId{0};
};

}  // namespace physfx::physics
