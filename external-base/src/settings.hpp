#pragma once
#include <Windows.h>

namespace sdk
{
    inline bool aimbotEnabled = true;
    inline bool aimbotDrawFov = true;
    inline float aimbotFov = 120.0f;
    inline float aimbotSmooth = 5.0f;
    inline int aimbotKey = VK_RBUTTON;
    inline int aimbotTargetBone = 9;
    inline bool aimbotTeamCheck = false;

    inline bool espSkeleton = false;
    inline bool espBox = true;
    inline int espBoxStyle = 0;
    inline bool espDistance = true;
    inline bool espTracers = true;
    inline bool espRadar = false;
    inline float espRadarPosX = -1.0f;
    inline float espRadarPosY = -1.0f;
    inline float espRadarZoom = 1.0f;
    inline float espRadarSize = 170.0f;
    inline float espRadarEnemySize = 2.8f;
    inline bool espChapeauChinois = false;
    inline float espChapeauRotationSpeed = 0.0f;
    inline float espMaxDistance = 300.0f;

    inline bool espLoot = false;
    inline float espLootMaxDistance = 100.0f;

    inline bool espShowBots = false;
    inline bool espHideBots = false;

    inline bool debugEntityScan = false;

    inline bool disableIntro = false;

    inline bool customCrosshair = true;
    inline bool watermark = true;
    inline bool fpsCounter = true;

    inline float crosshairSize = 8.0f;
    inline float crosshairThickness = 2.0f;
    inline float crosshairGap = 0.0f;
    inline float crosshairRotation = 0.0f;
    inline float crosshairRotationSpeed = 0.0f;
    inline int crosshairStyle = 0;
    inline bool crosshairDot = true;
    inline bool crosshairOutline = false;
    inline float crosshairOutlineThickness = 1.0f;
    inline float crosshairColor[4] = {1.0f, 0.0f, 0.0f, 0.78f};
    inline float crosshairOutlineColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

    inline const char* boneNames[] = {
        "Head", "Neck", "Chest", "Stomach", "Pelvis",
        "Left Hand", "Right Hand", "Left Foot", "Right Foot",
        "Neck", "Spine1", "Spine2", "Spine3", "Left Shoulder", "Pelvis"
    };
    inline int boneNameCount = 15;
}

namespace colors
{
    inline float espBox[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    inline float espDistance[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    inline float espBones[4] = {0.0f, 1.0f, 0.0f, 1.0f};
    inline float espLines[4] = {1.0f, 0.0f, 1.0f, 1.0f};
    inline float espChapeau[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    inline float espLoot[4] = {1.0f, 1.0f, 0.0f, 1.0f};
    inline float espBots[4] = {0.0f, 0.5f, 1.0f, 1.0f};
    inline float espBonesBot[4] = {0.0f, 0.3f, 0.8f, 1.0f};
    inline float espLinesBot[4] = {0.0f, 0.4f, 0.9f, 1.0f};
    inline float fovInactive[4] = {0.5f, 0.5f, 0.5f, 0.47f};
    inline float fovActive[4] = {0.0f, 1.0f, 0.0f, 0.78f};
}

namespace menu
{
    inline bool show = false;
    inline bool introComplete = false;
    inline float alpha = 0.95f;
    inline int backgroundEffect = 0;
    inline float effectIntensity = 50.0f;
    inline int toggleKey = VK_INSERT;
    inline float effectColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    inline float bgColor[4] = {0.14f, 0.14f, 0.18f, 0.95f};
    inline float accentColor[4] = {0.56f, 0.29f, 0.96f, 1.0f};
    inline float titleBg[4] = {0.14f, 0.14f, 0.18f, 1.0f};
    inline float tabBg[4] = {0.10f, 0.10f, 0.13f, 1.0f};
}
