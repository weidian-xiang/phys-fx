/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/semantics/StubSemantics.h"

namespace physfx::semantics {

core::Result<std::vector<core::Entity>> StubSegmenter::segment(const core::Frame& frame) {
  core::Entity entity{};
  entity.id = 1;
  entity.category = "stub_entity";
  entity.maskTimeline.push_back({frame.index, frame.width, frame.height, {}});
  return std::vector<core::Entity>{std::move(entity)};
}

core::Result<std::vector<core::Entity>> StubTracker::track(
    const core::Frame& frame, const std::vector<core::Entity>& entities) {
  auto tracked = entities;
  for (auto& entity : tracked) {
    entity.trajectory.push_back({frame.index, {}});
  }
  return tracked;
}

core::Status StubEntityAttributeEstimator::estimate(const core::Frame&, core::Entity& entity) {
  entity.attributes["source"] = "stub";
  return core::Status::success();
}

}  // namespace physfx::semantics
