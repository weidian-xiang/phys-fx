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

#include "physfx/core/SimulationResult.h"

namespace physfx::physics {

/** @brief 物理模块内的粒子别名。 @todo Phase2 实现：适配 GPU 粒子缓冲。 */
using Particle = core::Particle;

/** @brief 物理模块内的粒子集合别名。 @todo Phase2 实现：适配具体求解器。 */
using ParticleSet = core::ParticleSet;

/** @brief 物理模块内的模拟结果别名。 @todo Phase2 实现：适配具体求解器。 */
using SimulationResult = core::SimulationResult;

}  // namespace physfx::physics
