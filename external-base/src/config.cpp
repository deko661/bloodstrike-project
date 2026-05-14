#include "config.hpp"
#include "settings.hpp"
#include <Windows.h>
#include <cstdlib>
#include <fstream>
#include <string>

namespace config
{
    std::string GetConfigPath()
    {
        char* userProfile = nullptr;
        size_t userProfileLen = 0;
        if (_dupenv_s(&userProfile, &userProfileLen, "USERPROFILE") == 0 && userProfile && userProfile[0] != '\0') {
            std::string documentsPath = std::string(userProfile) + "\\Documents";
            std::string configFolder = documentsPath + "\\BloodStrike External";
            CreateDirectoryA(configFolder.c_str(), NULL); // no-op if already exists
            free(userProfile);
            return configFolder + "\\config.ini";
        }
        if (userProfile) {
            free(userProfile);
        }

        // Fallback: executable directory
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        std::string path(buffer);
        size_t pos = path.find_last_of("\\/");
        if (pos != std::string::npos) {
            path = path.substr(0, pos + 1);
        }
        return path + "config.ini";
    }

    static void LoadColor(float* color, const std::string& value)
    {
        sscanf_s(value.c_str(), "%f,%f,%f,%f", &color[0], &color[1], &color[2], &color[3]);
    }

