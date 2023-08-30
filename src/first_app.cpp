#include "first_app.h"
#include "fmt/core.h"
#include "lve_camera.h"
#include "movement_controller.h"
#include "simple_render_system.h"
#include <array>
#include <chrono>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "lve_buffer.h"
#include <glm/glm.hpp>

namespace lve {
struct GlobalUbo {
  glm::mat4 projectionView{1.f};
  glm::vec3 lightDirection = glm::normalize(glm::vec3(1.f, -3.f, -1.f));
};

FirstApp::FirstApp() { loadGameObjects(); }

FirstApp::~FirstApp() = default;

void FirstApp::run() {
  std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapchain::MAX_FRAMES_IN_FLIGHT);

  for (int i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<LveBuffer>(
        lveDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        lveDevice.properties.limits.minUniformBufferOffsetAlignment);
    uboBuffers[i]->map();
  }

  SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapchainRenderPass()};

  LveCamera camera{};

  auto viewerObject = LveGameObject::createGameObject();
  MovementController cameraController{lveWindow.getGLFWwindow()};

  auto currentTime = std::chrono::high_resolution_clock::now();

  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();

    currentTime = newTime;

    cameraController.handleMouseMovement(lveWindow.getGLFWwindow(), frameTime, viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
    cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);

    float aspect = lveRenderer.getAspectRatio();

    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      int frameIndex = lveRenderer.getFrameIndex();
      FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera};
      // update
      GlobalUbo ubo{};
      ubo.projectionView = camera.getProjection() * camera.getView();
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // render
      lveRenderer.beginSwapchainRenderPass(commandBuffer);

      simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);

      lveRenderer.endSwapchainRenderPass(commandBuffer);

      lveRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects() {
  std::shared_ptr<LveModel> lveModel =
      LveModel::createModelFromFile(lveDevice, "./assets/smooth_vase.obj");
  auto cube = LveGameObject::createGameObject();
  cube.model = lveModel;
  cube.transform.translation = {.0, .0f, 2.0f};
  cube.transform.scale = {1.5f, 1.5f, 1.5f};
  gameObjects.push_back(std::move(cube));

  auto cube2 = LveGameObject::createGameObject();
  cube2.model = lveModel;
  cube2.transform.translation = {1.0, .0f, 2.0f};
  cube2.transform.scale = {1.5f, 1.5f, 1.5f};
  gameObjects.push_back(std::move(cube2));
}
} // namespace lve
