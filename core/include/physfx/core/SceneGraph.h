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
#include <string>
#include <vector>

#include "physfx/core/Export.h"

namespace physfx::core {

/** @brief 场景图中的节点占位。 @todo Phase2 增加变换、材质和父子关系。 */
struct SceneNode {
  std::uint64_t id{0};
  std::string name{};
  /** @brief 可选关联实体编号，0 表示普通节点。 */
  std::uint64_t entityId{0};
};

/**
 * @brief 用于描述视频场景与特效对象关系的场景图。
 * @todo Phase2 实现场景图遍历、序列化和空间索引。
 */
class PHYSFX_API SceneGraph {
 public:
  /** @brief 添加一个节点。 @param node 要添加的节点。 */
  void addNode(SceneNode node);
  /** @brief 添加实体节点。 @param entityId 实体编号。 @param name 节点名称。 */
  void addEntityNode(std::uint64_t entityId, std::string name);
  /** @brief 返回节点列表。 @return 只读节点列表。 */
  [[nodiscard]] const std::vector<SceneNode>& nodes() const noexcept;

 private:
  std::vector<SceneNode> nodes_{};
};

}  // namespace physfx::core
