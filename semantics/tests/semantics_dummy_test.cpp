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

#include "physfx/semantics/StubSemantics.h"

int main() {
  physfx::semantics::StubSegmenter segmenter;
  auto result = segmenter.segment({.index = 7, .width = 4, .height = 3});
  assert(result.ok());
  assert(result.value().size() == 1);
  assert(result.value().front().maskTimeline.front().frameIndex == 7);
  return 0;
}
