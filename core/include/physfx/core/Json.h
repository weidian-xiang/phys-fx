/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "physfx/core/Status.h"

namespace physfx::core {

/** @brief 有资源上限的 JSON 值，用于解析不可信的工程、脚本和模板清单。 */
class PHYSFX_API JsonValue {
 public:
  using Array = std::vector<JsonValue>;
  using Object = std::map<std::string, JsonValue, std::less<>>;

  JsonValue() = default;
  explicit JsonValue(bool value) : value_(value) {}
  explicit JsonValue(double value) : value_(value) {}
  explicit JsonValue(std::string value) : value_(std::move(value)) {}
  explicit JsonValue(Array value) : value_(std::move(value)) {}
  explicit JsonValue(Object value) : value_(std::move(value)) {}

  [[nodiscard]] bool isNull() const noexcept;
  [[nodiscard]] const bool* boolean() const noexcept;
  [[nodiscard]] const double* number() const noexcept;
  [[nodiscard]] const std::string* string() const noexcept;
  [[nodiscard]] const Array* array() const noexcept;
  [[nodiscard]] const Object* object() const noexcept;
  [[nodiscard]] const JsonValue* find(std::string_view key) const noexcept;

 private:
  std::variant<std::nullptr_t, bool, double, std::string, Array, Object> value_{nullptr};

  friend std::string serializeJson(const JsonValue& value);
};

struct JsonLimits {
  std::size_t maxBytes{1024U * 1024U};
  std::size_t maxDepth{64U};
  std::size_t maxContainerElements{100000U};
  std::size_t maxStringBytes{256U * 1024U};
};

/** @brief 严格解析 UTF-8 JSON；拒绝重复键、非有限数字和尾随内容。 */
[[nodiscard]] PHYSFX_API Result<JsonValue> parseJson(std::string_view text, JsonLimits limits = {});
/** @brief 生成稳定、紧凑的 JSON，供跨模块传递已校验值。 */
[[nodiscard]] PHYSFX_API std::string serializeJson(const JsonValue& value);

}  // namespace physfx::core
