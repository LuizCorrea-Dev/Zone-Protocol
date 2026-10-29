#include "CombatSystem.hpp"
#include "../engine/Logger.hpp"
#include <cmath>
#include <random>
#include <algorithm>
#include <cstdio>

namespace ZoneProtocol::Game {

bool CombatSystem::hasLineOfSight(const DayDoom::Game::ZoneMap& map, DayDoom::Engine::Vec2 a, DayDoom::Engine::Vec2 b) const {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 0.001f) return true;

    int steps = std::max(1, static_cast<int>(dist * 6.0f));
    float stepX = dx / static_cast<float>(steps);
    float stepY = dy / static_cast<float>(steps);

    float cx = a.x;
    float cy = a.y;
    for (int i = 0; i < steps; ++i) {
        cx += stepX;
        cy += stepY;
        if (!map.isWalkable(cx, cy, 0.05f)) {
            return false;
        }
    }
    return true;
}

void CombatSystem::spawnMutantsForZone(const DayDoom::Game::ZoneMap& map, uint32_t seed, int count) {
    m_mutants.clear();
    m_projectiles.clear();
    m_arcs.clear();
    m_crates.clear();
    m_mutantsKilled = 0;
    m_dataPointsEarned = 0;

    std::mt19937 rng(seed + 999);
    DayDoom::Engine::Vec2 spawnPos = map.getSpawnPosition();

    // 1. Spawna Criaturas Mutantes (Rastejantes Pálidos e Brutamontes de Carne)
    int attempts = 0;
    while (static_cast<int>(m_mutants.size()) < count && attempts < 600) {
        attempts++;
        float rx = 3.5f + static_cast<float>(rng() % (map.getWidth() - 7));
        float ry = 3.5f + static_cast<float>(rng() % (map.getHeight() - 7));

        float distToSpawn = std::hypot(rx - spawnPos.x, ry - spawnPos.y);
        if (distToSpawn < 8.5f || !map.isWalkable(rx, ry, 0.35f)) continue;

        Mutant m;
        m.id = m_nextMutantId++;
        bool isBrute = (rng() % 100) < 28; // 28% Brutamontes, 72% Rastejante Pálido
        m.type = isBrute ? MutantType::BRUTE_FLESH : MutantType::CRAWLER_PALE;
        m.pos = { rx, ry };
        m.maxHp = isBrute ? 140.0f : 55.0f;
        m.hp = m.maxHp;
        m.speed = isBrute ? 1.85f : 3.1f;
        m.radius = isBrute ? 0.38f : 0.28f;

        m_mutants.push_back(m);
    }

    // 2. Spawna Caixas de Loot Militar Saqueáveis (com Artefactos e Suprimentos)
    const std::vector<ItemDef> lootTable = {
        { 10, "ISOTOPO ZARIA", ItemCategory::ARTIFACT, 1, 1, SynergyElement::RAD_GREEN,   0.0f, 0.30f, 1 },
        { 11, "NUCLEO SPARK",  ItemCategory::ARTIFACT, 1, 1, SynergyElement::ELEC_PURPLE, 0.0f, 0.25f, 1 },
        { 12, "CRISTAL CRIO",  ItemCategory::ARTIFACT, 1, 1, SynergyElement::CRYO_BLUE,   0.0f, 0.20f, 1 },
        { 13, "ORBE COMPASSO", ItemCategory::ARTIFACT, 1, 1, SynergyElement::RAD_GREEN,   0.0f, 0.35f, 1 },
        { 20, "KIT MEDICO",    ItemCategory::MEDICAL,  2, 1, SynergyElement::NONE,       45.0f, 0.0f,  2 },
        { 21, "ATADURA",       ItemCategory::MEDICAL,  1, 1, SynergyElement::NONE,       20.0f, 0.0f,  5 }
    };

    int crateAttempts = 0;
    while (m_crates.size() < 14 && crateAttempts < 400) {
        crateAttempts++;
        float cx = 4.5f + static_cast<float>(rng() % (map.getWidth() - 9));
        float cy = 4.5f + static_cast<float>(rng() % (map.getHeight() - 9));
        if (!map.isWalkable(cx, cy, 0.35f)) continue;

        bool tooClose = false;
        for (const auto& c : m_crates) {
            if (std::hypot(c.pos.x - cx, c.pos.y - cy) < 5.0f) {
                tooClose = true;
                break;
            }
        }
        if (tooClose) continue;

        LootCrate crate;
        crate.pos = { cx, cy };
        crate.opened = false;
        crate.rewardItem = lootTable[rng() % lootTable.size()];
        m_crates.push_back(crate);
    }

    int brutesCount = 0;
    for (const auto& m : m_mutants) {
        if (m.type == MutantType::BRUTE_FLESH) brutesCount++;
    }
    int crawlersCount = static_cast<int>(m_mutants.size()) - brutesCount;

    ZoneProtocol::Engine::Logger::info(
        "[CombatSystem] SpawnMutantsForZone(Seed=" + std::to_string(seed) + ") -> Spawned " +
        std::to_string(m_mutants.size()) + " mutants (" + std::to_string(crawlersCount) +
        "x CRAWLER_PALE, " + std::to_string(brutesCount) + "x BRUTE_FLESH) and " +
        std::to_string(m_crates.size()) + "x Military Loot Crates",
        "COMBAT"
    );
}

