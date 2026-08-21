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

#include <array>

#include "physfx/core/Export.h"

namespace physfx::core {

/** @brief 三维浮点向量。 @todo Phase2 实现完整的数学运算。 */
struct Vec3 {
  float x{0.0F};
  float y{0.0F};
  float z{0.0F};
};

/** @brief 四维齐次矩阵。 @todo Phase2 实现矩阵运算与相机投影。 */
struct Mat4 {
  std::array<float, 16> values{};
};

/** @brief 平面方程。 @todo Phase2 实现平面拟合与坐标变换。 */
struct Plane {
  Vec3 normal{0.0F, 1.0F, 0.0F};
  float distance{0.0F};
};

}  // namespace physfx::core
