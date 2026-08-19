/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

bool containsForbiddenInclude(const std::filesystem::path& file,
                              const std::string& ownModule) {
  std::ifstream stream(file);
  const std::string text((std::istreambuf_iterator<char>(stream)),
                         std::istreambuf_iterator<char>());
  const std::vector<std::string> peerModules = {
      "perception", "semantics", "editing", "physics", "compositing", "neural_render"};
  for (const auto& module : peerModules) {
    if (module != ownModule &&
        text.find("physfx/" + module + "/") != std::string::npos) {
      return true;
    }
  }
  return false;
}

}  // namespace

int main() {
  const std::filesystem::path sourceRoot{PHYSFX_SOURCE_DIR};
  const std::vector<std::string> modules = {
      "perception", "semantics", "editing", "physics", "compositing", "neural_render"};
  for (const auto& module : modules) {
    const auto includeRoot = sourceRoot / module / "include";
    for (const auto& entry : std::filesystem::recursive_directory_iterator(includeRoot)) {
      if (entry.is_regular_file()) {
        assert(!containsForbiddenInclude(entry.path(), module));
      }
    }
  }
  return 0;
}
