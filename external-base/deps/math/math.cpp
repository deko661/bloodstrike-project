#include "math.hpp"
#include <deps/memory/memory.h>
#include "../../src/offsets.hpp"

namespace bloodstrike
{
    namespace renderer
    {
        extern uint64_t camera;
        extern uint64_t localActor;
    }

    namespace camera_offsets
    {
        constexpr uintptr_t pos_x_local = 0x07C;
        constexpr uintptr_t pos_y_local = 0x080;
        constexpr uintptr_t pos_z_local = 0x084;
    }

    namespace offsets
    {
        constexpr uint64_t Messiah__ClientEngine = 0x65F7AD0;
        constexpr uint64_t Messiah__EntityList = 0x6E4D0D8;
    }
}

void BoneConnection(const glm::vec2 &a, const glm::vec2 &b, ImColor color)
{
    if (a.x == 0 && a.y == 0)
        return;
    if (b.x == 0 && b.y == 0)
        return;
    ImGui::GetBackgroundDrawList()->AddLine(ImVec2(a.x, a.y), ImVec2(b.x, b.y), color, 1.f);
}

bool MessiahMatrixAdd(const DirectX::XMFLOAT3X4 &bonemat, const DirectX::XMFLOAT3X4 &pos, glm::vec3 &out)
{
    out.x = (pos._11 * bonemat._32) + (pos._14 * bonemat._33) + (pos._23 * bonemat._34) + pos._32;
    out.y = (pos._12 * bonemat._32) + (pos._21 * bonemat._33) + (pos._24 * bonemat._34) + pos._33;
    out.z = (pos._13 * bonemat._32) + (pos._22 * bonemat._33) + (pos._31 * bonemat._34) + pos._34;
    return true;
}

void update_camera_and_local()
{
    uint64_t base = memory->get_module_address();

    uint64_t ClientEngine = memory->read<uint64_t>(base + bloodstrike::offsets::Messiah__ClientEngine);
    if (!ClientEngine)
        return;

    uint64_t IGameplay = memory->read<uint64_t>(ClientEngine + 0x58);
    if (!IGameplay)
        return;

    uint64_t ClientPlayer = memory->read<uint64_t>(IGameplay + 0x58);
    if (!ClientPlayer)
        return;

    bloodstrike::renderer::camera = memory->read<uint64_t>(ClientPlayer + 0x238);
    bloodstrike::renderer::localActor = memory->read<uint64_t>(ClientPlayer + 0x288);
}

bloodstrike::Vec3 get_camera_position()
{
    bloodstrike::Vec3 camera_pos = {0, 0, 0};

    update_camera_and_local();

    if (bloodstrike::renderer::camera)
    {
        camera_pos.x = memory->read<float>(bloodstrike::renderer::camera + bloodstrike::camera_offsets::pos_x_local);
        camera_pos.y = memory->read<float>(bloodstrike::renderer::camera + bloodstrike::camera_offsets::pos_y_local);
        camera_pos.z = memory->read<float>(bloodstrike::renderer::camera + bloodstrike::camera_offsets::pos_z_local);
    }

    return camera_pos;
}

glm::vec3 get_local_position()
{
    glm::vec3 local_pos = {0, 0, 0};

    update_camera_and_local();

    if (bloodstrike::renderer::localActor)
    {
        DirectX::XMFLOAT3X4 transform = memory->read<DirectX::XMFLOAT3X4>(bloodstrike::renderer::localActor + 0x58);
        local_pos.x = transform._32;  // Row 3, Col 2 - matches MessiahMatrixAdd
        local_pos.y = transform._33;  // Row 3, Col 3 - matches MessiahMatrixAdd
        local_pos.z = transform._34;  // Row 3, Col 4 - matches MessiahMatrixAdd
    }

    return local_pos;
}

bool w2s(uint64_t cam, const glm::vec3 &world, glm::vec2 &out, bool returnAnyway)
{
    if (!cam)
        return false;

    float relX = world.x - memory->read<float>(cam + 124);
    float relY = world.y - memory->read<float>(cam + 128);
    float relZ = world.z - memory->read<float>(cam + 132);

    float px = relX * memory->read<float>(cam + 772) + relY * memory->read<float>(cam + 784) + relZ * memory->read<float>(cam + 796);

    float py = relX * memory->read<float>(cam + 776) + relY * memory->read<float>(cam + 788) + relZ * memory->read<float>(cam + 800);

    float pzOrig = relX * memory->read<float>(cam + 780) + relY * memory->read<float>(cam + 792) + relZ * memory->read<float>(cam + 804);

    if (pzOrig >= -0.01f && !returnAnyway)
        return false;

    float pz = -pzOrig;

    float fov = memory->read<float>(cam + 824);
    float f = 1.0f / tanf((fov * 0.017453292f) * 0.5f);

    float screenW = (float)memory->read<uint16_t>(cam + 752);
    float screenH = (float)memory->read<uint16_t>(cam + 754);

    float invZ = 1.0f / std::fmax(fabsf(pz), 0.000001f);

    out.x = roundf(((px * invZ) * f * screenH + screenW) * 0.5f * 10.0f) * 0.1f;
    out.y = roundf(((screenH - ((py * invZ) * f * screenH)) * 0.5f) * 10.0f) * 0.1f;

    return true;
}

// Native game function WorldToScreen - potentially more stable after updates
// Usage: w2s_native(camera, worldPos, screenPos)
using W2SNative_t = bool(__fastcall*)(uint64_t camera, float* worldPos, float* screenPos);

bool w2s_native(uint64_t cam, const glm::vec3 &world, glm::vec2 &out, bool returnAnyway)
{
    if (!cam)
        return false;

    uint64_t base = memory->get_module_address();
    auto w2s_func = (W2SNative_t)(base + offsets::functions::Messiah_WorldToScreen);

    float worldPos[3] = { world.x, world.y, world.z };
    float screenPos[2] = { 0.0f, 0.0f };

    bool result = w2s_func(cam, worldPos, screenPos);

    if (!result && !returnAnyway)
        return false;

    out.x = screenPos[0];
    out.y = screenPos[1];

    return result;
}
