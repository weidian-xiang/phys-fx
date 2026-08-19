# 第三方依赖说明

第三方依赖统一通过 vcpkg manifest (`vcpkg.json`) 管理。本阶段不下载或集成 FFmpeg、OpenCV、Taichi、
OpenGL/Vulkan 等重型依赖；pybind11 仅用于可选 Python 骨架验证。`third_party` 目录只作为应急补丁位置，
其中的缓存与实体验证包不得入库。
