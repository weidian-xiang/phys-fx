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
#include <map>
#include <string>
#include <vector>

#include "physfx/core/Math.h"

namespace physfx::core {

/** @brief 实体在单帧中的语义掩码。 */
struct EntityMask {
  std::uint64_t frameIndex{0};
  std::uint32_t width{0};
  std::uint32_t height{0};
  std::vector<std::uint8_t> values{};
};

/** @brief 实体在单帧中的位姿。 */
struct EntityPose {
  std::uint64_t frameIndex{0};
  Vec3 position{};
};

/** @brief 可编辑材质契约。 */
struct MaterialProperties {
  std::string name{"default"};
  Vec3 baseColor{1.0F, 1.0F, 1.0F};
  float roughness{0.5F};
  float metallic{0.0F};
};

/** @brief 视频世界中的语义实体。 */
struct Entity {
  std::uint64_t id{0};
  std::string category{"unknown"};
  std::vector<EntityPose> trajectory{};
  std::map<std::string, std::string> attributes{};
  std::vector<EntityMask> maskTimeline{};
  MaterialProperties material{};
  bool deleted{false};
};

/** @brief 场景天气枚举。 */
enum class Weather { kOriginal, kClear, kRain, kSnow, kFog };

/** @brief 场景时段枚举。 */
enum class TimeOfDay { kOriginal, kDawn, kDay, kDusk, kNight };

/** @brief 语义场景契约，是理解、编辑、模拟和渲染共享的数据模型。 */
struct SemanticScene {
  std::vector<Entity> entities{};
  /** @brief 当前选中的实体编号；0 表示未选中。 */
  std::uint64_t selectedEntityId{0};
  Weather weather{Weather::kOriginal};
  TimeOfDay timeOfDay{TimeOfDay::kOriginal};

  /** @brief 按编号查找实体。 @param id 实体编号。 @return 实体指针，未找到时为空。 */
  [[nodiscard]] Entity* findEntity(std::uint64_t id) noexcept;
  /** @brief 按编号查找实体。 @param id 实体编号。 @return 实体指针，未找到时为空。 */
  [[nodiscard]] const Entity* findEntity(std::uint64_t id) const noexcept;
};

}  // namespace physfx::core
