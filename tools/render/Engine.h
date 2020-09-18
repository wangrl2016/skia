//
// Created by wangrl on 2020/9/18.
//

#include "src/entt/entt.hpp"
#include "tools/render/MoveOnly.h"
#include "tools/render/System.h"

namespace render {
    class Engine : public MoveOnly {
    public:
        void init();

        void reset();

        void update();

        bool destroy();

        static Engine& instance() {
            static Engine engine;
            return engine;
        }

        Engine(Engine&&) = default;

        Engine& operator=(Engine&&) = default;

        static entt::registry& registry() {
            return instance().mRegistry;
        }

        static std::vector<std::unique_ptr<System>>& systems() {
            return instance().mSystems;
        }

    private:
        Engine();

        ~Engine();

        entt::registry mRegistry;
        std::vector<std::unique_ptr<System>> mSystems;
    };
}
