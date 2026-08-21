/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/core/Json.h"

#include <charconv>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <utility>

namespace physfx::core {
namespace {

class Parser {
 public:
  Parser(std::string_view text, JsonLimits limits) : text_(text), limits_(limits) {}

  Result<JsonValue> parse() {
    if (text_.size() > limits_.maxBytes) return fail("输入超过允许大小");
    skipSpace();
    auto result = parseValue(0U);
    if (!result.ok()) return result.status();
    skipSpace();
    if (cursor_ != text_.size()) return fail("根值后存在多余内容");
    return std::move(result).value();
  }

 private:
  Result<JsonValue> fail(std::string_view reason) const {
    return Status{StatusCode::kInvalidArgument,
                  "JSON 解析失败（字节 " + std::to_string(cursor_) + "）：" +
                      std::string(reason) + "；请检查文件格式后重试"};
  }

  void skipSpace() {
    while (cursor_ < text_.size() &&
           (text_[cursor_] == ' ' || text_[cursor_] == '\n' || text_[cursor_] == '\r' ||
            text_[cursor_] == '\t')) {
      ++cursor_;
    }
  }

  Result<JsonValue> parseValue(std::size_t depth) {
    if (depth > limits_.maxDepth) return fail("嵌套层数过深");
    skipSpace();
    if (cursor_ >= text_.size()) return fail("缺少 JSON 值");
    switch (text_[cursor_]) {
      case 'n':
        return literal("null", JsonValue{});
      case 't':
        return literal("true", JsonValue{true});
      case 'f':
        return literal("false", JsonValue{false});
      case '"': {
        auto parsed = parseString();
        if (!parsed.ok()) return parsed.status();
        return JsonValue{std::move(parsed).value()};
      }
      case '[':
        return parseArray(depth + 1U);
      case '{':
        return parseObject(depth + 1U);
      default:
        if (text_[cursor_] == '-' || (text_[cursor_] >= '0' && text_[cursor_] <= '9')) {
          return parseNumber();
        }
        return fail("无法识别的值");
    }
  }

  Result<JsonValue> literal(std::string_view expected, JsonValue value) {
    if (text_.substr(cursor_, expected.size()) != expected) return fail("无效字面量");
    cursor_ += expected.size();
    return value;
  }

  static void appendUtf8(std::string& output, std::uint32_t codepoint) {
    if (codepoint <= 0x7FU) {
      output.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FFU) {
      output.push_back(static_cast<char>(0xC0U | (codepoint >> 6U)));
      output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    } else if (codepoint <= 0xFFFFU) {
      output.push_back(static_cast<char>(0xE0U | (codepoint >> 12U)));
      output.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
      output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    } else {
      output.push_back(static_cast<char>(0xF0U | (codepoint >> 18U)));
      output.push_back(static_cast<char>(0x80U | ((codepoint >> 12U) & 0x3FU)));
      output.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
      output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
    }
  }

  bool readHex4(std::uint32_t& value) {
    if (cursor_ + 4U > text_.size()) return false;
    value = 0U;
    for (std::size_t index = 0; index < 4U; ++index) {
      const char character = text_[cursor_++];
      value <<= 4U;
      if (character >= '0' && character <= '9')
        value |= static_cast<std::uint32_t>(character - '0');
      else if (character >= 'a' && character <= 'f')
        value |= static_cast<std::uint32_t>(character - 'a' + 10);
      else if (character >= 'A' && character <= 'F')
        value |= static_cast<std::uint32_t>(character - 'A' + 10);
      else
        return false;
    }
    return true;
  }

