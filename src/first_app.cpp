#include <stdexcept>
#include <array>
#include "first_app.h"
#include "fmt/core.h"

namespace lve {
    FirstApp::FirstApp() {
        loadModels();
        createPipelineLayout();
        createPipeline();
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
        PipelineConfigInfo pipelineConfig{};
        LvePipeline::defaultPipelineConfigInfo(pipelineConfig, lveSwapchain.width(),
                                               lveSwapchain.height());
        pipelineConfig.renderPass = lveSwapchain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        lvePipeline = std::make_unique<LvePipeline>(
                lveDevice,
                "./shaders/simple_shader.vert.spv",
                "./shaders/simple_shader.frag.spv",
                pipelineConfig);
    }

    void FirstApp::createCommandBuffers() {
        commandBuffers.resize(lveSwapchain.imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = lveDevice.getCommandPool();
        allocInfo.commandBufferCount = commandBuffers.size();

        if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (int i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = lveSwapchain.getRenderPass();
            renderPassInfo.framebuffer = lveSwapchain.getFrameBuffer(i);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = lveSwapchain.getSwapchainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1, 0.1, 0.1, 1.0};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = clearValues.size();
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            lvePipeline->bind(commandBuffers[i]);
            lveModel->bind(commandBuffers[i]);
            lveModel->draw(commandBuffers[i]);

            vkCmdEndRenderPass(commandBuffers[i]);

            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }

    void FirstApp::drawFrame() {
        uint32_t imageIndex;
        auto result = lveSwapchain.acquireNextImage(&imageIndex);

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        result = lveSwapchain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }

    std::vector<LveModel::Vertex> iterateSierpinski(std::vector<LveModel::Vertex> originalTriangle) {
        auto v1 = originalTriangle[0];
        auto v2 = originalTriangle[1];
        auto v3 = originalTriangle[2];

        auto v12 = LveModel::Vertex{(v1.position + v2.position) / 2.0f};
        auto v23 = LveModel::Vertex{(v2.position + v3.position) / 2.0f};
        auto v31 = LveModel::Vertex{(v3.position + v1.position) / 2.0f};

        return {v1, v12, v31,
                v12, v2, v23,
                v31, v23, v3};
    }

    void FirstApp::loadModels() {
        std::vector<LveModel::Vertex> vertices{
                {{0.0f,  -0.5f}},
                {{0.5f,  0.5f}},
                {{-0.5f, 0.5f}},
        };

        for (int i = 0; i < 9; i++) {
            std::vector<LveModel::Vertex> newVertices{};
            for (int j = 0; j < vertices.size(); j += 3) {
                const auto iteration = iterateSierpinski({vertices[j], vertices[j + 1], vertices[j + 2]});
                newVertices.insert(newVertices.end(), iteration.begin(), iteration.end());
            }

            vertices = newVertices;
        }


        lveModel = std::make_unique<LveModel>(lveDevice, vertices);
    }
}
