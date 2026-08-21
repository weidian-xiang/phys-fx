/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "physfx/core/Version.h"

#if defined(PHYSFX_SHARED)
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(PHYSFX_BUILDING_LIBRARY)
#define PHYSFX_API __declspec(dllexport)
#else
#define PHYSFX_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) || defined(__clang__)
#define PHYSFX_API __attribute__((visibility("default")))
#else
#define PHYSFX_API
#endif
#else
#define PHYSFX_API
#endif

#define PHYSFX_DEPRECATED(message) [[deprecated(message)]]
