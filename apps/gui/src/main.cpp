/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "EditorModel.h"
#include "Project.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace {

constexpr int kPreviewWidth = 1280;
constexpr int kPreviewHeight = 720;
constexpr GLint kClampToEdge = 0x812F;

struct EditorState {
  physfx::gui::Project project{};
  physfx::gui::PreviewWindow preview{};
  physfx::gui::Timeline timeline{};
  physfx::gui::CommandStackPanel commands{};
  physfx::gui::PreviewScheduler previewScheduler{};
  bool english{false};
  std::array<char, 512> inputPath{};
  std::array<char, 512> outputPath{};
  std::string status{"就绪"};
};

void glfwError(int, const char* description) { std::cerr << "GLFW: " << description << '\n'; }

void loadChineseFont(ImGuiIO& io) {
  const std::array<const char*, 5> candidates{
      "C:/Windows/Fonts/msyh.ttc",
      "C:/Windows/Fonts/simhei.ttf",
      "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
      "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
      "/System/Library/Fonts/PingFang.ttc",
  };
  for (const char* path : candidates) {
    if (std::filesystem::exists(path)) {
      io.Fonts->AddFontFromFileTTF(path, 18.0F, nullptr, io.Fonts->GetGlyphRangesChineseFull());
      return;
    }
  }
  io.Fonts->AddFontDefault();
}

GLuint createPreviewTexture() {
  GLuint texture = 0;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, kClampToEdge);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, kClampToEdge);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, kPreviewWidth, kPreviewHeight, 0, GL_RGB, GL_UNSIGNED_BYTE,
               nullptr);
  return texture;
}

void updatePreviewTexture(GLuint texture, std::uint64_t frame) {
  static std::vector<std::uint8_t> pixels(static_cast<std::size_t>(kPreviewWidth) * kPreviewHeight *
                                          3U);
  const float phase = static_cast<float>(frame % 240U) / 240.0F;
  for (int y = 0; y < kPreviewHeight; ++y) {
    for (int x = 0; x < kPreviewWidth; ++x) {
      const auto offset = (static_cast<std::size_t>(y) * kPreviewWidth + x) * 3U;
      const float horizon = static_cast<float>(y) / kPreviewHeight;
      const int grid = ((x / 80) + (y / 80)) % 2;
      pixels[offset] = static_cast<std::uint8_t>(30 + 70 * (1.0F - horizon) + 15 * grid);
      pixels[offset + 1U] = static_cast<std::uint8_t>(55 + 100 * (1.0F - horizon));
      pixels[offset + 2U] = static_cast<std::uint8_t>(85 + 95 * (1.0F - horizon));
      const float cx = kPreviewWidth * (0.25F + phase * 0.5F);
      const float cy = kPreviewHeight * 0.62F;
      if (std::hypot(static_cast<float>(x) - cx, static_cast<float>(y) - cy) < 72.0F) {
        pixels[offset] = 230;
        pixels[offset + 1U] = 115;
        pixels[offset + 2U] = 54;
      }
    }
  }
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kPreviewWidth, kPreviewHeight, GL_RGB, GL_UNSIGNED_BYTE,
                  pixels.data());
}

physfx::gui::Project activeProject(const EditorState& state) {
  auto project = state.project;
  const auto editing = std::find_if(project.nodes.begin(), project.nodes.end(),
                                    [](const auto& node) { return node.type == "editing"; });
  if (editing != project.nodes.end()) {
    editing->commandJson.resize(std::min(state.commands.cursor(), editing->commandJson.size()));
  }
  return project;
}

