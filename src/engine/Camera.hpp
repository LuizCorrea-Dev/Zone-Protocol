#pragma once

#include "MathDef.hpp"
#include <cmath>

namespace DayDoom::Engine {

class Camera {
public:
    // Posição central do foco da câmera no mundo (coordenadas do mapa em tiles)
    Vec2 pos{3.5f, 3.5f};

    // Direção e plano (para compatibilidade vetorial)
    Vec2 dir{1.0f, 0.0f};
    Vec2 plane{0.0f, 0.66f};

    // Parâmetros específicos para Top-Down 2D e Iluminação Tática
    float zoom{20.0f};               // Pixels por tile do mapa (escala de renderização Top-Down)
    float minZoom{8.0f};
    float maxZoom{48.0f};

    float flashlightAngle{0.0f};     // Ângulo da lanterna/mira em radianos (0 a 2*PI)
    float flashlightFOV{1.15f};      // Abertura angular do cone de luz (~66 graus)
    float flashlightRange{14.0f};    // Alcance máximo da luz em tiles
    float ambientLight{0.18f};       // Luz ambiente mínima nas áreas fora do cone (Fog of War)

    float zOffset{0.0f};             // Elevação vertical suave

    Camera() = default;
    Camera(float x, float y, float angleRadians = 0.0f, float zoomLevel = 20.0f)
        : pos{x, y}, zoom{zoomLevel} {
        setAngle(angleRadians);
    }

    void setAngle(float angleRadians) {
        flashlightAngle = angleRadians;
        dir.x = std::cos(angleRadians);
        dir.y = std::sin(angleRadians);
        plane.x = -dir.y * 0.66f;
        plane.y = dir.x * 0.66f;
    }

    void rotate(float angleRadians) {
        setAngle(flashlightAngle + angleRadians);
    }

    void setZoom(float z) {
        if (z < minZoom) z = minZoom;
        if (z > maxZoom) z = maxZoom;
        zoom = z;
    }

    void adjustZoom(float delta) {
        setZoom(zoom + delta);
    }

    // Conversão de coordenadas de Mundo para Tela (com a câmera centralizada na tela)
    void worldToScreen(float wx, float wy, int screenWidth, int screenHeight, int& outSx, int& outSy) const {
        float centerX = screenWidth * 0.5f;
        float centerY = screenHeight * 0.5f;
        outSx = static_cast<int>(centerX + (wx - pos.x) * zoom);
        outSy = static_cast<int>(centerY + (wy - pos.y) * zoom);
    }

    // Conversão de coordenadas da Tela para Mundo
    void screenToWorld(int sx, int sy, int screenWidth, int screenHeight, float& outWx, float& outWy) const {
        float centerX = screenWidth * 0.5f;
        float centerY = screenHeight * 0.5f;
        outWx = pos.x + (static_cast<float>(sx) - centerX) / zoom;
        outWy = pos.y + (static_cast<float>(sy) - centerY) / zoom;
    }
};

} // namespace DayDoom::Engine
