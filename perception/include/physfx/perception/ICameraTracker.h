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
 * @brief 相机运动跟踪抽象接口。
 * @param frame 当前视频帧。
 * @return 当前帧相机位姿。
 * @todo Phase2 接入 COLMAP 思想或学习式 SLAM。
 */
class ICameraTracker {
 public:
  virtual ~ICameraTracker() = default;
  /**
   * @brief 跟踪当前帧相机位姿。
   * @param frame 当前视频帧。
   * @return 当前帧相机位姿。
   * @todo Phase2 实现：接入 COLMAP 思想或学习式 SLAM。
   */
  virtual core::CameraPose track(const core::Frame& frame) = 0;
};

}  // namespace physfx::perception
