# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

set(PYBIND11_FINDPYTHON ON)
find_package(Python 3.11 REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_physfx MODULE src/bindings.cpp)
target_compile_features(_physfx PRIVATE cxx_std_20)
if(MINGW)
  target_link_options(_physfx PRIVATE -static -static-libgcc -static-libstdc++)
endif()
install(TARGETS _physfx LIBRARY DESTINATION physfx_py)
