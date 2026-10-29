#include "ZoneMap.hpp"
#include "../engine/Logger.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace DayDoom::Game {

ZoneMap::ZoneMap() {
    generateProceduralZone(1337, ZoneConfig{1, 0.5f, false});
}

void ZoneMap::fillRect(int x0, int y0, int x1, int y1, int wallType, int floorType) {
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            if (x >= 0 && x < ZONE_MAP_WIDTH && y >= 0 && y < ZONE_MAP_HEIGHT) {
                m_grid[y * ZONE_MAP_WIDTH + x] = wallType;
                if (floorType >= 0) {
                    m_floorGrid[y * ZONE_MAP_WIDTH + x] = floorType;
                }
            }
        }
    }
}

void ZoneMap::buildHollowBuilding(int x0, int y0, int x1, int y1, int wallType, int doorX, int doorY) {
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            if (x < 0 || x >= ZONE_MAP_WIDTH || y < 0 || y >= ZONE_MAP_HEIGHT) continue;
            bool isBorder = (x == x0 || x == x1 || y == y0 || y == y1);
            if (isBorder) {
                m_grid[y * ZONE_MAP_WIDTH + x] = wallType;
                m_floorGrid[y * ZONE_MAP_WIDTH + x] = FLOOR_CONCRETE;
            } else {
                m_grid[y * ZONE_MAP_WIDTH + x] = TYPE_EMPTY;
                m_floorGrid[y * ZONE_MAP_WIDTH + x] = FLOOR_WOOD;
            }
        }
    }
    if (doorX >= 0 && doorX < ZONE_MAP_WIDTH && doorY >= 0 && doorY < ZONE_MAP_HEIGHT) {
        m_grid[doorY * ZONE_MAP_WIDTH + doorX] = TYPE_DOOR_WOOD;
    }
}

