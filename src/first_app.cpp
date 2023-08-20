#include <stdexcept>
#include <array>
#include "first_app.h"

namespace lve {
    FirstApp::FirstApp() {
        loadModels();
        createPipelineLayout();
        recreateSwapchain();
        createCommandBuffers();
    }

    FirstApp::~FirstApp() {
        vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
    }

    void FirstApp::run() {
        while (!lveWindow.shouldClose()) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    void FirstApp::createPipelineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void FirstApp::createPipeline() {
        assert(lveSwapchain != nullptr && "Cannot create pipeline before swap chain");
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = lveSwapchain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LvePipeline>(
                lveDevice,
                "./shaders/simple_shader.vert.spv",
                "./shaders/simple_shader.frag.spv",
                pipelineConfig);
    }

    void FirstApp::recreateSwapchain() {
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
            lveSwapchain = std::make_unique<LveSwapchain>(lveDevice, extent, std::move(lveSwapchain));
            if (lveSwapchain->imageCount() != commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }

        // if render pass compatible do nothing else
        createPipeline();
    }

    void FirstApp::createCommandBuffers() {
        commandBuffers.resize(lveSwapchain->imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = lveDevice.getCommandPool();
        allocInfo.commandBufferCount = commandBuffers.size();

        if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void FirstApp::freeCommandBuffers() {
        vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(), commandBuffers.size(),
                             commandBuffers.data());
        commandBuffers.clear();
    }

    void FirstApp::drawFrame() {
        uint32_t imageIndex;
        auto result = lveSwapchain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapchain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        recordCommandBuffer(imageIndex);
        result = lveSwapchain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
            lveWindow.resetWindowResizedFlag();
            recreateSwapchain();
            return;
        }

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }

    void FirstApp::loadModels() {
        std::vector<LveModel::Vertex> vertices{
                {{0.0f,  -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f,  0.5f},  {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f},  {0.0f, 0.0f, 1.0f}},
        };

        lveModel = std::make_unique<LveModel>(lveDevice, vertices);
    }

    void FirstApp::recordCommandBuffer(uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = lveSwapchain->getRenderPass();
        renderPassInfo.framebuffer = lveSwapchain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = lveSwapchain->getSwapchainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1, 0.1, 0.1, 1.0};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = clearValues.size();
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(lveSwapchain->getSwapchainExtent().width);
        viewport.height = static_cast<float>(lveSwapchain->getSwapchainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, lveSwapchain->getSwapchainExtent()};
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

        lvePipeline->bind(commandBuffers[imageIndex]);
        lveModel->bind(commandBuffers[imageIndex]);
        lveModel->draw(commandBuffers[imageIndex]);

        vkCmdEndRenderPass(commandBuffers[imageIndex]);

        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

}
