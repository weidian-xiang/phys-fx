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

#include <vector>

#include "physfx/physics/IPhysicsSimulator.h"

namespace physfx::physics {

/**
 * @brief Taichi 进程级桥接模拟器。
 *
 * 本阶段不把 Taichi 加入 C++ 构建链；无 Python/Taichi 环境时返回可诊断状态。
 */
class PHYSFX_API TaichiFluidSimulator final : public IPhysicsSimulator {
 public:
  core::Status initialize(const SimConfig& config) override;
  core::Result<core::SimulationResult> simulate(const core::Frame& frame,
                                                const core::SceneContext& scene) override;

 private:
  SimConfig config_{};
  bool initialized_{false};
  std::uint32_t width_{0};
  std::uint32_t height_{0};
  std::vector<float> density_{};
};

}  // namespace physfx::physics
