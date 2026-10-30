#pragma once

#include "../ZoneMap.hpp"
#include "../Player.hpp"
#include "../../engine/Texture.hpp"
#include "../../engine/LightCaster2D.hpp"
#include "../../engine/SpriteAtlas.hpp"
#include <cstdint>
#include <vector>

namespace DayDoom::Game::UI {

struct RuinProp {
    float x = 0.0f;
    float y = 0.0f;
    int type = 0; // 0 = crate, 1 = barrel, 2 = generator, 3 = pine tree, 4 = rubble
    float animTimer = 0.0f;
};

class WorldView2D {
public:
    WorldView2D();

    // Renderiza a visão Top-Down 2D centralizada no jogador com blocos de 64x64 (20x12.5 em 1280x800)
    void render(
        uint32_t* screenBuffer,
        int screenWidth,
        int screenHeight,
        const ZoneMap& map,
        const Player& player,
        const Engine::TextureManager& textures,
        float time,
        const Engine::SpriteAtlas* entityAtlas = nullptr,
        const Engine::SpriteAtlas* objectsAtlas = nullptr,
        const Engine::SpriteAtlas* playerAnimAtlas = nullptr
    );

    float getTileSize(int screenWidth) const {
        // Mantém 20 blocos táticos na horizontal (64px em 1280x720 / 1280x800 e 96px em 1920x1080)
        return std::max(32.0f, static_cast<float>(screenWidth) / 20.0f);
    }

private:
    Engine::LightCaster2D m_lightCaster;
    std::vector<RuinProp> m_props;
    bool m_propsInitialized = false;
    uint32_t m_lastMapSeed = 0;

    void initProps(const ZoneMap& map);

    // Primitivas de renderização gráfica
    void drawCircle(uint32_t* buf, int sw, int sh, int cx, int cy, int radius, uint32_t color, bool filled = true);
    void drawLine(uint32_t* buf, int sw, int sh, int x0, int y0, int x1, int y1, uint32_t color);
    void drawChar(uint32_t* buf, int sw, int sh, int x, int y, char c, uint32_t color);
    void drawText(uint32_t* buf, int sw, int sh, int x, int y, const std::string& text, uint32_t color);
    void drawStalker(
        uint32_t* buf, int sw, int sh,
        int cx, int cy, float dirX, float dirY,
        bool attacking, float time,
        const Engine::SpriteAtlas* entityAtlas,
        float tileSize,
        const Player* player = nullptr,
        const Engine::SpriteAtlas* playerAnimAtlas = nullptr
    );

    std::vector<float> m_lightGrid;
    static constexpr int NUM_LIGHT_RAYS = 1440;
    std::vector<float> m_shadowDistances;
};

} // namespace DayDoom::Game::UI
