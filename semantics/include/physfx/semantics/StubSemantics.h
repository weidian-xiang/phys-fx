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

#include "physfx/semantics/IEntityAttributeEstimator.h"
#include "physfx/semantics/ISegmenter.h"
#include "physfx/semantics/ITracker.h"

namespace physfx::semantics {

/** @brief 返回单个假实体的分割桩。 */
class StubSegmenter final : public ISegmenter {
 public:
  core::Result<std::vector<core::Entity>> segment(const core::Frame& frame) override;
};

/** @brief 原样返回实体的跟踪桩。 */
class StubTracker final : public ITracker {
 public:
  core::Result<std::vector<core::Entity>> track(const core::Frame& frame,
                                                const std::vector<core::Entity>& entities) override;
};

/** @brief 写入占位属性的实体属性估计桩。 */
class StubEntityAttributeEstimator final : public IEntityAttributeEstimator {
 public:
  core::Status estimate(const core::Frame& frame, core::Entity& entity) override;
};

}  // namespace physfx::semantics
