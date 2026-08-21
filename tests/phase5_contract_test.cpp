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

#include "EditorModel.h"
#include "Project.h"
#include "physfx/editing/CommandFactory.h"
#include "physfx/physics/TaichiFluidSimulator.h"
#include "physfx/plugins/ILicenseVerifier.h"

int main() {
  physfx::gui::Project project{};
  project.nodes.push_back({"edit", "editing", "编辑", {R"({"type":"empty"})"}});
  const auto script = physfx::gui::ProjectCompiler::compileEditScript(project);
  assert(script == R"({"version":1,"commands":[{"type":"empty"}]})");
  physfx::gui::Timeline timeline;
  timeline.setTotalFrames(10);
  timeline.scrub(99);
  assert(timeline.currentFrame() == 9);
  physfx::gui::PreviewWindow preview;
  preview.setFrameSize(320, 180);
  preview.click(999, 999);
  assert(preview.hasSelection() && preview.selectedX() == 319 && preview.selectedY() == 179);

  auto command = physfx::editing::deserializeCommand(R"({"type":"set_season","season":"winter"})");
  assert(command.ok());
  physfx::core::SemanticScene scene{};
  assert(command.value()->execute(scene).ok());
  assert(scene.season == physfx::core::Season::kWinter);

  physfx::physics::TaichiFluidSimulator simulator;
  assert(simulator.initialize({}).ok());
  const auto result = simulator.simulate({.width = 320, .height = 240}, {});
  assert(result.ok() && !result.value().density.empty());

  physfx::plugins::TemplatePackageVerifier verifier;
  const auto status = verifier.verify({"demo", "0.4.0", "test", "", ""}, {});
  assert(status.ok());
  return 0;
}
