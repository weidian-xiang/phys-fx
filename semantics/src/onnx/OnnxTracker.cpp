/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/semantics/OnnxTracker.h"

#include <utility>
#include <string>

#if PHYSFX_ONNX_AVAILABLE
#include <onnxruntime_cxx_api.h>
#endif

namespace physfx::semantics {

struct OnnxTracker::Impl {
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

OnnxTracker::OnnxTracker(std::filesystem::path modelPath)
    : impl_(std::make_unique<Impl>(std::move(modelPath))) {}
OnnxTracker::~OnnxTracker() = default;
OnnxTracker::OnnxTracker(OnnxTracker&&) noexcept = default;
OnnxTracker& OnnxTracker::operator=(OnnxTracker&&) noexcept = default;
std::string OnnxTracker::executionProvider() const { return impl_->executionProvider; }

core::Result<std::vector<core::Entity>> OnnxTracker::track(const core::Frame&,
                                                           const std::vector<core::Entity>&) {
#if PHYSFX_ONNX_AVAILABLE
  if (!impl_->session) {
    return core::Status{core::StatusCode::kNotFound, "ONNX 跟踪模型不存在或无法加载"};
  }
  return core::Status::notImplemented(
      "ONNX 跟踪模型输入输出契约需按具体 XMem 导出版本配置，当前仅完成运行时装载");
#else
  return core::Status{core::StatusCode::kDisabled, "ONNX Runtime 未启用；请打开 PHYSFX_WITH_ONNX"};
#endif
}

}  // namespace physfx::semantics
