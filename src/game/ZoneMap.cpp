#include "ZoneMap.hpp"
#include "../engine/Logger.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>

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

void ZoneMap::markDoorwayProtection(int doorX, int doorY) {
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int nx = doorX + dx;
            int ny = doorY + dy;
            if (nx >= 0 && nx < ZONE_MAP_WIDTH && ny >= 0 && ny < ZONE_MAP_HEIGHT) {
                m_doorwayZone[ny * ZONE_MAP_WIDTH + nx] = true;
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
        markDoorwayProtection(doorX, doorY);
    }
}

void ZoneMap::buildMultiRoomComplex(int x0, int y0, int x1, int y1, int wallType, std::mt19937& rng) {
    int w = x1 - x0;
    int h = y1 - y0;
    int interiorFloor = (m_config.zoneTier == 3) ? FLOOR_STAIRS_DOWN : FLOOR_WOOD;

    // 1. Casco externo do complexo
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            if (x < 0 || x >= ZONE_MAP_WIDTH || y < 0 || y >= ZONE_MAP_HEIGHT) continue;
            bool isBorder = (x == x0 || x == x1 || y == y0 || y == y1);
            if (isBorder) {
                m_grid[y * ZONE_MAP_WIDTH + x] = wallType;
                m_floorGrid[y * ZONE_MAP_WIDTH + x] = FLOOR_CONCRETE;
            } else {
                m_grid[y * ZONE_MAP_WIDTH + x] = TYPE_EMPTY;
                m_floorGrid[y * ZONE_MAP_WIDTH + x] = interiorFloor;
            }
        }
    }

    // 2. Janelas táticas (permitem passagem do feixe da lanterna DDA mas bloqueiam movimento)
    if (w >= 6 && h >= 6) {
        int winY = y0 + h / 2;
        if (winY > y0 + 1 && winY < y1 - 1) {
            m_grid[winY * ZONE_MAP_WIDTH + x0] = TYPE_WINDOW_GLASS;
            m_grid[winY * ZONE_MAP_WIDTH + x1] = TYPE_WINDOW_GLASS;
        }
    }

    // 3. Particionamento Espacial Binário (BSP Simplificado) para complexes >= 8x7
    if (w >= 8 && h >= 6) {
        int splitX = x0 + w / 2;
        for (int y = y0 + 1; y < y1; ++y) {
            m_grid[y * ZONE_MAP_WIDTH + splitX] = wallType;
        }
        int innerDoorY = y0 + 2 + static_cast<int>(rng() % std::max(1, h - 3));
        m_grid[innerDoorY * ZONE_MAP_WIDTH + splitX] = (m_config.zoneTier >= 2) ? TYPE_DOOR_SLIDING : TYPE_DOOR_WOOD;
        markDoorwayProtection(splitX, innerDoorY);

        BunkerRoom rLeft{ x0, y0, splitX - x0, h, static_cast<RoomTheme>(rng() % 4) };
        BunkerRoom rRight{ splitX, y0, x1 - splitX, h, static_cast<RoomTheme>((rng() + 1) % 4) };
        m_rooms.push_back(rLeft);
        m_rooms.push_back(rRight);

        // Portas externas nas duas alas
        int extDoor1X = x0 + std::max(1, (splitX - x0) / 2);
        int extDoor1Y = (rng() % 2 == 0) ? y0 : y1;
        m_grid[extDoor1Y * ZONE_MAP_WIDTH + extDoor1X] = TYPE_DOOR_WOOD;
        markDoorwayProtection(extDoor1X, extDoor1Y);

        int extDoor2X = splitX + std::max(1, (x1 - splitX) / 2);
        int extDoor2Y = (extDoor1Y == y0) ? y1 : y0;
        m_grid[extDoor2Y * ZONE_MAP_WIDTH + extDoor2X] = TYPE_DOOR_WOOD;
        markDoorwayProtection(extDoor2X, extDoor2Y);
    } else {
        int doorX = (x0 + x1) / 2;
        int doorY = (rng() % 2 == 0) ? y0 : y1;
        m_grid[doorY * ZONE_MAP_WIDTH + doorX] = TYPE_DOOR_WOOD;
        markDoorwayProtection(doorX, doorY);

        BunkerRoom room{ x0, y0, w, h, static_cast<RoomTheme>(rng() % 4) };
        m_rooms.push_back(room);
    }
}