void CombatSystem::fireWeapon(
    const DayDoom::Engine::Vec2& playerPos,
    const DayDoom::Engine::Vec2& aimDir,
    const CombatModifiers& synergyMods
) {
    if (m_fireCooldown > 0.0f) return;

    m_fireCooldown = 0.16f / std::max(0.5f, synergyMods.fireRateMultiplier);

    Projectile p;
    p.pos = { playerPos.x + aimDir.x * 0.42f, playerPos.y + aimDir.y * 0.42f };
    p.dir = aimDir;
    p.speed = 22.0f;
    p.damage = 25.0f * synergyMods.damageMultiplier;
    p.ricochetsLeft = synergyMods.electricRicochets;
    p.radBurnDPS = synergyMods.radiationBurnDPS;
    p.cryoSlow = synergyMods.cryoSlowActive;
    p.active = true;

    m_projectiles.push_back(p);

    // O barulho do tiro alerta todos os mutantes num raio de 14 metros!
    for (auto& m : m_mutants) {
        float dist = std::hypot(m.pos.x - playerPos.x, m.pos.y - playerPos.y);
        if (dist < 14.0f && m.state == AIState::IDLE) {
            m.state = AIState::ALERT_SOUND;
            m.lastHeardTarget = playerPos;
        }
    }
}

bool CombatSystem::isNearClosedCrate(const DayDoom::Engine::Vec2& playerPos) const {
    for (const auto& c : m_crates) {
        if (!c.opened && std::hypot(c.pos.x - playerPos.x, c.pos.y - playerPos.y) < 1.55f) {
            return true;
        }
    }
    return false;
}

bool CombatSystem::tryInteractLootCrate(
    const DayDoom::Engine::Vec2& playerPos,
    GridInventory& inventory,
    std::string& outLootMessage
) {
    for (auto& c : m_crates) {
        if (!c.opened && std::hypot(c.pos.x - playerPos.x, c.pos.y - playerPos.y) < 1.55f) {
            c.opened = true;
            m_dataPointsEarned += 35;
            if (inventory.addItemAuto(c.rewardItem, 1)) {
                outLootMessage = "SAQUEADO: " + c.rewardItem.name + " ADICIONADO A MALETA!";
                ZoneProtocol::Engine::Logger::info(
                    "[LootCrate] Opened crate at (" + std::to_string(static_cast<int>(c.pos.x)) + "," +
                    std::to_string(static_cast<int>(c.pos.y)) + ") -> Looted '" + c.rewardItem.name + "' (+35 DataPoints)",
                    "COMBAT"
                );
            } else {
                outLootMessage = "MALETA CHEIA! [TAB] PARA ORGANIZAR SLOTS";
                ZP_LOG_WARN("INVENTORY", "[LootCrate] Briefcase 6x4 & SafePocket 2x2 full when looting '" + c.rewardItem.name + "'");
            }
            return true;
        }
    }
    return false;
}

