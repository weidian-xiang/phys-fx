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
#include <vector>

#include "physfx/core/Math.h"

namespace physfx::core {

/** @brief 单个模拟粒子。 @todo Phase2 实现：增加材质、生命周期和碰撞属性。 */
struct Particle {
  Vec3 position{};
  Vec3 velocity{};
  float lifetimeSeconds{0.0F};
  /** @brief 粒子剩余不透明度，范围为 0 到 1。 */
  float opacity{1.0F};
  /** @brief 屏幕空间粒子半径（像素）。 */
  float sizePixels{1.0F};
  /** @brief 线性 RGB 粒子颜色。 */
  Vec3 color{1.0F, 1.0F, 1.0F};
  /** @brief 是否使用加色混合；false 时使用普通 alpha 混合。 */
  bool additive{false};
};

/** @brief 跨模块传递的粒子集合。 @todo Phase2 实现：支持 GPU 缓冲与 SoA 布局。 */
struct ParticleSet {
  std::vector<Particle> particles{};
};

/** @brief 跨模块传递的物理模拟输出。 @todo Phase2 实现：增加网格和速度场输出。 */
struct SimulationResult {
  ParticleSet particles{};
  std::size_t stepCount{0};
};

}  // namespace physfx::core
