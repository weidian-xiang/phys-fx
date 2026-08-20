# PhysFX Engine —— 视频世界编辑器（Video World Editor）
# Copyright (c) 2026 向伟典
#
# 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
# 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.26)

project(PhysFX VERSION 0.3.0 LANGUAGES CXX)

option(PHYSFX_BUILD_TESTS "构建 C++ 测试" ON)
option(PHYSFX_BUILD_PYTHON "构建 Python 绑定骨架" OFF)
option(PHYSFX_WITH_FFMPEG "启用 FFmpeg 动态视频 IO（需要 LGPL 开发包）" OFF)
option(PHYSFX_WITH_ONNX "启用 ONNX Runtime 推理后端" OFF)
option(PHYSFX_WITH_TAICHI "启用 Taichi 进程级流体桥接" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(CTest)

add_library(physfx_core STATIC
  core/src/Config.cpp
  core/src/Log.cpp
  core/src/SceneGraph.cpp
  core/src/SemanticScene.cpp
)
target_include_directories(physfx_core PUBLIC core/include)
target_compile_features(physfx_core PUBLIC cxx_std_20)

add_library(physfx_perception STATIC
  perception/src/stub/StubEstimators.cpp
)
target_include_directories(physfx_perception PUBLIC perception/include)
target_link_libraries(physfx_perception PUBLIC physfx_core)

add_library(physfx_semantics STATIC
  semantics/src/stub/StubSemantics.cpp
  semantics/src/onnx/OnnxSegmenter.cpp
  semantics/src/onnx/OnnxTracker.cpp
  semantics/src/traditional/TraditionalSemantics.cpp
)
target_include_directories(physfx_semantics PUBLIC semantics/include)
target_link_libraries(physfx_semantics PUBLIC physfx_core)

set(PHYSFX_ONNX_AVAILABLE OFF)
if(PHYSFX_WITH_ONNX)
  unset(ONNXRUNTIME_INCLUDE_DIR CACHE)
  unset(ONNXRUNTIME_LIBRARY CACHE)
  find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_cxx_api.h
    PATHS "${CMAKE_SOURCE_DIR}/third_party/onnxruntime" ENV ONNXRUNTIME_ROOT
    PATH_SUFFIXES include build/native/include NO_DEFAULT_PATH)
  find_library(ONNXRUNTIME_LIBRARY NAMES onnxruntime
    PATHS "${CMAKE_SOURCE_DIR}/third_party/onnxruntime" ENV ONNXRUNTIME_ROOT
    PATH_SUFFIXES lib runtimes/win-x64/native runtimes/linux-x64/native NO_DEFAULT_PATH)
  if(ONNXRUNTIME_INCLUDE_DIR AND ONNXRUNTIME_LIBRARY)
    set(PHYSFX_ONNX_AVAILABLE ON)
    message(STATUS "启用 ONNX Runtime 语义后端")
  else()
    message(WARNING "请求启用 ONNX Runtime，但未找到开发包；回退到语义桩")
  endif()
endif()
if(PHYSFX_ONNX_AVAILABLE)
  target_compile_definitions(physfx_semantics PRIVATE PHYSFX_ONNX_AVAILABLE=1)
  if(MINGW)
    # ONNX Runtime 1.18 Windows 头在 specstrings.h 后使用 _stdcall；MinGW 需显式映射。
    target_compile_definitions(physfx_semantics PRIVATE _stdcall=__stdcall)
  endif()
  target_include_directories(physfx_semantics PRIVATE "${ONNXRUNTIME_INCLUDE_DIR}")
  target_link_libraries(physfx_semantics PRIVATE "${ONNXRUNTIME_LIBRARY}")
else()
  target_compile_definitions(physfx_semantics PRIVATE PHYSFX_ONNX_AVAILABLE=0)
endif()

add_library(physfx_editing STATIC
  editing/src/Commands.cpp
  editing/src/EditCommandStack.cpp
  editing/src/CommandFactory.cpp
)
target_include_directories(physfx_editing PUBLIC editing/include)
target_link_libraries(physfx_editing PUBLIC physfx_core)

add_library(physfx_physics STATIC
  physics/src/mock/MockSimulator.cpp
  physics/src/particles/SimpleParticleSystem.cpp
  physics/src/taichi_bridge/TaichiFluidSimulator.cpp
)
target_include_directories(physfx_physics PUBLIC physics/include)
target_link_libraries(physfx_physics PUBLIC physfx_core)