    void Save()
    {
        std::string path = GetConfigPath();
        std::ofstream file(path);
        if (!file.is_open()) return;

        file << "[Aimbot]\n";
        file << "Enabled=" << sdk::aimbotEnabled << "\n";
        file << "DrawFov=" << sdk::aimbotDrawFov << "\n";
        file << "TeamCheck=" << sdk::aimbotTeamCheck << "\n";
        file << "Fov=" << sdk::aimbotFov << "\n";
        file << "Smooth=" << sdk::aimbotSmooth << "\n";
        file << "Key=" << sdk::aimbotKey << "\n";
        file << "Bone=" << sdk::aimbotTargetBone << "\n";

        file << "[ESP]\n";
        file << "Skeleton=" << sdk::espSkeleton << "\n";
        file << "Box=" << sdk::espBox << "\n";
        file << "BoxStyle=" << sdk::espBoxStyle << "\n";
        file << "Distance=" << sdk::espDistance << "\n";
        file << "Tracers=" << sdk::espTracers << "\n";
        file << "Radar=" << sdk::espRadar << "\n";
        file << "RadarPosX=" << sdk::espRadarPosX << "\n";
        file << "RadarPosY=" << sdk::espRadarPosY << "\n";
        file << "RadarZoom=" << sdk::espRadarZoom << "\n";
        file << "RadarSize=" << sdk::espRadarSize << "\n";
        file << "RadarEnemySize=" << sdk::espRadarEnemySize << "\n";
        file << "ChapeauChinois=" << sdk::espChapeauChinois << "\n";
        file << "ChapeauRotationSpeed=" << sdk::espChapeauRotationSpeed << "\n";
        file << "MaxDistance=" << sdk::espMaxDistance << "\n";

        file << "[Visuals]\n";
        file << "Watermark=" << sdk::watermark << "\n";
        file << "FPSCounter=" << sdk::fpsCounter << "\n";
        file << "DisableIntro=" << sdk::disableIntro << "\n";

        file << "[Crosshair]\n";
        file << "Enabled=" << sdk::customCrosshair << "\n";
        file << "Style=" << sdk::crosshairStyle << "\n";
        file << "Size=" << sdk::crosshairSize << "\n";
        file << "Thickness=" << sdk::crosshairThickness << "\n";
        file << "Gap=" << sdk::crosshairGap << "\n";
        file << "Rotation=" << sdk::crosshairRotation << "\n";
        file << "RotationSpeed=" << sdk::crosshairRotationSpeed << "\n";
        file << "Dot=" << sdk::crosshairDot << "\n";
        file << "Outline=" << sdk::crosshairOutline << "\n";
        file << "OutlineThickness=" << sdk::crosshairOutlineThickness << "\n";
        file << "Color=" << sdk::crosshairColor[0] << "," << sdk::crosshairColor[1] << "," << sdk::crosshairColor[2] << "," << sdk::crosshairColor[3] << "\n";
        file << "OutlineColor=" << sdk::crosshairOutlineColor[0] << "," << sdk::crosshairOutlineColor[1] << "," << sdk::crosshairOutlineColor[2] << "," << sdk::crosshairOutlineColor[3] << "\n";

        file << "[Colors]\n";
        file << "BoxColor=" << colors::espBox[0] << "," << colors::espBox[1] << "," << colors::espBox[2] << "," << colors::espBox[3] << "\n";
        file << "DistColor=" << colors::espDistance[0] << "," << colors::espDistance[1] << "," << colors::espDistance[2] << "," << colors::espDistance[3] << "\n";
        file << "BoneColor=" << colors::espBones[0] << "," << colors::espBones[1] << "," << colors::espBones[2] << "," << colors::espBones[3] << "\n";
        file << "LineColor=" << colors::espLines[0] << "," << colors::espLines[1] << "," << colors::espLines[2] << "," << colors::espLines[3] << "\n";
        file << "ChapeauColor=" << colors::espChapeau[0] << "," << colors::espChapeau[1] << "," << colors::espChapeau[2] << "," << colors::espChapeau[3] << "\n";
        file << "FovInactive=" << colors::fovInactive[0] << "," << colors::fovInactive[1] << "," << colors::fovInactive[2] << "," << colors::fovInactive[3] << "\n";
        file << "FovActive=" << colors::fovActive[0] << "," << colors::fovActive[1] << "," << colors::fovActive[2] << "," << colors::fovActive[3] << "\n";

        file << "[Menu]\n";
        file << "Alpha=" << menu::alpha << "\n";
        file << "BackgroundEffect=" << menu::backgroundEffect << "\n";
        file << "EffectIntensity=" << menu::effectIntensity << "\n";
        file << "ToggleKey=" << menu::toggleKey << "\n";
        file << "EffectColor=" << menu::effectColor[0] << "," << menu::effectColor[1] << "," << menu::effectColor[2] << "," << menu::effectColor[3] << "\n";
        file << "BgColor=" << menu::bgColor[0] << "," << menu::bgColor[1] << "," << menu::bgColor[2] << "," << menu::bgColor[3] << "\n";
        file << "AccentColor=" << menu::accentColor[0] << "," << menu::accentColor[1] << "," << menu::accentColor[2] << "," << menu::accentColor[3] << "\n";
        file << "TitleColor=" << menu::titleBg[0] << "," << menu::titleBg[1] << "," << menu::titleBg[2] << "," << menu::titleBg[3] << "\n";
        file << "TabColor=" << menu::tabBg[0] << "," << menu::tabBg[1] << "," << menu::tabBg[2] << "," << menu::tabBg[3] << "\n";

        file.close();
    }