void CombatSystem::update(
    float dt,
    const DayDoom::Game::ZoneMap& map,
    DayDoom::Game::Player& player,
    const CombatModifiers& synergyMods
) {
    (void)synergyMods;
    if (m_fireCooldown > 0.0f) m_fireCooldown -= dt;

    for (auto& arc : m_arcs) arc.timer -= dt;
    m_arcs.erase(std::remove_if(m_arcs.begin(), m_arcs.end(),
        [](const ElectricArcFX& a) { return a.timer <= 0.0f; }), m_arcs.end());

    DayDoom::Engine::Vec2 pPos = player.getCamera().pos;

    // 1. ATUALIZA PROJÉTEIS E COLISÃO DE BALAS
    for (auto& proj : m_projectiles) {
        if (!proj.active) continue;

        float stepX = proj.dir.x * proj.speed * dt;
        float stepY = proj.dir.y * proj.speed * dt;
        DayDoom::Engine::Vec2 nextPos = { proj.pos.x + stepX, proj.pos.y + stepY };

        if (!map.isWalkable(nextPos.x, nextPos.y, 0.05f)) {
            if (proj.ricochetsLeft > 0) {
                proj.ricochetsLeft--;
                if (!map.isWalkable(nextPos.x, proj.pos.y, 0.05f)) proj.dir.x = -proj.dir.x;
                if (!map.isWalkable(proj.pos.x, nextPos.y, 0.05f)) proj.dir.y = -proj.dir.y;
            } else {
                proj.active = false;
                continue;
            }
        } else {
            proj.pos = nextPos;
        }

        // Colisão Bala -> Mutante
        for (auto& m : m_mutants) {
            if (m.hp <= 0.0f) continue;
            float dist = std::hypot(proj.pos.x - m.pos.x, proj.pos.y - m.pos.y);
            if (dist <= m.radius + 0.14f) {
                m.hp -= proj.damage;
                m.hitFlashTimer = 0.10f;
                m.state = AIState::CHASE_VISIBLE;
                m.lastHeardTarget = pPos;

                if (proj.radBurnDPS > 0.0f) {
                    m.radBurnTimer = 4.0f;
                    m.radDPS = proj.radBurnDPS;
                }
                if (proj.cryoSlow) {
                    m.cryoSlowTimer = 3.0f;
                }

                // Se a arma tem Sinergia Elétrica, salta um raio roxo para o mutante vizinho!
                if (proj.ricochetsLeft > 0) {
                    for (auto& other : m_mutants) {
                        if (other.id != m.id && other.hp > 0.0f) {
                            float chainDist = std::hypot(other.pos.x - m.pos.x, other.pos.y - m.pos.y);
                            if (chainDist < 4.8f) {
                                other.hp -= proj.damage * 0.65f;
                                other.hitFlashTimer = 0.12f;
                                m_arcs.push_back({ m.pos, other.pos, 0.18f });
                                if (other.hp <= 0.0f) {
                                    m_mutantsKilled++;
                                    m_dataPointsEarned += 50;
                                }
                                break;
                            }
                        }
                    }
                }

                proj.active = false;
                if (m.hp <= 0.0f) {
                    m_mutantsKilled++;
                    int xp = (m.type == MutantType::BRUTE_FLESH) ? 80 : 40;
                    m_dataPointsEarned += xp;
                    ZoneProtocol::Engine::Logger::info(
                        "[CombatSystem] Mutant #" + std::to_string(m.id) + " (" +
                        (m.type == MutantType::BRUTE_FLESH ? "BRUTE_FLESH" : "CRAWLER_PALE") +
                        ") eliminated at (" + std::to_string(static_cast<int>(m.pos.x)) + "," +
                        std::to_string(static_cast<int>(m.pos.y)) + ") | TotalKills=" + std::to_string(m_mutantsKilled) +
                        " | +" + std::to_string(xp) + " DataPoints",
                        "COMBAT"
                    );
                }
                break;
            }
        }
    }

    m_projectiles.erase(std::remove_if(m_projectiles.begin(), m_projectiles.end(),
        [](const Projectile& p) { return !p.active; }), m_projectiles.end());

    // 2. ATUALIZA IA E FÍSICA DOS MUTANTES (Com Wall Sliding)
    for (auto& m : m_mutants) {
        if (m.hp <= 0.0f) continue;

        if (m.hitFlashTimer > 0.0f) m.hitFlashTimer -= dt;
        if (m.attackCooldown > 0.0f) m.attackCooldown -= dt;
        if (m.cryoSlowTimer > 0.0f) m.cryoSlowTimer -= dt;

        // Dano contínuo de Radiação (Sinergia Verde)
        if (m.radBurnTimer > 0.0f) {
            m.radBurnTimer -= dt;
            m.hp -= m.radDPS * dt;
            if (m.hp <= 0.0f) {
                m_mutantsKilled++;
                m_dataPointsEarned += (m.type == MutantType::BRUTE_FLESH) ? 80 : 40;
                continue;
            }
        }

        float distToPlayer = std::hypot(pPos.x - m.pos.x, pPos.y - m.pos.y);
        bool canSeePlayer = (distToPlayer < 9.5f) && hasLineOfSight(map, m.pos, pPos);

        if (canSeePlayer) {
            m.state = AIState::CHASE_VISIBLE;
            m.lastHeardTarget = pPos;
        }

        if (m.state != AIState::IDLE) {
            float dx = m.lastHeardTarget.x - m.pos.x;
            float dy = m.lastHeardTarget.y - m.pos.y;
            float d = std::hypot(dx, dy);

            if (d > 0.42f) {
                float moveSpeed = m.speed * ((m.cryoSlowTimer > 0.0f) ? 0.5f : 1.0f);
                float vx = (dx / d) * moveSpeed * dt;
                float vy = (dy / d) * moveSpeed * dt;
                m.vel = { dx / d, dy / d };

                if (map.isWalkable(m.pos.x + vx, m.pos.y, m.radius)) m.pos.x += vx;
                if (map.isWalkable(m.pos.x, m.pos.y + vy, m.radius)) m.pos.y += vy;
            } else if (!canSeePlayer) {
                m.state = AIState::IDLE;
            }
        }

        // Ataque Corpo-a-Corpo se encostar no jogador
        if (distToPlayer < (m.radius + 0.35f) && m.attackCooldown <= 0.0f) {
            m.attackCooldown = 1.1f;
            float dmg = (m.type == MutantType::BRUTE_FLESH) ? 22.0f : 10.0f;
            player.takeDamage(dmg);
            ZoneProtocol::Engine::Logger::info(
                "[CombatSystem] Operator hit by Mutant #" + std::to_string(m.id) + " (" +
                (m.type == MutantType::BRUTE_FLESH ? "BRUTE_FLESH" : "CRAWLER_PALE") +
                ") for -" + std::to_string(static_cast<int>(dmg)) + " HP -> Operator HP=" +
                std::to_string(static_cast<int>(player.getHealth())),
                "COMBAT"
            );
        }
    }

    m_mutants.erase(std::remove_if(m_mutants.begin(), m_mutants.end(),
        [](const Mutant& m) { return m.hp <= 0.0f; }), m_mutants.end());
}

