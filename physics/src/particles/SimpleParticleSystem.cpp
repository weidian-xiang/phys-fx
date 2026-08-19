/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/physics/SimpleParticleSystem.h"

#include <algorithm>
#include <cmath>

#include "physfx/core/SemanticScene.h"

namespace physfx::physics {

core::Status SimpleParticleSystem::initialize(const SimConfig& config) {
  if (config.timeStepSeconds <= 0.0F || config.substeps <= 0 || config.emissionRate < 0.0F ||
      config.lifetimeSeconds <= 0.0F || config.maxParticles == 0) {
    return {core::StatusCode::kInvalidArgument, "粒子配置中的时间步、发射率、生命周期或上限无效"};
  }
  config_ = config;
  if (config_.particlePreset == ParticlePreset::kSmoke) {
    config_.initialVelocity = {0.0F, -25.0F, 0.0F};
    config_.gravity = {0.0F, -4.0F, 0.0F};
    config_.lifetimeSeconds = std::max(config_.lifetimeSeconds, 2.0F);
    config_.startSizePixels = std::max(config_.startSizePixels, 5.0F);
    config_.endSizePixels = std::max(config_.endSizePixels, 12.0F);
  }
  active_.clear();
  emissionCarry_ = 0.0F;
  initialized_ = true;
  return core::Status::success();
}

core::Vec3 SimpleParticleSystem::emitterFor(const core::Frame& frame,
                                            const core::SceneContext& scene) const {
  if (config_.boundEntityId == 0 || !scene.semanticScene) {
    return config_.emitterPosition;
  }
  const auto* entity = scene.semanticScene->findEntity(config_.boundEntityId);
  if (entity == nullptr) {
    return config_.emitterPosition;
  }
  for (auto pose = entity->trajectory.rbegin(); pose != entity->trajectory.rend(); ++pose) {
    if (pose->frameIndex <= frame.index) {
      return pose->position;
    }
  }
  return config_.emitterPosition;
}

float SimpleParticleSystem::randomSigned() noexcept {
  randomState_ = randomState_ * 1664525U + 1013904223U;
  const float normalized = static_cast<float>((randomState_ >> 8U) & 0x00FFFFFFU) /
                           static_cast<float>(0x00FFFFFFU);
  return normalized * 2.0F - 1.0F;
}

core::Result<core::SimulationResult> SimpleParticleSystem::simulate(
    const core::Frame& frame, const core::SceneContext& scene) {
  if (!initialized_) {
    return core::Status{core::StatusCode::kInvalidArgument, "粒子系统尚未初始化"};
  }
  const float dt = config_.timeStepSeconds / static_cast<float>(config_.substeps);
  for (int step = 0; step < config_.substeps; ++step) {
    for (auto& active : active_) {
      auto& particle = active.particle;
      particle.velocity.x = (particle.velocity.x + config_.gravity.x * dt) * config_.velocityDecay;
      particle.velocity.y = (particle.velocity.y + config_.gravity.y * dt) * config_.velocityDecay;
      particle.velocity.z = (particle.velocity.z + config_.gravity.z * dt) * config_.velocityDecay;
      particle.position.x += particle.velocity.x * dt;
      particle.position.y += particle.velocity.y * dt;
      particle.position.z += particle.velocity.z * dt;
      particle.lifetimeSeconds -= dt;
      const float life = std::clamp(particle.lifetimeSeconds / active.initialLifetime, 0.0F, 1.0F);
      particle.opacity = life;
      particle.sizePixels =
          config_.endSizePixels + (config_.startSizePixels - config_.endSizePixels) * life;
    }
    active_.erase(std::remove_if(active_.begin(), active_.end(), [](const ActiveParticle& item) {
                    return item.particle.lifetimeSeconds <= 0.0F;
                  }),
                  active_.end());

    emissionCarry_ += config_.emissionRate * dt;
    std::size_t spawnCount = static_cast<std::size_t>(emissionCarry_);
    emissionCarry_ -= static_cast<float>(spawnCount);
    spawnCount = std::min(spawnCount, config_.maxParticles - active_.size());
    const core::Vec3 emitter = emitterFor(frame, scene);
    for (std::size_t index = 0; index < spawnCount; ++index) {
      core::Particle particle{};
      particle.position = emitter;
      particle.velocity = {config_.initialVelocity.x + randomSigned() * 35.0F,
                           config_.initialVelocity.y + randomSigned() * 20.0F,
                           config_.initialVelocity.z + randomSigned() * 10.0F};
      particle.lifetimeSeconds = config_.lifetimeSeconds * (0.8F + 0.2F * (randomSigned() + 1.0F));
      particle.opacity = 1.0F;
      particle.sizePixels = config_.startSizePixels;
      if (config_.particlePreset == ParticlePreset::kSparks) {
        particle.color = {1.0F, 0.58F, 0.12F};
        particle.additive = true;
      } else {
        particle.color = {0.48F, 0.46F, 0.43F};
        particle.additive = false;
      }
      active_.push_back({particle, particle.lifetimeSeconds});
    }
  }
  core::SimulationResult result{};
  result.stepCount = static_cast<std::size_t>(config_.substeps);
  result.particles.particles.reserve(active_.size());
  for (const auto& active : active_) {
    result.particles.particles.push_back(active.particle);
  }
  return result;
}

}  // namespace physfx::physics
