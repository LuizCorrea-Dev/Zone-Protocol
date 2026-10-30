#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <random>
#include "../engine/MathDef.hpp"
#include "../objetos/Porta.hpp"
#include "ZoneLocationsDef.hpp"

namespace DayDoom::Game {

using Door = Objetos::Porta;

constexpr int ZONE_MAP_WIDTH = 64;
constexpr int ZONE_MAP_HEIGHT = 64;

struct ZoneConfig {
    int zoneTier = 1;           // 1: Periferia, 2: Área Industrial, 3: Marco Zero
    float mutantDensity = 0.5f; // Densidade de spawns hostis
    bool snowWeather = false;   // Bioma invernal ativado
};

enum class RoomTheme : uint8_t {
    STORAGE = 0,
    ARMORY = 1,
    GENERATOR_ROOM = 2,
    ABANDONED_QUARTERS = 3
};

struct BunkerRoom {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    RoomTheme theme = RoomTheme::STORAGE;

    int centerX() const { return x + w / 2; }
    int centerY() const { return y + h / 2; }
};

enum class MapPropType : uint8_t {
    NONE = 0,
    CRATE_WOOD = 1,
    BARREL_RAD = 2,
    GENERATOR = 3,
    PINE_TREE = 4,
    RUBBLE_CAR = 5,
    LOCKER = 6,
    ELECTRIC_PANEL = 7,
    WORKBENCH = 8,
    LOOT_CHEST = 9
};

class ZoneMap {
public:
    ZoneMap();

    // Gera uma nova zona procedural em 5 Passos (Terreno, BSP/Salas, Bitmask 4-Bit, Props Topológicos e Flood Fill BFS)
    void generateProceduralZone(uint32_t seed = 1337, const ZoneConfig& config = ZoneConfig{1, 0.5f, false});

    // Consultas de ambiente e topologia
    int getCell(int x, int y) const;
    int getFloorType(int x, int y) const;
    uint8_t getWallBitmask(int x, int y) const;
    bool isDoorwayZone(int x, int y) const;
    int getDistanceToSpawn(int x, int y) const;
    MapPropType getPropType(int x, int y) const;
    bool blocksLight(int x, int y) const;
    bool isWalkable(float x, float y, float radius = 0.28f) const;

    uint32_t getSeed() const { return m_seed; }
    int getZoneTier() const { return m_config.zoneTier; }
    const std::vector<BunkerRoom>& getRooms() const { return m_rooms; }

    int getWidth() const { return ZONE_MAP_WIDTH; }
    int getHeight() const { return ZONE_MAP_HEIGHT; }

    void update(float deltaTime);

    // Gestão de portas e portões gerados proceduralmente
    bool isDoor(int x, int y) const;
    bool isDoorOpen(int x, int y) const;
    void toggleDoor(int x, int y);
    Door* findDoor(int x, int y);
    const Door* findDoor(int x, int y) const;
    const std::vector<Door>& getDoors() const { return m_doors; }

    // Pontos Táticos (Infiltração e Extração da Zona)
    Engine::Vec2 getSpawnPosition() const { return m_spawnPos; }
    float getSpawnAngle() const { return -1.5707963f; }
    Engine::Vec2 getExtractionPosition() const { return m_extractionPos; }
    Engine::Vec2 getExtractionPoint() const { return m_extractionPos; }
    bool isNearExtraction(float x, float y, float radius = 3.5f) const;
    float getExtractionDistance(float x, float y) const;
    bool isInsideMissionBunker(float x, float y) const;
    const char* getSectorName(float x, float y) const;

private:
    uint32_t m_seed = 1337;
    ZoneConfig m_config{1, 0.5f, false};

    int m_grid[ZONE_MAP_WIDTH * ZONE_MAP_HEIGHT] = {0};
    int m_floorGrid[ZONE_MAP_WIDTH * ZONE_MAP_HEIGHT] = {0};
    uint8_t m_wallBitmask[ZONE_MAP_WIDTH * ZONE_MAP_HEIGHT] = {0};
    bool m_doorwayZone[ZONE_MAP_WIDTH * ZONE_MAP_HEIGHT] = {false};
    int m_distanceToSpawn[ZONE_MAP_WIDTH * ZONE_MAP_HEIGHT] = {0};
    MapPropType m_propGrid[ZONE_MAP_WIDTH * ZONE_MAP_HEIGHT] = {MapPropType::NONE};

    std::vector<BunkerRoom> m_rooms;
    std::vector<Door> m_doors;

    Engine::Vec2 m_spawnPos{ 4.5f, 59.5f };
    Engine::Vec2 m_extractionPos{ 59.5f, 4.5f };

    void fillRect(int x0, int y0, int x1, int y1, int wallType, int floorType);
    void buildHollowBuilding(int x0, int y0, int x1, int y1, int wallType, int doorX, int doorY);
    void buildMultiRoomComplex(int x0, int y0, int x1, int y1, int wallType, std::mt19937& rng);
    void markDoorwayProtection(int doorX, int doorY);
    void computeWallBitmasks();
    void populatePropsAndCover(std::mt19937& rng, int mainRoadX, int mainRoadY);
    void computeFloodFillAndExtraction(std::mt19937& rng);
    void registerDoor(int x, int y, int type);
};

using CityMap = ZoneMap;

} // namespace DayDoom::Game
