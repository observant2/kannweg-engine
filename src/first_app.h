#pragma once

#include "lve_game_object.h"
#include "rendering/lve_descriptors.h"
#include "rendering/lve_renderer.h"
#include "rendering/lve_window.h"
#include <memory>

namespace lve {
class FirstApp {
public:
  static constexpr int WIDTH = 1800;
  static constexpr int HEIGHT = 1800;

  FirstApp();

  ~FirstApp();

  FirstApp(const FirstApp&) = delete;

  FirstApp& operator=(const FirstApp&) = delete;

  void run();

private:
  void loadGameObjects();

  LveWindow lveWindow{WIDTH, HEIGHT, "engine"};
  LveDevice lveDevice{lveWindow};
  LveRenderer lveRenderer{lveWindow, lveDevice};

  // order matters
  std::unique_ptr<LveDescriptorPool> globalPool{};
  LveGameObject::Map gameObjects{};
};
} // namespace lve