void addCommand(EditorState& state, std::string command) {
  auto editing = std::find_if(state.project.nodes.begin(), state.project.nodes.end(),
                              [](const auto& node) { return node.type == "editing"; });
  if (editing == state.project.nodes.end()) {
    state.project.nodes.push_back({"edit", "editing", "编辑命令", {}});
    editing = std::prev(state.project.nodes.end());
  }
  if (state.commands.cursor() < editing->commandJson.size()) {
    editing->commandJson.resize(state.commands.cursor());
  }
  state.commands.record(command);
  editing->commandJson.push_back(std::move(command));
  state.status = "命令已加入，可撤销或导出";
  state.previewScheduler.markDirty();
}

bool exportScript(EditorState& state) {
  state.project.input = state.inputPath.data();
  state.project.output = state.outputPath.data();
  const auto project = activeProject(state);
  const std::filesystem::path output = state.outputPath.data();
  if (output.empty()) {
    state.status = "请先填写导出路径";
    return false;
  }
  std::ofstream stream(output, std::ios::binary);
  if (!stream) {
    state.status = "无法创建导出文件";
    return false;
  }
  stream << physfx::gui::ProjectCompiler::compileEditScript(project);
  state.status = stream ? "编辑脚本已导出" : "写入导出文件失败";
  return static_cast<bool>(stream);
}

void drawNodeGraph(const EditorState& state) {
  ImGui::TextUnformatted("节点图");
  ImGui::Separator();
  const ImVec2 origin = ImGui::GetCursorScreenPos();
  const ImVec2 available = ImGui::GetContentRegionAvail();
  ImGui::InvisibleButton("node-canvas", available);
  ImDrawList* draw = ImGui::GetWindowDrawList();
  draw->AddRectFilled(origin, ImVec2(origin.x + available.x, origin.y + available.y),
                      IM_COL32(25, 28, 32, 255));
  float x = origin.x + 24.0F;
  const float y = origin.y + 45.0F;
  for (std::size_t index = 0; index < state.project.nodes.size(); ++index) {
    const auto& node = state.project.nodes[index];
    const ImVec2 min(x, y);
    const ImVec2 max(x + 150.0F, y + 72.0F);
    if (index > 0U) {
      draw->AddLine(ImVec2(x - 42.0F, y + 36.0F), ImVec2(x, y + 36.0F), IM_COL32(84, 174, 134, 255),
                    3.0F);
    }
    draw->AddRectFilled(min, max, IM_COL32(47, 54, 62, 255), 4.0F);
    draw->AddRect(min, max, IM_COL32(84, 174, 134, 255), 4.0F, 0, 2.0F);
    draw->AddText(ImVec2(x + 12.0F, y + 12.0F), IM_COL32_WHITE, node.label.c_str());
    const std::string detail = std::to_string(node.commandJson.size()) + " 条命令";
    draw->AddText(ImVec2(x + 12.0F, y + 42.0F), IM_COL32(180, 188, 194, 255), detail.c_str());
    x += 192.0F;
  }
}

