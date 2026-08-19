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

#include <string>
#include <string_view>

#include "physfx/core/SemanticScene.h"
#include "physfx/core/Status.h"

namespace physfx::editing {

/** @brief 可执行、可撤销、可序列化的编辑命令统一接口。 */
class IEditCommand {
 public:
  virtual ~IEditCommand() = default;
  /** @brief 返回命令名称。 @return 稳定名称。 */
  [[nodiscard]] virtual std::string_view name() const noexcept = 0;
  /** @brief 执行命令。 @param scene 语义场景。 @return 操作状态。 */
  virtual core::Status execute(core::SemanticScene& scene) = 0;
  /** @brief 撤销命令。 @param scene 语义场景。 @return 操作状态。 */
  virtual core::Status undo(core::SemanticScene& scene) = 0;
  /** @brief 序列化命令描述。 @return 稳定文本表示。 */
  [[nodiscard]] virtual std::string serialize() const = 0;
};

}  // namespace physfx::editing
