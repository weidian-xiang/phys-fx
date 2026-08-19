/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/compositing/PassthroughCompositor.h"

#include <utility>

#include "physfx/compositing/ColorMatcher.h"
#include "physfx/compositing/CompositingRenderPath.h"
#include "physfx/compositing/OcclusionBlender.h"
#include "physfx/compositing/ShadowProjector.h"

namespace physfx::compositing {

core::Frame PassthroughCompositor::compose(const core::Frame& frame, const core::SceneContext&,
                                           const core::SimulationResult&) {
  return frame;
}

core::Frame OcclusionBlender::blend(const core::Frame& frame, const core::OcclusionMask&) const {
  return frame;
}

bool ShadowProjector::canProject(const core::SceneContext&) const noexcept { return false; }

core::Frame ColorMatcher::match(const core::Frame& frame, const core::SceneContext&) const {
  return frame;
}

CompositingRenderPath::CompositingRenderPath(std::unique_ptr<ICompositor> compositor)
    : compositor_(std::move(compositor)) {}

std::string_view CompositingRenderPath::name() const noexcept { return "compositing_passthrough"; }

core::Result<core::Frame> CompositingRenderPath::render(const core::Frame& frame,
                                                        const core::SceneContext& scene,
                                                        const core::SimulationResult& simulation) {
  if (!compositor_) {
    return core::Status{core::StatusCode::kInternalError, "合成器未装配"};
  }
  return compositor_->compose(frame, scene, simulation);
}

}  // namespace physfx::compositing
