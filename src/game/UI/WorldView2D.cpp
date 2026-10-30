#include "WorldView2D.hpp"
#include "../../engine/MathDef.hpp"
#include <cmath>
#include <algorithm>
#include <vector>

namespace DayDoom::Game::UI {

WorldView2D::WorldView2D() : m_lightCaster(640, 400) {}

static inline uint32_t blendColors(uint32_t bg, uint32_t fg, uint8_t alpha) {
    if (alpha == 0) return bg;
    if (alpha >= 255) return fg;
    uint32_t invA = 255 - alpha;
    uint32_t r = (((bg >> 16) & 0xFF) * invA + ((fg >> 16) & 0xFF) * alpha) >> 8;
    uint32_t g = (((bg >> 8)  & 0xFF) * invA + ((fg >> 8)  & 0xFF) * alpha) >> 8;
    uint32_t b = ((bg & 0xFF)         * invA + (fg & 0xFF)         * alpha) >> 8;
    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

static uint16_t getGlyph(char c) {
    switch (c) {
        case '0': return 0b111110011001111;
        case '1': return 0b010010010010010;
        case '2': return 0b111001111100111;
        case '3': return 0b111001011001111;
        case '4': return 0b101101111001001;
        case '5': return 0b111100111001111;
        case '6': return 0b111100111101111;
        case '7': return 0b111001001010010;
        case '8': return 0b111101111101111;
        case '9': return 0b111101111001111;
        case ':': return 0b000010000010000;
        case 'A': return 0b010101111101101;
        case 'B': return 0b110101110101110;
        case 'C': return 0b011100100100011;
        case 'D': return 0b110101101101110;
        case 'E': return 0b111100110100111;
        case 'F': return 0b111100110100100;
        case 'G': return 0b011100101101011;
        case 'H': return 0b101101111101101;
        case 'I': return 0b111010010010111;
        case 'L': return 0b100100100100111;
        case 'M': return 0b101111101101101;
        case 'N': return 0b101111101101101;
        case 'O': return 0b111101101101111;
        case 'P': return 0b111101111100100;
        case 'R': return 0b110101110101101;
        case 'S': return 0b011100010001110;
        case 'T': return 0b111010010010010;
        case 'U': return 0b101101101101111;
        case 'X': return 0b101101010101101;
        case 'Z': return 0b111001010100111;
        case '[': return 0b011010010010011;
        case ']': return 0b110010010010110;
        case '-': return 0b000000111000000;
        case ' ': return 0;
        default:  return 0b111101101101111;
    }
}

void WorldView2D::drawChar(uint32_t* buf, int sw, int sh, int x, int y, char c, uint32_t color) {
    if (!buf || sw <= 0 || sh <= 0) return;
    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    uint16_t glyph = getGlyph(c);
    if (glyph == 0) return;

    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            int bit = (row * 3) + col;
            if ((glyph >> (14 - bit)) & 1) {
                int px = x + col;
                int py = y + row;
                if (px >= 0 && px < sw && py >= 0 && py < sh) {
                    buf[py * sw + px] = color;
                }
            }
        }
    }
}

void WorldView2D::drawText(uint32_t* buf, int sw, int sh, int x, int y, const std::string& text, uint32_t color) {
    if (!buf || sw <= 0 || sh <= 0) return;
    int curX = x;
    for (char c : text) {
        drawChar(buf, sw, sh, curX, y, c, color);
        curX += 4;
    }
}