void drawEditor(EditorState& state, GLuint previewTexture) {
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_NoBringToFrontOnFocus;
  ImGui::Begin("PhysFX", nullptr, flags);

  if (ImGui::Button(state.english ? "中文" : "English")) state.english = !state.english;
  ImGui::SameLine();
  ImGui::TextUnformatted("PhysFX Engine");
  ImGui::SameLine();
  ImGui::TextDisabled("0.6.0 | 本地编辑器");
  ImGui::Separator();
  ImGui::SetNextItemWidth(300.0F);
  ImGui::InputTextWithHint("##input", "输入视频路径", state.inputPath.data(),
                           state.inputPath.size());
  ImGui::SameLine();
  ImGui::SetNextItemWidth(300.0F);
  ImGui::InputTextWithHint("##output", "导出脚本路径", state.outputPath.data(),
                           state.outputPath.size());
  ImGui::SameLine();
  if (ImGui::Button("导出")) exportScript(state);
  ImGui::SameLine();
  ImGui::TextDisabled("%s", state.status.c_str());

  const float footer = 164.0F;
  const float sidebar = 285.0F;
  ImGui::BeginChild("preview", ImVec2(-sidebar - 8.0F, -footer), true);
  ImGui::Text("%s  %.1f fps%s", state.english ? "Preview" : "预览", ImGui::GetIO().Framerate,
              state.previewScheduler.quality() == physfx::gui::PreviewQuality::kQuickHalfResolution
                  ? (state.english ? "  quick" : "  快速半分辨率")
                  : "");
  const ImVec2 available = ImGui::GetContentRegionAvail();
  const float scale = std::min(available.x / kPreviewWidth, available.y / kPreviewHeight);
  const ImVec2 size(kPreviewWidth * scale, kPreviewHeight * scale);
  ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(previewTexture)), size,
               ImVec2(0, 1), ImVec2(1, 0));
  if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 mouse = ImGui::GetMousePos();
    state.preview.click(static_cast<std::uint32_t>((mouse.x - min.x) / scale),
                        static_cast<std::uint32_t>((mouse.y - min.y) / scale));
    state.status = "已选择预览对象";
  }
  if (ImGui::BeginPopupContextItem("preview-context")) {
    ImGui::Text("选点：%u, %u", state.preview.selectedX(), state.preview.selectedY());
    if (ImGui::MenuItem("移除对象")) addCommand(state, R"({"type":"remove_object"})");
    if (ImGui::MenuItem("向右移动")) {
      addCommand(state, R"({"type":"move_object","dx":40,"dy":0})");
    }
    if (ImGui::MenuItem("替换为红色材质")) {
      addCommand(state, R"({"type":"replace_appearance","material":"red"})");
    }
    ImGui::EndPopup();
  }
  if (state.preview.hasSelection()) {
    const ImVec2 min = ImGui::GetItemRectMin();
    const float x = min.x + state.preview.selectedX() * scale;
    const float y = min.y + state.preview.selectedY() * scale;
    ImGui::GetWindowDrawList()->AddCircle(ImVec2(x, y), 9.0F, IM_COL32(255, 235, 90, 255), 0, 3.0F);
  }
  ImGui::EndChild();

  ImGui::BeginChild("curves", ImVec2(0.0F, 132.0F), true);
  ImGui::TextUnformatted(state.english ? "Keyframe curves" : "关键帧曲线");
  if (state.project.curves.empty()) {
    state.project.curves.push_back({"intensity", "linear", {{0U, 0.0F}, {299U, 1.0F}}});
  }
  auto& curve = state.project.curves.front();
  ImGui::SameLine();
  const char* modes[] = {"linear", "smooth"};
  int mode = curve.interpolation == "smooth" ? 1 : 0;
  if (ImGui::Combo("##curve-mode", &mode, modes, 2)) {
    curve.interpolation = modes[mode];
    state.previewScheduler.markDirty();
  }
  float values[2] = {curve.keyframes[0].value, curve.keyframes[1].value};
  if (ImGui::SliderFloat2("##curve-values", values, 0.0F, 1.0F)) {
    curve.keyframes[0].value = values[0];
    curve.keyframes[1].value = values[1];
    state.previewScheduler.markDirty();
  }
  if (state.previewScheduler.dirty()) {
    ImGui::SameLine();
    if (ImGui::Button(state.english ? "Render full" : "全量渲染"))
      state.previewScheduler.confirmFullRender();
  }
  ImGui::EndChild();

  ImGui::SameLine();
  ImGui::BeginChild("commands", ImVec2(sidebar, -footer), true);
  ImGui::TextUnformatted("命令栈");
  if (ImGui::Button("撤销")) state.commands.undo();
  ImGui::SameLine();
  if (ImGui::Button("重做")) state.commands.redo();
  ImGui::Separator();
  for (std::size_t index = 0; index < state.commands.history().size(); ++index) {
    const bool active = index < state.commands.cursor();
    ImGui::TextColored(active ? ImVec4(0.55F, 0.9F, 0.7F, 1.0F) : ImVec4(0.5F, 0.5F, 0.5F, 1.0F),
                       "%02zu  %s", index + 1U, state.commands.history()[index].c_str());
  }
  ImGui::EndChild();

  ImGui::BeginChild("timeline", ImVec2(0.0F, 54.0F), true);
  if (ImGui::Button(state.timeline.playing() ? "暂停" : "播放")) {
    state.timeline.playing() ? state.timeline.pause() : state.timeline.play();
  }
  ImGui::SameLine();
  int frame = static_cast<int>(state.timeline.currentFrame());
  ImGui::SetNextItemWidth(-1.0F);
  if (ImGui::SliderInt("##timeline", &frame, 0, 299)) state.timeline.scrub(frame);
  ImGui::EndChild();
  ImGui::BeginChild("nodes", ImVec2(0.0F, 0.0F), true);
  drawNodeGraph(state);
  ImGui::EndChild();
  ImGui::End();
}

