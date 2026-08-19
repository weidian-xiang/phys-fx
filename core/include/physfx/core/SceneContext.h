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
#include <memory>
#include <vector>

#include "physfx/core/Math.h"

namespace physfx::core {

struct SemanticScene;

/** @brief 深度图数据。 @todo Phase2 接入真实深度估计模型。 */
struct DepthMap {
  std::uint32_t width{0};
  std::uint32_t height{0};
  std::vector<float> values{};
};

/** @brief 光照估计参数。 @todo Phase2 接入学习式光照估计。 */
struct LightParams {
  float ambientIntensity{1.0F};
  Vec3 dominantDirection{0.0F, -1.0F, 0.0F};
  Vec3 color{1.0F, 1.0F, 1.0F};
};

/** @brief 单帧相机位姿。 @todo Phase2 接入相机跟踪器。 */
struct CameraPose {
  Mat4 transform{};
  double timestampSeconds{0.0};
};

/** @brief 相机轨迹。 @todo Phase2 支持关键帧与插值。 */
struct CameraTrack {
  std::vector<CameraPose> poses{};
};

/** @brief 遮挡掩码。 @todo Phase2 接入实例分割与深度排序。 */
struct OcclusionMask {
  std::uint32_t width{0};
  std::uint32_t height{0};
  std::vector<std::uint8_t> values{};
};

/**
 * @brief 场景理解结果，是 perception、physics、compositing 之间的共享契约。
 * @todo Phase2 增加置信度、坐标系和时间同步信息。
 */
struct SceneContext {
  LightParams light{};
  DepthMap depth{};
  CameraTrack camera{};
  Plane ground{};
  OcclusionMask occlusion{};
  /** @brief 可选语义场景；为空时保持 Phase 1 行为。 */
  std::shared_ptr<SemanticScene> semanticScene{};
};

}  // namespace physfx::core
