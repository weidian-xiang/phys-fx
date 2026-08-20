/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/physics/TaichiFluidSimulator.h"

namespace physfx::physics {

core::Status TaichiFluidSimulator::initialize(const SimConfig& config) {
  if (config.timeStepSeconds <= 0.0F || config.substeps <= 0) {
    return {core::StatusCode::kInvalidArgument, "Taichi 烟雾时间步或子步数无效"};
  }
  config_ = config;
  initialized_ = true;
  return core::Status::success();
}

core::Result<core::SimulationResult> TaichiFluidSimulator::simulate(const core::Frame&,
                                                                     const core::SceneContext&) {
  if (!initialized_) return core::Status{core::StatusCode::kInvalidArgument, "Taichi 模拟器尚未初始化"};
  return core::Status::notImplemented(
      "Taichi 进程桥尚未在当前环境安装；请安装 taichi 并运行 tools/taichi_bridge/smoke.py");
}

}  // namespace physfx::physics