int selfTest() {
  EditorState state{};
  state.project.input = "input.mp4";
  state.project.output = "output.mp4";
  state.project.nodes.push_back({"input", "source", "输入", {}});
  state.project.nodes.push_back({"edit", "editing", "编辑", {R"({"type":"empty"})"}});
  const auto script = physfx::gui::ProjectCompiler::compileEditScript(state.project);
  if (script != R"({"version":1,"commands":[{"type":"empty"}]})") return 1;
  std::cout << "GUI_SELF_TEST=PASS\n";
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc > 1 && std::string(argv[1]) == "--self-test") return selfTest();
  const bool benchmark = argc > 1 && std::string(argv[1]) == "--benchmark";

  glfwSetErrorCallback(glfwError);
  if (glfwInit() == GLFW_FALSE) return 1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  if (benchmark) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(1440, 900, "PhysFX Engine", nullptr, nullptr);
  if (window == nullptr) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(benchmark ? 0 : 1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  loadChineseFont(io);
  ImGui::StyleColorsDark();
  ImGui::GetStyle().WindowRounding = 0.0F;
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  EditorState state{};
  state.preview.setFrameSize(kPreviewWidth, kPreviewHeight);
  state.timeline.setTotalFrames(300);
  state.project.nodes.push_back({"input", "source", "输入视频", {}});
  state.project.nodes.push_back({"edit", "editing", "编辑命令", {}});
  state.project.nodes.push_back({"render", "output", "导出", {}});
  std::snprintf(state.outputPath.data(), state.outputPath.size(), "%s", "physfx-edit.json");
  const GLuint previewTexture = createPreviewTexture();
  auto previous = std::chrono::steady_clock::now();
  const auto benchmarkStarted = previous;
  std::uint64_t uploadedFrame = std::numeric_limits<std::uint64_t>::max();
  int benchmarkFrames = 0;

  while (glfwWindowShouldClose(window) == GLFW_FALSE) {
    glfwPollEvents();
    const auto now = std::chrono::steady_clock::now();
    if (benchmark) {
      state.timeline.stepForward();
    } else if (state.timeline.playing() && now - previous >= std::chrono::milliseconds(33)) {
      state.timeline.stepForward();
      previous = now;
    }
    if (uploadedFrame != state.timeline.currentFrame()) {
      updatePreviewTexture(previewTexture, state.timeline.currentFrame());
      uploadedFrame = state.timeline.currentFrame();
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    drawEditor(state, previewTexture);
    ImGui::Render();
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.08F, 0.09F, 0.10F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    if (benchmark && ++benchmarkFrames >= 120) glfwSetWindowShouldClose(window, GLFW_TRUE);
  }

  if (benchmark) {
    const double seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - benchmarkStarted).count();
    std::cout << "GUI_BENCHMARK_FPS=" << benchmarkFrames / seconds << " frames=" << benchmarkFrames
              << " size=1280x720\n";
  }

  glDeleteTextures(1, &previewTexture);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
