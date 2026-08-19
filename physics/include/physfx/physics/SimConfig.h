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

namespace physfx::physics {

/** @brief 模拟器通用配置。 @todo Phase2 增加时间步长、重力和求解器参数。 */
struct SimConfig {
  float timeStepSeconds{1.0F / 60.0F};
  int substeps{1};
};

}  // namespace physfx::physics
