#include "lve_renderer.h"
#include "glm/gtc/constants.hpp"
#include <array>
#include <stdexcept>

namespace lve {

LveRenderer::LveRenderer(LveWindow& window, LveDevice& device)
    : lveWindow{window}, lveDevice{device} {
  recreateSwapchain();
  createCommandBuffers();
}

LveRenderer::~LveRenderer() { freeCommandBuffers(); }

void LveRenderer::recreateSwapchain() {
  auto extent = lveWindow.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = lveWindow.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(lveDevice.device());

  if (lveSwapchain == nullptr) {
    lveSwapchain.reset(nullptr);
    lveSwapchain = std::make_unique<LveSwapchain>(lveDevice, extent);
  } else {
    std::shared_ptr<LveSwapchain> oldSwapchain = std::move(lveSwapchain);
    lveSwapchain =
        std::make_unique<LveSwapchain>(lveDevice, extent, oldSwapchain);

    if (!oldSwapchain->compareSwapFormats(*lveSwapchain)) {
      throw std::runtime_error("Swap chain image or depth format has changed!");
    }
  }
}

void LveRenderer::createCommandBuffers() {
  commandBuffers.resize(LveSwapchain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = lveDevice.getCommandPool();
  allocInfo.commandBufferCount = commandBuffers.size();

  if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void LveRenderer::freeCommandBuffers() {
  vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(),
                       commandBuffers.size(), commandBuffers.data());
  commandBuffers.clear();
}

VkCommandBuffer LveRenderer::beginFrame() {
  assert(!isFrameStarted && "Can't call beginFrame while already in progress");
  auto result = lveSwapchain->acquireNextImage(&currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapchain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
  return commandBuffer;
}

void LveRenderer::endFrame() {
  assert(isFrameStarted &&
         "Can't call endFrame while frame is not in progress");

  auto commandBuffer = getCurrentCommandBuffer();

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }

  auto result =
      lveSwapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      lveWindow.wasWindowResized()) {
    lveWindow.resetWindowResizedFlag();
    recreateSwapchain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  isFrameStarted = false;
  currentFrameIndex =
      (currentFrameIndex + 1) % LveSwapchain::MAX_FRAMES_IN_FLIGHT;
}

void LveRenderer::beginSwapchainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted &&
         "Can't call beginSwapchainRenderPass if frame is not in progress");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Can't begin render pass on command buffer from a different frame");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = lveSwapchain->getRenderPass();
  renderPassInfo.framebuffer = lveSwapchain->getFrameBuffer(currentImageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = lveSwapchain->getSwapchainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.1, 0.1, 0.1, 1.0};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = clearValues.size();
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(lveSwapchain->getSwapchainExtent().width);
  viewport.height =
      static_cast<float>(lveSwapchain->getSwapchainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, lveSwapchain->getSwapchainExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void LveRenderer::endSwapchainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted &&
         "Can't call beginSwapchainRenderPass if frame is not in progress");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Can't end render pass on command buffer from a different frame");

  vkCmdEndRenderPass(commandBuffer);
}

} // namespace lve