void ZoneMap::computeWallBitmasks() {
    for (int y = 0; y < ZONE_MAP_HEIGHT; ++y) {
        for (int x = 0; x < ZONE_MAP_WIDTH; ++x) {
            int idx = y * ZONE_MAP_WIDTH + x;
            if (m_grid[idx] == TYPE_EMPTY) {
                m_wallBitmask[idx] = 0;
                continue;
            }
            uint8_t mask = 0;
            if (y > 0 && m_grid[(y - 1) * ZONE_MAP_WIDTH + x] != TYPE_EMPTY) mask |= 1; // Norte
            if (x + 1 < ZONE_MAP_WIDTH && m_grid[y * ZONE_MAP_WIDTH + (x + 1)] != TYPE_EMPTY) mask |= 2; // Este
            if (y + 1 < ZONE_MAP_HEIGHT && m_grid[(y + 1) * ZONE_MAP_WIDTH + x] != TYPE_EMPTY) mask |= 4; // Sul
            if (x > 0 && m_grid[y * ZONE_MAP_WIDTH + (x - 1)] != TYPE_EMPTY) mask |= 8; // Oeste
            m_wallBitmask[idx] = mask;
        }
    }
}

void ZoneMap::populatePropsAndCover(std::mt19937& rng, int mainRoadX, int mainRoadY) {
    // 1. Regras Topológicas dentro de cada BunkerRoom gerada no Passo 2
    for (const auto& room : m_rooms) {
        bool chestPlaced = false;
        for (int y = room.y + 1; y < room.y + room.h; ++y) {
            for (int x = room.x + 1; x < room.x + room.w; ++x) {
                if (x <= 1 || x >= ZONE_MAP_WIDTH - 2 || y <= 1 || y >= ZONE_MAP_HEIGHT - 2) continue;
                int idx = y * ZONE_MAP_WIDTH + x;
                if (m_grid[idx] != TYPE_EMPTY || m_doorwayZone[idx]) continue;

                bool wallN = (m_grid[(y - 1) * ZONE_MAP_WIDTH + x] != TYPE_EMPTY) && !isDoor(x, y - 1);
                bool wallS = (m_grid[(y + 1) * ZONE_MAP_WIDTH + x] != TYPE_EMPTY) && !isDoor(x, y + 1);
                bool wallE = (m_grid[y * ZONE_MAP_WIDTH + (x + 1)] != TYPE_EMPTY) && !isDoor(x + 1, y);
                bool wallW = (m_grid[y * ZONE_MAP_WIDTH + (x - 1)] != TYPE_EMPTY) && !isDoor(x - 1, y);
                int wallCount = (wallN ? 1 : 0) + (wallS ? 1 : 0) + (wallE ? 1 : 0) + (wallW ? 1 : 0);

                // REGRA 1: Cantos internos (2 ou mais paredes adjacentes) -> Baús de Loot
                if (wallCount >= 2 && !chestPlaced && (rng() % 100 < 65)) {
                    m_propGrid[idx] = MapPropType::LOOT_CHEST;
                    chestPlaced = true;
                    continue;
                }

                // REGRA 2: Encostado na parede Norte (virado para a câmera) -> Armários, Painéis ou Bancadas
                if (wallN && !wallS && (rng() % 100 < 38)) {
                    if (room.theme == RoomTheme::ARMORY) {
                        m_propGrid[idx] = MapPropType::LOCKER;
                    } else if (room.theme == RoomTheme::GENERATOR_ROOM) {
                        m_propGrid[idx] = (rng() % 2 == 0) ? MapPropType::ELECTRIC_PANEL : MapPropType::GENERATOR;
                    } else {
                        m_propGrid[idx] = MapPropType::WORKBENCH;
                    }
                    continue;
                }

                // REGRA 3: Barris Radioativos e Caixotes em pequenos grupos longe das portas
                if (rng() % 100 < 10) {
                    m_propGrid[idx] = (room.theme == RoomTheme::STORAGE) ? MapPropType::CRATE_WOOD : MapPropType::BARREL_RAD;
                }
            }
        }
    }

    // 2. Cobertura Tática Urbana e Florestal no Exterior (Avenidas, Clareiras e Pátios)
    for (int y = 3; y < ZONE_MAP_HEIGHT - 3; ++y) {
        for (int x = 3; x < ZONE_MAP_WIDTH - 3; ++x) {
            int idx = y * ZONE_MAP_WIDTH + x;
            if (m_grid[idx] != TYPE_EMPTY || m_doorwayZone[idx] || m_propGrid[idx] != MapPropType::NONE) continue;

            int floor = m_floorGrid[idx];
            bool onMainAvenue = (std::abs(x - mainRoadX) <= 2 || std::abs(y - mainRoadY) <= 2);

            if (onMainAvenue && (rng() % 100 < 5)) {
                // Carros destruídos / barricadas e barris nas avenidas para cover contra hordas
                m_propGrid[idx] = (rng() % 3 == 0) ? MapPropType::BARREL_RAD : MapPropType::RUBBLE_CAR;
            } else if (floor == FLOOR_SAND && (rng() % 100 < 6)) {
                // Pinheiros e escombros na floresta/periferia
                m_propGrid[idx] = (rng() % 100 < 68) ? MapPropType::PINE_TREE : MapPropType::RUBBLE_CAR;
            } else if (floor == FLOOR_CONCRETE && (rng() % 100 < 5)) {
                m_propGrid[idx] = (rng() % 2 == 0) ? MapPropType::CRATE_WOOD : MapPropType::BARREL_RAD;
            }
        }
    }
}