add_library(physfx_compositing STATIC
  compositing/src/PassthroughCompositor.cpp
  compositing/src/sprite/SpriteCompositor.cpp
  compositing/src/layered/LayeredCompositor.cpp
)
target_include_directories(physfx_compositing PUBLIC compositing/include)
target_link_libraries(physfx_compositing PUBLIC physfx_core)

add_library(physfx_neural_render STATIC
  neural_render/src/passthrough/PassthroughNeuralRenderer.cpp
  neural_render/src/inpainting/InpaintingRenderer.cpp
)
target_include_directories(physfx_neural_render PUBLIC neural_render/include)
target_link_libraries(physfx_neural_render PUBLIC physfx_core)

add_library(physfx_plugins STATIC
  plugins/src/PluginRegistry.cpp
  plugins/src/BuiltInEffectTemplates.cpp
  plugins/src/BuiltInEditTemplates.cpp
  plugins/examples/empty_effect/EmptyEffectPlugin.cpp
)
target_include_directories(physfx_plugins PUBLIC plugins/include)
target_link_libraries(physfx_plugins PUBLIC physfx_core)

set(PHYSFX_PLATFORM_SOURCES
  platform/src/StubVideoReader.cpp
  platform/src/StubVideoWriter.cpp
  platform/src/StubGpuContext.cpp
  platform/src/VideoIoFactory.cpp
)
set(PHYSFX_FFMPEG_AVAILABLE OFF)
if(PHYSFX_WITH_FFMPEG)
  unset(FFMPEG_INCLUDE_DIR CACHE)
  unset(FFMPEG_AVFORMAT_LIBRARY CACHE)
  unset(FFMPEG_AVCODEC_LIBRARY CACHE)
  unset(FFMPEG_AVUTIL_LIBRARY CACHE)
  unset(FFMPEG_SWSCALE_LIBRARY CACHE)
  find_path(FFMPEG_INCLUDE_DIR libavformat/avformat.h
    PATHS "${CMAKE_SOURCE_DIR}/third_party/ffmpeg" ENV FFMPEG_ROOT
    PATH_SUFFIXES include NO_DEFAULT_PATH)
  find_library(FFMPEG_AVFORMAT_LIBRARY NAMES avformat libavformat
    PATHS "${CMAKE_SOURCE_DIR}/third_party/ffmpeg" ENV FFMPEG_ROOT PATH_SUFFIXES lib
    NO_DEFAULT_PATH)
  find_library(FFMPEG_AVCODEC_LIBRARY NAMES avcodec libavcodec
    PATHS "${CMAKE_SOURCE_DIR}/third_party/ffmpeg" ENV FFMPEG_ROOT PATH_SUFFIXES lib
    NO_DEFAULT_PATH)
  find_library(FFMPEG_AVUTIL_LIBRARY NAMES avutil libavutil
    PATHS "${CMAKE_SOURCE_DIR}/third_party/ffmpeg" ENV FFMPEG_ROOT PATH_SUFFIXES lib
    NO_DEFAULT_PATH)
  find_library(FFMPEG_SWSCALE_LIBRARY NAMES swscale libswscale
    PATHS "${CMAKE_SOURCE_DIR}/third_party/ffmpeg" ENV FFMPEG_ROOT PATH_SUFFIXES lib
    NO_DEFAULT_PATH)
  if(FFMPEG_INCLUDE_DIR AND FFMPEG_AVFORMAT_LIBRARY AND FFMPEG_AVCODEC_LIBRARY
     AND FFMPEG_AVUTIL_LIBRARY AND FFMPEG_SWSCALE_LIBRARY)
    get_filename_component(PHYSFX_FFMPEG_ROOT "${FFMPEG_INCLUDE_DIR}" DIRECTORY)
    if(WIN32)
      file(GLOB PHYSFX_FFMPEG_CORE_DLLS
        "${PHYSFX_FFMPEG_ROOT}/bin/avformat-*.dll"
        "${PHYSFX_FFMPEG_ROOT}/bin/avcodec-*.dll"
        "${PHYSFX_FFMPEG_ROOT}/bin/avutil-*.dll"
        "${PHYSFX_FFMPEG_ROOT}/bin/swscale-*.dll")
      file(GLOB PHYSFX_FFMPEG_RUNTIME_DLLS "${PHYSFX_FFMPEG_ROOT}/bin/*.dll")
      list(LENGTH PHYSFX_FFMPEG_CORE_DLLS PHYSFX_FFMPEG_CORE_DLL_COUNT)
      if(PHYSFX_FFMPEG_CORE_DLL_COUNT LESS 4)
        message(WARNING "找到 FFmpeg 导入库但未找到运行时 DLL；拒绝可能的静态构建并回退视频 IO 桩")
      else()
        set(PHYSFX_FFMPEG_AVAILABLE ON)
      endif()
    elseif(FFMPEG_AVFORMAT_LIBRARY MATCHES "\\.a$")
      message(WARNING "找到静态 FFmpeg 库；许可策略禁止静态链接并回退视频 IO 桩")
    else()
      set(PHYSFX_FFMPEG_AVAILABLE ON)
    endif()
    if(PHYSFX_FFMPEG_AVAILABLE)
      list(APPEND PHYSFX_PLATFORM_SOURCES
        platform/src/ffmpeg/FFmpegVideoReader.cpp
        platform/src/ffmpeg/FFmpegVideoWriter.cpp)
      message(STATUS "启用 FFmpeg LGPL 动态视频 IO")
    endif()
  else()
    message(WARNING "请求启用 FFmpeg，但未找到 LGPL 开发包；回退到视频 IO 桩")
  endif()
