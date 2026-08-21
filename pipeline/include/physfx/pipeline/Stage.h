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
#include "physfx/core/Status.h"

namespace physfx::pipeline {

/** @brief 一个流水线阶段共享的上下文。 @todo Phase2 增加取消、进度与错误信息。 */
struct PipelineContext {
  core::Frame frame{};
  core::SceneContext scene{};
  core::SimulationResult simulation{};
};

/**
 * @brief 可插拔流水线阶段接口。
 * @param context 当前帧上下文。
 * @return 阶段是否成功。
 * @todo Phase2 支持异步执行和 DAG 调度。
 */
class PHYSFX_API Stage {
 public:
  virtual ~Stage() = default;
  /** @brief 返回阶段名称。 @return 稳定的阶段名称。 @todo Phase2 实现：支持配置别名。 */
  [[nodiscard]] virtual const char* name() const noexcept = 0;
  /**
   * @brief 处理一个流水线上下文。
   * @param context 当前帧上下文。
   * @return 阶段是否成功。
   * @todo Phase2 实现：支持异步执行和 DAG 调度。
   */
  virtual core::Status process(PipelineContext& context) = 0;
};

}  // namespace physfx::pipeline