void ZoneMap::computeFloodFillAndExtraction(std::mt19937& rng) {
    auto runBFS = [&]() -> int {
        std::fill(std::begin(m_distanceToSpawn), std::end(m_distanceToSpawn), -1);
        int sx = std::clamp(static_cast<int>(m_spawnPos.x), 1, ZONE_MAP_WIDTH - 2);
        int sy = std::clamp(static_cast<int>(m_spawnPos.y), 1, ZONE_MAP_HEIGHT - 2);

        m_grid[sy * ZONE_MAP_WIDTH + sx] = TYPE_EMPTY;
        m_propGrid[sy * ZONE_MAP_WIDTH + sx] = MapPropType::NONE;

        std::queue<std::pair<int, int>> frontier;
        frontier.push({ sx, sy });
        m_distanceToSpawn[sy * ZONE_MAP_WIDTH + sx] = 0;

        int maxDist = 0;
        const int dx[4] = { 0, 1, 0, -1 };
        const int dy[4] = { -1, 0, 1, 0 };

        while (!frontier.empty()) {
            auto [cx, cy] = frontier.front();
            frontier.pop();

            int curDist = m_distanceToSpawn[cy * ZONE_MAP_WIDTH + cx];
            if (curDist > maxDist) maxDist = curDist;

            for (int dir = 0; dir < 4; ++dir) {
                int nx = cx + dx[dir];
                int ny = cy + dy[dir];
                if (nx <= 0 || nx >= ZONE_MAP_WIDTH - 1 || ny <= 0 || ny >= ZONE_MAP_HEIGHT - 1) continue;

                int nIdx = ny * ZONE_MAP_WIDTH + nx;
                int cell = m_grid[nIdx];
                bool passable = (cell == TYPE_EMPTY || isDoorType(cell));
                if (passable && m_distanceToSpawn[nIdx] == -1) {
                    m_distanceToSpawn[nIdx] = curDist + 1;
                    frontier.push({ nx, ny });
                }
            }
        }
        return maxDist;
    };

    int maxDistanceFound = runBFS();

    // Verifica se alguma BunkerRoom ficou isolada; se sim, abre porta automaticamente e recalcula BFS
    bool anyFixed = false;
    for (const auto& room : m_rooms) {
        int cx = std::clamp(room.centerX(), 1, ZONE_MAP_WIDTH - 2);
        int cy = std::clamp(room.centerY(), 1, ZONE_MAP_HEIGHT - 2);
        if (m_distanceToSpawn[cy * ZONE_MAP_WIDTH + cx] == -1) {
            int doorX = cx;
            int doorY = room.y;
            m_grid[doorY * ZONE_MAP_WIDTH + doorX] = TYPE_DOOR_WOOD;
            markDoorwayProtection(doorX, doorY);
            anyFixed = true;
        }
    }
    if (anyFixed) {
        maxDistanceFound = runBFS();
    }

    // Filtra todos os candidatos entre 85% e 100% da distância máxima real alcançável
    std::vector<std::pair<int, int>> extractionCandidates;
    int minTargetDist = static_cast<int>(maxDistanceFound * 0.85f);

    for (int y = 3; y < ZONE_MAP_HEIGHT - 3; ++y) {
        for (int x = 3; x < ZONE_MAP_WIDTH - 3; ++x) {
            int idx = y * ZONE_MAP_WIDTH + x;
            if (m_distanceToSpawn[idx] >= minTargetDist && m_grid[idx] == TYPE_EMPTY) {
                extractionCandidates.push_back({ x, y });
            }
        }
    }

    if (!extractionCandidates.empty()) {
        int chosenIdx = static_cast<int>(rng() % extractionCandidates.size());
        int ex = extractionCandidates[chosenIdx].first;
        int ey = extractionCandidates[chosenIdx].second;
        m_extractionPos = { static_cast<float>(ex) + 0.5f, static_cast<float>(ey) + 0.5f };
        fillRect(ex - 1, ey - 1, ex + 1, ey + 1, TYPE_EMPTY, FLOOR_CONCRETE);
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                m_propGrid[(ey + dy) * ZONE_MAP_WIDTH + (ex + dx)] = MapPropType::NONE;
            }
        }
    } else {
        m_extractionPos = { 58.5f, 5.5f };
        fillRect(57, 4, 59, 6, TYPE_EMPTY, FLOOR_CONCRETE);
    }
}

