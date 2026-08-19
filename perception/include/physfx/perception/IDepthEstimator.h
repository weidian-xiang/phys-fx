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

#include "physfx/core/Frame.h"
#include "physfx/core/SceneContext.h"

namespace physfx::perception {

/**
 * @brief 深度估计器抽象接口。
 * @param frame 当前视频帧。
 * @return 深度图。
 * @todo Phase2 接入 MiDaS 或 Depth-Anything。
 */
class IDepthEstimator {
 public:
  virtual ~IDepthEstimator() = default;
  /**
   * @brief 估计单帧深度。
   * @param frame 当前视频帧。
   * @return 深度图。
   * @todo Phase2 实现：接入 MiDaS 或 Depth-Anything。
   */
  virtual core::DepthMap estimate(const core::Frame& frame) = 0;
};

}  // namespace physfx::perception
