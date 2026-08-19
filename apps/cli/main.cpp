/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <string>

#include "physfx/core/Config.h"
#include "physfx/pipeline/StageFactory.h"

namespace {

constexpr const char* kVersion = "PhysFX Engine 0.2.0-dev";

void printHelp() {
  std::cout << "PhysFX Engine —— 视频世界编辑器\n"
               "用法:\n"
               "  physfx --version\n"
               "  physfx --help\n"
               "  physfx run --config <路径>\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc <= 1 || std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
    printHelp();
    return 0;
  }

  if (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-V") {
    std::cout << kVersion << '\n';
    return 0;
  }

  if (std::string(argv[1]) == "run") {
    physfx::core::Config config{};
    for (int index = 2; index + 1 < argc; ++index) {
      if (std::string(argv[index]) == "--config") {
        config = physfx::core::Config::fromFile(argv[index + 1]);
        break;
      }
    }
    auto pipeline = physfx::pipeline::StageFactory::createPipeline(config);
    return pipeline->run(config) ? 0 : 1;
  }

  std::cerr << "未知命令: " << argv[1] << "\n";
  printHelp();
  return 2;
}
