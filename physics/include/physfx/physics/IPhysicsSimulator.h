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

#include "physfx/core/Frame.h"
#include "physfx/core/SceneContext.h"
#include "physfx/core/Status.h"
#include "physfx/physics/ParticleSet.h"
#include "physfx/physics/SimConfig.h"

namespace physfx::physics {

/**
 * @brief 流体、刚体、布料、粒子模拟器统一抽象。
 * @param frame 当前视频帧。
 * @param scene 场景理解结果。
 * @param config 模拟配置。
 * @return 当前帧模拟结果。
 * @todo Phase2 接入 Taichi，并预留 NVIDIA FleX 适配器。
 */
class IPhysicsSimulator {
 public:
  virtual ~IPhysicsSimulator() = default;
  /**
   * @brief 初始化模拟器。
   * @param config 模拟配置。
   * @return 初始化是否成功。
   * @todo Phase2 实现：初始化 Taichi 或其他真实后端。
   */
  virtual core::Status initialize(const SimConfig& config) = 0;
  /**
   * @brief 推进一帧物理模拟。
   * @param frame 当前视频帧。
   * @param scene 场景理解结果。
   * @return 当前帧模拟结果。
   * @todo Phase2 实现：接入流体、刚体、布料和粒子求解器。
   */
  virtual core::Result<core::SimulationResult> simulate(const core::Frame& frame,
                                                        const core::SceneContext& scene) = 0;
};

/** @brief Phase 1 空模拟器。 @todo Phase2 替换为真实物理后端。 */
class MockSimulator final : public IPhysicsSimulator {
 public:
  core::Status initialize(const SimConfig& config) override;
  core::Result<core::SimulationResult> simulate(
      const core::Frame& frame, const core::SceneContext& scene) override;

 private:
  SimConfig config_{};
};

}  // namespace physfx::physics
