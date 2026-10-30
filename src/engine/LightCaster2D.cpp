#include "LightCaster2D.hpp"
#include "../game/ZoneMap.hpp"
#include "../objetos/Porta.hpp"
#include <cmath>
#include <algorithm>

namespace DayDoom::Engine {

LightCaster2D::LightCaster2D(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth), m_screenHeight(screenHeight) {
}

void LightCaster2D::setScreenDimensions(int w, int h) {
    m_screenWidth = w;
    m_screenHeight = h;
}

LightRayHit LightCaster2D::castRay(
    float startX, float startY,
    float dirX, float dirY,
    float maxDistance,
    const Game::ZoneMap& map
) const {
    LightRayHit result;
    result.hit = false;
    result.dist = maxDistance;

    float len = std::sqrt(dirX * dirX + dirY * dirY);
    if (len < 0.0001f) return result;
    dirX /= len;
    dirY /= len;

    int mapX = static_cast<int>(std::floor(startX));
    int mapY = static_cast<int>(std::floor(startY));

    float deltaDistX = (dirX == 0.0f) ? 1e30f : std::abs(1.0f / dirX);
    float deltaDistY = (dirY == 0.0f) ? 1e30f : std::abs(1.0f / dirY);

    float sideDistX = 0.0f;
    float sideDistY = 0.0f;
    int stepX = 0;
    int stepY = 0;

    if (dirX < 0.0f) {
        stepX = -1;
        sideDistX = (startX - static_cast<float>(mapX)) * deltaDistX;
    } else {
        stepX = 1;
        sideDistX = (static_cast<float>(mapX) + 1.0f - startX) * deltaDistX;
    }

    if (dirY < 0.0f) {
        stepY = -1;
        sideDistY = (startY - static_cast<float>(mapY)) * deltaDistY;
    } else {
        stepY = 1;
        sideDistY = (static_cast<float>(mapY) + 1.0f - startY) * deltaDistY;
    }

    int side = 0;
    float currentDist = 0.0f;

    // Algoritmo DDA: Avanço célula por célula
    while (currentDist < maxDistance) {
        if (sideDistX < sideDistY) {
            currentDist = sideDistX;
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        } else {
            currentDist = sideDistY;
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }

        if (currentDist > maxDistance) break;

        if (mapX < 0 || mapX >= 64 || mapY < 0 || mapY >= 64) {
            result.hit = true;
            result.dist = currentDist;
            result.hitX = mapX;
            result.hitY = mapY;
            result.side = side;
            return result;
        }

        int cell = map.getCell(mapX, mapY);
        if (cell == 0) continue;

        // Se for porta, verificar se a folha da porta bloqueia fisicamente o raio
        if (map.isDoor(mapX, mapY)) {
            const auto* d = map.findDoor(mapX, mapY);
            if (d) {
                auto dHit = d->intersectRay(startX, startY, dirX, dirY);
                if (dHit.hit && dHit.dist <= maxDistance) {
                    result.hit = true;
                    result.dist = dHit.dist;
                    result.hitX = mapX;
                    result.hitY = mapY;
                    result.side = dHit.side;
                    result.cellType = cell;
                    return result;
                }
            }
            // Se a porta estiver aberta e o raio passar pelo vão, a luz viaja livremente!
            continue;
        }

        // Se a célula não bloqueia luz (ex: cerca de arame farpado ou janela), a lanterna DDA atravessa!
        if (!map.blocksLight(mapX, mapY)) {
            continue;
        }

        // Parede sólida encontrada: bloqueia a luz
        result.hit = true;
        result.dist = currentDist;
        result.hitX = mapX;
        result.hitY = mapY;
        result.side = side;
        result.cellType = cell;
        return result;
    }

    return result;
}

bool LightCaster2D::hasLineOfSight(
    float x0, float y0,
    float x1, float y1,
    const Game::ZoneMap& map
) const {
    float dx = x1 - x0;
    float dy = y1 - y0;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 0.05f) return true;

    LightRayHit hit = castRay(x0, y0, dx / dist, dy / dist, dist - 0.05f, map);
    return !hit.hit;
}

float LightCaster2D::getLightIntensityAt(
    float worldX, float worldY,
    const Camera& camera,
    const Game::ZoneMap& map
) const {
    float dx = worldX - camera.pos.x;
    float dy = worldY - camera.pos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    // Luz de proximidade imediata ao redor do jogador (360° íntimo)
    float closeRadius = 1.6f;
    float baseAmbient = camera.ambientLight;

    if (dist < closeRadius) {
        float closeIntensity = 1.0f - (dist / closeRadius) * 0.45f;
        if (hasLineOfSight(camera.pos.x, camera.pos.y, worldX, worldY, map)) {
            baseAmbient = std::max(baseAmbient, closeIntensity);
        }
    }

    if (dist > camera.flashlightRange) {
        return baseAmbient;
    }

    // Ângulo relativo da lanterna tática
    float angleToPoint = std::atan2(dy, dx);
    float angleDiff = std::abs(angleToPoint - camera.flashlightAngle);
    while (angleDiff > 3.14159265f) angleDiff = std::abs(angleDiff - 6.2831853f);

    float halfFOV = camera.flashlightFOV * 0.5f;
    if (angleDiff <= halfFOV) {
        // Ponto dentro do cone da lanterna: verifica oclusão com DDA
        if (hasLineOfSight(camera.pos.x, camera.pos.y, worldX, worldY, map)) {
            float distFactor = 1.0f - (dist / camera.flashlightRange);
            float angleFactor = 1.0f - (angleDiff / halfFOV);
            float beamIntensity = distFactor * (0.6f + 0.4f * angleFactor);
            return std::clamp(std::max(baseAmbient, beamIntensity), 0.0f, 1.0f);
        }
    }

    return baseAmbient;
}

uint32_t LightCaster2D::shadePixel(uint32_t color, float lightIntensity) const {
    float factor = std::clamp(lightIntensity, 0.05f, 1.0f);
    uint8_t a = (color >> 24) & 0xFF;
    uint8_t r = static_cast<uint8_t>(((color >> 16) & 0xFF) * factor);
    uint8_t g = static_cast<uint8_t>(((color >> 8)  & 0xFF) * factor);
    uint8_t b = static_cast<uint8_t>((color & 0xFF)         * factor);

    return (static_cast<uint32_t>(a) << 24) |
           (static_cast<uint32_t>(r) << 16) |
           (static_cast<uint32_t>(g) << 8)  |
           (static_cast<uint32_t>(b));
}

} // namespace DayDoom::Engine
