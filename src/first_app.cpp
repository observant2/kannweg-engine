#include "first_app.h"
#include "fmt/core.h"
#include "lve_camera.h"
#include "movement_controller.h"
#include "rendering/systems/point_light_system.h"
#include "rendering/systems/simple_render_system.h"
#include <array>
#include <chrono>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "rendering/lve_buffer.h"
#include <glm/glm.hpp>

namespace lve {

FirstApp::FirstApp() {
  globalPool =
      LveDescriptorPool::Builder(lveDevice)
          .setMaxSets(LveSwapchain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapchain::MAX_FRAMES_IN_FLIGHT)
          .build();
  loadGameObjects();
}

FirstApp::~FirstApp() = default;

void FirstApp::run() {
  std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapchain::MAX_FRAMES_IN_FLIGHT);

  for (auto& uboBuffer : uboBuffers) {
    uboBuffer = std::make_unique<LveBuffer>(
        lveDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        lveDevice.properties.limits.minUniformBufferOffsetAlignment);
    uboBuffer->map();
  }

  auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
                             .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                             .build();

  std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapchain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    LveDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
  }

  SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapchainRenderPass(),
                                        globalSetLayout->getDescriptorSetLayout()};

  PointLightSystem pointLightSystem{lveDevice, lveRenderer.getSwapchainRenderPass(),
                                    globalSetLayout->getDescriptorSetLayout()};
  LveCamera camera{};

  auto viewerObject = LveGameObject::createGameObject();
  viewerObject.transform.translation.z = -1.0f;
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
      FrameInfo frameInfo{
          frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex],
          gameObjects};
      // update
      GlobalUbo ubo{};
      ubo.projection = camera.getProjection();
      ubo.view = camera.getView();
      pointLightSystem.update(frameInfo, ubo);
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // render
      lveRenderer.beginSwapchainRenderPass(commandBuffer);

      simpleRenderSystem.renderGameObjects(frameInfo);
      pointLightSystem.render(frameInfo);

      lveRenderer.endSwapchainRenderPass(commandBuffer);

      lveRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects() {
  std::shared_ptr<LveModel> lveModel =
      LveModel::createModelFromFile(lveDevice, "./assets/smooth_vase.obj");

  std::shared_ptr<LveModel> floor = LveModel::createModelFromFile(lveDevice, "./assets/quad.obj");

  auto floorObj = LveGameObject::createGameObject();
  floorObj.model = floor;
  floorObj.transform.translation = {.5, .0f, 0.0f};
  floorObj.transform.scale = {1.5f, 1.5f, 1.5f};
  gameObjects.emplace(floorObj.getId(), std::move(floorObj));

  auto cube = LveGameObject::createGameObject();
  cube.model = lveModel;
  cube.transform.translation = {.0, .0f, 0.0f};
  cube.transform.scale = {1.5f, 1.5f, 1.5f};
  gameObjects.emplace(cube.getId(), std::move(cube));

  auto cube2 = LveGameObject::createGameObject();
  cube2.model = lveModel;
  cube2.transform.translation = {1.0, .0f, 0.0f};
  cube2.transform.scale = {1.5f, 1.5f, 1.5f};
  gameObjects.emplace(cube2.getId(), std::move(cube2));

  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f}, {.1f, .1f, 1.f}, {.1f, 1.f, .1f},
      {1.f, 1.f, .1f}, {.1f, 1.f, 1.f}, {1.f, 1.f, 1.f} //
  };

  for (int i = 0; i < lightColors.size(); i++) {
    auto pointLight = LveGameObject::makePointLight(0.2f);
    pointLight.color = lightColors[i];
    auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(),
                                   {0.f, -1.f, 0.f});
    pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
    gameObjects.emplace(pointLight.getId(), std::move(pointLight));
  }
}
} // namespace lve
