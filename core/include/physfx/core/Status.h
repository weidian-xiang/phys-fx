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

#include <optional>
#include <string>
#include <utility>

#include "physfx/core/Export.h"

namespace physfx::core {

/** @brief 全引擎统一状态码。 */
enum class StatusCode {
  kOk,
  kNotImplemented,
  kInvalidArgument,
  kIoError,
  kNotFound,
  kDisabled,
  kInternalError
};

/** @brief 不携带返回值的统一操作状态。 */
struct PHYSFX_API Status {
  StatusCode code{StatusCode::kOk};
  std::string message{};

  /** @brief 判断操作是否成功。 @return 成功时为 true。 */
  [[nodiscard]] bool ok() const noexcept { return code == StatusCode::kOk; }
  /** @brief 创建成功状态。 @return 成功状态。 */
  [[nodiscard]] static Status success() { return {}; }
  /** @brief 创建未实现状态。 @param message 说明。 @return 未实现状态。 */
  [[nodiscard]] static Status notImplemented(std::string message) {
    return {StatusCode::kNotImplemented, std::move(message)};
  }
};

/** @brief 携带返回值或错误状态的统一结果类型。 @tparam T 成功值类型。 */
template <typename T>
class Result {
 public:
  /** @brief 从成功值构造结果。 @param value 成功值。 */
  Result(T value) : value_(std::move(value)), status_(Status::success()) {}
  /** @brief 从错误状态构造结果。 @param status 错误状态。 */
  Result(Status status) : status_(std::move(status)) {}

  /** @brief 判断结果是否成功。 @return 成功时为 true。 */
  [[nodiscard]] bool ok() const noexcept { return status_.ok() && value_.has_value(); }
  /** @brief 获取状态。 @return 状态引用。 */
  [[nodiscard]] const Status& status() const noexcept { return status_; }
  /** @brief 获取成功值。 @return 成功值引用。 */
  [[nodiscard]] const T& value() const& { return value_.value(); }
  /** @brief 移出成功值。 @return 成功值。 */
  [[nodiscard]] T&& value() && { return std::move(value_.value()); }

 private:
  std::optional<T> value_{};
  Status status_{};
};

}  // namespace physfx::core
