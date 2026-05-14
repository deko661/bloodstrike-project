#pragma once
#include <cstdint>
#include <DirectXMath.h>
#include <deps/glm/glm.hpp>

void RenderAim();
bool w2s(uint64_t cam, const glm::vec3 &world, glm::vec2 &out, bool returnAnyway = false);
bool MessiahMatrixAdd(const DirectX::XMFLOAT3X4 &bonemat, const DirectX::XMFLOAT3X4 &pos, glm::vec3 &out);