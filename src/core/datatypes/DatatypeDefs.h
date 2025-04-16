#ifndef DATATYPEDEFS_H
#define DATATYPEDEFS_H

#include <cstdint>
#include <glm/vec3.hpp>

constexpr glm::vec3 WorldUp = glm::vec3(0, 1, 0);
constexpr glm::vec3 WorldForward = glm::vec3(0, 0, -1);
constexpr glm::vec3 WorldRight = glm::vec3(1, 0, 0);

enum Axis {
    X = 0,
    Y,
    Z,
};

enum class AxisDirection : uint8_t {
    PositiveX = 0u,
    NegativeX,
    PositiveY,
    NegativeY,
    PositiveZ,
    NegativeZ,
};

constexpr Axis allAxis[3] = {
    Axis::X,
    Axis::Y,
    Axis::Z,
};

constexpr AxisDirection allAxisDirections[6] = {
    AxisDirection::PositiveX,
    AxisDirection::NegativeX,
    AxisDirection::PositiveY,
    AxisDirection::NegativeY,
    AxisDirection::PositiveZ,
    AxisDirection::NegativeZ,
};

enum Zone {
    Surface,
    TwilightHollows,
    Vault,
    GreenMaw,
    ChasmsOfOld,
    MycelicWilds,
    Swamp,
    Deep,
    DyingInferno,
    FleshThatHates,
    Void,
    Core
};

enum Biome {
    // Surface
    Plains,
    Mountains,
    Ocean,
    Desert,

    // Twilight Hollows
    LushCaves,
    CrystalCanyons,
    UndergroundSeas,
    UndergroundRivers,

    // Transition Zone: The Vault
    VaultBiome,

    // Green Maw
    Jungle,
    MossCaves,

    // Chasms of Old
    Ruins,
    SpiderCaves,
    MiningSites,

    // Mycelic Wilds
    MushroomForest,
    GlowingFungiField,
    AberrantPlains,

    // Transition Zone: The Swamp
    SkiningForest,

    // The Deep
    WaterCaves,
    UnderwaterGrooves,
    Trenches,

    // Dying Inferno
    AshenWastes,
    SkeletonRuins,
    FurnaceSite,

    // Flesh That Hates
    Organism,
    Suffering,
    Dispair,

    // The Void
    FloatingIsles,
    EchoingLandscapes,

    // The Core
    CoreBiome
};

enum StructureType {
    Decoration,
    Regional,
    SuperStructure
};

enum BlockGenPriority {
    FILLER,
    MEDIUM,
    HIGH,
    OVERRIDE
};

#endif