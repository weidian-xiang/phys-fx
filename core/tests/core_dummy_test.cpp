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

#include "physfx/core/SceneGraph.h"
#include "physfx/core/Config.h"

int main() {
  physfx::core::SceneGraph graph;
  graph.addNode({1, "dummy"});
  assert(graph.nodes().size() == 1);

  const auto configPath = std::filesystem::temp_directory_path() / "physfx_phase2_config.json";
  {
    std::ofstream stream(configPath);
    stream << R"({"frame_count": 3, "stages": {"semantics": {"enabled": false}, "render": {"path": "neural"}}})";
  }
  const auto config = physfx::core::Config::fromFile(configPath);
  assert(config.frameCount == 3);
  assert(!config.semanticsEnabled);
  assert(config.renderPath == physfx::core::RenderPathKind::kNeural);
  std::filesystem::remove(configPath);
  return 0;
}
