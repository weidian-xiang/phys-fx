/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <utility>

#include "physfx/cloud/IRenderServiceClient.h"
#include "physfx/cloud/ITemplateMarketplaceClient.h"

namespace physfx::cloud {
namespace {

std::filesystem::path defaultRoot() {
  if (const char* value = std::getenv("PHYSFX_CLOUD_ROOT"); value != nullptr && *value != '\0') {
    return value;
  }
  return std::filesystem::current_path() / ".physfx-cloud";
}

std::string readField(const std::string& text, std::string_view key) {
  const auto marker = "\"" + std::string(key) + "\"";
  const auto position = text.find(marker);
  if (position == std::string::npos) return {};
  const auto colon = text.find(':', position + marker.size());
  const auto first = text.find('"', colon == std::string::npos ? position : colon + 1U);
  if (first == std::string::npos) return {};
  const auto second = text.find('"', first + 1U);
  return second == std::string::npos ? std::string{} : text.substr(first + 1U, second - first - 1U);
}

std::string makeId(const RenderJobRequest& request) {
  const auto seed = request.projectPath.string() + request.inputPath.string() + request.scriptJson;
  const auto hash = std::hash<std::string>{}(seed);
  std::ostringstream stream;
  stream << "local-" << std::hex << hash;
  return stream.str();
}

core::Result<RenderJob> readJob(const std::filesystem::path& path) {
  std::ifstream stream(path);
  if (!stream) return core::Status{core::StatusCode::kNotFound, "云渲染任务不存在"};
  const std::string text((std::istreambuf_iterator<char>(stream)), {});
  RenderJob job{};
  job.jobId = readField(text, "jobId");
  job.state = readField(text, "state");
  job.message = readField(text, "message");
  job.artifactPath = readField(text, "artifactPath");
  return job;
}

void writeJob(const std::filesystem::path& path, const RenderJob& job) {
  std::ofstream stream(path, std::ios::trunc);
  stream << "{\"jobId\":\"" << job.jobId << "\",\"state\":\"" << job.state << "\",\"message\":\""
         << job.message << "\",\"artifactPath\":\"" << job.artifactPath.string() << "\"}\n";
}

}  // namespace

LocalRenderServiceClient::LocalRenderServiceClient(std::filesystem::path root)
    : root_(root.empty() ? defaultRoot() : std::move(root)) {}

core::Result<RenderJob> LocalRenderServiceClient::submit(const RenderJobRequest& request) {
  if (request.apiKey.empty()) {
    return core::Status{core::StatusCode::kInvalidArgument,
                        "云渲染需要 API Key；本地渲染无需 API Key"};
  }
  if (request.projectPath.empty() && request.scriptJson.empty()) {
    return core::Status{core::StatusCode::kInvalidArgument, "缺少编辑脚本或工程文件"};
  }
  if (!request.projectPath.empty() && !std::filesystem::exists(request.projectPath)) {
    return core::Status{core::StatusCode::kNotFound, "编辑工程不存在"};
  }
  const auto jobs = root_ / "jobs";
  std::error_code error;
  std::filesystem::create_directories(jobs, error);
  if (error) return core::Status{core::StatusCode::kIoError, "无法创建云任务目录"};
  RenderJob job{};
  job.jobId = makeId(request);
  job.state = "queued";
  job.message = "任务已入队；本地离线客户端不会上传素材";
  job.artifactPath = request.outputPath;
  writeJob(jobs / (job.jobId + ".json"), job);
  return job;
}

core::Result<RenderJob> LocalRenderServiceClient::query(const std::string& jobId) {
  if (jobId.empty()) return core::Status{core::StatusCode::kInvalidArgument, "任务编号为空"};
  return readJob(root_ / "jobs" / (jobId + ".json"));
}

core::Status LocalRenderServiceClient::cancel(const std::string& jobId) {
  const auto path = root_ / "jobs" / (jobId + ".json");
  auto job = readJob(path);
  if (!job.ok()) return job.status();
  auto value = std::move(job).value();
  value.state = "cancelled";
  value.message = "任务已取消";
  writeJob(path, value);
  return core::Status::success();
}

LocalTemplateMarketplaceClient::LocalTemplateMarketplaceClient(std::filesystem::path root)
    : root_(root.empty() ? std::filesystem::current_path() / "plugins" / "templates"
                         : std::move(root)) {}

core::Result<std::vector<MarketplaceTemplate>> LocalTemplateMarketplaceClient::search(
    const std::string& keyword) {
  std::vector<MarketplaceTemplate> result;
  std::error_code error;
  if (!std::filesystem::exists(root_, error)) return result;
  for (const auto& entry : std::filesystem::directory_iterator(root_, error)) {
    if (error || !entry.is_regular_file() || entry.path().extension() != ".json") continue;
    const auto text = [&]() {
      std::ifstream stream(entry.path());
      return std::string((std::istreambuf_iterator<char>(stream)), {});
    }();
    MarketplaceTemplate item{};
    item.templateId = readField(text, "id");
    if (item.templateId.empty()) item.templateId = entry.path().stem().string();
    item.name = readField(text, "name");
    item.version = readField(text, "version");
    item.publisher = readField(text, "publisher");
    item.signatureStatus = "local-unsigned";
    item.packagePath = entry.path();
    if (keyword.empty() || item.templateId.find(keyword) != std::string::npos ||
        item.name.find(keyword) != std::string::npos) {
      result.push_back(std::move(item));
    }
  }
  return result;
}

core::Result<MarketplaceTemplate> LocalTemplateMarketplaceClient::describe(
    const std::string& templateId) {
  const auto matches = search(templateId);
  if (!matches.ok()) return matches.status();
  for (const auto& item : matches.value()) {
    if (item.templateId == templateId) return item;
  }
  return core::Status{core::StatusCode::kNotFound, "模板不存在"};
}

core::Status LocalTemplateMarketplaceClient::install(const std::string& templateId,
                                                     const std::filesystem::path& destination) {
  auto item = describe(templateId);
  if (!item.ok()) return item.status();
  std::error_code error;
  std::filesystem::create_directories(destination, error);
  if (error) return {core::StatusCode::kIoError, "无法创建模板安装目录"};
  std::filesystem::copy_file(item.value().packagePath,
                             destination / item.value().packagePath.filename(),
                             std::filesystem::copy_options::overwrite_existing, error);
  return error ? core::Status{core::StatusCode::kIoError, "模板安装失败"} : core::Status::success();
}

}  // namespace physfx::cloud
