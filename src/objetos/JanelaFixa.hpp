#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>
#include "../engine/MathDef.hpp"
#include "../engine/Texture.hpp"
#include "../game/ZoneLocationsDef.hpp"

namespace DayDoom::Objetos {

// Estrutura de acerto de raio contra geometrias de janelas (fixas ou de abrir)
struct JanelaRayHit {
    bool hit = false;
    float dist = 1e9f;
    float u = 0.0f;     // Coordenada horizontal da textura [0, 1] ao longo do vão/folha
    int side = 0;       // Orientação para sombreamento (0 = vertical, 1 = horizontal)
    int texID = 0;      // ID da textura atingida
    int leafIndex = 0;  // 0 = fixa / marco / batente, 1 = folha esquerda, 2 = folha direita
};

using WindowRayHit = JanelaRayHit;

/**
 * @brief Janela fixa com caixilho de madeira e vidro quadriculado.
 * 
 * Posicionada em vãos de alvenaria e concreto, possui caixilho de madeira
 * nobre e malha de travessas de madeira (mullions/muntins) dividindo os
 * quadrículos de vidro reflexivo. É sólida, não passável e suporta dano balístico/físico.
 */
class JanelaFixa {
public:
    static constexpr float ESPESSURA = 0.08f; // Espessura do caixilho de madeira = 8cm
    static constexpr float THICKNESS = ESPESSURA;

    JanelaFixa() = default;
    JanelaFixa(int x, int y, int tipo = Game::TYPE_WINDOW_FIXED);

    // Configuração geométrica do vão e orientação no mapa
    void configurar(
        float cx, float cy,
        float dirX, float dirY,
        float comprimento = 0.96f,
        float espessura = ESPESSURA,
        bool vertical = false
    );

    // Interações e integridade estrutural
    void receberDano(float dano);
    void quebrar();
    void reparar(float valor = 100.0f);

    // Consultas de estado
    bool isQuebrada() const { return m_quebrada; }
    bool isBroken() const { return m_quebrada; }

    bool isPassavel() const { return false; }
    bool isWalkable() const { return false; }

    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getTipo() const { return m_tipo; }
    int getType() const { return m_tipo; }
    void setTipo(int tipo) { m_tipo = tipo; }
    void setType(int tipo) { m_tipo = tipo; }

    float getCentroX() const { return m_centroX; }
    float getCentroY() const { return m_centroY; }
    float getComprimento() const { return m_comprimento; }
    float getLength() const { return m_comprimento; }
    float getEspessura() const { return m_espessura; }
    float getThickness() const { return m_espessura; }
    bool isVertical() const { return m_isVertical; }

    float getDurabilidade() const { return m_durabilidade; }
    float getHealth() const { return m_durabilidade; }

    // Geometria 3D: Obtém os 4 vértices do paralelepípedo sólido da janela fixa
    void getVertices(
        Engine::Vec2& c0,
        Engine::Vec2& c1,
        Engine::Vec2& c2,
        Engine::Vec2& c3
    ) const;

    // Teste de interseção de raio contra as 4 faces sólidas da janela
    // (Face frontal, face traseira e marcos laterais do caixilho de madeira)
    JanelaRayHit intersectRay(
        float rayStartX, float rayStartY,
        float rayDirX, float rayDirY
    ) const;

private:
    int m_x = 0;
    int m_y = 0;
    int m_tipo = Game::TYPE_WINDOW_FIXED;
    bool m_isVertical = false;

    float m_centroX = 0.5f;
    float m_centroY = 0.5f;
    float m_dirX = 1.0f;
    float m_dirY = 0.0f;
    float m_comprimento = 0.96f;
    float m_espessura = ESPESSURA;

    bool m_quebrada = false;
    float m_durabilidade = 80.0f; // Resistência do vidro e caixilho
};

using FixedWindow = JanelaFixa;

} // namespace DayDoom::Objetos

namespace DayDoom::Game {
    using JanelaRayHit = Objetos::JanelaRayHit;
    using WindowRayHit = Objetos::WindowRayHit;
    using JanelaFixa = Objetos::JanelaFixa;
    using FixedWindow = Objetos::FixedWindow;
}
