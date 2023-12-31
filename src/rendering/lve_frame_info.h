
#pragma once

#include "../lve_camera.h"
#include "../lve_game_object.h"
#include <vulkan/vulkan.h>

namespace lve {

constexpr int MAX_LIGHTS = 10;

struct PointLight {
  glm::vec4 position{};
  glm::vec4 color{}; // w is intensity
};

struct GlobalUbo {
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};
  glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.02f}; // w is intensity
  PointLight pointLights[MAX_LIGHTS];
  int numLights;
};

struct FrameInfo {
  int frameIndex;
  float frameTime;
  VkCommandBuffer commandBuffer;
  LveCamera& camera;
  VkDescriptorSet globalDescriptorSet;
  LveGameObject::Map& gameObjects;
};

} // namespace lve
