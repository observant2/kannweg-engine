#pragma once

#include <memory>
#include "lve_window.h"
#include "lve_pipeline.h"
#include "lve_swapchain.h"

namespace lve {
    class FirstApp {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        FirstApp();

        ~FirstApp();

        FirstApp(const FirstApp&) = delete;

        FirstApp& operator=(const FirstApp&) = delete;

        void run();

    private:
        void createPipelineLayout();

        void createPipeline();

        void createCommandBuffers();

        void drawFrame();


        LveWindow lveWindow{WIDTH, HEIGHT, "engine"};
        LveDevice lveDevice{lveWindow};
        LveSwapchain lveSwapchain{lveDevice, lveWindow.getExtent()};
        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
    };
}
