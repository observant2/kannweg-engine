#pragma once

#include <memory>
#include "lve_window.h"
#include "lve_pipeline.h"
#include "lve_swapchain.h"
#include "lve_model.h"

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
        void loadModels();

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
        std::unique_ptr<LveModel> lveModel;
    };
}
