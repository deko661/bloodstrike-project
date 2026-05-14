#pragma once

#include <deps/glm/glm.hpp>
#include <DirectXMath.h>
#include <cmath>
#include <memory>
#include "../imgui/imgui.h"

namespace bloodstrike
{
    struct Vec2
    {
        float x, y;
    };

    struct Vec3
    {
        float x, y, z;
    };

    struct Vec4
    {
        float x, y, z, w;
    };

    struct Matrix3x4
    {
        float m[3][4];
        Vec3 origin() const { return {m[0][3], m[1][3], m[2][3]}; }
    };

    struct Matrix4x4
    {
        float m[4][4];
    };
}

extern std::unique_ptr<class c_memory> memory;

void BoneConnection(const glm::vec2 &a, const glm::vec2 &b, ImColor color);

bool MessiahMatrixAdd(const DirectX::XMFLOAT3X4 &bonemat, const DirectX::XMFLOAT3X4 &pos, glm::vec3 &out);

bool w2s(uint64_t cam, const glm::vec3 &world, glm::vec2 &out, bool returnAnyway);

// Native game WorldToScreen function - more stable after game updates
bool w2s_native(uint64_t cam, const glm::vec3 &world, glm::vec2 &out, bool returnAnyway);

bloodstrike::Vec3 get_camera_position();

glm::vec3 get_local_position();

void update_camera_and_local();
