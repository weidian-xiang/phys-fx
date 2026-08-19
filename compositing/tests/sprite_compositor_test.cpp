/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <memory>

#include "physfx/compositing/SpriteCompositor.h"
#include "physfx/core/SemanticScene.h"

int main() {
  physfx::core::Frame frame{};
  frame.index = 1;
  frame.width = 8;
  frame.height = 8;
  frame.format = physfx::core::PixelFormat::kRgb8;
  frame.pixels.assign(8U * 8U * 3U, 0);

  physfx::core::SceneContext context{};
  context.semanticScene = std::make_shared<physfx::core::SemanticScene>();
  physfx::core::Entity entity{};
  entity.id = 1;
  physfx::core::EntityMask mask{1, 8, 8, std::vector<std::uint8_t>(64, 0)};
  mask.values[4U * 8U + 4U] = 255;
  entity.maskTimeline.push_back(mask);
  context.semanticScene->entities.push_back(entity);

  physfx::core::SimulationResult simulation{};
  physfx::core::Particle particle{};
  particle.position = {4.5F, 4.5F, 0.0F};
  particle.sizePixels = 2.5F;
  particle.opacity = 1.0F;
  particle.color = {1.0F, 0.5F, 0.0F};
  particle.additive = true;
  simulation.particles.particles.push_back(particle);

  physfx::compositing::SpriteCompositor compositor;
  const auto output = compositor.compose(frame, context, simulation);
  const std::size_t center = (4U * 8U + 4U) * 3U;
  const std::size_t visible = (3U * 8U + 4U) * 3U;
  if (output.width != frame.width || output.height != frame.height) return 1;
  if (output.pixels[center] != 0) return 2;
  if (output.pixels[visible] == 0) return 3;
  return 0;
}
