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

#include "physfx/core/Status.h"

namespace physfx::plugins {

/** @brief 模板包签名与授权信息骨架。 */
struct TemplatePackageManifest {
  std::string packageId{};
  std::string version{};
  std::string publisher{};
  std::string signatureAlgorithm{};
  std::string signature{};
  std::string engineVersion{};
  std::string contentDigest{};
  std::string publicKey{};
};

/** @brief 可选商业模板授权校验接口；开源核心不会强制调用。 */
class ILicenseVerifier {
 public:
  virtual ~ILicenseVerifier() = default;
  /**
   * @brief 校验模板包授权与签名。
   * @param manifest 模板包清单。
   * @param licenseToken 外部传入的授权令牌。
   * @return 校验状态。
   */
  virtual core::Status verify(const TemplatePackageManifest& manifest,
                              std::string_view licenseToken) const = 0;
};

/** @brief 开源核心的轻量包清单校验器。签名是信任标识，不是功能解锁。 */
class TemplatePackageVerifier final : public ILicenseVerifier {
 public:
  core::Status verify(const TemplatePackageManifest& manifest,
                      std::string_view licenseToken) const override;
};

}  // namespace physfx::plugins
