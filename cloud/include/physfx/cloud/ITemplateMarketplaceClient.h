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
#include <vector>

#include "physfx/core/Status.h"

namespace physfx::cloud {

/** @brief 模板市场条目。 */
struct MarketplaceTemplate {
  std::string templateId{};
  std::string name{};
  std::string version{};
};

/** @brief 模板市场客户端接口；本阶段不提供实现和网络调用。 */
class ITemplateMarketplaceClient {
 public:
  virtual ~ITemplateMarketplaceClient() = default;
  /** @brief 查询模板。 @param keyword 搜索词。 @return 模板列表或错误状态。 */
  virtual core::Result<std::vector<MarketplaceTemplate>> search(const std::string& keyword) = 0;
  /** @brief 获取模板清单。 @param templateId 模板编号。 @return 模板条目或错误状态。 */
  virtual core::Result<MarketplaceTemplate> describe(const std::string& templateId) = 0;
};

}  // namespace physfx::cloud
