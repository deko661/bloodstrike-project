#pragma once
#include <cstdint>
#include <iostream>
#include "scanner.hpp"
#include "sigs.hpp"

namespace offsets
{
  namespace module
  {
    inline uint64_t EntityList = 0x8A6EA58;
    inline uint64_t ClientEngine = 0x7FCD2E8;
    inline uint64_t EntityListSteam = 0x6E4D0D8;
    inline uint64_t ClientEngineSteam = 0x65F7AD0;
  }

  namespace singleton
  {
    constexpr uint64_t MessiahIEntity = 0x3D80048;
    constexpr uint64_t MessiahICamera = 0x3F9B1D0;
    constexpr uint64_t MessiahAnimationCorePose = 0x3FA2F18;
    constexpr uint64_t MessiahSkeletonComponent = 0x4103698;
    constexpr uint64_t MessiahActor = 0x5001C20;
    constexpr uint64_t MessiahActorComponent = 0x4138AB0;
    constexpr uint64_t MessiahIArea = 0x3FBAA68;
    constexpr uint64_t MessiahTachComponent = 0x4101FC8;
    constexpr uint64_t MessiahFontType = 0x3C189F0;
  }

  namespace functions
  {
    constexpr uint64_t Messiah_WorldToScreen = 0x940F60;
    constexpr uint64_t MessiahGetBoneTransform = 0x0D2BEC0;
    constexpr uint64_t GetRawInputData = 0x3BE8FF8;
    constexpr uint64_t MessiahIObjectInitalizer = 0x02CF160;
    constexpr uint64_t MessiahIObjectDeconstructor = 0x2CF890;
    constexpr uint64_t MessiahIEntity__Constructor = 0x02CF160;
  }

  namespace entity
  {
    constexpr uint64_t actorInstance = 0x18;
    constexpr uint64_t actorProps = 0x278;
    constexpr uint64_t actorComponent = 0x18;
    constexpr uint64_t IEntity = 0x40;
    constexpr uint64_t entityMask = 0x2e0;
    constexpr uint64_t IArea = 0x88;
    constexpr uint64_t transform = 0x58;
    constexpr uint64_t pose = 0x18;
    constexpr uint64_t BipedPose = 0x90;
    constexpr uint64_t BipedPoseBase = 0x8;
    // Additional offsets from dump
    constexpr uint64_t PoseSenderComponent = 0x28;
    constexpr uint64_t CollisionSkeleton = 0xF8;
    constexpr uint64_t m_SkeletonData = 0x10;
    constexpr uint64_t m_Pose = 0x18;
    constexpr uint64_t m_SkeletonName = 0x10;
  }

  namespace player
  {
    constexpr uint64_t IGameplay = 0x58;
    constexpr uint64_t ClientPlayer = 0x58;
    constexpr uint64_t ClientController = 0x50;
    constexpr uint64_t ClientScenario = 0x60;
    constexpr uint64_t camera = 0x238;
    constexpr uint64_t localActor = 0x288;
    constexpr uint64_t SceneViewportClient = 0x340;
    constexpr uint64_t teamID = 0x2D0;  // LocalActor_TeamID
    // View/Camera related
    constexpr uint64_t MainRenderViewport = 0x18;
    constexpr uint64_t PerspectiveCamera = 0x40;
    constexpr uint64_t ViewMatrix = 0x570;
    constexpr uint64_t vecOrigin = 0x7C;
  }

  namespace loot
  {
    // Object list structure (linked to MessiahIArea or MessiahActorComponent)
    inline uint64_t addrObjects = 0x39C8B98;      // Base of object list (OUTDATED - needs update)
    constexpr uint64_t ptrObject1 = 0x38;         // First pointer in chain
    constexpr uint64_t ptrObject2 = 0x470;        // Second pointer to object list
    constexpr uint64_t entityOffset = 0x10;       // Entity = curObj - 0x10
    constexpr uint64_t objectType = 0x118;      // Type: 1=players, 2/4/5=loot/items
    constexpr uint64_t objectPosition = 0x7C;     // World position Vec3
    // Bot detection
    constexpr uint64_t actorType = 0x8;           // 3 = human player, other = bot/object
    constexpr uint64_t isBotFlag = 0x2A0;         // In actorProps (example offset)
  }

  namespace entitylist
  {
    constexpr uint64_t head = 0x8;
    constexpr uint64_t next = 0x0;
  }

  namespace bone
  {
    constexpr uint64_t boneMatrix = 0x30;

    // Bone indices (multiplied by 0x8)
    constexpr int head = 8;
    constexpr int neck = 7;
    constexpr int spine1 = 6;
    constexpr int spine2 = 5;
    constexpr int spine3 = 4;
    constexpr int pelvis = 3;
    constexpr int buttCheekL = 22;
    constexpr int buttCheekR = 18;
    constexpr int kneeL = 23;
    constexpr int kneeR = 19;
    constexpr int footL = 24;
    constexpr int footR = 20;
    constexpr int sholL = 14;
    constexpr int sholR = 9;
    constexpr int elbowL = 15;
    constexpr int elbowR = 10;
    constexpr int wristL = 16;
    constexpr int wristR = 11;
  }

  namespace object_types
  {
    constexpr int PLAYER = 1;      // Human players
    constexpr int LOOT_1 = 2;      // Loot type 1 (items)
    constexpr int LOOT_2 = 4;      // Loot type 2 (weapons)
    constexpr int LOOT_3 = 5;      // Loot type 3 (other)
    constexpr int HUMAN_ACTOR = 3; // Actor type for human player
  }

  inline bool auto_update()
  {
    std::cout << "[Offsets] Scanning for module-level offsets..." << std::endl;

    uint64_t entity_list_addr = scanner::find_entitylist();
    if (entity_list_addr != 0)
    {
      module::EntityList = entity_list_addr - memory->get_module_address();
      std::cout << "[Offsets] EntityList found: 0x" << std::hex << module::EntityList << std::dec << std::endl;
    }
    else
    {
      std::cout << "[Offsets] Failed to find EntityList, using fallback" << std::endl;
    }

    uint64_t client_engine_addr = scanner::find_clientengine();
    if (client_engine_addr != 0)
    {
      module::ClientEngine = client_engine_addr - memory->get_module_address();
      std::cout << "[Offsets] ClientEngine found: 0x" << std::hex << module::ClientEngine << std::dec << std::endl;
    }
    else
    {
      std::cout << "[Offsets] Failed to find ClientEngine, using fallback" << std::endl;
    }

    std::cout << "[Offsets] Auto-update complete!" << std::endl;
    return (module::EntityList != 0 && module::ClientEngine != 0);
  }

  inline void auto_update_or_fallback()
  {
    if (!auto_update())
    {
      std::cout << "[Offsets] Using fallback static offsets..." << std::endl;
      module::EntityList = 0x8A6EA58;
      module::ClientEngine = 0x7FCD2E8;
      module::EntityListSteam = 0x6E4D0D8;
      module::ClientEngineSteam = 0x65F7AD0;
    }
  }
} // namespace offsets
