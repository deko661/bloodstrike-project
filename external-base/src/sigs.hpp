#pragma once
#include <string>

namespace signatures
{
  namespace module
  {
    constexpr const char* EntityList = "48 8B D9 48 8D 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 83 C1 ? 48 89 5C 24 ? 48 8B 51";
    constexpr int EntityList_RIP_Offset = 11;
    constexpr int EntityList_RIP_Size = 7;

    constexpr const char* ClientEngine = "49 8B D6 48 8B CF E8 ? ? ? ? 48 83 3D ? ? ? ? ?";
    constexpr int ClientEngine_RIP_Offset = 13;
    constexpr int ClientEngine_RIP_Size = 7;

    constexpr const char* EntityListSteam = "48 8B D9 48 8D 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 83 C1 ? 48 89 5C 24 ? 48 8B 51";
    constexpr const char* ClientEngineSteam = "49 8B D6 48 8B CF E8 ? ? ? ? 48 83 3D ? ? ? ? ?";
  }

}