void ZoneMap::generateProceduralZone(uint32_t seed, const ZoneConfig& config) {
    ZoneProtocol::Engine::Logger::ScopedIndent indent;
    m_seed = seed;
    m_config = config;
    std::mt19937 rng(seed);

    m_doors.clear();
    m_rooms.clear();
    std::fill(std::begin(m_wallBitmask), std::end(m_wallBitmask), 0);
    std::fill(std::begin(m_doorwayZone), std::end(m_doorwayZone), false);
    std::fill(std::begin(m_distanceToSpawn), std::end(m_distanceToSpawn), -1);
    std::fill(std::begin(m_propGrid), std::end(m_propGrid), MapPropType::NONE);

    int buildingsStamped = 0;
    int coverObstaclesStamped = 0;

    ZoneProtocol::Engine::Logger::info(
        "ZoneMap::generateProceduralZone [5-PASS] -> Start (Seed=" + std::to_string(seed) +
        ", ZoneTier=" + std::to_string(config.zoneTier) +
        ", Grid=" + std::to_string(ZONE_MAP_WIDTH) + "x" + std::to_string(ZONE_MAP_HEIGHT) + ")",
        "WORLD"
    );

    // =========================================================================
    // PASSO 1: TERRENO EXTERIOR E BIOMAS POR SETOR (TIER 1, 2, 3)
    // =========================================================================
    int baseOutdoorFloor = (config.zoneTier == 2) ? FLOOR_CONCRETE : FLOOR_SAND;
    fillRect(0, 0, ZONE_MAP_WIDTH - 1, ZONE_MAP_HEIGHT - 1, TYPE_EMPTY, baseOutdoorFloor);

    // Eixos de avenidas cruzadas
    int mainRoadX = 26 + static_cast<int>(rng() % 12);
    int mainRoadY = 26 + static_cast<int>(rng() % 12);

    fillRect(mainRoadX - 2, 0, mainRoadX + 2, ZONE_MAP_HEIGHT - 1, TYPE_EMPTY, FLOOR_ASPHALT);
    fillRect(0, mainRoadY - 2, ZONE_MAP_WIDTH - 1, mainRoadY + 2, TYPE_EMPTY, FLOOR_ASPHALT);

    // =========================================================================
    // PASSO 2: PARTICIONAMENTO ESPACIAL (BSP), SALAS TEMÁTICAS E PROTEÇÃO 3x3 DE PORTAS
    // =========================================================================
    constexpr int CHUNK_SIZE = 16;
    for (int cy = 0; cy < 4; ++cy) {
        for (int cx = 0; cx < 4; ++cx) {
            int startX = cx * CHUNK_SIZE + 2;
            int startY = cy * CHUNK_SIZE + 2;
            int endX   = (cx + 1) * CHUNK_SIZE - 3;
            int endY   = (cy + 1) * CHUNK_SIZE - 3;

            // Protege o cruzamento central das avenidas
            if (mainRoadX >= startX - 2 && mainRoadX <= endX + 2 &&
                mainRoadY >= startY - 2 && mainRoadY <= endY + 2) {
                continue;
            }

            // Ponto de Infiltração (Canto inferior esquerdo)
            if (cx == 0 && cy == 3) {
                m_spawnPos = { static_cast<float>(startX + 2) + 0.5f, static_cast<float>(endY - 2) + 0.5f };
                fillRect(startX, startY, endX, endY, TYPE_EMPTY, FLOOR_CONCRETE);
                continue;
            }

            // Reserva candidato de canto oposto
            if (cx == 3 && cy == 0) {
                fillRect(startX, startY, endX, endY, TYPE_EMPTY, FLOOR_CONCRETE);
                continue;
            }

            int roll = static_cast<int>(rng() % 100);
            int buildingChance = (config.zoneTier == 1) ? 55 : ((config.zoneTier == 2) ? 70 : 78);

            if (roll < buildingChance) {
                int bW = 6 + static_cast<int>(rng() % 5);
                int bH = 6 + static_cast<int>(rng() % 4);
                int x1 = std::min(endX, startX + bW);
                int y1 = std::min(endY, startY + bH);

                int wallTypes[] = { TYPE_WALL_BRICK, TYPE_WALL_CONCRETE, TYPE_HANGAR_BLUE };
                int chosenWall = (config.zoneTier == 3) ? TYPE_WALL_CONCRETE : wallTypes[rng() % 3];

                buildMultiRoomComplex(startX, startY, x1, y1, chosenWall, rng);
                buildingsStamped++;

                // Cerca metálica tática anexa ao pátio (bloqueia passagem mas deixa luz DDA passar!)
                if (x1 + 2 <= endX && (rng() % 100 < 50)) {
                    for (int fy = startY; fy <= std::min(y1, startY + 3); ++fy) {
                        if (!m_doorwayZone[fy * ZONE_MAP_WIDTH + (x1 + 2)]) {
                            m_grid[fy * ZONE_MAP_WIDTH + (x1 + 2)] = TYPE_FENCE_WIRE;
                        }
                    }
                }
            } else {
                // Pátio de Cobertura Tática (Muros Soviéticos PO-2 e Contêineres)
                int coverCount = 3 + static_cast<int>(rng() % 3);
                for (int i = 0; i < coverCount; ++i) {
                    int rx = startX + static_cast<int>(rng() % std::max(1, (endX - startX)));
                    int ry = startY + static_cast<int>(rng() % std::max(1, (endY - startY)));
                    if (rx < ZONE_MAP_WIDTH - 2 && ry < ZONE_MAP_HEIGHT - 2 && !m_doorwayZone[ry * ZONE_MAP_WIDTH + rx]) {
                        m_grid[ry * ZONE_MAP_WIDTH + rx] = (rng() % 2 == 0) ? TYPE_MURO_SOVIET : TYPE_CONTAINER_BLU;
                        coverObstaclesStamped++;
                    }
                }
            }
        }
    }

    // Muro perimetral intransponível nas bordas da Zona 64x64
    for (int i = 0; i < ZONE_MAP_WIDTH; ++i) {
        m_grid[0 * ZONE_MAP_WIDTH + i] = TYPE_MURO_SOVIET;
        m_grid[(ZONE_MAP_HEIGHT - 1) * ZONE_MAP_WIDTH + i] = TYPE_MURO_SOVIET;
        m_grid[i * ZONE_MAP_WIDTH + 0] = TYPE_MURO_SOVIET;
        m_grid[i * ZONE_MAP_WIDTH + (ZONE_MAP_WIDTH - 1)] = TYPE_MURO_SOVIET;
    }

    // =========================================================================
    // PASSO 3: AUTOTILING DE PAREDES POR BITMASK DE 4 BITS (0..15)
    // =========================================================================
    computeWallBitmasks();

    // =========================================================================
    // PASSO 4: DISTRIBUIÇÃO TOPOLÓGICA DE PROPS E COBERTURA TÁTICA (COVER)
    // =========================================================================
    populatePropsAndCover(rng, mainRoadX, mainRoadY);

    // =========================================================================
    // PASSO 5: VALIDAÇÃO DE CONECTIVIDADE (FLOOD FILL BFS) E EXTRAÇÃO (85%-100%)
    // =========================================================================
    computeFloodFillAndExtraction(rng);

    // Registra todas as portas para o sistema físico de dobradiças
    for (int y = 0; y < ZONE_MAP_HEIGHT; ++y) {
        for (int x = 0; x < ZONE_MAP_WIDTH; ++x) {
            if (isDoorType(m_grid[y * ZONE_MAP_WIDTH + x])) {
                registerDoor(x, y, m_grid[y * ZONE_MAP_WIDTH + x]);
            }
        }
    }

    ZoneProtocol::Engine::Logger::info(
        "ZoneMap::generateProceduralZone [5-PASS] -> Complete | Rooms=" + std::to_string(m_rooms.size()) +
        " | Buildings=" + std::to_string(buildingsStamped) +
        " | Cover=" + std::to_string(coverObstaclesStamped) +
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

uint8_t ZoneMap::getWallBitmask(int x, int y) const {
    if (x < 0 || x >= ZONE_MAP_WIDTH || y < 0 || y >= ZONE_MAP_HEIGHT) return 15;
    return m_wallBitmask[y * ZONE_MAP_WIDTH + x];
}

bool ZoneMap::isDoorwayZone(int x, int y) const {
    if (x < 0 || x >= ZONE_MAP_WIDTH || y < 0 || y >= ZONE_MAP_HEIGHT) return false;
    return m_doorwayZone[y * ZONE_MAP_WIDTH + x];
}

int ZoneMap::getDistanceToSpawn(int x, int y) const {
    if (x < 0 || x >= ZONE_MAP_WIDTH || y < 0 || y >= ZONE_MAP_HEIGHT) return -1;
    return m_distanceToSpawn[y * ZONE_MAP_WIDTH + x];
}

MapPropType ZoneMap::getPropType(int x, int y) const {
    if (x < 0 || x >= ZONE_MAP_WIDTH || y < 0 || y >= ZONE_MAP_HEIGHT) return MapPropType::NONE;
    return m_propGrid[y * ZONE_MAP_WIDTH + x];
}

bool ZoneMap::blocksLight(int x, int y) const {
    int cell = getCell(x, y);
    if (cell == TYPE_EMPTY) return false;
    // Cercas de arame farpado e janelas táticas deixam o feixe da lanterna DDA atravessar!
    if (cell == TYPE_FENCE_WIRE || isWindowType(cell)) return false;
    if (isDoorType(cell)) {
        return !isDoorOpen(x, y);
    }
    return true;
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

bool ZoneMap::isInsideMissionBunker(float x, float y) const {
    int cx = static_cast<int>(std::floor(x));
    int cy = static_cast<int>(std::floor(y));
    int ft = getFloorType(cx, cy);
    return (ft == FLOOR_WOOD || ft == FLOOR_STAIRS_DOWN);
}

const char* ZoneMap::getSectorName(float x, float y) const {
    (void)x;
    (void)y;
    if (m_config.zoneTier == 1) return "ZONA 1: SUBURBIO & FLORESTA SOMBRIA DE KRASTOV";
    if (m_config.zoneTier == 2) return "ZONA 2: AREA INDUSTRIAL & CENTRO NOVO-BRYANSK";
    return "ZONA 3: COMPLEXO ZERO & LABORATORIOS OBELISK";
}

} // namespace DayDoom::Game