void WorldView2D::drawLine(uint32_t* buf, int sw, int sh, int x0, int y0, int x1, int y1, uint32_t color) {
    if (!buf || sw <= 0 || sh <= 0) return;
    // Rejeição total se a linha estiver totalmente fora da tela
    if ((x0 < 0 && x1 < 0) || (x0 >= sw && x1 >= sw) ||
        (y0 < 0 && y1 < 0) || (y0 >= sh && y1 >= sh)) {
        return;
    }

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int maxSteps = dx + dy + 2;

    for (int step = 0; step < maxSteps; ++step) {
        if (x0 >= 0 && x0 < sw && y0 >= 0 && y0 < sh) {
            buf[y0 * sw + x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void WorldView2D::drawCircle(uint32_t* buf, int sw, int sh, int cx, int cy, int radius, uint32_t color, bool filled) {
    if (!buf || sw <= 0 || sh <= 0 || radius < 0) return;
    if (cx + radius < 0 || cx - radius >= sw || cy + radius < 0 || cy - radius >= sh) return;

    int r2 = radius * radius;
    int minX = std::clamp(cx - radius, 0, sw - 1);
    int maxX = std::clamp(cx + radius, 0, sw - 1);
    int minY = std::clamp(cy - radius, 0, sh - 1);
    int maxY = std::clamp(cy + radius, 0, sh - 1);

    if (minX > maxX || minY > maxY) return;

    for (int y = minY; y <= maxY; ++y) {
        int bufferRow = y * sw;
        for (int x = minX; x <= maxX; ++x) {
            int dx = x - cx;
            int dy = y - cy;
            int d2 = dx * dx + dy * dy;
            if (filled) {
                if (d2 <= r2) buf[bufferRow + x] = color;
            } else {
                if (d2 <= r2 && d2 >= (radius - 1) * (radius - 1)) buf[bufferRow + x] = color;
            }
        }
    }
}

void WorldView2D::initProps(const ZoneMap& map) {
    if (m_propsInitialized && m_lastMapSeed == map.getSeed()) return;
    m_props.clear();
    m_lastMapSeed = map.getSeed();

    for (int y = 0; y < map.getHeight(); ++y) {
        for (int x = 0; x < map.getWidth(); ++x) {
            auto propType = map.getPropType(x, y);
            // LOOT_CHEST é renderizado pelo CombatSystem como caixa militar interativa
            if (propType == DayDoom::Game::MapPropType::NONE || propType == DayDoom::Game::MapPropType::LOOT_CHEST) {
                continue;
            }

            RuinProp p;
            p.x = static_cast<float>(x) + 0.5f;
            p.y = static_cast<float>(y) + 0.5f;
            p.type = static_cast<int>(propType);
            m_props.push_back(p);
        }
    }
    m_propsInitialized = true;
}

void WorldView2D::drawStalker(
    uint32_t* buf, int sw, int sh,
    int cx, int cy, float dirX, float dirY,
    bool attacking, float time,
    const Engine::SpriteAtlas* entityAtlas,
    float tileSize,
    const Player* player,
    const Engine::SpriteAtlas* playerAnimAtlas
) {
    float perpX = -dirY;
    float perpY = dirX;

    // 1. Prioridade: Folha fluida de animação sem rotação no próprio eixo (sprites_player_anim.png)
    if (playerAnimAtlas && playerAnimAtlas->isLoaded() && player) {
        bool isMoving = player->isMoving();
        bool isRunning = player->isRunning();
        bool hasWeapon = player->hasWeaponEquipped();
        float hitStagger = player->getHitStaggerTimer();

        // Fixa a Coluna da folha baseada estritamente na direção que o jogador olha/mira:
        // Coluna 0: Baixo (Frente) | Coluna 1: Cima (Costas) | Coluna 2: Direita | Coluna 3: Esquerda
        int dirCol = 0;
        if (std::abs(dirX) > std::abs(dirY)) {
            dirCol = (dirX > 0.0f) ? 2 : 3;
        } else {
            dirCol = (dirY > 0.0f) ? 0 : 1;
        }

        int cellW = playerAnimAtlas->getWidth() / 4;
        int cellH = playerAnimAtlas->getHeight() / 4;
        int spriteSize = static_cast<int>(tileSize * 1.35f);
        drawCircle(buf, sw, sh, cx, cy + spriteSize / 4, static_cast<int>(tileSize * 0.28f), 0x8805080C, true);

        // Se estiver a sofrer dano (Hit Stagger): Linha 3 na coluna da direção
        if (hitStagger > 0.0f) {
            Engine::SpriteRect playerRect = { dirCol * cellW, 3 * cellH, cellW, cellH };
            playerAnimAtlas->drawSprite(
                buf, sw, sh,
                playerRect,
                cx - spriteSize / 2, cy - spriteSize / 2,
                spriteSize, spriteSize,
                2.0f
            );
        } else if (!isMoving && !attacking) {
            // PARADO (Idle): Linha 0 de sprites_entities (com arma) ou Linha 1 de playerAnim (sem arma)
            if (hasWeapon && entityAtlas && entityAtlas->isLoaded()) {
                int entCellW = entityAtlas->getWidth() / 4;
                int entCellH = entityAtlas->getHeight() / 4;
                Engine::SpriteRect playerRect = { dirCol * entCellW, 0 * entCellH, entCellW, entCellH };
                entityAtlas->drawSprite(
                    buf, sw, sh,
                    playerRect,
                    cx - spriteSize / 2, cy - spriteSize / 2,
                    spriteSize, spriteSize,
                    1.10f
                );
            } else {
                Engine::SpriteRect playerRect = { dirCol * cellW, 1 * cellH, cellW, cellH };
                playerAnimAtlas->drawSprite(
                    buf, sw, sh,
                    playerRect,
                    cx - spriteSize / 2, cy - spriteSize / 2,
                    spriteSize, spriteSize,
                    1.10f
                );
            }
        } else {
            // A ANDAR / CORRER: Alterna passos na MESMA direção (dirCol) com Head Bobbing!
            float stepSpeed = isRunning ? 0.12f : 0.20f;
            int stepPhase = static_cast<int>(time / stepSpeed) % 2;
            int bobOffsetY = (stepPhase == 1) ? -2 : 0;

            if (hasWeapon) {
                // Passo com arma: alterna entre pose base e passada (Linha 0 de animAtlas)
                if (stepPhase == 0 && entityAtlas && entityAtlas->isLoaded()) {
                    int entCellW = entityAtlas->getWidth() / 4;
                    int entCellH = entityAtlas->getHeight() / 4;
                    Engine::SpriteRect playerRect = { dirCol * entCellW, 0 * entCellH, entCellW, entCellH };
                    entityAtlas->drawSprite(
                        buf, sw, sh,
                        playerRect,
                        cx - spriteSize / 2, cy - spriteSize / 2 + bobOffsetY,
                        spriteSize, spriteSize,
                        attacking ? 1.35f : 1.10f
                    );
                } else {
                    Engine::SpriteRect playerRect = { dirCol * cellW, 0 * cellH, cellW, cellH };
                    playerAnimAtlas->drawSprite(
                        buf, sw, sh,
                        playerRect,
                        cx - spriteSize / 2, cy - spriteSize / 2 + bobOffsetY,
                        spriteSize, spriteSize,
                        attacking ? 1.35f : 1.10f
                    );
                }
            } else {
                // Desarmado: Linha 1 (Andar) e Linha 2 (Correr)
                int row = (stepPhase == 0) ? 1 : 2;
                Engine::SpriteRect playerRect = { dirCol * cellW, row * cellH, cellW, cellH };
                playerAnimAtlas->drawSprite(
                    buf, sw, sh,
                    playerRect,
                    cx - spriteSize / 2, cy - spriteSize / 2 + bobOffsetY,
                    spriteSize, spriteSize,
                    1.10f
                );
            }
        }

        if (attacking) {
            int fx = cx + static_cast<int>(dirX * (tileSize * 0.55f) + perpX * (tileSize * 0.10f));
            int fy = cy + static_cast<int>(dirY * (tileSize * 0.55f) + perpY * (tileSize * 0.10f));
            drawCircle(buf, sw, sh, fx, fy, static_cast<int>(tileSize * 0.18f), 0xFFFFF690, true);
            drawCircle(buf, sw, sh, fx, fy, static_cast<int>(tileSize * 0.08f), 0xFFFFFFFF, true);
        }
        return;
    }

    // 2. Fallback: Folha original de entidades (sprites_entities.png)
    if (entityAtlas && entityAtlas->isLoaded()) {
        // Escolhe a pose direcional do sobrevivente Hazmat na Folha 2 (sprites_entities.png)
        Engine::SpriteRect playerRect = Engine::AtlasCoords::PLAYER_IDLE_DOWN;
        if (std::abs(dirX) > std::abs(dirY) * 0.85f) {
            if (dirX >= 0.0f) {
                playerRect = attacking ? Engine::AtlasCoords::PLAYER_AIM_RIGHT : Engine::AtlasCoords::PLAYER_WALK_RIGHT;
            } else {
                playerRect = Engine::AtlasCoords::PLAYER_AIM_LEFT;
            }
        } else if (dirY < 0.0f) {
            playerRect = Engine::AtlasCoords::PLAYER_IDLE_UP;
        } else {
            playerRect = Engine::AtlasCoords::PLAYER_IDLE_DOWN;
        }

        int spriteSize = static_cast<int>(tileSize * 1.35f);
        drawCircle(buf, sw, sh, cx, cy + spriteSize / 4, static_cast<int>(tileSize * 0.28f), 0x8805080C, true);

        entityAtlas->drawSprite(
            buf, sw, sh,
            playerRect,
            cx - spriteSize / 2,
            cy - spriteSize / 2,
            spriteSize,
            spriteSize,
            attacking ? 1.35f : 1.10f
        );

        if (attacking) {
            int fx = cx + static_cast<int>(dirX * (tileSize * 0.55f) + perpX * (tileSize * 0.10f));
            int fy = cy + static_cast<int>(dirY * (tileSize * 0.55f) + perpY * (tileSize * 0.10f));
            drawCircle(buf, sw, sh, fx, fy, static_cast<int>(tileSize * 0.18f), 0xFFFFF690, true);
            drawCircle(buf, sw, sh, fx, fy, static_cast<int>(tileSize * 0.08f), 0xFFFFFFFF, true);
        }
        return;
    }

    (void)time;
    // Fallback procedural caso o Atlas não esteja carregado
    drawCircle(buf, sw, sh, cx + 1, cy + 3, 9, 0x9905080C, true);
    int bagX = cx - static_cast<int>(dirX * 6.5f);
    int bagY = cy - static_cast<int>(dirY * 6.5f);
    drawCircle(buf, sw, sh, bagX, bagY, 5, 0xFF362B1F, true);
    drawCircle(buf, sw, sh, cx, cy, 7, 0xFF46563F, true);
    drawCircle(buf, sw, sh, cx, cy, 6, 0xFF354230, true);
    int headX = cx + static_cast<int>(dirX * 1.5f);
    int headY = cy + static_cast<int>(dirY * 1.5f);
    drawCircle(buf, sw, sh, headX, headY, 5, 0xFF2A3626, true);
    drawCircle(buf, sw, sh, headX, headY, 4, 0xFF191D1E, true);
}

void WorldView2D::render(
    uint32_t* screenBuffer,
    int screenWidth,
    int screenHeight,
    const ZoneMap& map,
    const Player& player,
    const Engine::TextureManager& textures,
    float time,
    const Engine::SpriteAtlas* entityAtlas,
    const Engine::SpriteAtlas* objectsAtlas,
    const Engine::SpriteAtlas* playerAnimAtlas
) {
    initProps(map);

    // 1. Fundo escuro atmosférico da Zona (Névoa profunda de floresta noturna)
    const uint32_t DEEP_NIGHT_FOG = Engine::makeColor(10, 13, 16);
    std::fill(screenBuffer, screenBuffer + (screenWidth * screenHeight), DEEP_NIGHT_FOG);

    float pX = player.getCamera().pos.x;
    float pY = player.getCamera().pos.y;
    float dirX = player.getCamera().dir.x;
    float dirY = player.getCamera().dir.y;
    float aimAngle = std::atan2(dirY, dirX);

    // Padrão 720p / Steam Deck 1280x800 / 1080p: 20 blocos táticos na horizontal (64px ou 96px)
    float tileSize = getTileSize(screenWidth);
    const int lightStep = (screenWidth >= 1600) ? 8 : 6;

    float screenCenterX = static_cast<float>(screenWidth) * 0.5f;
    float screenCenterY = static_cast<float>(screenHeight) * 0.5f;

    // Posição física exata da ponta da arma / lanterna no mundo
    float perpX = -dirY;
    float perpY = dirX;
    float gunTipWorldX = pX + dirX * 0.58f + perpX * 0.15f;
    float gunTipWorldY = pY + dirY * 0.58f + perpY * 0.15f;

    // =========================================================================
    // 2. BUFFER DE SOMBRAS DDA ANGULAR DE ALTA DEFINIÇÃO (1440 RAIOS = 0.25° PASSO)
    // =========================================================================
    if (m_shadowDistances.size() != NUM_LIGHT_RAYS) {
        m_shadowDistances.resize(NUM_LIGHT_RAYS, 14.5f);
    }
    constexpr float TWO_PI = 6.28318530718f;
    constexpr float INV_TWO_PI = 1.0f / TWO_PI;

    for (int i = 0; i < NUM_LIGHT_RAYS; ++i) {
        float rayAngle = static_cast<float>(i) * (TWO_PI / static_cast<float>(NUM_LIGHT_RAYS));
        float rDirX = std::cos(rayAngle);
        float rDirY = std::sin(rayAngle);
        auto hit = m_lightCaster.castRay(gunTipWorldX, gunTipWorldY, rDirX, rDirY, 14.5f, map);
        m_shadowDistances[i] = hit.hit ? hit.dist : 14.5f;
    }

    // =========================================================================
    // 3. GRADE DE ILUMINAÇÃO VOLUMÉTRICA SUAVE (160x100 COM INTERPOLAÇÃO BILINEAR)
    // =========================================================================
    const int GW = (screenWidth + lightStep - 1) / lightStep;
    const int GH = (screenHeight + lightStep - 1) / lightStep;
    if (m_lightGrid.size() != static_cast<size_t>(GW * GH)) {
        m_lightGrid.resize(GW * GH);
    }
    std::fill(m_lightGrid.begin(), m_lightGrid.end(), 0.05f);
    auto& lightGrid = m_lightGrid;
    auto& shadowDistances = m_shadowDistances;

    constexpr float HALF_FOV = 0.58f; // Cone amplo de 66°
    constexpr float MAX_BEAM_DIST = 11.5f;

    for (int gy = 0; gy < GH; ++gy) {
        float sy = static_cast<float>(gy * lightStep + lightStep / 2);
        float wy = pY + (sy - screenCenterY) / tileSize;
        int rowIdx = gy * GW;

        for (int gx = 0; gx < GW; ++gx) {
            float sx = static_cast<float>(gx * lightStep + lightStep / 2);
            float wx = pX + (sx - screenCenterX) / tileSize;

            float dx = wx - gunTipWorldX;
            float dy = wy - gunTipWorldY;
            float dist = std::sqrt(dx * dx + dy * dy);

            float angle = std::atan2(dy, dx);
            if (angle < 0.0f) angle += TWO_PI;
            int rayIdx = static_cast<int>(angle * (static_cast<float>(NUM_LIGHT_RAYS) * INV_TWO_PI)) % NUM_LIGHT_RAYS;
            rayIdx = std::clamp(rayIdx, 0, NUM_LIGHT_RAYS - 1);
            float maxVisDist = shadowDistances[rayIdx];

            float intensity = 0.06f; // Luz residual da Zona

            if (dist <= maxVisDist + 0.16f) {
                float angleDiff = std::abs(angle - aimAngle);
                while (angleDiff > 3.14159265f) angleDiff = std::abs(angleDiff - TWO_PI);

                if (angleDiff < HALF_FOV && dist < MAX_BEAM_DIST) {
                    float angleFactor = std::cos((angleDiff / HALF_FOV) * 1.5707963f);
                    float distFactor = std::max(0.0f, 1.0f - (dist / MAX_BEAM_DIST));
                    float beam = std::pow(angleFactor, 1.4f) * distFactor * 1.25f;

                    if (angleDiff < HALF_FOV * 0.35f) {
                        beam += (1.0f - angleDiff / (HALF_FOV * 0.35f)) * distFactor * 0.35f;
                    }
                    intensity = std::max(intensity, beam);
                }

                float bodyDist = std::sqrt((wx - pX) * (wx - pX) + (wy - pY) * (wy - pY));
                if (bodyDist < 1.9f) {
                    float halo = (1.0f - (bodyDist / 1.9f)) * 0.48f;
                    intensity = std::max(intensity, halo);
                }
            }

            lightGrid[rowIdx + gx] = std::clamp(intensity, 0.06f, 1.45f);
        }
    }

    // =========================================================================
    // 4. RENDERIZAÇÃO DE PISOS, TERRENO DA FLORESTA E RUÍNAS 2.5D
    // =========================================================================
    int mapW = map.getWidth();
    int mapH = map.getHeight();

    int renderRadiusX = (screenWidth / 2) / static_cast<int>(tileSize) + 3;
    int renderRadiusY = (screenHeight / 2) / static_cast<int>(tileSize) + 3;
    int minCX = std::max(0, static_cast<int>(pX) - renderRadiusX);
    int maxCX = std::min(mapW - 1, static_cast<int>(pX) + renderRadiusX);
    int minCY = std::max(0, static_cast<int>(pY) - renderRadiusY);
    int maxCY = std::min(mapH - 1, static_cast<int>(pY) + renderRadiusY);

    // Passo 4.1: Renderiza o Piso contínuo com texturas ricas
    for (int cy = minCY; cy <= maxCY; ++cy) {
        for (int cx = minCX; cx <= maxCX; ++cx) {
            float tileScreenX = screenCenterX + (static_cast<float>(cx) - pX) * tileSize;
            float tileScreenY = screenCenterY + (static_cast<float>(cy) - pY) * tileSize;

            if (tileScreenX + tileSize < 0 || tileScreenX >= screenWidth ||
                tileScreenY + tileSize < 0 || tileScreenY >= screenHeight) {
                continue;
            }

            int floorType = map.getFloorType(cx, cy);
            int floorTexID = Engine::TextureManager::FOREST_EARTH;

            if (floorType == FLOOR_CONCRETE) floorTexID = Engine::TextureManager::CONC_FLOOR;
            else if (floorType == FLOOR_WOOD) floorTexID = Engine::TextureManager::WOOD_FLOOR;
            else if (floorType == FLOOR_ASPHALT) floorTexID = Engine::TextureManager::ASPHALT;

            int px0 = std::clamp(static_cast<int>(tileScreenX), 0, screenWidth);
            int py0 = std::clamp(static_cast<int>(tileScreenY), 0, screenHeight);
            int px1 = std::clamp(static_cast<int>(tileScreenX + tileSize), 0, screenWidth);
            int py1 = std::clamp(static_cast<int>(tileScreenY + tileSize), 0, screenHeight);

            float invW = 64.0f / tileSize;
            float invH = 64.0f / tileSize;
            float invLightStep = 1.0f / static_cast<float>(lightStep);

            for (int sy = py0; sy < py1; ++sy) {
                int ty = static_cast<int>((sy - tileScreenY) * invH) & 63;
                int bufferRow = sy * screenWidth;

                int gy = std::clamp(sy / lightStep, 0, GH - 1);
                int gy1 = std::min(GH - 1, gy + 1);
                float fy = (sy % lightStep) * invLightStep;

                for (int sx = px0; sx < px1; ++sx) {
                    int tx = static_cast<int>((sx - tileScreenX) * invW) & 63;

                    int gx = std::clamp(sx / lightStep, 0, GW - 1);
                    int gx1 = std::min(GW - 1, gx + 1);
                    float fx = (sx % lightStep) * invLightStep;

                    float l00 = lightGrid[gy * GW + gx];
                    float l10 = lightGrid[gy * GW + gx1];
                    float l01 = lightGrid[gy1 * GW + gx];
                    float l11 = lightGrid[gy1 * GW + gx1];
                    float light = (1.0f - fy) * ((1.0f - fx) * l00 + fx * l10) +
                                  fy * ((1.0f - fx) * l01 + fx * l11);

                    uint32_t texColor = textures.getPixel(floorTexID, tx, ty);

                    if (light > 0.15f) {
                        float warmR = std::min(1.08f, 1.0f + (light - 0.15f) * 0.08f);
                        float warmG = std::min(1.02f, 1.0f + (light - 0.15f) * 0.03f);
                        float warmB = std::max(0.85f, 1.0f - (light - 0.15f) * 0.12f);

                        uint8_t r = static_cast<uint8_t>(std::clamp(((texColor >> 16) & 0xFF) * light * warmR, 0.0f, 255.0f));
                        uint8_t g = static_cast<uint8_t>(std::clamp(((texColor >> 8)  & 0xFF) * light * warmG, 0.0f, 255.0f));
                        uint8_t b = static_cast<uint8_t>(std::clamp((texColor & 0xFF)         * light * warmB, 0.0f, 255.0f));
                        screenBuffer[bufferRow + sx] = Engine::makeColor(r, g, b);
                    } else {
                        screenBuffer[bufferRow + sx] = m_lightCaster.shadePixel(texColor, light);
                    }
                }
            }
        }
    }

    // Passo 4.2: Renderiza Paredes de Ruínas em 2.5D (Topo chanfrado + Face frontal vertical de tijolos expostos)
    int wallFaceHeight = std::max(8, static_cast<int>(tileSize * 0.25f));
    for (int cy = minCY; cy <= maxCY; ++cy) {
        for (int cx = minCX; cx <= maxCX; ++cx) {
            int cell = map.getCell(cx, cy);
            if (cell == 0 || map.isDoor(cx, cy)) continue;

            float tileScreenX = screenCenterX + (static_cast<float>(cx) - pX) * tileSize;
            float tileScreenY = screenCenterY + (static_cast<float>(cy) - pY) * tileSize;

            if (tileScreenX + tileSize < 0 || tileScreenX >= screenWidth ||
                tileScreenY + tileSize + wallFaceHeight < 0 || tileScreenY >= screenHeight) {
                continue;
            }

            int px0 = std::clamp(static_cast<int>(tileScreenX), 0, screenWidth);
            int py0 = std::clamp(static_cast<int>(tileScreenY), 0, screenHeight);
            int px1 = std::clamp(static_cast<int>(tileScreenX + tileSize), 0, screenWidth);
            int py1 = std::clamp(static_cast<int>(tileScreenY + tileSize), 0, screenHeight);

            float invW = 64.0f / tileSize;
            float invH = 64.0f / tileSize;

            bool hasSouthOpen = (cy + 1 < mapH) && (map.getCell(cx, cy + 1) == 0 || map.isDoor(cx, cy + 1));

            int texID = Engine::TextureManager::CONCRETE;
            if (cell == TYPE_WALL_BRICK) texID = Engine::TextureManager::BRICK_DARK;
            else if (cell == TYPE_HANGAR_BLUE) texID = Engine::TextureManager::HANGAR_BLUE;
            else if (cell == TYPE_MURO_SOVIET) texID = Engine::TextureManager::MURO_SOVIET;
            else if (cell == TYPE_CONTAINER_BLU) texID = Engine::TextureManager::CONTAINER_BLUE;
            else if (cell == TYPE_CONTAINER_RED) texID = Engine::TextureManager::CONTAINER_RED;
            else if (cell == TYPE_FENCE_WIRE) texID = Engine::TextureManager::FENCE_WIRE;
            else if (isWindowType(cell)) texID = Engine::TextureManager::WINDOW_WOOD_FIXED;

            for (int sy = py0; sy < py1; ++sy) {
                int ty = static_cast<int>((sy - tileScreenY) * invH) & 63;
                int bufferRow = sy * screenWidth;
                int gy = std::clamp(sy / lightStep, 0, GH - 1);

                for (int sx = px0; sx < px1; ++sx) {
                    int tx = static_cast<int>((sx - tileScreenX) * invW) & 63;
                    int gx = std::clamp(sx / lightStep, 0, GW - 1);
                    float light = lightGrid[gy * GW + gx];

                    uint32_t wallColor = textures.getPixel(texID, tx, ty);

                    // Cerca vazada: pixels muito escuros deixam ver o chão e inimigos do outro lado
                    if (cell == TYPE_FENCE_WIRE) {
                        uint8_t cr = (wallColor >> 16) & 0xFF;
                        uint8_t cg = (wallColor >> 8) & 0xFF;
                        uint8_t cb = wallColor & 0xFF;
                        if (cr < 35 && cg < 35 && cb < 35) {
                            continue;
                        }
                    }

                    if (ty <= 3 || tx <= 2) {
                        wallColor = Engine::applySideShade(wallColor, 0);
                    } else if (ty >= 60 || tx >= 61) {
                        wallColor = Engine::applySideShade(wallColor, 1);
                    }

                    screenBuffer[bufferRow + sx] = m_lightCaster.shadePixel(wallColor, light);
                }
            }

            if (hasSouthOpen && px0 < px1 && cell != TYPE_FENCE_WIRE && !isWindowType(cell)) {
                int rawFaceY0 = static_cast<int>(tileScreenY + tileSize);
                int faceY0 = std::clamp(rawFaceY0, 0, screenHeight);
                int faceY1 = std::clamp(rawFaceY0 + wallFaceHeight, 0, screenHeight);
                float faceInvH = 64.0f / static_cast<float>(wallFaceHeight);
                int frontTex = ((cx + cy) % 3 == 0) ? Engine::TextureManager::TACTICAL_PIPES : Engine::TextureManager::RUIN_BRICK_MOSS;

                for (int sy = faceY0; sy < faceY1; ++sy) {
                    int ty = static_cast<int>((sy - rawFaceY0) * faceInvH) & 63;
                    int bufferRow = sy * screenWidth;
                    int gy = std::clamp(sy / lightStep, 0, GH - 1);

                    for (int sx = px0; sx < px1; ++sx) {
                        int tx = static_cast<int>((sx - tileScreenX) * invW) & 63;
                        int gx = std::clamp(sx / lightStep, 0, GW - 1);
                        float light = lightGrid[gy * GW + gx];

                        uint32_t brickColor = textures.getPixel(frontTex, tx, ty);
                        brickColor = Engine::applySideShade(brickColor, 1);

                        screenBuffer[bufferRow + sx] = m_lightCaster.shadePixel(brickColor, light);
                    }
                }

                int rawShadowY0 = rawFaceY0 + wallFaceHeight;
                int shadowY0 = std::clamp(rawShadowY0, 0, screenHeight);
                int shadowY1 = std::clamp(rawShadowY0 + 4, 0, screenHeight);
                for (int sy = shadowY0; sy < shadowY1; ++sy) {
                    int bufferRow = sy * screenWidth;
                    uint8_t shadowAlpha = static_cast<uint8_t>(std::clamp(160 - (sy - rawShadowY0) * 35, 0, 255));
                    for (int sx = px0; sx < px1; ++sx) {
                        screenBuffer[bufferRow + sx] = blendColors(screenBuffer[bufferRow + sx], 0xFF05080C, shadowAlpha);
                    }
                }
            }
        }
    }

    // =========================================================================
    // 5. ADEREÇOS DAS RUÍNAS E FLORESTA (sprites_map_objects.png)
    // =========================================================================
    for (const auto& prop : m_props) {
        float propSx = screenCenterX + (prop.x - pX) * tileSize;
        float propSy = screenCenterY + (prop.y - pY) * tileSize;

        if (propSx < -80 || propSx > screenWidth + 80 || propSy < -80 || propSy > screenHeight + 80) continue;

        int gx = std::clamp(static_cast<int>(propSx) / lightStep, 0, GW - 1);
        int gy = std::clamp(static_cast<int>(propSy) / lightStep, 0, GH - 1);
        float light = lightGrid[gy * GW + gx];

        int isx = static_cast<int>(propSx);
        int isy = static_cast<int>(propSy);

        if (objectsAtlas && objectsAtlas->isLoaded()) {
            Engine::SpriteRect objRect = Engine::AtlasCoords::OBJ_CRATE_MOSS;
            int w = static_cast<int>(tileSize * 0.72f);
            int h = static_cast<int>(tileSize * 0.72f);

            auto pType = static_cast<DayDoom::Game::MapPropType>(prop.type);
            switch (pType) {
                case DayDoom::Game::MapPropType::BARREL_RAD:
                    objRect = Engine::AtlasCoords::OBJ_BARREL_RAD;
                    w = static_cast<int>(tileSize * 0.55f);
                    h = static_cast<int>(tileSize * 0.75f);
                    break;
                case DayDoom::Game::MapPropType::GENERATOR:
                    objRect = Engine::AtlasCoords::OBJ_GENERATOR;
                    w = static_cast<int>(tileSize * 0.90f);
                    h = static_cast<int>(tileSize * 0.80f);
                    break;
                case DayDoom::Game::MapPropType::PINE_TREE:
                    objRect = ((static_cast<int>(prop.x + prop.y)) % 2 == 0) ? Engine::AtlasCoords::OBJ_PINE_TREE_1 : Engine::AtlasCoords::OBJ_PINE_TREE_2;
                    w = static_cast<int>(tileSize * 1.15f);
                    h = static_cast<int>(tileSize * 1.85f);
                    break;
                case DayDoom::Game::MapPropType::RUBBLE_CAR:
                    objRect = ((static_cast<int>(prop.x + prop.y)) % 2 == 0) ? Engine::AtlasCoords::OBJ_RUBBLE_1 : Engine::AtlasCoords::OBJ_RUBBLE_2;
                    w = static_cast<int>(tileSize * 0.95f);
                    h = static_cast<int>(tileSize * 0.65f);
                    break;
                case DayDoom::Game::MapPropType::LOCKER:
                    objRect = Engine::AtlasCoords::OBJ_LOCKER_DOUBLE;
                    w = static_cast<int>(tileSize * 0.75f);
                    h = static_cast<int>(tileSize * 1.15f);
                    break;
                case DayDoom::Game::MapPropType::ELECTRIC_PANEL:
                    objRect = Engine::AtlasCoords::OBJ_PANEL_ELEC;
                    w = static_cast<int>(tileSize * 0.85f);
                    h = static_cast<int>(tileSize * 0.85f);
                    break;
                case DayDoom::Game::MapPropType::WORKBENCH:
                    objRect = Engine::AtlasCoords::OBJ_CRATE_CLOSED;
                    w = static_cast<int>(tileSize * 0.80f);
                    h = static_cast<int>(tileSize * 0.65f);
                    break;
                case DayDoom::Game::MapPropType::CRATE_WOOD:
                default:
                    objRect = Engine::AtlasCoords::OBJ_CRATE_MOSS;
                    w = static_cast<int>(tileSize * 0.72f);
                    h = static_cast<int>(tileSize * 0.72f);
                    break;
            }

            objectsAtlas->drawSprite(
                screenBuffer, screenWidth, screenHeight,
                objRect,
                isx - w / 2, isy - h / 2,
                w, h,
                light
            );
        } else if (prop.type == 0 || prop.type == 1) {
            int sz = static_cast<int>(tileSize * 0.5f);
            int x0 = isx - sz / 2;
            int y0 = isy - sz / 2;
            for (int py = 0; py < sz; ++py) {
                int sy = y0 + py;
                if (sy < 0 || sy >= screenHeight) continue;
                int ty = (py * 64) / sz;
                for (int px = 0; px < sz; ++px) {
                    int sx = x0 + px;
                    if (sx < 0 || sx >= screenWidth) continue;
                    int tx = (px * 64) / sz;
                    uint32_t c = textures.getPixel(Engine::TextureManager::WOOD_CRATE, tx, ty);
                    screenBuffer[sy * screenWidth + sx] = m_lightCaster.shadePixel(c, light);
                }
            }
        }
    }

    // =========================================================================
    // 6. OLHOS VERMELHOS SINISTROS NA ESCURIDÃO DA FLORESTA (ATMOSFERA STALKER)
    // =========================================================================
    for (int eyeIdx = 0; eyeIdx < 12; ++eyeIdx) {
        float angleOffset = static_cast<float>(eyeIdx) * (TWO_PI / 12.0f) + 0.35f;
        float eyeDist = 9.5f + (eyeIdx % 3) * 2.5f;
        float ewX = pX + std::cos(angleOffset) * eyeDist;
        float ewY = pY + std::sin(angleOffset) * eyeDist;

        float esX = screenCenterX + (ewX - pX) * tileSize;
        float esY = screenCenterY + (ewY - pY) * tileSize;

        if (esX >= 20 && esX < screenWidth - 20 && esY >= 20 && esY < screenHeight - 20) {
            int gx = std::clamp(static_cast<int>(esX) / lightStep, 0, GW - 1);
            int gy = std::clamp(static_cast<int>(esY) / lightStep, 0, GH - 1);
            float light = lightGrid[gy * GW + gx];

            if (light < 0.12f) {
                float blink = std::sin(time * 3.0f + eyeIdx * 2.3f);
                if (blink > -0.6f) {
                    float glowAlpha = 0.5f + 0.5f * std::sin(time * 2.0f + eyeIdx * 1.5f);
                    uint32_t eyeColor = (glowAlpha > 0.4f) ? 0xFFFF2515 : 0xFF991008;
                    int ix = static_cast<int>(esX);
                    int iy = static_cast<int>(esY);
                    drawCircle(screenBuffer, screenWidth, screenHeight, ix - 3, iy, 1, eyeColor, true);
                    drawCircle(screenBuffer, screenWidth, screenHeight, ix + 3, iy, 1, eyeColor, true);
                }
            }
        }
    }

    // =========================================================================
    // 7. PORTAS GIRATÓRIAS E ZONA DE EXTRAÇÃO TÁTICA (HELIPONTO COM PULSAÇÃO)
    // =========================================================================
    for (const auto& door : map.getDoors()) {
        float hx = door.getHingeX();
        float hy = door.getHingeY();
        float angle = door.getCurrentAngle();
        float length = door.getLength();

        float tipX = hx + std::cos(angle) * length;
        float tipY = hy + std::sin(angle) * length;

        int sx0 = static_cast<int>(screenCenterX + (hx - pX) * tileSize);
        int sy0 = static_cast<int>(screenCenterY + (hy - pY) * tileSize);
        int sx1 = static_cast<int>(screenCenterX + (tipX - pX) * tileSize);
        int sy1 = static_cast<int>(screenCenterY + (tipY - pY) * tileSize);

        if ((sx0 < -30 && sx1 < -30) || (sx0 > screenWidth + 30 && sx1 > screenWidth + 30) ||
            (sy0 < -30 && sy1 < -30) || (sy0 > screenHeight + 30 && sy1 > screenHeight + 30)) {
            continue;
        }

        float midX = (hx + tipX) * 0.5f;
        float midY = (hy + tipY) * 0.5f;
        int gx = std::clamp(static_cast<int>(screenCenterX + (midX - pX) * tileSize) / lightStep, 0, GW - 1);
        int gy = std::clamp(static_cast<int>(screenCenterY + (midY - pY) * tileSize) / lightStep, 0, GH - 1);
        float light = lightGrid[gy * GW + gx];

        uint32_t doorCol = m_lightCaster.shadePixel(0xFF8B5A2B, light);
        drawLine(screenBuffer, screenWidth, screenHeight, sx0, sy0, sx1, sy1, doorCol);
        drawLine(screenBuffer, screenWidth, screenHeight, sx0 + 1, sy0, sx1 + 1, sy1, doorCol);
        drawCircle(screenBuffer, screenWidth, screenHeight, sx0, sy0, 2, m_lightCaster.shadePixel(0xFFFFD700, light), true);
    }

    auto extPos = map.getExtractionPosition();
    int extSx = static_cast<int>(screenCenterX + (extPos.x - pX) * tileSize);
    int extSy = static_cast<int>(screenCenterY + (extPos.y - pY) * tileSize);

    if (extSx >= -60 && extSx <= screenWidth + 60 && extSy >= -60 && extSy <= screenHeight + 60) {
        float pulse = 0.5f + 0.5f * std::sin(time * 4.5f);
        uint32_t extColor = (pulse > 0.5f) ? 0xFF00FF88 : 0xFF00AA55;
        drawCircle(screenBuffer, screenWidth, screenHeight, extSx, extSy, static_cast<int>(tileSize * 1.6f), extColor, false);
        drawText(screenBuffer, screenWidth, screenHeight, extSx - 6, extSy - 6, "[H]", 0xFF00FF88);
        drawText(screenBuffer, screenWidth, screenHeight, extSx - 26, extSy + 10, "EXTRACTION ZONE", 0xFF00FF88);
    }

    for (int pIdx = 0; pIdx < 28; ++pIdx) {
        float pDist = 2.0f + static_cast<float>((pIdx * 17) % 110) * 0.1f;
        float pSpread = (static_cast<float>((pIdx * 31) % 100) / 100.0f - 0.5f) * 0.5f;
        float pWave = std::sin(time * 1.5f + pIdx) * 0.3f;
        float px = gunTipWorldX + dirX * pDist + perpX * (pSpread * pDist + pWave);
        float py = gunTipWorldY + dirY * pDist + perpY * (pSpread * pDist + pWave);

        int psX = static_cast<int>(screenCenterX + (px - pX) * tileSize);
        int psY = static_cast<int>(screenCenterY + (py - pY) * tileSize);

        if (psX >= 0 && psX < screenWidth && psY >= 0 && psY < screenHeight) {
            int gx = std::clamp(psX / lightStep, 0, GW - 1);
            int gy = std::clamp(psY / lightStep, 0, GH - 1);
            if (lightGrid[gy * GW + gx] > 0.45f) {
                screenBuffer[psY * screenWidth + psX] = blendColors(screenBuffer[psY * screenWidth + psX], 0xFFFFF8D0, 140);
            }
        }
    }

    // =========================================================================
    // 8. RENDERIZAÇÃO DO OPERADOR STALKER NO CENTRO DA TELA (sprites_player_anim.png / sprites_entities.png)
    // =========================================================================
    drawStalker(
        screenBuffer, screenWidth, screenHeight,
        static_cast<int>(screenCenterX), static_cast<int>(screenCenterY),
        dirX, dirY, player.isAttacking(), time,
        entityAtlas, tileSize,
        &player, playerAnimAtlas
    );
}

} // namespace DayDoom::Game::UI
