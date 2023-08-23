#include "first_app.h"
#include "fmt/core.h"
#include "keyboard_movement_controller.h"
#include "lve_camera.h"
#include "simple_render_system.h"
#include <array>
#include <chrono>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>

namespace lve {
FirstApp::FirstApp() { loadGameObjects(); }

FirstApp::~FirstApp() = default;

void FirstApp::run() {

  SimpleRenderSystem simpleRenderSystem{lveDevice,
                                        lveRenderer.getSwapchainRenderPass()};

  LveCamera camera{};

  auto viewerObject = LveGameObject::createGameObject();
  KeyboardMovementController cameraController{};

  auto currentTime = std::chrono::high_resolution_clock::now();

  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime -
                                                                   currentTime)
            .count();

    currentTime = newTime;

    cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime,
                                   viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation,
                      viewerObject.transform.rotation);

    float aspect = lveRenderer.getAspectRatio();

    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      lveRenderer.beginSwapchainRenderPass(commandBuffer);

      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);

      lveRenderer.endSwapchainRenderPass(commandBuffer);

      lveRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(lveDevice.device());
}

// temporary helper function, creates a 1x1x1 cube centered at offset with an
// index buffer
std::unique_ptr<LveModel> loadModel(LveDevice& device, glm::vec3 offset) {
  LveModel::Builder modelBuilder{};
  Assimp::Importer importer{};
  const aiScene* scene =
      importer.ReadFile("./assets/cog.gltf", aiProcess_Triangulate);

  assert(scene != nullptr && "Couldn't load  model!");

  modelBuilder.vertices.resize(scene->mMeshes[0]->mNumVertices);

  const auto mesh = scene->mMeshes[0];

  for (int i = 0; i < mesh->mNumVertices; i++) {
    const auto vertex = mesh->mVertices[i];
    modelBuilder.vertices[i].position = {vertex.x, vertex.y, vertex.z};
    modelBuilder.vertices[i].color = {0.3f, 0.0f, 0.5f};
  }

  for (auto& v : modelBuilder.vertices) {
    v.position += offset;
  }

  modelBuilder.indices.resize(mesh->mNumFaces * 3);

  for (int i = 0; i < mesh->mNumFaces; i++) {
    const auto face = mesh->mFaces[i];
    assert(face.mNumIndices == 3 &&
           "model faces are expected to be triangles!");
    for (int j = 0; j < face.mNumIndices; j++) {
      modelBuilder.indices[i * 3 + j] = face.mIndices[j];
    }
  }

  return std::make_unique<LveModel>(device, modelBuilder);
}

void FirstApp::loadGameObjects() {
  std::shared_ptr<LveModel> lveModel{loadModel(lveDevice, {.0f, .0f, .0f})};
  auto cube = LveGameObject::createGameObject();
  cube.model = lveModel;
  cube.transform.translation = {.0, .0f, 2.5f};
  cube.transform.scale = {.5f, .5f, .5f};
  gameObjects.push_back(std::move(cube));
}
} // namespace lve
