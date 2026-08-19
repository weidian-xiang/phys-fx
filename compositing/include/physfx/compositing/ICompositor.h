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
#include "physfx/core/SimulationResult.h"

namespace physfx::compositing {

/**
 * @brief 将模拟结果合成到原始视频帧的抽象接口。
 * @param frame 输入视频帧。
 * @param scene 场景理解结果。
 * @param simulation 物理模拟结果。
 * @return 合成后的帧。
 * @todo Phase2 接入 OpenGL/Vulkan 渲染、遮挡、阴影和色彩匹配。
 */
class ICompositor {
 public:
  virtual ~ICompositor() = default;
  /**
   * @brief 将模拟结果合成到原始帧。
   * @param frame 输入视频帧。
   * @param scene 场景理解结果。
   * @param simulation 物理模拟结果。
   * @return 合成后的帧。
   * @todo Phase2 实现：接入 OpenGL/Vulkan 渲染、遮挡、阴影和色彩匹配。
   */
  virtual core::Frame compose(const core::Frame& frame,
                              const core::SceneContext& scene,
                              const core::SimulationResult& simulation) = 0;
};

}  // namespace physfx::compositing
