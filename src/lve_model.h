
#pragma once

#include "lve_buffer.h"
#include "lve_device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <memory>

namespace lve {
class LveModel {
public:
  struct Vertex {
    glm::vec3 position{};
    glm::vec3 color{};
    glm::vec3 normal{};
    glm::vec2 uv{};

    static std::vector<VkVertexInputBindingDescription> getBindingDescription();

    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescription();
  };

  struct Builder {
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};

    void loadModel(const std::string& filepath);
  };

  LveModel(LveDevice& device, const LveModel::Builder& builder);
  ~LveModel();

  LveModel(const LveModel&) = delete;
  LveModel& operator=(const LveModel&) = delete;

  static std::unique_ptr<LveModel>
  createModelFromFile(LveDevice& device, const std::string& filepath);

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer) const;

private:
  void createVertexBuffers(const std::vector<Vertex>& vertices);
  void createIndexBuffer(const std::vector<uint32_t>& indices);

  LveDevice& lveDevice;
  std::unique_ptr<LveBuffer> vertexBuffer;
  uint32_t vertexCount{};

  bool hasIndexBuffer = false;
  std::unique_ptr<LveBuffer> indexBuffer;
  uint32_t indexCount{};
};
} // namespace lve
