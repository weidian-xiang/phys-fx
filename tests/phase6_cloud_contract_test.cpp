/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cassert>
#include <filesystem>
#include <fstream>

#include "physfx/cloud/IRenderServiceClient.h"
#include "physfx/cloud/ITemplateMarketplaceClient.h"

int main() {
  const auto root = std::filesystem::temp_directory_path() / "physfx-phase6-cloud-contract";
  std::error_code error;
  std::filesystem::remove_all(root, error);
  physfx::cloud::LocalRenderServiceClient render(root);
  const auto missing = render.submit({root / "missing.json", {}, {}, {}, "balanced", "test-key"});
  assert(!missing.ok());
  const auto project = root / "project.json";
  std::filesystem::create_directories(root, error);
  std::ofstream(project) << "{}";
  const auto queued = render.submit({project, {}, {}, {}, "balanced", "test-key"});
  assert(queued.ok() && queued.value().state == "queued");
  assert(render.query(queued.value().jobId).ok());
  assert(render.cancel(queued.value().jobId).ok());

  physfx::cloud::LocalTemplateMarketplaceClient market(std::filesystem::path(PHYSFX_SOURCE_DIR) /
                                                       "plugins" / "templates");
  const auto templates = market.search("splash");
  assert(templates.ok() && !templates.value().empty());
  return 0;
}
