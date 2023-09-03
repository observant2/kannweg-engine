
#pragma once

#include "lve_game_object.h"
#include "lve_window.h"

namespace lve {

class MovementController {
public:
  explicit MovementController(GLFWwindow* window) {
    int width{}, height{};
    glfwGetWindowSize(window, &width, &height);

    lastX = static_cast<float>(width) / 2.0f;
    lastY = static_cast<float>(height) / 2.0f;

    glfwSetCursorPos(window, lastX, lastY);
  }

  struct KeyMappings {
    int moveLeft = GLFW_KEY_A;
    int moveRight = GLFW_KEY_D;
    int moveForward = GLFW_KEY_W;
    int moveBackward = GLFW_KEY_S;
    int moveUp = GLFW_KEY_E;
    int moveDown = GLFW_KEY_C;
    int lookLeft = GLFW_KEY_LEFT;
    int lookRight = GLFW_KEY_RIGHT;
    int lookUp = GLFW_KEY_UP;
    int lookDown = GLFW_KEY_DOWN;
  };

  void moveInPlaneXZ(GLFWwindow* window, float dt, LveGameObject& gameObject) const;

  void handleMouseMovement(GLFWwindow* window, float dt, LveGameObject& gameObject);

  KeyMappings keys{};
  float moveSpeed{3.f};
  float lookSpeed{1.5f};

  float lastX{};
  float lastY{};
};

} // namespace lve
