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

#include "physfx/semantics/ISegmenter.h"

namespace physfx::semantics {

/** @brief SAM 类 ONNX 首帧分割适配器；模型输入输出由模型卡片约定。 */
class OnnxSegmenter final : public ISegmenter {
 public:
  explicit OnnxSegmenter(std::filesystem::path modelPath);
  ~OnnxSegmenter() override;
  OnnxSegmenter(OnnxSegmenter&&) noexcept;
  OnnxSegmenter& operator=(OnnxSegmenter&&) noexcept;
  OnnxSegmenter(const OnnxSegmenter&) = delete;
  OnnxSegmenter& operator=(const OnnxSegmenter&) = delete;

  core::Result<std::vector<core::Entity>> segment(const core::Frame& frame) override;
  [[nodiscard]] std::string executionProvider() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace physfx::semantics
