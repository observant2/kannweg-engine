
#include "lve_model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <fmt/core.h>

namespace lve {
LveModel::LveModel(LveDevice& device, const LveModel::Builder& builder) : lveDevice(device) {
  createVertexBuffers(builder.vertices);
  createIndexBuffer(builder.indices);
}

LveModel::~LveModel() {}

void LveModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");

  uint32_t vertexSize = sizeof(vertices[0]);

  LveBuffer stagingBuffer{lveDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void*)vertices.data());

  vertexBuffer = std::make_unique<LveBuffer>(lveDevice, vertexSize, vertexCount,

                                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  lveDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(),
                       // kann weg?
                       stagingBuffer.getBufferSize());
}

void LveModel::createIndexBuffer(const std::vector<uint32_t>& indices) {
  indexCount = static_cast<uint32_t>(indices.size());
  hasIndexBuffer = indexCount > 0;

  if (!hasIndexBuffer) {
    return;
  }

  uint32_t indexSize = sizeof(indices[0]);

  LveBuffer stagingBuffer{lveDevice, indexSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void*)indices.data());

  indexBuffer = std::make_unique<LveBuffer>(lveDevice, indexSize, vertexCount,

                                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  lveDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(),
                       // kann weg?
                       stagingBuffer.getBufferSize());
}

void LveModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
  if (hasIndexBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
  }
}

void LveModel::draw(VkCommandBuffer commandBuffer) const {
  if (hasIndexBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
  }
  vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}

std::unique_ptr<LveModel> LveModel::createModelFromFile(LveDevice& device,
                                                        const std::string& filepath) {
  Builder builder{};
  builder.loadModel(filepath);

  fmt::println("Vertex count: {}", builder.vertices.size());

  return std::make_unique<LveModel>(device, builder);
}

std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescription() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions{1};
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescription() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
  attributeDescriptions.push_back(
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>(offsetof(Vertex, position))});

  attributeDescriptions.push_back(
      {1, 0, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>(offsetof(Vertex, color))});

  attributeDescriptions.push_back(
      {2, 0, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>(offsetof(Vertex, normal))});

  attributeDescriptions.push_back(
      {3, 0, VK_FORMAT_R32G32_SFLOAT, static_cast<uint32_t>(offsetof(Vertex, uv))});
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
    vertices[i].color = {1.f, 1.f, 0.8f};

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
    assert(face.mNumIndices == 3 && "model faces are expected to be triangles!");
    for (int j = 0; j < face.mNumIndices; j++) {
      indices[i * 3 + j] = face.mIndices[j];
    }
  }
}
} // namespace lve
