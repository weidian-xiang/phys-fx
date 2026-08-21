/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/editing/EditScript.h"

#include <cmath>
#include <fstream>
#include <iterator>
#include <utility>

#include "physfx/core/Json.h"

namespace physfx::editing {

core::Result<std::vector<std::string>> parseEditScript(std::string_view json) {
  auto parsed = core::parseJson(json);
  if (!parsed.ok()) return parsed.status();
  const auto root = std::move(parsed).value();
  if (root.object() == nullptr) {
    return core::Status{core::StatusCode::kInvalidArgument,
                        "编辑脚本根节点必须是对象；请参考 docs/edit-script-format.md"};
  }
  const auto* version = root.find("version");
  if (version == nullptr || version->number() == nullptr ||
      std::floor(*version->number()) != *version->number() || *version->number() != 1.0) {
    return core::Status{core::StatusCode::kInvalidArgument,
                        "编辑脚本 version 必须是整数 1；请升级或修正脚本"};
  }
  const auto* commands = root.find("commands");
  if (commands == nullptr || commands->array() == nullptr) {
    return core::Status{core::StatusCode::kInvalidArgument,
                        "编辑脚本缺少 commands 数组；请至少提供一个命令"};
  }
  if (commands->array()->size() > 10000U) {
    return core::Status{core::StatusCode::kInvalidArgument,
                        "编辑脚本命令超过 10000 条；请拆分工程后重试"};
  }
  std::vector<std::string> serialized;
  serialized.reserve(commands->array()->size());
  for (std::size_t index = 0; index < commands->array()->size(); ++index) {
    const auto& command = (*commands->array())[index];
    const auto* type = command.find("type");
    if (command.object() == nullptr || type == nullptr || type->string() == nullptr ||
        type->string()->empty()) {
      return core::Status{
          core::StatusCode::kInvalidArgument,
          "编辑脚本第 " + std::to_string(index + 1U) + " 条命令缺少字符串 type；请修正后重试"};
    }
    serialized.push_back(core::serializeJson(command));
  }
  return serialized;
}

core::Result<std::vector<std::string>> loadEditScript(const std::filesystem::path& path) {
  std::error_code error;
  const auto bytes = std::filesystem::file_size(path, error);
  if (error) {
    return core::Status{core::StatusCode::kIoError,
                        "无法读取编辑脚本 “" + path.string() + "”；请检查路径与权限"};
  }
  if (bytes > 1024U * 1024U) {
    return core::Status{core::StatusCode::kInvalidArgument, "编辑脚本超过 1 MiB；请拆分工程后重试"};
  }
  std::ifstream stream(path, std::ios::binary);
  if (!stream) {
    return core::Status{core::StatusCode::kIoError,
                        "无法打开编辑脚本 “" + path.string() + "”；请检查文件权限"};
  }
  const std::string text((std::istreambuf_iterator<char>(stream)),
                         std::istreambuf_iterator<char>());
  return parseEditScript(text);
}

}  // namespace physfx::editing
