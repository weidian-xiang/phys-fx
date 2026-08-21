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
#include "physfx/core/SemanticScene.h"
#include "physfx/core/Status.h"

namespace physfx::semantics {

/** @brief 实体类别、材质与边界属性估计器抽象接口。 */
class PHYSFX_API IEntityAttributeEstimator {
 public:
  virtual ~IEntityAttributeEstimator() = default;
  /**
   * @brief 补充实体属性。
   * @param frame 当前帧。
   * @param entity 待更新实体。
   * @return 操作状态。
   */
  virtual core::Status estimate(const core::Frame& frame, core::Entity& entity) = 0;
};

}  // namespace physfx::semantics
