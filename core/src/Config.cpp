/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/core/Config.h"

#include <fstream>
#include <string_view>

namespace physfx::core {

namespace {

bool readEnabled(const std::string& text, const std::string& stage, bool fallback) {
  const auto stagePosition = text.find('"' + stage + '"');
  if (stagePosition == std::string::npos) {
    return fallback;
  }
  const auto enabledPosition = text.find("\"enabled\"", stagePosition);
  if (enabledPosition == std::string::npos) {
    return fallback;
  }
  const auto falsePosition = text.find("false", enabledPosition);
  const auto blockEnd = text.find('}', enabledPosition);
  return falsePosition == std::string::npos ||
         (blockEnd != std::string::npos && falsePosition > blockEnd);
}

std::size_t readSize(const std::string& text, std::string_view key, std::size_t fallback) {
  const auto keyPosition = text.find('"' + std::string(key) + '"');
  if (keyPosition == std::string::npos) return fallback;
  const auto colon = text.find(':', keyPosition);
  if (colon == std::string::npos) return fallback;
  const auto firstDigit = text.find_first_of("0123456789", colon);
  if (firstDigit == std::string::npos) return fallback;
  try {
    return static_cast<std::size_t>(std::stoull(text.substr(firstDigit)));
  } catch (...) {
    return fallback;
  }
}

std::string readString(const std::string& text, std::string_view key) {
  const auto keyPosition = text.find('"' + std::string(key) + '"');
  if (keyPosition == std::string::npos) return {};
  const auto firstQuote = text.find('"', text.find(':', keyPosition) + 1);
  if (firstQuote == std::string::npos) return {};
  const auto secondQuote = text.find('"', firstQuote + 1);
  return secondQuote == std::string::npos
             ? std::string{}
             : text.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

std::string readStringInBlock(const std::string& text, std::string_view block, std::string_view key,
                              std::string fallback) {
  const auto blockPosition = text.find('"' + std::string(block) + '"');
  if (blockPosition == std::string::npos) return fallback;
  const auto blockEnd = text.find('}', blockPosition);
  const auto keyPosition = text.find('"' + std::string(key) + '"', blockPosition);
  if (keyPosition == std::string::npos ||
      (blockEnd != std::string::npos && keyPosition > blockEnd)) {
    return fallback;
  }
  const auto firstQuote = text.find('"', text.find(':', keyPosition) + 1);
  const auto secondQuote =
      firstQuote == std::string::npos ? std::string::npos : text.find('"', firstQuote + 1);
  return secondQuote == std::string::npos
             ? fallback
             : text.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

std::size_t readSizeInBlock(const std::string& text, std::string_view block, std::string_view key,
                            std::size_t fallback) {
  const auto blockPosition = text.find('"' + std::string(block) + '"');
  if (blockPosition == std::string::npos) return fallback;
  const auto blockEnd = text.find('}', blockPosition);
  const auto keyPosition = text.find('"' + std::string(key) + '"', blockPosition);
  if (keyPosition == std::string::npos ||
      (blockEnd != std::string::npos && keyPosition > blockEnd)) {
    return fallback;
  }
  return readSize(text.substr(keyPosition, blockEnd - keyPosition), key, fallback);
}

std::uint32_t readPointCoordinate(const std::string& text, std::size_t coordinate,
                                  std::uint32_t fallback) {
  const auto point = text.find("\"point\"");
  const auto bracket = point == std::string::npos ? std::string::npos : text.find('[', point);
  if (bracket == std::string::npos) return fallback;
  auto cursor = bracket + 1U;
  for (std::size_t index = 0; index <= coordinate; ++index) {
    cursor = text.find_first_of("0123456789", cursor);
    if (cursor == std::string::npos) return fallback;
    if (index == coordinate) {
      try {
        return static_cast<std::uint32_t>(std::stoul(text.substr(cursor)));
      } catch (...) {
        return fallback;
      }
    }
    cursor = text.find(',', cursor);
    if (cursor == std::string::npos) return fallback;
    ++cursor;
  }
  return fallback;
}

}  // namespace

Config Config::fromFile(const std::filesystem::path& path) {
  Config config{};
  const auto configPath = std::filesystem::absolute(path).lexically_normal();
  std::ifstream stream(configPath);
  if (!stream) {
    return config;
  }

  const std::string text((std::istreambuf_iterator<char>(stream)),
                         std::istreambuf_iterator<char>());
  config.frameCount = readSize(text, "frame_count", config.frameCount);
  config.inputPath = readString(text, "input");
  config.outputPath = readString(text, "output");
  config.perceptionEnabled = readEnabled(text, "perception", true);
  config.semanticsEnabled = readEnabled(text, "semantics", true);
  config.editingEnabled = readEnabled(text, "editing", true);
  config.physicsEnabled = readEnabled(text, "physics", true);
  config.renderEnabled = readEnabled(text, "render", true);
  if (readStringInBlock(text, "render", "path", "compositing") == "neural") {
    config.renderPath = RenderPathKind::kNeural;
  }
  config.semanticsBackend =
      readStringInBlock(text, "semantics", "implementation", config.semanticsBackend);
  config.segmenterModelPath =
      readStringInBlock(text, "semantics", "segmenter_model", config.segmenterModelPath);
  config.trackerModelPath =
      readStringInBlock(text, "semantics", "tracker_model", config.trackerModelPath);
  config.promptX = readPointCoordinate(text, 0, config.promptX);
  config.promptY = readPointCoordinate(text, 1, config.promptY);
  config.physicsBackend =
      readStringInBlock(text, "physics", "implementation", config.physicsBackend);
  config.compositorBackend =
      readStringInBlock(text, "render", "implementation", config.compositorBackend);
  config.particlePreset = readStringInBlock(text, "physics", "preset", config.particlePreset);
  config.boundEntityId = static_cast<std::uint64_t>(
      readSizeInBlock(text, "physics", "bound_entity_id", config.boundEntityId));
  config.editScriptPath = readString(text, "edit_script");
  if (!config.editScriptPath.empty()) {
    const std::filesystem::path scriptPath(config.editScriptPath);
    if (scriptPath.is_relative()) {
      // 编辑脚本的相对路径始终相对于配置文件，避免 CTest/CLI 工作目录差异。
      config.editScriptPath = (configPath.parent_path() / scriptPath).lexically_normal().string();
    } else {
      config.editScriptPath = scriptPath.lexically_normal().string();
    }
  }
  config.editOperation = readString(text, "operation");
  config.appearanceName = readString(text, "material");
  config.templateName = readString(text, "template");
  config.seasonTarget = readString(text, "season");
  config.editTarget.x = static_cast<float>(readSizeInBlock(text, "editing", "target_x", 0));
  config.editTarget.y = static_cast<float>(readSizeInBlock(text, "editing", "target_y", 0));
  return config;
}

}  // namespace physfx::core