void CombatSystem::drawLine(uint32_t* buf, int sw, int sh, int x0, int y0, int x1, int y1, uint32_t color) const {
    if (!buf) return;
    int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int steps = dx + dy + 2;
    for (int i = 0; i < steps; ++i) {
        if (x0 >= 0 && x0 < sw && y0 >= 0 && y0 < sh) buf[y0 * sw + x0] = color;
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void CombatSystem::render(
    uint32_t* screenBuffer, int screenW, int screenH,
    const DayDoom::Game::ZoneMap& map,
    const DayDoom::Game::Player& player,
    const Engine::SpriteAtlas& entityAtlas,
    const Engine::SpriteAtlas* objectsAtlas,
    float tileSize
) const {
    float pX = player.getCamera().pos.x;
    float pY = player.getCamera().pos.y;
    float dirX = player.getCamera().dir.x;
    float dirY = player.getCamera().dir.y;

    float centerX = static_cast<float>(screenW) / 2.0f;
    float centerY = static_cast<float>(screenH) / 2.0f;

    // 0. RENDERIZA CAIXAS DE LOOT SAQUEÁVEIS (sprites_map_objects.png)
    if (objectsAtlas && objectsAtlas->isLoaded()) {
        for (const auto& crate : m_crates) {
            int cx = static_cast<int>(centerX + (crate.pos.x - pX) * tileSize);
            int cy = static_cast<int>(centerY + (crate.pos.y - pY) * tileSize);
            int cSize = static_cast<int>(tileSize * 0.78f);
            if (cx + cSize < 0 || cx - cSize >= screenW || cy + cSize < 0 || cy - cSize >= screenH) continue;

            float dist = std::hypot(crate.pos.x - pX, crate.pos.y - pY);
            float dot = (dist > 0.001f) ? (((crate.pos.x - pX) / dist) * dirX + ((crate.pos.y - pY) / dist) * dirY) : 1.0f;
            bool inLight = (dist < 9.5f && dot > 0.35f && hasLineOfSight(map, {pX, pY}, crate.pos)) ||
                           (dist < 2.2f && hasLineOfSight(map, {pX, pY}, crate.pos));

            float lightFactor = inLight ? std::clamp(1.25f - dist * 0.08f, 0.35f, 1.2f) : 0.14f;
            const auto& rect = crate.opened ? Engine::AtlasCoords::OBJ_CRATE_OPEN : Engine::AtlasCoords::OBJ_CRATE_CLOSED;

            objectsAtlas->drawSprite(
                screenBuffer, screenW, screenH,
                rect,
                cx - cSize / 2, cy - cSize / 2,
                cSize, cSize,
                lightFactor
            );
        }
    }

    // 1. RENDERIZA MUTANTES (Corpo no cone de luz OU Olhos Vermelhos no Fog of War!)
    for (const auto& m : m_mutants) {
        int screenX = static_cast<int>(centerX + (m.pos.x - pX) * tileSize);
        int screenY = static_cast<int>(centerY + (m.pos.y - pY) * tileSize);

        float dx = m.pos.x - pX;
        float dy = m.pos.y - pY;
        float dist = std::hypot(dx, dy);
        float dot = (dist > 0.001f) ? ((dx / dist) * dirX + (dy / dist) * dirY) : 1.0f;

        bool inLightCone = (dist < 10.5f && dot > 0.40f && hasLineOfSight(map, {pX, pY}, m.pos));
        bool inClosePeriphery = (dist < 2.2f && hasLineOfSight(map, {pX, pY}, m.pos));

        if ((inLightCone || inClosePeriphery) && entityAtlas.isLoaded()) {
            Engine::SpriteRect rect;
            if (m.type == MutantType::CRAWLER_PALE) {
                if (std::abs(m.vel.y) > std::abs(m.vel.x)) {
                    rect = (m.vel.y < 0.0f) ? Engine::AtlasCoords::CRAWLER_BACK : Engine::AtlasCoords::CRAWLER_FRONT;
                } else {
                    rect = (m.vel.x < 0.0f) ? Engine::AtlasCoords::CRAWLER_LEFT : Engine::AtlasCoords::CRAWLER_RIGHT;
                }
            } else {
                if (std::abs(m.vel.y) > std::abs(m.vel.x)) {
                    rect = (m.vel.y < 0.0f) ? Engine::AtlasCoords::BRUTE_BACK : Engine::AtlasCoords::BRUTE_FRONT;
                } else {
                    rect = (m.vel.x < 0.0f) ? Engine::AtlasCoords::BRUTE_LEFT : Engine::AtlasCoords::BRUTE_RIGHT;
                }
            }

            int spriteSize = (m.type == MutantType::BRUTE_FLESH) ? static_cast<int>(tileSize * 1.45f) : static_cast<int>(tileSize * 1.15f);
            float light = (m.hitFlashTimer > 0.0f) ? 2.2f : (inLightCone ? std::clamp(1.3f - dist * 0.07f, 0.45f, 1.25f) : 0.45f);

            entityAtlas.drawSprite(
                screenBuffer, screenW, screenH,
                rect,
                screenX - spriteSize / 2, screenY - spriteSize / 2,
                spriteSize, spriteSize,
                light
            );

            // Barra de HP tática acima do mutante ferido
            if (m.hp < m.maxHp) {
                int barW = static_cast<int>(tileSize * 0.75f);
                int barH = (screenW >= 1000) ? 4 : 2;
                int bx = screenX - barW / 2;
                int by = screenY - spriteSize / 2 - 6;
                int fillW = static_cast<int>((m.hp / m.maxHp) * barW);
                uint32_t barCol = (m.radBurnTimer > 0.0f) ? DayDoom::Engine::makeColor(80, 255, 100) : DayDoom::Engine::makeColor(255, 55, 45);
                for (int py = by; py < by + barH; ++py) {
                    if (py < 0 || py >= screenH) continue;
                    for (int px = bx; px < bx + barW; ++px) {
                        if (px < 0 || px >= screenW) continue;
                        screenBuffer[py * screenW + px] = (px < bx + fillW) ? barCol : 0xFF14181C;
                    }
                }
            }
        }
        else if (dist < 13.5f) {
            // FORA DO CONE DE LUZ (NO ESCURO): Desenha apenas os 2 olhos vermelhos brilhantes!
            uint32_t eyeColor = DayDoom::Engine::makeColor(255, 30, 30);
            int eyeSpread = (screenW >= 1000) ? 4 : 2;
            int eyeRadius = (screenW >= 1000) ? 2 : 1;
            for (int ox : {-eyeSpread, eyeSpread}) {
                for (int oy = -eyeRadius; oy <= eyeRadius; ++oy) {
                    for (int rx = -eyeRadius; rx <= eyeRadius; ++rx) {
                        int ex = screenX + ox + rx;
                        int ey = screenY - 6 + oy;
                        if (ex >= 0 && ex < screenW && ey >= 0 && ey < screenH) {
                            screenBuffer[ey * screenW + ex] = eyeColor;
                        }
                    }
                }
            }
        }
    }

    // 2. RENDERIZA ARCOS ELÉTRICOS ROXOS (Sinergia de Ricochete em Cadeia)
    for (const auto& arc : m_arcs) {
        int x0 = static_cast<int>(centerX + (arc.startPos.x - pX) * tileSize);
        int y0 = static_cast<int>(centerY + (arc.startPos.y - pY) * tileSize);
        int x1 = static_cast<int>(centerX + (arc.endPos.x   - pX) * tileSize);
        int y1 = static_cast<int>(centerY + (arc.endPos.y   - pY) * tileSize);
        drawLine(screenBuffer, screenW, screenH, x0, y0, x1, y1, DayDoom::Engine::makeColor(210, 90, 255));
        drawLine(screenBuffer, screenW, screenH, x0 + 1, y0, x1 + 1, y1, 0xFFFFFFFF);
    }

    // 3. RENDERIZA PROJÉTEIS (Traçantes amarelos brilhantes)
    int projRadius = (screenW >= 1000) ? 2 : 1;
    for (const auto& proj : m_projectiles) {
        int px = static_cast<int>(centerX + (proj.pos.x - pX) * tileSize);
        int py = static_cast<int>(centerY + (proj.pos.y - pY) * tileSize);
        uint32_t tracerColor = (proj.ricochetsLeft > 0)
            ? DayDoom::Engine::makeColor(220, 130, 255)
            : DayDoom::Engine::makeColor(255, 240, 120);
        for (int dy = -projRadius; dy <= projRadius; ++dy) {
            for (int dx = -projRadius; dx <= projRadius; ++dx) {
                if (px + dx >= 0 && px + dx < screenW && py + dy >= 0 && py + dy < screenH) {
                    screenBuffer[(py + dy) * screenW + (px + dx)] = tracerColor;
                }
            }
        }
    }
}

} // namespace ZoneProtocol::Game
