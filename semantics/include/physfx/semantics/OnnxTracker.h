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

#include <filesystem>
#include <memory>

#include "physfx/semantics/ITracker.h"

namespace physfx::semantics {

/** @brief XMem 类 ONNX 时序掩码传播适配器。 */
class OnnxTracker final : public ITracker {
 public:
  explicit OnnxTracker(std::filesystem::path modelPath);
  ~OnnxTracker() override;
  OnnxTracker(OnnxTracker&&) noexcept;
  OnnxTracker& operator=(OnnxTracker&&) noexcept;
  OnnxTracker(const OnnxTracker&) = delete;
  OnnxTracker& operator=(const OnnxTracker&) = delete;

  core::Result<std::vector<core::Entity>> track(const core::Frame& frame,
                                                const std::vector<core::Entity>& entities) override;
  [[nodiscard]] std::string executionProvider() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace physfx::semantics
