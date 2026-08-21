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

#include "physfx/core/Math.h"
#include "physfx/core/SceneContext.h"

namespace physfx::perception {

/**
 * @brief 地面平面检测抽象接口。
 * @param depth 当前帧深度图。
 * @return 检测到的地面平面。
 * @todo Phase2 实现点云拟合与坐标系校准。
 */
class PHYSFX_API IGroundPlaneDetector {
 public:
  virtual ~IGroundPlaneDetector() = default;
  /**
   * @brief 从深度图检测地面。
   * @param depth 当前帧深度图。
   * @return 检测到的地面平面。
   * @todo Phase2 实现：完成点云拟合与坐标系校准。
   */
  virtual core::Plane detect(const core::DepthMap& depth) = 0;
};

}  // namespace physfx::perception
