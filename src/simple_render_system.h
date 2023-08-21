#pragma once

#include <memory>
#include "lve_window.h"
#include "lve_pipeline.h"
#include "lve_model.h"
#include "lve_game_object.h"
#include "lve_renderer.h"

namespace lve {
    class SimpleRenderSystem {
    public:
        SimpleRenderSystem(LveDevice& device, VkRenderPass renderPass);

        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem&) = delete;

        SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;


        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& gameObjects);

    private:

        void createPipelineLayout();

        void createPipeline(VkRenderPass renderPass);


        LveDevice& lveDevice;

        std::unique_ptr<LvePipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
    };
}
