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

namespace physfx::core {

/** @brief 日志级别。 @todo Phase2 支持结构化日志与可插拔 sink。 */
enum class LogLevel { kInfo, kWarning, kError };

/**
 * @brief 轻量日志门面，Phase 1 输出到标准输出。
 * @todo Phase2 增加时间戳、线程信息和文件输出。
 */
class Logger {
 public:
  /** @brief 写入日志。 @param level 日志级别。 @param message 日志文本。 */
  static void log(LogLevel level, std::string_view message);
  /** @brief 写入信息日志。 @param message 日志文本。 */
  static void info(std::string_view message);
  /** @brief 写入警告日志。 @param message 日志文本。 */
  static void warning(std::string_view message);
  /** @brief 写入错误日志。 @param message 日志文本。 */
  static void error(std::string_view message);
};

}  // namespace physfx::core
