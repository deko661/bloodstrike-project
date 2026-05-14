#pragma once
#include <deps/glm/glm.hpp>

namespace aim
{
    void Run();
    void FindTarget();
    void FindTargetViaSingleton();  // New: via MessiahIEntity
    void AimAt(const glm::vec2& target);
}
