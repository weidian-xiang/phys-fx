/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/core/Log.h"

#include <iostream>

namespace physfx::core {

void Logger::log(LogLevel level, std::string_view message) {
  const char* prefix = "INFO";
  if (level == LogLevel::kWarning) {
    prefix = "WARN";
  } else if (level == LogLevel::kError) {
    prefix = "ERROR";
  }
  std::cout << "[" << prefix << "] " << message << '\n';
}

void Logger::info(std::string_view message) { log(LogLevel::kInfo, message); }
void Logger::warning(std::string_view message) { log(LogLevel::kWarning, message); }
void Logger::error(std::string_view message) { log(LogLevel::kError, message); }

}  // namespace physfx::core