    void Load()
    {
        std::string path = GetConfigPath();
        std::ifstream file(path);
        if (!file.is_open()) return;

        std::string line;
        std::string section;

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            if (line[0] == '[') {
                section = line.substr(1, line.find(']') - 1);
                continue;
            }

            size_t eqPos = line.find('=');
            if (eqPos == std::string::npos) continue;

            std::string key = line.substr(0, eqPos);
            std::string value = line.substr(eqPos + 1);

            if (section == "Aimbot") {
                if (key == "Enabled") sdk::aimbotEnabled = std::stoi(value);
                else if (key == "DrawFov") sdk::aimbotDrawFov = std::stoi(value);
                else if (key == "TeamCheck") sdk::aimbotTeamCheck = std::stoi(value);
                else if (key == "Fov") sdk::aimbotFov = std::stof(value);
                else if (key == "Smooth") sdk::aimbotSmooth = std::stof(value);
                else if (key == "Key") sdk::aimbotKey = std::stoi(value);
                else if (key == "Bone") sdk::aimbotTargetBone = std::stoi(value);
            }
            else if (section == "ESP") {
                if (key == "Skeleton") sdk::espSkeleton = std::stoi(value);
                else if (key == "Box") sdk::espBox = std::stoi(value);
                else if (key == "BoxStyle") sdk::espBoxStyle = std::stoi(value);
                else if (key == "Distance") sdk::espDistance = std::stoi(value);
                else if (key == "Tracers") sdk::espTracers = std::stoi(value);
                else if (key == "Radar") sdk::espRadar = std::stoi(value);
                else if (key == "RadarPosX") sdk::espRadarPosX = std::stof(value);
                else if (key == "RadarPosY") sdk::espRadarPosY = std::stof(value);
                else if (key == "RadarZoom") sdk::espRadarZoom = std::stof(value);
                else if (key == "RadarSize") sdk::espRadarSize = std::stof(value);
                else if (key == "RadarEnemySize") sdk::espRadarEnemySize = std::stof(value);
                else if (key == "ChapeauChinois") sdk::espChapeauChinois = std::stoi(value);
                else if (key == "ChapeauRotationSpeed") sdk::espChapeauRotationSpeed = std::stof(value);
                else if (key == "MaxDistance") sdk::espMaxDistance = std::stof(value);
            }
            else if (section == "Visuals") {
                if (key == "Watermark") sdk::watermark = std::stoi(value);
                else if (key == "FPSCounter") sdk::fpsCounter = std::stoi(value);
                else if (key == "DisableIntro") sdk::disableIntro = std::stoi(value);
            }
            else if (section == "Crosshair") {
                if (key == "Enabled") sdk::customCrosshair = std::stoi(value);
                else if (key == "Style") sdk::crosshairStyle = std::stoi(value);
                else if (key == "Size") sdk::crosshairSize = std::stof(value);
                else if (key == "Thickness") sdk::crosshairThickness = std::stof(value);
                else if (key == "Gap") sdk::crosshairGap = std::stof(value);
                else if (key == "Rotation") sdk::crosshairRotation = std::stof(value);
                else if (key == "RotationSpeed") sdk::crosshairRotationSpeed = std::stof(value);
                else if (key == "Dot") sdk::crosshairDot = std::stoi(value);
                else if (key == "Outline") sdk::crosshairOutline = std::stoi(value);
                else if (key == "OutlineThickness") sdk::crosshairOutlineThickness = std::stof(value);
                else if (key == "Color") LoadColor(sdk::crosshairColor, value);
                else if (key == "OutlineColor") LoadColor(sdk::crosshairOutlineColor, value);
            }
            else if (section == "Colors") {
                if (key == "BoxColor") LoadColor(colors::espBox, value);
                else if (key == "DistColor") LoadColor(colors::espDistance, value);
                else if (key == "BoneColor") LoadColor(colors::espBones, value);
                else if (key == "LineColor") LoadColor(colors::espLines, value);
                else if (key == "ChapeauColor") LoadColor(colors::espChapeau, value);
                else if (key == "FovInactive") LoadColor(colors::fovInactive, value);
                else if (key == "FovActive") LoadColor(colors::fovActive, value);
            }
            else if (section == "Menu") {
                if (key == "Alpha") menu::alpha = std::stof(value);
                else if (key == "BackgroundEffect") menu::backgroundEffect = std::stoi(value);
                else if (key == "EffectIntensity") menu::effectIntensity = std::stof(value);
                else if (key == "ToggleKey") menu::toggleKey = std::stoi(value);
                else if (key == "EffectColor") LoadColor(menu::effectColor, value);
                else if (key == "BgColor") LoadColor(menu::bgColor, value);
                else if (key == "AccentColor") LoadColor(menu::accentColor, value);
                else if (key == "TitleColor") LoadColor(menu::titleBg, value);
                else if (key == "TabColor") LoadColor(menu::tabBg, value);
            }
        }

        file.close();
    }
}
