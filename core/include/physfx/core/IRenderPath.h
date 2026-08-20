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

#include <string_view>

#include "physfx/core/Frame.h"
#include "physfx/core/SceneContext.h"
#include "physfx/core/SimulationResult.h"
#include "physfx/core/Status.h"

namespace physfx::core {

/** @brief 可用的渲染路径类型。 */
enum class RenderPathKind { kCompositing, kNeural };

/** @brief 物理合成与神经重渲染共享的单出口契约。 */
class IRenderPath {
 public:
  virtual ~IRenderPath() = default;
  /** @brief 返回渲染路径名称。 @return 稳定名称。 */
  [[nodiscard]] virtual std::string_view name() const noexcept = 0;
  /**
   * @brief 渲染当前帧。
   * @param frame 输入帧。
   * @param scene 场景上下文。
   * @param simulation 模拟结果。
   * @return 渲染后的帧或错误状态。
   */
  virtual Result<Frame> render(const Frame& frame, const SceneContext& scene,
                               const SimulationResult& simulation) = 0;
};

}  // namespace physfx::core
