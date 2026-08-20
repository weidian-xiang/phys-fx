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

#include "physfx/compositing/ICompositor.h"

namespace physfx::compositing {

/** @brief CPU 精灵合成器，支持 alpha/加色混合和语义实体掩码遮挡。 */
class SpriteCompositor final : public ICompositor {
 public:
  /** @brief 创建合成器。 @param gain 全局粒子亮度增益。 @param warmth 色温偏移。 */
  explicit SpriteCompositor(float gain = 1.0F, float warmth = 0.0F);

  core::Frame compose(const core::Frame& frame, const core::SceneContext& scene,
                      const core::SimulationResult& simulation) override;

 private:
  [[nodiscard]] bool occluded(const core::SceneContext& scene, std::uint64_t frameIndex,
                              std::uint32_t x, std::uint32_t y) const;

  float gain_{1.0F};
  float warmth_{0.0F};
};

}  // namespace physfx::compositing
