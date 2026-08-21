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

#include <cstdint>
#include <vector>

#include "physfx/semantics/ISegmenter.h"
#include "physfx/semantics/ITracker.h"

namespace physfx::semantics {

/** @brief 以提示点颜色为种子进行四连通区域生长的传统分割器。 */
class PHYSFX_API ColorPromptSegmenter final : public ISegmenter {
 public:
  /** @brief 创建分割器。 @param x 提示点横坐标。 @param y 提示点纵坐标。 @param tolerance RGB
   * 欧氏距离阈值。 */
  ColorPromptSegmenter(std::uint32_t x, std::uint32_t y, float tolerance = 60.0F);
  core::Result<std::vector<core::Entity>> segment(const core::Frame& frame) override;

 private:
  std::uint32_t x_{0};
  std::uint32_t y_{0};
  float tolerance_{60.0F};
};

/** @brief 以实体编号合并掩码时序并计算质心轨迹的轻量跟踪器。 */
class PHYSFX_API CentroidTracker final : public ITracker {
 public:
  core::Result<std::vector<core::Entity>> track(const core::Frame& frame,
                                                const std::vector<core::Entity>& entities) override;

 private:
  std::vector<core::Entity> previous_{};
};

}  // namespace physfx::semantics
