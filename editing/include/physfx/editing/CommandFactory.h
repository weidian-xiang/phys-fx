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
#include <string_view>

#include "physfx/editing/IEditCommand.h"

namespace physfx::editing {

/** @brief 从编辑脚本中的单条 JSON 对象构造命令。 */
[[nodiscard]] core::Result<std::unique_ptr<IEditCommand>> deserializeCommand(
    std::string_view json);

}  // namespace physfx::editing
