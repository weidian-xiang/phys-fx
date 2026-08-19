/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cassert>
#include <memory>

#include "physfx/compositing/SpriteCompositor.h"
#include "physfx/physics/SimpleParticleSystem.h"
#include "physfx/semantics/TraditionalSemantics.h"

int main() {
  physfx::core::Frame frame{};
  frame.width = 16;
  frame.height = 16;
  frame.format = physfx::core::PixelFormat::kRgb8;
  frame.pixels.assign(16U * 16U * 3U, 0);
  const std::size_t centerOffset = (8U * 16U + 8U) * 3U;
  frame.pixels[centerOffset] = 255;

  physfx::semantics::ColorPromptSegmenter segmenter(8, 8, 5.0F);
  auto segmented = segmenter.segment(frame);
  assert(segmented.ok());
  physfx::semantics::CentroidTracker tracker;
  auto tracked = tracker.track(frame, segmented.value());
  assert(tracked.ok());

  physfx::core::SceneContext scene{};
  scene.semanticScene = std::make_shared<physfx::core::SemanticScene>();
  scene.semanticScene->entities = tracked.value();

  physfx::physics::SimpleParticleSystem particles;
  physfx::physics::SimConfig config{};
  config.timeStepSeconds = 1.0F / 30.0F;
  config.emissionRate = 30.0F;
  config.startSizePixels = 4.0F;
  config.boundEntityId = 1;
  assert(particles.initialize(config).ok());
  auto simulation = particles.simulate(frame, scene);
  assert(simulation.ok() && simulation.value().particles.particles.size() == 1);

  physfx::compositing::SpriteCompositor compositor;
  const auto output = compositor.compose(frame, scene, simulation.value());
  assert(output.pixels[centerOffset + 1U] == 0);
  const std::size_t visibleOffset = (8U * 16U + 9U) * 3U;
  assert(output.pixels[visibleOffset] > 0);
  return 0;
}
