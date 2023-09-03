
#include "movement_controller.h"
#include "fmt/core.h"

namespace lve {

void MovementController::moveInPlaneXZ(GLFWwindow* window, float dt,
                                       LveGameObject& gameObject) const {

  float yaw = gameObject.transform.rotation.y;
  const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
  const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
  const glm::vec3 upDir{0.f, -1.f, 0.f};

  glm::vec3 moveDir{0.f};

  if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) {
    moveDir += forwardDir;
  }
  if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) {
    moveDir -= forwardDir;
  }
  if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) {
    moveDir += rightDir;
  }
  if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) {
    moveDir -= rightDir;
  }
  if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) {
    moveDir += upDir;
  }
  if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) {
    moveDir -= upDir;
  }

  if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
    gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
  }
}
void MovementController::handleMouseMovement(GLFWwindow* window, float dt,
                                             LveGameObject& gameObject) {
  glm::vec3 rotate{};

  double x{}, y{};

  glfwGetCursorPos(window, &x, &y);

  float xDiff = (static_cast<float>(x) - lastX);
  float yDiff = (static_cast<float>(y) - lastY);

  lastX = static_cast<float>(x);
  lastY = static_cast<float>(y);

  gameObject.transform.rotation.y += glm::mod(dt * xDiff * lookSpeed, glm::two_pi<float>());
  gameObject.transform.rotation.x -= glm::clamp(dt * yDiff * lookSpeed, -1.5f, 1.5f);

  int width, height;
  glfwGetWindowSize(window, &width, &height);
  glfwSetCursorPos(window, width / 2.f, height / 2.f);
  lastX = width / 2.f;
  lastY = height / 2.f;
}

} // namespace lve
