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

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>

#include "physfx/core/IRenderPath.h"

namespace physfx::core {

/**
 * @brief 空管线运行配置。
 * @todo Phase2 从 JSON/YAML 解析完整的模块工厂配置。
 */
struct Config {
  std::string inputPath{};
  std::string outputPath{};
  std::size_t frameCount{1};
  bool verbose{false};
  bool perceptionEnabled{true};
  bool semanticsEnabled{true};
  bool editingEnabled{true};
  bool physicsEnabled{true};
  bool renderEnabled{true};
  RenderPathKind renderPath{RenderPathKind::kCompositing};
  std::string semanticsBackend{"stub"};
  std::string segmenterModelPath{};
  std::string trackerModelPath{};
  std::string physicsBackend{"mock"};
  std::string compositorBackend{"passthrough"};
  std::string particlePreset{"sparks"};
  std::uint64_t boundEntityId{0};

  /** @brief 从文件读取配置骨架。 @param path 配置文件路径。 @return 配置对象。 */
  [[nodiscard]] static Config fromFile(const std::filesystem::path& path);
};

}  // namespace physfx::core
