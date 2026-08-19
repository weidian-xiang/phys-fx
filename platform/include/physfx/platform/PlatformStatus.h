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

#include "physfx/core/Status.h"

namespace physfx::platform {

/** @brief 平台层统一状态码。 @todo Phase2 映射 FFmpeg/GPU 详细错误。 */
using PlatformStatus = core::Status;

/** @brief 将状态码转换为中文描述。 @param status 状态码。 @return 描述文本。 */
[[nodiscard]] inline std::string_view toString(const PlatformStatus& status) noexcept {
  switch (status.code) {
    case core::StatusCode::kOk:
      return "成功";
    case core::StatusCode::kNotImplemented:
      return "未实现";
    case core::StatusCode::kInvalidArgument:
      return "参数无效";
    case core::StatusCode::kIoError:
      return "输入输出错误";
    case core::StatusCode::kNotFound:
      return "未找到";
    case core::StatusCode::kDisabled:
      return "已禁用";
    case core::StatusCode::kInternalError:
      return "内部错误";
  }
  return "未知错误";
}

}  // namespace physfx::platform