void ZoneMap::generateProceduralZone(uint32_t seed, const ZoneConfig& config) {
    ZoneProtocol::Engine::Logger::ScopedIndent indent;
    std::mt19937 rng(seed);
    m_doors.clear();
    int buildingsStamped = 0;
    int coverObstaclesStamped = 0;

    ZoneProtocol::Engine::Logger::info(
        "ZoneMap::generateProceduralZone -> Start (Seed=" + std::to_string(seed) +
        ", ZoneTier=" + std::to_string(config.zoneTier) +
        ", Grid=" + std::to_string(ZONE_MAP_WIDTH) + "x" + std::to_string(ZONE_MAP_HEIGHT) + ")",
        "WORLD"
    );

    // 1. Preenche o mapa base com terreno natural (Areia/Terra) e estradas de asfalto
    fillRect(0, 0, ZONE_MAP_WIDTH - 1, ZONE_MAP_HEIGHT - 1, TYPE_EMPTY, FLOOR_SAND);

    // Sorteia o eixo das estradas principais cruzadas
    int mainRoadX = 26 + static_cast<int>(rng() % 12);
    int mainRoadY = 26 + static_cast<int>(rng() % 12);

    fillRect(mainRoadX - 2, 0, mainRoadX + 2, ZONE_MAP_HEIGHT - 1, TYPE_EMPTY, FLOOR_ASPHALT);
    fillRect(0, mainRoadY - 2, ZONE_MAP_WIDTH - 1, mainRoadY + 2, TYPE_EMPTY, FLOOR_ASPHALT);

    // 2. Sistema de Chunks 4x4 (Blocos de 16x16 células)
    constexpr int CHUNK_SIZE = 16;
    for (int cy = 0; cy < 4; ++cy) {
        for (int cx = 0; cx < 4; ++cx) {
            int startX = cx * CHUNK_SIZE + 2;
            int startY = cy * CHUNK_SIZE + 2;
            int endX   = (cx + 1) * CHUNK_SIZE - 3;
            int endY   = (cy + 1) * CHUNK_SIZE - 3;

            // Protege as avenidas principais
            if (mainRoadX >= startX - 2 && mainRoadX <= endX + 2) continue;
            if (mainRoadY >= startY - 2 && mainRoadY <= endY + 2) continue;

            // Ponto de Spawn (Canto inferior esquerdo)
            if (cx == 0 && cy == 3) {
                m_spawnPos = { static_cast<float>(startX + 2), static_cast<float>(endY - 2) };
                fillRect(startX, startY, endX, endY, TYPE_EMPTY, FLOOR_CONCRETE);
                continue;
            }

            // Ponto de Extração (Canto superior direito)
            if (cx == 3 && cy == 0) {
                m_extractionPos = { static_cast<float>(endX - 2), static_cast<float>(startY + 2) };
                fillRect(startX, startY, endX, endY, TYPE_EMPTY, FLOOR_CONCRETE);
                m_grid[startY * ZONE_MAP_WIDTH + startX] = TYPE_CONTAINER_RED;
                continue;
            }

            // Sorteia construções nos blocos intermediários
            int roll = static_cast<int>(rng() % 100);
            if (roll < 55) {
                int bW = 5 + static_cast<int>(rng() % 4);
                int bH = 5 + static_cast<int>(rng() % 4);
                int x1 = std::min(endX, startX + bW);
                int y1 = std::min(endY, startY + bH);

                int wallTypes[] = { TYPE_WALL_BRICK, TYPE_WALL_CONCRETE, TYPE_HANGAR_BLUE };
                int chosenWall = wallTypes[rng() % 3];

                int doorX = (startX + x1) / 2;
                int doorY = startY;

                buildHollowBuilding(startX, startY, x1, y1, chosenWall, doorX, doorY);
                buildingsStamped++;
            } else {
                // Pátio com obstáculos e cobertura tática (muros PO-2 ou contêineres)
                for (int i = 0; i < 4; ++i) {
                    int rx = startX + static_cast<int>(rng() % std::max(1, (endX - startX)));
                    int ry = startY + static_cast<int>(rng() % std::max(1, (endY - startY)));
                    if (rx < ZONE_MAP_WIDTH && ry < ZONE_MAP_HEIGHT) {
                        m_grid[ry * ZONE_MAP_WIDTH + rx] = (rng() % 2 == 0) ? TYPE_MURO_SOVIET : TYPE_CONTAINER_BLU;
                        coverObstaclesStamped++;
                    }
                }
            }
        }
    }

    // 3. Muro perimetral intransponível nas bordas da Zona
    for (int i = 0; i < ZONE_MAP_WIDTH; ++i) {
        m_grid[0 * ZONE_MAP_WIDTH + i] = TYPE_MURO_SOVIET;
        m_grid[(ZONE_MAP_HEIGHT - 1) * ZONE_MAP_WIDTH + i] = TYPE_MURO_SOVIET;
        m_grid[i * ZONE_MAP_WIDTH + 0] = TYPE_MURO_SOVIET;
        m_grid[i * ZONE_MAP_WIDTH + (ZONE_MAP_WIDTH - 1)] = TYPE_MURO_SOVIET;
    }

    // 4. Registra todas as portas para o sistema de dobradiças
    for (int y = 0; y < ZONE_MAP_HEIGHT; ++y) {
        for (int x = 0; x < ZONE_MAP_WIDTH; ++x) {
            if (isDoorType(m_grid[y * ZONE_MAP_WIDTH + x])) {
                registerDoor(x, y, m_grid[y * ZONE_MAP_WIDTH + x]);
            }
        }
    }

    ZoneProtocol::Engine::Logger::info(
        "ZoneMap::generateProceduralZone -> Complete | RoadsCross=(" + std::to_string(mainRoadX) + "," + std::to_string(mainRoadY) +
        ") | Buildings=" + std::to_string(buildingsStamped) +
        " | CoverProps=" + std::to_string(coverObstaclesStamped) +
        " | Doors=" + std::to_string(m_doors.size()) +
        " | Spawn=(" + std::to_string(static_cast<int>(m_spawnPos.x)) + "," + std::to_string(static_cast<int>(m_spawnPos.y)) + ")" +
        " | Extraction=(" + std::to_string(static_cast<int>(m_extractionPos.x)) + "," + std::to_string(static_cast<int>(m_extractionPos.y)) + ")",
        "WORLD"
    );
}

