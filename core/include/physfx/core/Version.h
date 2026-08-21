/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string_view>

namespace physfx {

namespace cloud {}
namespace compositing {}
namespace core {}
namespace editing {}
namespace neural_render {}
namespace perception {}
namespace physics {}
namespace pipeline {}
namespace platform {}
namespace plugins {}
namespace semantics {}

/** @brief PhysFX 1.x 的稳定源码命名入口。 */
namespace v1 {
namespace cloud = ::physfx::cloud;
namespace compositing = ::physfx::compositing;
namespace core = ::physfx::core;
namespace editing = ::physfx::editing;
namespace neural_render = ::physfx::neural_render;
namespace perception = ::physfx::perception;
namespace physics = ::physfx::physics;
namespace pipeline = ::physfx::pipeline;
namespace platform = ::physfx::platform;
namespace plugins = ::physfx::plugins;
namespace semantics = ::physfx::semantics;
}  // namespace v1

inline constexpr unsigned int kApiMajor = 1;
inline constexpr std::string_view kApiNamespace = "v1";

}  // namespace physfx
