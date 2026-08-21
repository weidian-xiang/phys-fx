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

#include <vector>

#include "physfx/core/Frame.h"
#include "physfx/core/SemanticScene.h"
#include "physfx/core/Status.h"

namespace physfx::semantics {

/** @brief 多目标时序跟踪器抽象接口。 */
class PHYSFX_API ITracker {
 public:
  virtual ~ITracker() = default;
  /**
   * @brief 更新实体轨迹。
   * @param frame 当前帧。
   * @param entities 当前检测实体。
   * @return 更新后的实体或错误状态。
   */
  virtual core::Result<std::vector<core::Entity>> track(
      const core::Frame& frame, const std::vector<core::Entity>& entities) = 0;
};

}  // namespace physfx::semantics
