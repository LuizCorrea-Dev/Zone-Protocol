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

class ZoneMap {
public:
    ZoneMap();

    // Gera uma nova zona procedural baseada em uma semente determinística e nível de perigo
    void generateProceduralZone(uint32_t seed = 1337, const ZoneConfig& config = ZoneConfig{1, 0.5f, false});

    // Consultas de ambiente
    int getCell(int x, int y) const;
    int getFloorType(int x, int y) const;
    bool isWalkable(float x, float y, float radius = 0.28f) const;

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
    const char* getSectorName(float x, float y) const;

private:
    int m_grid[ZONE_MAP_WIDTH * ZONE_MAP_HEIGHT] = {0};
    int m_floorGrid[ZONE_MAP_WIDTH * ZONE_MAP_HEIGHT] = {0};
    std::vector<Door> m_doors;

    Engine::Vec2 m_spawnPos{ 4.5f, 59.5f };
    Engine::Vec2 m_extractionPos{ 59.5f, 4.5f };

    void fillRect(int x0, int y0, int x1, int y1, int wallType, int floorType);
    void buildHollowBuilding(int x0, int y0, int x1, int y1, int wallType, int doorX, int doorY);
    void registerDoor(int x, int y, int type);
};

using CityMap = ZoneMap;

} // namespace DayDoom::Game
