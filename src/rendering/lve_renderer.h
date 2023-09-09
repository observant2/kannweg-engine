#pragma once

#include "lve_swapchain.h"
#include "lve_window.h"
#include <cassert>
#include <memory>

namespace lve {
class LveRenderer {
public:
  LveRenderer(LveWindow& window, LveDevice& device);

  ~LveRenderer();

  LveRenderer(const LveRenderer&) = delete;

  LveRenderer& operator=(const LveRenderer&) = delete;

  [[nodiscard]] VkRenderPass getSwapchainRenderPass() const {
    return lveSwapchain->getRenderPass();
  }

  [[nodiscard]] float getAspectRatio() const { return lveSwapchain->extentAspectRatio(); }

  [[nodiscard]] bool isFrameInProgress() const { return isFrameStarted; };

  [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
    return commandBuffers[currentFrameIndex];
  }

  [[nodiscard]] int getFrameIndex() const {
    assert(isFrameStarted && "Cannot get frame index when frame not in progress");
    return currentFrameIndex;
  }

  VkCommandBuffer beginFrame();

  void endFrame();

  void beginSwapchainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapchainRenderPass(VkCommandBuffer commandBuffer);

private:
  void createCommandBuffers();

  void freeCommandBuffers();

  void recreateSwapchain();

  LveWindow& lveWindow;
  LveDevice& lveDevice;
  std::unique_ptr<LveSwapchain> lveSwapchain;
  std::vector<VkCommandBuffer> commandBuffers;

  uint32_t currentImageIndex{};
  int currentFrameIndex{};
  bool isFrameStarted{false};
};
} // namespace lve
