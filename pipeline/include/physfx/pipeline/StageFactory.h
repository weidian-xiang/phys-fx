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

#include <memory>

#include "physfx/pipeline/Pipeline.h"

namespace physfx::pipeline {

/** @brief 默认桩模块工厂。 @todo Phase2 根据配置选择真实后端。 */
class PHYSFX_API StageFactory {
 public:
  /** @brief 创建由桩实现组成的默认管线。 @return 可运行的管线。 */
  [[nodiscard]] static std::unique_ptr<Pipeline> createDefaultPipeline();
  /** @brief 根据配置装配可禁用、可替换路径的桩管线。 @param config 配置。 @return 管线。 */
  [[nodiscard]] static std::unique_ptr<Pipeline> createPipeline(const core::Config& config);
};

}  // namespace physfx::pipeline