endif()
add_library(physfx_platform STATIC ${PHYSFX_PLATFORM_SOURCES})
target_include_directories(physfx_platform PUBLIC platform/include)
target_link_libraries(physfx_platform PUBLIC physfx_core)
if(PHYSFX_FFMPEG_AVAILABLE)
  target_include_directories(physfx_platform PRIVATE "${FFMPEG_INCLUDE_DIR}")
  target_compile_definitions(physfx_platform PRIVATE PHYSFX_FFMPEG_AVAILABLE=1)
  target_link_libraries(physfx_platform PRIVATE
    "${FFMPEG_AVFORMAT_LIBRARY}" "${FFMPEG_AVCODEC_LIBRARY}"
    "${FFMPEG_AVUTIL_LIBRARY}" "${FFMPEG_SWSCALE_LIBRARY}")
  # LGPL 边界：配置阶段检查开发包声明，运行时 FFmpeg 也会拒绝 GPL 构建。
  # C API 不提供稳定的许可证枚举，因此禁止在发布构建中静态链接并要求动态库。
else()
  target_compile_definitions(physfx_platform PRIVATE PHYSFX_FFMPEG_AVAILABLE=0)
endif()

add_library(physfx_cloud INTERFACE)
target_include_directories(physfx_cloud INTERFACE cloud/include)
target_link_libraries(physfx_cloud INTERFACE physfx_core)

add_library(physfx_pipeline STATIC
  pipeline/src/Pipeline.cpp
  pipeline/src/StageFactory.cpp
)
target_include_directories(physfx_pipeline PUBLIC pipeline/include)
target_link_libraries(physfx_pipeline PUBLIC
  physfx_core
  physfx_perception
  physfx_semantics
  physfx_editing
  physfx_physics
  physfx_compositing
  physfx_neural_render
  physfx_plugins
  physfx_platform
)

