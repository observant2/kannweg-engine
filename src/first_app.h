#pragma once

#include <memory>
#include "lve_window.h"
#include "lve_pipeline.h"
#include "lve_swapchain.h"
#include "lve_model.h"
#include "lve_game_object.h"

namespace lve {
    class FirstApp {
    public:
        static constexpr int WIDTH = 1800;
        static constexpr int HEIGHT = 1800;

        FirstApp();

        ~FirstApp();

        FirstApp(const FirstApp&) = delete;

        FirstApp& operator=(const FirstApp&) = delete;

        void run();

    private:
        void loadGameObjects();

        void createPipelineLayout();

        void createPipeline();

        void createCommandBuffers();

        void freeCommandBuffers();

        void drawFrame();

        void recreateSwapchain();

        void recordCommandBuffer(uint32_t imageIndex);

        void renderGameObjects(VkCommandBuffer commandBuffer);

        LveWindow lveWindow{WIDTH, HEIGHT, "engine"};
        LveDevice lveDevice{lveWindow};
        std::unique_ptr<LveSwapchain> lveSwapchain;
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<LveGameObject> gameObjects{};
    };
}
