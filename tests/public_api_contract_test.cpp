/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 * SPDX-License-Identifier: Apache-2.0
 */

#include <type_traits>

#include "physfx/cloud/IRenderServiceClient.h"
#include "physfx/compositing/ICompositor.h"
#include "physfx/core/Config.h"
#include "physfx/editing/IEditCommand.h"
#include "physfx/neural_render/INeuralRenderer.h"
#include "physfx/perception/IDepthEstimator.h"
#include "physfx/physics/IPhysicsSimulator.h"
#include "physfx/pipeline/Pipeline.h"
#include "physfx/platform/IVideoReader.h"
#include "physfx/plugins/IEffectPlugin.h"
#include "physfx/semantics/ISegmenter.h"

static_assert(std::is_same_v<physfx::v1::core::Config, physfx::core::Config>);
static_assert(
    std::is_same_v<physfx::v1::cloud::IRenderServiceClient, physfx::cloud::IRenderServiceClient>);
static_assert(
    std::is_same_v<physfx::v1::compositing::ICompositor, physfx::compositing::ICompositor>);
static_assert(std::is_same_v<physfx::v1::editing::IEditCommand, physfx::editing::IEditCommand>);
static_assert(std::is_same_v<physfx::v1::neural_render::INeuralRenderer,
                             physfx::neural_render::INeuralRenderer>);
static_assert(
    std::is_same_v<physfx::v1::perception::IDepthEstimator, physfx::perception::IDepthEstimator>);
static_assert(
    std::is_same_v<physfx::v1::physics::IPhysicsSimulator, physfx::physics::IPhysicsSimulator>);
static_assert(std::is_same_v<physfx::v1::pipeline::Pipeline, physfx::pipeline::Pipeline>);
static_assert(std::is_same_v<physfx::v1::platform::IVideoReader, physfx::platform::IVideoReader>);
static_assert(std::is_same_v<physfx::v1::plugins::IEffectPlugin, physfx::plugins::IEffectPlugin>);
static_assert(std::is_same_v<physfx::v1::semantics::ISegmenter, physfx::semantics::ISegmenter>);

int main() {
  static_assert(physfx::kApiMajor == 1);
  return 0;
}