function(physfx_deploy_optional_runtimes runtime_target)
  if(PHYSFX_ONNX_AVAILABLE AND WIN32)
    unset(ONNXRUNTIME_RUNTIME_DLL CACHE)
    find_file(ONNXRUNTIME_RUNTIME_DLL NAMES onnxruntime.dll
      PATHS "${CMAKE_SOURCE_DIR}/third_party/onnxruntime" ENV ONNXRUNTIME_ROOT
      PATH_SUFFIXES bin runtimes/win-x64/native NO_DEFAULT_PATH)
    if(ONNXRUNTIME_RUNTIME_DLL)
      add_custom_command(TARGET ${runtime_target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${ONNXRUNTIME_RUNTIME_DLL}" "$<TARGET_FILE_DIR:${runtime_target}>")
    endif()
  endif()
  if(PHYSFX_FFMPEG_AVAILABLE AND WIN32)
    foreach(runtime_dll IN LISTS PHYSFX_FFMPEG_RUNTIME_DLLS)
      add_custom_command(TARGET ${runtime_target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${runtime_dll}" "$<TARGET_FILE_DIR:${runtime_target}>")
    endforeach()
  endif()
endfunction()

add_subdirectory(apps/cli)

if(PHYSFX_BUILD_TESTS)
  enable_testing()
  find_package(GTest CONFIG QUIET)
  add_executable(physfx_core_tests core/tests/core_dummy_test.cpp)
  target_link_libraries(physfx_core_tests PRIVATE physfx_core)
  add_test(NAME core_dummy_test COMMAND physfx_core_tests)

  add_executable(physfx_perception_tests perception/tests/perception_dummy_test.cpp)
  target_link_libraries(physfx_perception_tests PRIVATE physfx_perception)
  add_test(NAME perception_dummy_test COMMAND physfx_perception_tests)

  add_executable(physfx_physics_tests physics/tests/physics_dummy_test.cpp)
  target_link_libraries(physfx_physics_tests PRIVATE physfx_physics)
  add_test(NAME physics_dummy_test COMMAND physfx_physics_tests)
  add_executable(physfx_particle_system_tests physics/tests/simple_particle_system_test.cpp)
  target_link_libraries(physfx_particle_system_tests PRIVATE physfx_physics)
  add_test(NAME simple_particle_system_test COMMAND physfx_particle_system_tests)

  add_executable(physfx_semantics_tests semantics/tests/semantics_dummy_test.cpp)
  target_link_libraries(physfx_semantics_tests PRIVATE physfx_semantics)
  add_test(NAME semantics_dummy_test COMMAND physfx_semantics_tests)
  add_executable(physfx_traditional_semantics_tests
    semantics/tests/traditional_semantics_test.cpp)
  target_link_libraries(physfx_traditional_semantics_tests PRIVATE physfx_semantics)
  add_test(NAME traditional_semantics_test COMMAND physfx_traditional_semantics_tests)
  if(PHYSFX_ONNX_AVAILABLE)
    add_executable(physfx_onnx_runtime_tests semantics/tests/onnx_runtime_test.cpp)
    target_link_libraries(physfx_onnx_runtime_tests PRIVATE physfx_semantics)
    physfx_deploy_optional_runtimes(physfx_onnx_runtime_tests)
    add_test(NAME onnx_runtime_test COMMAND physfx_onnx_runtime_tests)
    set_tests_properties(onnx_runtime_test PROPERTIES LABELS "needs_onnx")
  endif()

  add_executable(physfx_editing_tests editing/tests/editing_dummy_test.cpp)
  target_link_libraries(physfx_editing_tests PRIVATE physfx_editing)
  add_test(NAME editing_dummy_test COMMAND physfx_editing_tests)

  add_executable(physfx_compositing_tests compositing/tests/compositing_dummy_test.cpp)
  target_link_libraries(physfx_compositing_tests PRIVATE physfx_compositing)
  add_test(NAME compositing_dummy_test COMMAND physfx_compositing_tests)
  add_executable(physfx_sprite_compositor_tests compositing/tests/sprite_compositor_test.cpp)
  target_link_libraries(physfx_sprite_compositor_tests PRIVATE physfx_compositing)
  add_test(NAME sprite_compositor_test COMMAND physfx_sprite_compositor_tests)

  add_executable(physfx_neural_render_tests neural_render/tests/neural_render_dummy_test.cpp)
  target_link_libraries(physfx_neural_render_tests PRIVATE physfx_neural_render)
  add_test(NAME neural_render_dummy_test COMMAND physfx_neural_render_tests)

  add_executable(physfx_pipeline_tests pipeline/tests/pipeline_dummy_test.cpp)
  target_link_libraries(physfx_pipeline_tests PRIVATE physfx_pipeline)
  physfx_deploy_optional_runtimes(physfx_pipeline_tests)
  add_test(NAME pipeline_dummy_test COMMAND physfx_pipeline_tests)

  add_executable(physfx_plugins_tests plugins/tests/plugins_dummy_test.cpp)
  target_link_libraries(physfx_plugins_tests PRIVATE physfx_plugins)
  add_test(NAME plugins_dummy_test COMMAND physfx_plugins_tests)

  add_executable(physfx_platform_tests platform/tests/platform_dummy_test.cpp)
  target_link_libraries(physfx_platform_tests PRIVATE physfx_platform)
  add_test(NAME platform_dummy_test COMMAND physfx_platform_tests)
  if(PHYSFX_FFMPEG_AVAILABLE)
    add_executable(physfx_ffmpeg_video_io_tests platform/tests/ffmpeg_video_io_test.cpp)
    target_link_libraries(physfx_ffmpeg_video_io_tests PRIVATE physfx_platform)
    physfx_deploy_optional_runtimes(physfx_ffmpeg_video_io_tests)
    add_test(NAME ffmpeg_video_io_test COMMAND physfx_ffmpeg_video_io_tests)
    set_tests_properties(ffmpeg_video_io_test PROPERTIES LABELS "needs_ffmpeg")
  endif()

  add_executable(physfx_architecture_tests tests/architecture_dependency_test.cpp)
  target_link_libraries(physfx_architecture_tests PRIVATE physfx_core)
  target_compile_definitions(physfx_architecture_tests PRIVATE
    PHYSFX_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
  add_test(NAME architecture_dependency_test COMMAND physfx_architecture_tests)
  add_executable(physfx_particle_occlusion_chain_tests
    tests/particle_occlusion_chain_test.cpp)
  target_link_libraries(physfx_particle_occlusion_chain_tests PRIVATE
    physfx_semantics physfx_physics physfx_compositing)
  add_test(NAME particle_occlusion_chain_test COMMAND physfx_particle_occlusion_chain_tests)

  add_test(NAME cli_version_test COMMAND physfx --version)
  set_tests_properties(cli_version_test PROPERTIES PASS_REGULAR_EXPRESSION "PhysFX Engine 0.3.0-dev")
  add_test(NAME cli_empty_pipeline_test
           COMMAND physfx run --config "${CMAKE_SOURCE_DIR}/examples/empty_pipeline.json")
  set_tests_properties(cli_empty_pipeline_test PROPERTIES
    PASS_REGULAR_EXPRESSION "stage=perception.*stage=semantics.*stage=editing.*stage=physics.*stage=render")
  add_test(NAME cli_semantic_pipeline_test
           COMMAND physfx run --config "${CMAKE_SOURCE_DIR}/examples/semantic_edit_pipeline.json")
  set_tests_properties(cli_semantic_pipeline_test PROPERTIES
    PASS_REGULAR_EXPRESSION "stage=perception.*stage=semantics.*stage=editing.*stage=physics.*stage=render")
  add_test(NAME cli_edit_script_test
           COMMAND physfx run --config "${CMAKE_SOURCE_DIR}/examples/edit_script_pipeline.json")
  set_tests_properties(cli_edit_script_test PROPERTIES
    PASS_REGULAR_EXPRESSION "stage=perception.*stage=semantics.*stage=editing.*stage=physics.*stage=render")

  find_package(Python 3.11 COMPONENTS Interpreter QUIET)
  if(Python_Interpreter_FOUND)
    add_test(NAME copyright_header_test
      COMMAND "${Python_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/check_copyright.py"
              --root "${CMAKE_SOURCE_DIR}")
    if(PHYSFX_ONNX_AVAILABLE AND
       NOT (EXISTS "${CMAKE_SOURCE_DIR}/models/sam.onnx" AND
            EXISTS "${CMAKE_SOURCE_DIR}/models/xmem.onnx"))
      add_test(NAME onnx_model_smoke_test
        COMMAND "${CMAKE_COMMAND}" -E echo "SKIP: 未发现经校验的 SAM/XMem 权重")
      set_tests_properties(onnx_model_smoke_test PROPERTIES
        LABELS "needs_models" SKIP_REGULAR_EXPRESSION "^SKIP:")
    endif()
    if(PHYSFX_FFMPEG_AVAILABLE)
      find_program(PHYSFX_FFMPEG_EXECUTABLE NAMES ffmpeg
        PATHS "${PHYSFX_FFMPEG_ROOT}/bin" NO_DEFAULT_PATH)
      find_program(PHYSFX_FFPROBE_EXECUTABLE NAMES ffprobe
        PATHS "${PHYSFX_FFMPEG_ROOT}/bin" NO_DEFAULT_PATH)
      if(PHYSFX_FFMPEG_EXECUTABLE AND PHYSFX_FFPROBE_EXECUTABLE)
        add_test(NAME particle_occlusion_demo_smoke_test
          COMMAND "${Python_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/run_demo_smoke.py"
            --ffmpeg "${PHYSFX_FFMPEG_EXECUTABLE}"
            --ffprobe "${PHYSFX_FFPROBE_EXECUTABLE}"
            --physfx "$<TARGET_FILE:physfx>"
            --config "${CMAKE_SOURCE_DIR}/examples/particle_occlusion_demo.json"
            --root "${CMAKE_SOURCE_DIR}")
        set_tests_properties(particle_occlusion_demo_smoke_test PROPERTIES LABELS "needs_ffmpeg")
      else()
        message(WARNING "FFmpeg 开发包缺少 ffmpeg/ffprobe，跳过端到端视频冒烟测试")
      endif()
    endif()
  else()
    message(STATUS "未找到 Python 3.11；跳过版权头检查测试")
  endif()

  if(GTest_FOUND)
    add_executable(physfx_gtest_smoke tests/google_test_smoke.cpp)
    target_link_libraries(physfx_gtest_smoke PRIVATE GTest::gtest_main physfx_core)
    include(GoogleTest)
    gtest_discover_tests(physfx_gtest_smoke)
  else()
    message(STATUS "未找到 GoogleTest；使用无第三方依赖的 CTest 桩测试")
  endif()
endif()

if(PHYSFX_BUILD_PYTHON)
  add_subdirectory(bindings/python)
endif()
