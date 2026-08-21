/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

/** @brief 创建 PhysFX Python 扩展模块。 @todo Phase2 实现：暴露管线和核心数据类型。 */
PYBIND11_MODULE(_physfx, module) {
  module.doc() = "PhysFX Engine Python 绑定骨架";
  module.def("version", []() { return "0.6.0"; }, "返回 PhysFX Engine 版本号");
}