void ZoneMap::registerDoor(int x, int y, int type) {
    if (x < 0 || x >= ZONE_MAP_WIDTH || y < 0 || y >= ZONE_MAP_HEIGHT) return;
    for (auto& d : m_doors) {
        if (d.getX() == x && d.getY() == y) return;
    }
    Door d(x, y, type);
    d.configurarDobradica(static_cast<float>(x) + 0.02f, static_cast<float>(y) + 0.50f, 1.0f, 0.0f, 1.0f, 0.96f, false);
    m_doors.push_back(d);
}

void ZoneMap::update(float deltaTime) {
    for (auto& d : m_doors) {
        d.update(deltaTime);
    }
}

int ZoneMap::getCell(int x, int y) const {
    if (x < 0 || x >= ZONE_MAP_WIDTH || y < 0 || y >= ZONE_MAP_HEIGHT) return TYPE_WALL_CONCRETE;
    return m_grid[y * ZONE_MAP_WIDTH + x];
}

int ZoneMap::getFloorType(int x, int y) const {
    if (x < 0 || x >= ZONE_MAP_WIDTH || y < 0 || y >= ZONE_MAP_HEIGHT) return FLOOR_ASPHALT;
    return m_floorGrid[y * ZONE_MAP_WIDTH + x];
}

bool ZoneMap::isWalkable(float x, float y, float radius) const {
    int minX = static_cast<int>(x - radius);
    int maxX = static_cast<int>(x + radius);
    int minY = static_cast<int>(y - radius);
    int maxY = static_cast<int>(y + radius);

    for (int cy = minY; cy <= maxY; ++cy) {
        for (int cx = minX; cx <= maxX; ++cx) {
            int cell = getCell(cx, cy);
            if (cell == TYPE_EMPTY) continue;
            if (isDoorType(cell)) {
                const Door* d = findDoor(cx, cy);
                if (d && d->isPassavel()) continue;
            }
            return false;
        }
    }
    return true;
}

bool ZoneMap::isDoor(int x, int y) const {
    return isDoorType(getCell(x, y));
}

bool ZoneMap::isDoorOpen(int x, int y) const {
    const Door* d = findDoor(x, y);
    return d ? d->isAberta() : false;
}

Door* ZoneMap::findDoor(int x, int y) {
    for (auto& d : m_doors) {
        if (d.getX() == x && d.getY() == y) return &d;
    }
    return nullptr;
}

const Door* ZoneMap::findDoor(int x, int y) const {
    for (const auto& d : m_doors) {
        if (d.getX() == x && d.getY() == y) return &d;
    }
    return nullptr;
}

void ZoneMap::toggleDoor(int x, int y) {
    Door* d = findDoor(x, y);
    if (d) {
        d->toggle();
    }
}

bool ZoneMap::isNearExtraction(float x, float y, float radius) const {
    return getExtractionDistance(x, y) <= radius;
}

float ZoneMap::getExtractionDistance(float x, float y) const {
    float dx = x - m_extractionPos.x;
    float dy = y - m_extractionPos.y;
    return std::sqrt(dx * dx + dy * dy);
}

const char* ZoneMap::getSectorName(float x, float y) const {
    if (y >= 48.0f) return "SETOR 04: PERIFERIA DE KRASTOV";
    if (y >= 32.0f) return "SETOR 02: COMPLEXO INDUSTRIAL NOVO-BRYANSK";
    if (y >= 16.0f) return "SETOR 07: INSTALACAO AEREO-MILITAR 11-B";
    return "SETOR 01: ZONA DE EXTRACAO 'OBELISK'";
}

} // namespace DayDoom::Game