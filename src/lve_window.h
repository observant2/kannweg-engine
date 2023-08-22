#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <string>

namespace lve {
class LveWindow {
public:
  LveWindow(int w, int h, std::string name);

  ~LveWindow();

  LveWindow(const LveWindow&) = delete;

  LveWindow& operator=(const LveWindow&) = delete;

  bool shouldClose() { return glfwWindowShouldClose(window); }

  [[nodiscard]] VkExtent2D getExtent() const {
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
  }

  void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

  void resetWindowResizedFlag() { framebufferResized = false; }

  [[nodiscard]] bool wasWindowResized() const { return framebufferResized; }

  [[nodiscard]] GLFWwindow* getGLFWwindow() const { return window; }

private:
  static void framebufferResizeCallback(GLFWwindow* window, int width,
                                        int height);

  void initWindow();

  int width;
  int height;
  bool framebufferResized = false;

  std::string windowName;
  GLFWwindow* window;
};
} // namespace lve
