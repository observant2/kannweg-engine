#pragma once

#include <memory>
#include "lve_window.h"
#include "lve_model.h"
#include "lve_game_object.h"
#include "lve_renderer.h"

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

        std::vector<LveGameObject> gameObjects{};
    };
}
