/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "physfx/cloud/IRenderServiceClient.h"
#include "physfx/cloud/ITemplateMarketplaceClient.h"
#include "physfx/core/Config.h"
#include "physfx/pipeline/StageFactory.h"

namespace {

constexpr const char* kVersion = "PhysFX Engine 0.5.0";

void printHelp() {
  std::cout << "PhysFX Engine —— 视频世界编辑器\n"
               "用法:\n"
               "  physfx --version\n"
               "  physfx --help\n"
               "  physfx run --config <路径>\n"
               "  physfx remove <输入> --at x,y -o <输出>\n"
               "  physfx move <输入> --at x,y --to x,y -o <输出>\n"
               "  physfx appearance <输入> --at x,y --material 名称 -o <输出>\n"
               "  physfx smoke <输入> --anchor x,y -o <输出>\n"
               "  physfx splash <输入> --at x,y -o <输出>\n"
               "  physfx explode <输入> --at x,y --scale s -o <输出>\n"
               "  physfx season <输入> --to winter|autumn|spring|summer -o <输出>\n"
               "  physfx cloud render <脚本.json> [--api-key KEY] [-o <输出>]\n"
               "  physfx market search [关键词]\n"
               "  physfx market install <模板ID> [-d <目录>]\n";
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

  if (std::string(argv[1]) == "cloud" && argc >= 4 && std::string(argv[2]) == "render") {
    physfx::cloud::RenderJobRequest request{};
    request.projectPath = argv[3];
    request.apiKey = std::getenv("PHYSFX_API_KEY") == nullptr ? "" : std::getenv("PHYSFX_API_KEY");
    for (int index = 4; index < argc; ++index) {
      const std::string option = argv[index];
      if ((option == "--api-key" || option == "--token") && index + 1 < argc) {
        request.apiKey = argv[++index];
      } else if ((option == "-o" || option == "--output") && index + 1 < argc) {
        request.outputPath = argv[++index];
      } else if (option == "--input" && index + 1 < argc) {
        request.inputPath = argv[++index];
      } else if (option == "--quality" && index + 1 < argc) {
        request.qualityPreset = argv[++index];
      }
    }
    physfx::cloud::LocalRenderServiceClient client;
    const auto submitted = client.submit(request);
    if (!submitted.ok()) {
      std::cerr << "云渲染提交失败: " << submitted.status().message << "\n";
      return 1;
    }
    std::cout << "云渲染任务已提交 job_id=" << submitted.value().jobId
              << " state=" << submitted.value().state << "\n";
    return 0;
  }

  if (std::string(argv[1]) == "market" && argc >= 3) {
    physfx::cloud::LocalTemplateMarketplaceClient client;
    const std::string action = argv[2];
    if (action == "search") {
      const std::string keyword = argc >= 4 ? argv[3] : "";
      const auto result = client.search(keyword);
      if (!result.ok()) {
        std::cerr << "模板市场查询失败: " << result.status().message << "\n";
        return 1;
      }
      for (const auto& item : result.value()) {
        std::cout << item.templateId << "\t" << item.name << "\t" << item.version << "\t"
                  << item.signatureStatus << "\n";
      }
      return 0;
    }
    if (action == "install" && argc >= 4) {
      std::filesystem::path destination = std::filesystem::current_path() / "templates";
      for (int index = 4; index + 1 < argc; ++index) {
        if (std::string(argv[index]) == "-d" || std::string(argv[index]) == "--destination") {
          destination = argv[++index];
        }
      }
      const auto status = client.install(argv[3], destination);
      if (!status.ok()) {
        std::cerr << "模板安装失败: " << status.message << "\n";
        return 1;
      }
      std::cout << "模板已安装: " << argv[3] << " -> " << destination.string() << "\n";
      return 0;
    }
  }

  const std::string command = argv[1];
  if (command == "remove" || command == "move" || command == "appearance" || command == "smoke" ||
      command == "splash" || command == "explode" || command == "season") {
    if (argc < 4) {
      std::cerr << "参数不足，请使用 --help 查看用法\n";
      return 2;
    }
    physfx::core::Config config{};
    config.inputPath = argv[2];
    config.semanticsBackend = "traditional";
    config.physicsBackend = (command == "smoke" || command == "splash") ? "taichi_smoke"
                            : command == "explode"                      ? "simple_particles"
                                                                        : "mock";
    config.particlePreset = "smoke";
    config.editOperation =
        (command == "smoke" || command == "splash" || command == "explode") ? "" : command;
    config.templateName = command;
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
      if (option == "--to") config.seasonTarget = argv[++index];
      if (option == "--scale") config.effectScale = std::stof(argv[++index]);
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
