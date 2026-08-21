/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/plugins/ILicenseVerifier.h"

namespace physfx::plugins {

core::Status TemplatePackageVerifier::verify(const TemplatePackageManifest& manifest,
                                             std::string_view licenseToken) const {
  (void)licenseToken;
  if (manifest.packageId.empty() || manifest.version.empty()) {
    return {core::StatusCode::kInvalidArgument, "模板包清单缺少 packageId 或 version"};
  }
  if (manifest.signature.empty()) {
    // 红线：开源版永远允许未签名模板继续加载。
    return {core::StatusCode::kOk, "未认证来源：模板未提供 Ed25519 签名"};
  }
  if (manifest.signatureAlgorithm != "Ed25519" || manifest.publicKey.empty()) {
    return {core::StatusCode::kInvalidArgument, "模板包签名算法或公钥字段无效"};
  }
  return {core::StatusCode::kOk, "已认证模板：Ed25519 签名由工具链验签"};
}

}  // namespace physfx::plugins
