
#include "lve_model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fmt/core.h>

namespace lve {
LveModel::LveModel(LveDevice& device, const LveModel::Builder& builder)
    : lveDevice(device) {
  createVertexBuffers(builder.vertices);
  createIndexBuffer(builder.indices);
}

LveModel::~LveModel() {
  vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
  vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);

  if (hasIndexBuffer) {
    vkDestroyBuffer(lveDevice.device(), indexBuffer, nullptr);
    vkFreeMemory(lveDevice.device(), indexBufferMemory, nullptr);
  }
}

void LveModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");

  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  lveDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

  lveDevice.createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
  lveDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

  vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
  vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);
}

void LveModel::createIndexBuffer(const std::vector<uint32_t>& indices) {
  indexCount = static_cast<uint32_t>(indices.size());
  hasIndexBuffer = indexCount > 0;

  if (!hasIndexBuffer) {
    return;
  }

  VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  lveDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(lveDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(lveDevice.device(), stagingBufferMemory);

  lveDevice.createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
  lveDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(lveDevice.device(), stagingBuffer, nullptr);
  vkFreeMemory(lveDevice.device(), stagingBufferMemory, nullptr);
}

void LveModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
  if (hasIndexBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
  }
}

void LveModel::draw(VkCommandBuffer commandBuffer) const {
  if (hasIndexBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
  }
  vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}

std::unique_ptr<LveModel>
LveModel::createModelFromFile(LveDevice& device, const std::string& filepath) {
  Builder builder{};
  builder.loadModel(filepath);

  fmt::println("Vertex count: {}", builder.vertices.size());

  return std::make_unique<LveModel>(device, builder);
}

std::vector<VkVertexInputBindingDescription>
LveModel::Vertex::getBindingDescription() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions{1};
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>
LveModel::Vertex::getAttributeDescription() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{2};
  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(Vertex, position);

  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(Vertex, color);

  return attributeDescriptions;
}

void LveModel::Builder::loadModel(const std::string& filepath) {
  Assimp::Importer importer{};
  const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate);

  assert(scene != nullptr && "Couldn't load  model!");

  vertices.clear();
  indices.clear();
  vertices.resize(scene->mMeshes[0]->mNumVertices);

  const auto mesh = scene->mMeshes[0];

  for (int i = 0; i < mesh->mNumVertices; i++) {
    const auto vertex = mesh->mVertices[i];
    vertices[i].position = {vertex.x, vertex.y, vertex.z};
    vertices[i].color = {0.3f, 0.0f, 0.5f};

    const auto normal = mesh->mNormals[i];
    vertices[i].normal = {normal.x, normal.y, normal.z};
  }

  if (mesh->HasTextureCoords(0)) {
    for (int i = 0; i < mesh->mNumVertices; i++) {
      const auto texCoord = mesh->mTextureCoords[0][i];
      vertices[i].uv = {texCoord.x, texCoord.y};
    }
  }

  if (mesh->HasVertexColors(0)) {
    for (int i = 0; i < mesh->mNumVertices; i++) {
      const auto texCoord = mesh->mColors[0][i];
      vertices[i].color = {texCoord.r, texCoord.g, texCoord.b};
    }
  }

  //  for (auto& v : modelBuilder.vertices) {
  //    v.position += offset;
  //  }

  indices.clear();
  indices.resize(mesh->mNumFaces * 3);

  for (int i = 0; i < mesh->mNumFaces; i++) {
    const auto face = mesh->mFaces[i];
    assert(face.mNumIndices == 3 &&
           "model faces are expected to be triangles!");
    for (int j = 0; j < face.mNumIndices; j++) {
      indices[i * 3 + j] = face.mIndices[j];
    }
  }
}
} // namespace lve
