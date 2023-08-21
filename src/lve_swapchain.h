#pragma once

#include "lve_device.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace lve {

    class LveSwapchain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        LveSwapchain(LveDevice& deviceRef, VkExtent2D windowExtent);

        LveSwapchain(LveDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<LveSwapchain> previous);

        ~LveSwapchain();

        LveSwapchain(const LveSwapchain&) = delete;

        LveSwapchain& operator=(const LveSwapchain&) = delete;

        VkFramebuffer getFrameBuffer(uint32_t index) { return swapChainFramebuffers[index]; }

        VkRenderPass getRenderPass() { return renderPass; }

        VkImageView getImageView(int index) { return swapChainImageViews[index]; }

        size_t imageCount() { return swapChainImages.size(); }

        VkFormat getSwapchainImageFormat() { return swapChainImageFormat; }

        VkExtent2D getSwapchainExtent() { return swapChainExtent; }

        [[nodiscard]] uint32_t width() const { return swapChainExtent.width; }

        [[nodiscard]] uint32_t height() const { return swapChainExtent.height; }

        [[nodiscard]] float extentAspectRatio() const {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }

        VkFormat findDepthFormat();

        VkResult acquireNextImage(uint32_t* imageIndex);

        VkResult submitCommandBuffers(const VkCommandBuffer* buffers, const uint32_t* imageIndex);

        [[nodiscard]]
        bool compareSwapFormats(const LveSwapchain& swapchain) const {
            return swapchain.swapchainDepthFormat == swapchainDepthFormat &&
                   swapchain.swapChainImageFormat == swapChainImageFormat;
        }

    private:
        void init();

        void createSwapchain();

        void createImageViews();

        void createDepthResources();

        void createRenderPass();

        void createFramebuffers();

        void createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
                const std::vector<VkSurfaceFormatKHR>& availableFormats);

        VkPresentModeKHR chooseSwapPresentMode(
                const std::vector<VkPresentModeKHR>& availablePresentModes);

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        VkFormat swapChainImageFormat;
        VkFormat swapchainDepthFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;

        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        LveDevice& device;
        VkExtent2D windowExtent;

        VkSwapchainKHR swapChain;
        std::shared_ptr<LveSwapchain> oldSwapchain;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
    };

}  // namespace lve
