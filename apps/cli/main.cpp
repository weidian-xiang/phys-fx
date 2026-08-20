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
#include <cstdlib>

#include "physfx/core/Config.h"
#include "physfx/pipeline/StageFactory.h"

namespace {

constexpr const char* kVersion = "PhysFX Engine 0.3.0-dev";

void printHelp() {
  std::cout << "PhysFX Engine —— 视频世界编辑器\n"
               "用法:\n"
               "  physfx --version\n"
               "  physfx --help\n"
               "  physfx run --config <路径>\n"
               "  physfx remove <输入> --at x,y -o <输出>\n"
               "  physfx move <输入> --at x,y --to x,y -o <输出>\n"
               "  physfx appearance <输入> --at x,y --material 名称 -o <输出>\n"
               "  physfx smoke <输入> --anchor x,y -o <输出>\n";
}

bool readPoint(const std::string& text, physfx::core::Vec3& point) {
  const auto comma = text.find(',');
  if (comma == std::string::npos) return false;
  try {
    point.x = std::stof(text.substr(0, comma));
    point.y = std::stof(text.substr(comma + 1));
  } catch (...) {
    return false;
  }
  return true;
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

  const std::string command = argv[1];
  if (command == "remove" || command == "move" || command == "appearance" || command == "smoke") {
    if (argc < 4) {
      std::cerr << "参数不足，请使用 --help 查看用法\n";
      return 2;
    }
    physfx::core::Config config{};
    config.inputPath = argv[2];
    config.semanticsBackend = "traditional";
    config.physicsBackend = command == "smoke" ? "taichi_smoke" : "mock";
    config.particlePreset = "smoke";
    config.editOperation = command == "smoke" ? "" : command;
    config.boundEntityId = 1;
    config.editTarget = {320.0F, 180.0F, 0.0F};
    for (int index = 3; index + 1 < argc; ++index) {
      const std::string option = argv[index];
      if (option == "-o" || option == "--output") config.outputPath = argv[++index];
      if (option == "--at" || option == "--anchor") {
        if (!readPoint(argv[++index], config.editTarget)) return 2;
        config.promptX = static_cast<std::uint32_t>(config.editTarget.x);
        config.promptY = static_cast<std::uint32_t>(config.editTarget.y);
      }
      if (option == "--to" && index + 1 < argc) {
        if (!readPoint(argv[++index], config.editTarget)) return 2;
      }
      if (option == "--material") config.appearanceName = argv[++index];
    }
    if (config.outputPath.empty()) {
      std::cerr << "缺少输出路径 -o\n";
      return 2;
    }
    auto pipeline = physfx::pipeline::StageFactory::createPipeline(config);
    return pipeline->run(config) ? 0 : 1;
  }

  std::cerr << "未知命令: " << argv[1] << "\n";
  printHelp();
  return 2;
}
