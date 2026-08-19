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
 * @brief 光照估计器抽象接口。
 * @param frame 当前视频帧。
 * @return 场景光照参数。
 * @todo Phase2 接入学习式或可微光照估计模型。
 */
class ILightEstimator {
 public:
  virtual ~ILightEstimator() = default;
  /**
   * @brief 估计单帧光照。
   * @param frame 当前视频帧。
   * @return 场景光照参数。
   * @todo Phase2 实现：接入学习式或可微光照估计模型。
   */
  virtual core::LightParams estimate(const core::Frame& frame) = 0;
};

}  // namespace physfx::perception
