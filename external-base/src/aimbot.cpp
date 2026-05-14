#include "aimbot.hpp"
#include "settings.hpp"
#include "esp.hpp"
#include "aim.hpp"
#include "crosshair.hpp"
#include "visuals.hpp"
#include <deps/imgui/imgui.h>

void RenderAim()
{
    esp::Render();
    aim::Run();
    crosshair::Render();
    visuals::RenderWatermark();
    visuals::RenderFPS();
}