  Result<std::string> parseString() {
    ++cursor_;
    std::string output;
    while (cursor_ < text_.size()) {
      const unsigned char character = static_cast<unsigned char>(text_[cursor_++]);
      if (character == '"') return output;
      if (character < 0x20U) return fail("字符串包含未转义控制字符").status();
      if (character != '\\') {
        output.push_back(static_cast<char>(character));
      } else {
        if (cursor_ >= text_.size()) return fail("字符串转义不完整").status();
        const char escaped = text_[cursor_++];
        switch (escaped) {
          case '"':
          case '\\':
          case '/':
            output.push_back(escaped);
            break;
          case 'b':
            output.push_back('\b');
            break;
          case 'f':
            output.push_back('\f');
            break;
          case 'n':
            output.push_back('\n');
            break;
          case 'r':
            output.push_back('\r');
            break;
          case 't':
            output.push_back('\t');
            break;
          case 'u': {
            std::uint32_t codepoint = 0U;
            if (!readHex4(codepoint)) return fail("Unicode 转义无效").status();
            if (codepoint >= 0xD800U && codepoint <= 0xDBFFU) {
              if (cursor_ + 2U > text_.size() || text_[cursor_] != '\\' ||
                  text_[cursor_ + 1U] != 'u') {
                return fail("Unicode 高代理项缺少低代理项").status();
              }
              cursor_ += 2U;
              std::uint32_t low = 0U;
              if (!readHex4(low) || low < 0xDC00U || low > 0xDFFFU) {
                return fail("Unicode 低代理项无效").status();
              }
              codepoint = 0x10000U + ((codepoint - 0xD800U) << 10U) + (low - 0xDC00U);
            } else if (codepoint >= 0xDC00U && codepoint <= 0xDFFFU) {
              return fail("Unicode 代理项顺序无效").status();
            }
            appendUtf8(output, codepoint);
            break;
          }
          default:
            return fail("未知字符串转义").status();
        }
      }
      if (output.size() > limits_.maxStringBytes) return fail("字符串超过允许大小").status();
    }
    return fail("字符串缺少结束引号").status();
  }

  Result<JsonValue> parseNumber() {
    const std::size_t start = cursor_;
    if (text_[cursor_] == '-') ++cursor_;
    if (cursor_ >= text_.size()) return fail("数字不完整");
    if (text_[cursor_] == '0') {
      ++cursor_;
      if (cursor_ < text_.size() && text_[cursor_] >= '0' && text_[cursor_] <= '9') {
        return fail("数字包含多余前导零");
      }
    } else {
      if (text_[cursor_] < '1' || text_[cursor_] > '9') return fail("数字整数部分无效");
      while (cursor_ < text_.size() && text_[cursor_] >= '0' && text_[cursor_] <= '9') ++cursor_;
    }
    if (cursor_ < text_.size() && text_[cursor_] == '.') {
      ++cursor_;
      const std::size_t fraction = cursor_;
      while (cursor_ < text_.size() && text_[cursor_] >= '0' && text_[cursor_] <= '9') ++cursor_;
      if (cursor_ == fraction) return fail("小数点后缺少数字");
    }
    if (cursor_ < text_.size() && (text_[cursor_] == 'e' || text_[cursor_] == 'E')) {
      ++cursor_;
      if (cursor_ < text_.size() && (text_[cursor_] == '+' || text_[cursor_] == '-')) ++cursor_;
      const std::size_t exponent = cursor_;
      while (cursor_ < text_.size() && text_[cursor_] >= '0' && text_[cursor_] <= '9') ++cursor_;
      if (cursor_ == exponent) return fail("指数部分缺少数字");
    }
    double value = 0.0;
    const auto converted =
        std::from_chars(text_.data() + start, text_.data() + cursor_, value, std::chars_format::general);
    if (converted.ec != std::errc{} || converted.ptr != text_.data() + cursor_ ||
        !std::isfinite(value)) {
      return fail("数字超出有限范围");
    }
    return JsonValue{value};
  }

  Result<JsonValue> parseArray(std::size_t depth) {
    ++cursor_;
    JsonValue::Array values;
    skipSpace();
    if (cursor_ < text_.size() && text_[cursor_] == ']') {
      ++cursor_;
      return JsonValue{std::move(values)};
    }
    while (true) {
      if (values.size() >= limits_.maxContainerElements) return fail("数组元素过多");
      auto value = parseValue(depth);
      if (!value.ok()) return value.status();
      values.push_back(std::move(value).value());
      skipSpace();
      if (cursor_ >= text_.size()) return fail("数组缺少结束符");
      if (text_[cursor_] == ']') {
        ++cursor_;
        return JsonValue{std::move(values)};
      }
      if (text_[cursor_++] != ',') return fail("数组元素之间缺少逗号");
    }
  }

