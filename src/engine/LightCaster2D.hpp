#pragma once

#include "Camera.hpp"
#include "MathDef.hpp"
#include <vector>
#include <cstdint>

namespace DayDoom::Game {
    class ZoneMap;
}

namespace DayDoom::Engine {

struct LightRayHit {
    bool hit = false;
    float dist = 0.0f;
    int hitX = 0;
    int hitY = 0;
    int side = 0; // 0 = vertical, 1 = horizontal
    int cellType = 0;
};

class LightCaster2D {
public:
    LightCaster2D(int screenWidth = 480, int screenHeight = 270);

    void setScreenDimensions(int w, int h);

    // Algoritmo DDA canônico: traça um raio de luz 2D contra a geometria de paredes e portas
    LightRayHit castRay(
        float startX, float startY,
        float dirX, float dirY,
        float maxDistance,
        const Game::ZoneMap& map
    ) const;

    // Calcula a iluminação pontual de uma coordenada do mundo (0.0f = escuridão total, 1.0f = brilho máximo)
    float getLightIntensityAt(
        float worldX, float worldY,
        const Camera& camera,
        const Game::ZoneMap& map
    ) const;

    // Teste de linha de visão direta (Line of Sight) usando DDA 2D
    bool hasLineOfSight(
        float x0, float y0,
        float x1, float y1,
        const Game::ZoneMap& map
    ) const;

    // Aplica o Fog of War dinâmico a uma cor ARGB com base na intensidade de luz calculada
    uint32_t shadePixel(uint32_t color, float lightIntensity) const;

    int getScreenWidth() const { return m_screenWidth; }
    int getScreenHeight() const { return m_screenHeight; }

private:
    int m_screenWidth;
    int m_screenHeight;
};

} // namespace DayDoom::Engine
