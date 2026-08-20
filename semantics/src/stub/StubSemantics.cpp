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

#include <cstddef>
#include <cstdint>
#include <utility>

namespace physfx::semantics {

core::Result<std::vector<core::Entity>> StubSegmenter::segment(const core::Frame& frame) {
  core::Entity entity{};
  entity.id = 1;
  entity.category = "stub_entity";
  core::EntityMask mask{frame.index, frame.width, frame.height, {}};
  mask.values.assign(static_cast<std::size_t>(frame.width) * frame.height, 0);
  for (std::uint32_t y = frame.height / 3; y < frame.height * 2 / 3; ++y) {
    for (std::uint32_t x = frame.width / 3; x < frame.width * 2 / 3; ++x) {
      mask.values[static_cast<std::size_t>(y) * frame.width + x] = 255;
    }
  }
  entity.maskTimeline.push_back(std::move(mask));
  return std::vector<core::Entity>{std::move(entity)};
}

core::Result<std::vector<core::Entity>> StubTracker::track(
    const core::Frame& frame, const std::vector<core::Entity>& entities) {
  auto tracked = entities;
  for (auto& entity : tracked) {
    core::Vec3 center{};
    std::size_t count = 0;
    if (!entity.maskTimeline.empty()) {
      const auto& mask = entity.maskTimeline.back();
      for (std::uint32_t y = 0; y < mask.height; ++y) {
        for (std::uint32_t x = 0; x < mask.width; ++x) {
          const std::size_t offset = static_cast<std::size_t>(y) * mask.width + x;
          if (offset < mask.values.size() && mask.values[offset] != 0) {
            center.x += static_cast<float>(x);
            center.y += static_cast<float>(y);
            ++count;
          }
        }
      }
    }
    if (count > 0) {
      center.x /= static_cast<float>(count);
      center.y /= static_cast<float>(count);
    }
    entity.trajectory.push_back({frame.index, center});
  }
  return tracked;
}

core::Status StubEntityAttributeEstimator::estimate(const core::Frame&, core::Entity& entity) {
  entity.attributes["source"] = "stub";
  return core::Status::success();
}

}  // namespace physfx::semantics
