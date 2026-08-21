/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/semantics/OnnxSegmenter.h"

#include <string>
#include <utility>

#if PHYSFX_ONNX_AVAILABLE
#include <onnxruntime_cxx_api.h>
#endif

namespace physfx::semantics {

struct OnnxSegmenter::Impl {
  std::filesystem::path modelPath;
  std::string executionProvider{"CPUExecutionProvider"};
#if PHYSFX_ONNX_AVAILABLE
  Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "physfx"};
  std::unique_ptr<Ort::Session> session;
#endif

  explicit Impl(std::filesystem::path path) : modelPath(std::move(path)) {
#if PHYSFX_ONNX_AVAILABLE
    if (std::filesystem::exists(modelPath)) {
      try {
        Ort::SessionOptions options;
        options.SetIntraOpNumThreads(1);
        options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);
        for (const auto& provider : Ort::GetAvailableProviders()) {
          if (provider == "CUDAExecutionProvider") {
            options.AppendExecutionProvider("CUDA");
            executionProvider = provider;
            break;
          }
          if (provider == "DmlExecutionProvider") {
            options.AppendExecutionProvider("DML");
            executionProvider = provider;
            break;
          }
        }
#ifdef _WIN32
        session = std::make_unique<Ort::Session>(env, modelPath.wstring().c_str(), options);
#else
        session = std::make_unique<Ort::Session>(env, modelPath.string().c_str(), options);
#endif
      } catch (const Ort::Exception&) {
        session.reset();
        executionProvider = "CPUExecutionProvider";
      }
    }
#endif
  }
};

OnnxSegmenter::OnnxSegmenter(std::filesystem::path modelPath)
    : impl_(std::make_unique<Impl>(std::move(modelPath))) {}
OnnxSegmenter::~OnnxSegmenter() = default;
OnnxSegmenter::OnnxSegmenter(OnnxSegmenter&&) noexcept = default;
OnnxSegmenter& OnnxSegmenter::operator=(OnnxSegmenter&&) noexcept = default;
std::string OnnxSegmenter::executionProvider() const { return impl_->executionProvider; }

core::Result<std::vector<core::Entity>> OnnxSegmenter::segment(const core::Frame&) {
#if PHYSFX_ONNX_AVAILABLE
  if (!impl_->session) {
    return core::Status{core::StatusCode::kNotFound, "ONNX 分割模型不存在或无法加载"};
  }
  return core::Status::notImplemented(
      "ONNX 分割模型输入输出契约需按具体 SAM 导出版本配置，当前仅完成运行时装载");
#else
  return core::Status{core::StatusCode::kDisabled, "ONNX Runtime 未启用；请打开 PHYSFX_WITH_ONNX"};
#endif
}

}  // namespace physfx::semantics
