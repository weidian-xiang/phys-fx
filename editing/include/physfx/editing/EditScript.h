/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "physfx/core/Status.h"

namespace physfx::editing {

/** @brief 校验完整的 version=1 编辑脚本并返回规范化命令 JSON。 */
[[nodiscard]] core::Result<std::vector<std::string>> parseEditScript(std::string_view json);
/** @brief 从磁盘读取并校验编辑脚本，限制最大文件大小。 */
[[nodiscard]] core::Result<std::vector<std::string>> loadEditScript(
    const std::filesystem::path& path);

}  // namespace physfx::editing
