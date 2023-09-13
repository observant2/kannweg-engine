
#pragma once

#include "glm/vec3.hpp"
#include "lve_model.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <unordered_map>

namespace lve {

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.0f, 1.0f, 1.0f};
  glm::vec3 rotation{};

  // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
  // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4();
};

struct PointLightComponent {
  float lightIntensity = 1.0f;
};

class LveGameObject {
public:
  using id_t = unsigned int;
  using Map = std::unordered_map<id_t, LveGameObject>;

  static LveGameObject createGameObject() {
    static id_t currentId = 0;

    return LveGameObject{currentId++};
  }

  [[nodiscard]] id_t getId() const { return id; }

  static LveGameObject makePointLight(float intensity = 10.f, float radius = 0.1f,
                                      glm::vec3 color = glm::vec3(1.f));

  LveGameObject(const LveGameObject&) = delete;
  LveGameObject& operator=(const LveGameObject&) = delete;
  LveGameObject(LveGameObject&&) = default;
  LveGameObject& operator=(LveGameObject&&) = default;

  glm::vec3 color{};
  TransformComponent transform{};

  std::shared_ptr<LveModel> model{};
  std::unique_ptr<PointLightComponent> pointLightComponent = nullptr;

private:
  explicit LveGameObject(id_t objId) : id{objId} {}

  id_t id;
};
} // namespace lve
