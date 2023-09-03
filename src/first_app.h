#pragma once

#include "lve_descriptors.h"
#include "lve_game_object.h"
#include "lve_renderer.h"
#include "lve_window.h"
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
  std::vector<LveGameObject> gameObjects{};
};
} // namespace lve
