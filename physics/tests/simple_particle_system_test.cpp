/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cmath>
#include <memory>

#include "physfx/core/SemanticScene.h"
#include "physfx/physics/SimpleParticleSystem.h"

int main() {
  physfx::physics::SimpleParticleSystem system;
  physfx::physics::SimConfig config{};
  config.timeStepSeconds = 1.0F / 30.0F;
  config.emissionRate = 60.0F;
  config.boundEntityId = 7;
  if (!system.initialize(config).ok()) return 1;

  physfx::core::Frame frame{};
  frame.index = 3;
  physfx::core::SceneContext context{};
  context.semanticScene = std::make_shared<physfx::core::SemanticScene>();
  physfx::core::Entity entity{};
  entity.id = 7;
  entity.trajectory.push_back({3, {10.0F, 20.0F, 0.0F}});
  context.semanticScene->entities.push_back(entity);
  auto result = system.simulate(frame, context);
  if (!result.ok() || result.value().particles.particles.size() != 2) return 2;
  const auto& particle = result.value().particles.particles.front();
  if (std::abs(particle.position.x - 10.0F) > 0.001F ||
      std::abs(particle.position.y - 20.0F) > 0.001F || !particle.additive) {
    return 3;
  }
  return 0;
}
