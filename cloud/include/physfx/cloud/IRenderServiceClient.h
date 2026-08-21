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
#include <filesystem>
#include <string>

#include "physfx/core/Status.h"

namespace physfx::cloud {

/** @brief 云渲染任务描述。 */
struct RenderJobRequest {
  std::filesystem::path projectPath{};
  std::filesystem::path inputPath{};
  std::filesystem::path outputPath{};
  std::string scriptJson{};
  std::string qualityPreset{};
  std::string apiKey{};
};

/** @brief 云渲染任务状态。 */
struct RenderJob {
  std::string jobId{};
  std::string state{};
  std::string message{};
  std::filesystem::path artifactPath{};
  std::uint64_t renderSeconds{0};
};

/** @brief 云渲染客户端接口；本阶段不提供实现和网络调用。 */
class PHYSFX_API IRenderServiceClient {
 public:
  virtual ~IRenderServiceClient() = default;
  /** @brief 提交渲染任务。 @param request 任务描述。 @return 任务或错误状态。 */
  virtual core::Result<RenderJob> submit(const RenderJobRequest& request) = 0;
  /** @brief 查询任务。 @param jobId 任务编号。 @return 任务或错误状态。 */
  virtual core::Result<RenderJob> query(const std::string& jobId) = 0;
  /** @brief 取消任务。 @param jobId 任务编号。 @return 操作状态。 */
  virtual core::Status cancel(const std::string& jobId) = 0;
};

/** @brief 零网络依赖的本地云服务客户端；用于 CLI、开发和合同测试。 */
class PHYSFX_API LocalRenderServiceClient final : public IRenderServiceClient {
 public:
  explicit LocalRenderServiceClient(std::filesystem::path root = {});
  core::Result<RenderJob> submit(const RenderJobRequest& request) override;
  core::Result<RenderJob> query(const std::string& jobId) override;
  core::Status cancel(const std::string& jobId) override;

 private:
  std::filesystem::path root_{};
};

}  // namespace physfx::cloud
