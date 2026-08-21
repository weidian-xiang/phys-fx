/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cassert>
#include <string>

#include "physfx/core/Json.h"
#include "physfx/editing/CommandFactory.h"
#include "physfx/editing/EditScript.h"

int main() {
  auto valid = physfx::core::parseJson(R"({"a":[true,null,{"text":"\\u4e2d"}]})");
  assert(valid.ok() && valid.value().find("a") != nullptr);
  assert(!physfx::core::parseJson(R"({"a":1,"a":2})").ok());
  assert(!physfx::core::parseJson("[" + std::string(70, '[') + std::string(70, ']') + "]",
                                  {.maxBytes = 100000, .maxDepth = 32})
              .ok());
  assert(!physfx::core::parseJson(R"({"x":1e999})").ok());
  const auto script = physfx::editing::parseEditScript(
      R"({"version":1,"commands":[{"type":"animate_parameter","parameter":"opacity","interpolation":"smooth","keyframes":[{"frame":0,"value":0},{"frame":10,"value":1}]}]})");
  assert(script.ok() && script.value().size() == 1U);
  auto command = physfx::editing::deserializeCommand(script.value().front());
  assert(command.ok());
  physfx::core::SemanticScene scene{};
  assert(command.value()->execute(scene).ok());
  assert(scene.parameterCurves.size() == 1U && scene.parameterCurves.front().keyframes.size() == 2U);
  assert(command.value()->undo(scene).ok());
  assert(scene.parameterCurves.empty());
  assert(!physfx::editing::parseEditScript(R"({"version":2,"commands":[]})").ok());
  assert(!physfx::editing::parseEditScript(R"({"version":1,"commands":[1]})").ok());
  return 0;
}
