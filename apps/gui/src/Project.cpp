/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Project.h"

#include <fstream>
#include <sstream>

namespace physfx::gui {
namespace {

std::string escape(std::string value) {
  std::string result;
  for (const char character : value) {
    if (character == '\\' || character == '"') result.push_back('\\');
    result.push_back(character);
  }
  return result;
}

std::string value(const std::string& text, const std::string& key, std::size_t start = 0) {
  const auto keyPosition = text.find('"' + key + '"', start);
  if (keyPosition == std::string::npos) return {};
  const auto colon = text.find(':', keyPosition);
  const auto first = text.find('"', colon + 1U);
  const auto second = first == std::string::npos ? std::string::npos : text.find('"', first + 1U);
  return first == std::string::npos || second == std::string::npos
             ? std::string{}
             : text.substr(first + 1U, second - first - 1U);
}

}  // namespace

std::string ProjectCompiler::compileEditScript(const Project& project) {
  std::ostringstream stream;
  stream << "{\"version\":1,\"commands\":[";
  bool first = true;
  for (const auto& node : project.nodes) {
    for (const auto& command : node.commandJson) {
      if (!first) stream << ',';
      first = false;
      stream << command;
    }
  }
  stream << "]}";
  return stream.str();
}

std::string ProjectCompiler::serialize(const Project& project) {
  std::ostringstream stream;
  stream << "{\"version\":1,\"input\":\"" << escape(project.input) << "\",\"output\":\""
         << escape(project.output) << "\",\"nodes\":[";
  for (std::size_t index = 0; index < project.nodes.size(); ++index) {
    if (index != 0U) stream << ',';
    const auto& node = project.nodes[index];
    stream << "{\"id\":\"" << escape(node.id) << "\",\"type\":\"" << escape(node.type)
           << "\",\"label\":\"" << escape(node.label) << "\",\"commands\":[";
    for (std::size_t commandIndex = 0; commandIndex < node.commandJson.size(); ++commandIndex) {
      if (commandIndex != 0U) stream << ',';
      stream << node.commandJson[commandIndex];
    }
    stream << "]}";
  }
  stream << "]}";
  return stream.str();
}

bool ProjectCompiler::save(const Project& project, const std::string& path) {
  std::ofstream stream(path, std::ios::binary);
  if (!stream) return false;
  stream << serialize(project);
  return static_cast<bool>(stream);
}

bool ProjectCompiler::load(const std::string& path, Project& project) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) return false;
  const std::string text((std::istreambuf_iterator<char>(stream)),
                         std::istreambuf_iterator<char>());
  project.input = value(text, "input");
  project.output = value(text, "output");
  project.nodes.clear();
  const auto nodesKey = text.find("\"nodes\"");
  const auto arrayStart =
      nodesKey == std::string::npos ? std::string::npos : text.find('[', nodesKey);
  if (arrayStart == std::string::npos) return true;
  std::size_t cursor = arrayStart + 1U;
  while (cursor < text.size()) {
    const auto nodeStart = text.find('{', cursor);
    if (nodeStart == std::string::npos) break;
    std::size_t depth = 0;
    std::size_t nodeEnd = nodeStart;
    for (; nodeEnd < text.size(); ++nodeEnd) {
      if (text[nodeEnd] == '{') ++depth;
      if (text[nodeEnd] == '}' && --depth == 0U) break;
    }
    if (nodeEnd >= text.size()) break;
    const auto nodeText = text.substr(nodeStart, nodeEnd - nodeStart + 1U);
    Node node{};
    node.id = value(nodeText, "id");
    node.type = value(nodeText, "type");
    node.label = value(nodeText, "label");
    const auto commandsKey = nodeText.find("\"commands\"");
    const auto commandStart =
        commandsKey == std::string::npos ? std::string::npos : nodeText.find('[', commandsKey);
    if (commandStart != std::string::npos) {
      std::size_t commandCursor = commandStart + 1U;
      while (commandCursor < nodeText.size() && nodeText[commandCursor] != ']') {
        const auto objectStart = nodeText.find('{', commandCursor);
        if (objectStart == std::string::npos) break;
        std::size_t objectDepth = 0;
        std::size_t objectEnd = objectStart;
        for (; objectEnd < nodeText.size(); ++objectEnd) {
          if (nodeText[objectEnd] == '{') ++objectDepth;
          if (nodeText[objectEnd] == '}' && --objectDepth == 0U) break;
        }
        if (objectEnd >= nodeText.size()) break;
        node.commandJson.push_back(nodeText.substr(objectStart, objectEnd - objectStart + 1U));
        commandCursor = objectEnd + 1U;
      }
    }
    if (!node.id.empty()) project.nodes.push_back(std::move(node));
    cursor = nodeEnd + 1U;
  }
  return true;
}

}  // namespace physfx::gui
