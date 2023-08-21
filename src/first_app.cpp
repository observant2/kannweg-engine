#include <stdexcept>
#include <array>
#include "first_app.h"
#include "glm/gtc/constants.hpp"
#include "simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

namespace lve {
    FirstApp::FirstApp() {
        loadGameObjects();
    }

    FirstApp::~FirstApp() = default;

    void FirstApp::run() {
        SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapchainRenderPass()};

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            if (auto commandBuffer = lveRenderer.beginFrame()) {
                lveRenderer.beginSwapchainRenderPass(commandBuffer);

                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);

                lveRenderer.endSwapchainRenderPass(commandBuffer);

                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    void FirstApp::loadGameObjects() {
        std::vector<LveModel::Vertex> vertices{
                {{0.0f,  -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f,  0.5f},  {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f},  {0.0f, 0.0f, 1.0f}},
        };

        auto lveModel = std::make_shared<LveModel>(lveDevice, vertices);

        auto triangle = LveGameObject::createGameObject();
        triangle.model = lveModel;
        triangle.color = {0.1f, 0.8f, 0.1f};
        triangle.transform2d.translation.x = 0.2f;
        triangle.transform2d.scale = {2.f, 0.5};
        triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
    }
}