  Result<JsonValue> parseObject(std::size_t depth) {
    ++cursor_;
    JsonValue::Object values;
    skipSpace();
    if (cursor_ < text_.size() && text_[cursor_] == '}') {
      ++cursor_;
      return JsonValue{std::move(values)};
    }
    while (true) {
      skipSpace();
      if (cursor_ >= text_.size() || text_[cursor_] != '"') return fail("对象键必须是字符串");
      auto key = parseString();
      if (!key.ok()) return key.status();
      skipSpace();
      if (cursor_ >= text_.size() || text_[cursor_++] != ':') return fail("对象键后缺少冒号");
      auto parsed = parseValue(depth);
      if (!parsed.ok()) return parsed.status();
      if (values.size() >= limits_.maxContainerElements) return fail("对象字段过多");
      const auto [unused, inserted] =
          values.emplace(std::move(key).value(), std::move(parsed).value());
      (void)unused;
      if (!inserted) return fail("对象包含重复键");
      skipSpace();
      if (cursor_ >= text_.size()) return fail("对象缺少结束符");
      if (text_[cursor_] == '}') {
        ++cursor_;
        return JsonValue{std::move(values)};
      }
      if (text_[cursor_++] != ',') return fail("对象字段之间缺少逗号");
    }
  }

  std::string_view text_;
  JsonLimits limits_;
  std::size_t cursor_{0U};
};

void appendEscaped(std::ostringstream& stream, std::string_view value) {
  stream << '"';
  for (const unsigned char character : value) {
    switch (character) {
      case '"':
        stream << "\\\"";
        break;
      case '\\':
        stream << "\\\\";
        break;
      case '\b':
        stream << "\\b";
        break;
      case '\f':
        stream << "\\f";
        break;
      case '\n':
        stream << "\\n";
        break;
      case '\r':
        stream << "\\r";
        break;
      case '\t':
        stream << "\\t";
        break;
      default:
        if (character < 0x20U) {
          stream << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                 << static_cast<unsigned int>(character) << std::dec << std::setfill(' ');
        } else {
          stream << static_cast<char>(character);
        }
    }
  }
  stream << '"';
}

void appendJson(std::ostringstream& stream, const JsonValue& value) {
  if (value.isNull()) {
    stream << "null";
  } else if (const auto* boolean = value.boolean()) {
    stream << (*boolean ? "true" : "false");
  } else if (const auto* number = value.number()) {
    stream << std::setprecision(std::numeric_limits<double>::max_digits10) << *number;
  } else if (const auto* string = value.string()) {
    appendEscaped(stream, *string);
  } else if (const auto* array = value.array()) {
    stream << '[';
    for (std::size_t index = 0; index < array->size(); ++index) {
      if (index != 0U) stream << ',';
      appendJson(stream, (*array)[index]);
    }
    stream << ']';
  } else if (const auto* object = value.object()) {
    stream << '{';
    bool first = true;
    for (const auto& [key, child] : *object) {
      if (!first) stream << ',';
      first = false;
      appendEscaped(stream, key);
      stream << ':';
      appendJson(stream, child);
    }
    stream << '}';
  }
}

}  // namespace

bool JsonValue::isNull() const noexcept { return std::holds_alternative<std::nullptr_t>(value_); }
const bool* JsonValue::boolean() const noexcept { return std::get_if<bool>(&value_); }
const double* JsonValue::number() const noexcept { return std::get_if<double>(&value_); }
const std::string* JsonValue::string() const noexcept { return std::get_if<std::string>(&value_); }
const JsonValue::Array* JsonValue::array() const noexcept { return std::get_if<Array>(&value_); }
const JsonValue::Object* JsonValue::object() const noexcept { return std::get_if<Object>(&value_); }
const JsonValue* JsonValue::find(std::string_view key) const noexcept {
  const auto* values = object();
  if (values == nullptr) return nullptr;
  const auto iterator = values->find(key);
  return iterator == values->end() ? nullptr : &iterator->second;
}

Result<JsonValue> parseJson(std::string_view text, JsonLimits limits) {
  return Parser{text, limits}.parse();
}

std::string serializeJson(const JsonValue& value) {
  std::ostringstream stream;
  appendJson(stream, value);
  return stream.str();
}

}  // namespace physfx::core
