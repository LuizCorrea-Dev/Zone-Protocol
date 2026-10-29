#include "JanelaFixa.hpp"
#include <iostream>

namespace DayDoom::Objetos {

JanelaFixa::JanelaFixa(int x, int y, int tipo)
    : m_x(x), m_y(y), m_tipo(tipo),
      m_centroX(static_cast<float>(x) + 0.5f),
      m_centroY(static_cast<float>(y) + 0.5f) {
}

void JanelaFixa::configurar(
    float cx, float cy,
    float dirX, float dirY,
    float comprimento,
    float espessura,
    bool vertical
) {
    m_centroX = cx;
    m_centroY = cy;

    float len = std::sqrt(dirX * dirX + dirY * dirY);
    if (len > 1e-5f) {
        m_dirX = dirX / len;
        m_dirY = dirY / len;
    } else {
        m_dirX = 1.0f;
        m_dirY = 0.0f;
    }

    m_comprimento = comprimento;
    m_espessura = espessura;
    m_isVertical = vertical;
}

void JanelaFixa::receberDano(float dano) {
    m_durabilidade = std::max(0.0f, m_durabilidade - dano);
    if (m_durabilidade <= 0.0f && !m_quebrada) {
        quebrar();
    }
}

void JanelaFixa::quebrar() {
    m_quebrada = true;
    m_durabilidade = 0.0f;
    std::cout << "[JANELA] Vidro quadriculado estilhacado em: (" << m_x << ", " << m_y << ")!" << std::endl;
}

void JanelaFixa::reparar(float valor) {
    m_durabilidade = std::min(100.0f, m_durabilidade + valor);
    if (m_durabilidade > 30.0f) {
        m_quebrada = false;
    }
}

void JanelaFixa::getVertices(
    Engine::Vec2& c0,
    Engine::Vec2& c1,
    Engine::Vec2& c2,
    Engine::Vec2& c3
) const {
    float hL = 0.5f * m_comprimento;
    float hT = 0.5f * m_espessura;

    // Vetor unitário diretor da janela e seu vetor normal perpendicular
    float ux = m_dirX;
    float uy = m_dirY;

    float nx = -uy;
    float ny = ux;

    // c0 = C - hL*u - hT*n
    c0.x = m_centroX - hL * ux - hT * nx;
    c0.y = m_centroY - hL * uy - hT * ny;

    // c1 = C + hL*u - hT*n
    c1.x = m_centroX + hL * ux - hT * nx;
    c1.y = m_centroY + hL * uy - hT * ny;

    // c2 = C + hL*u + hT*n
    c2.x = m_centroX + hL * ux + hT * nx;
    c2.y = m_centroY + hL * uy + hT * ny;

    // c3 = C - hL*u + hT*n
    c3.x = m_centroX - hL * ux + hT * nx;
    c3.y = m_centroY - hL * uy + hT * ny;
}

JanelaRayHit JanelaFixa::intersectRay(
    float rayStartX, float rayStartY,
    float rayDirX, float rayDirY
) const {
    JanelaRayHit result;

    Engine::Vec2 c0, c1, c2, c3;
    getVertices(c0, c1, c2, c3);

    auto testSegment = [&](float ax, float ay, float bx, float by, int texID) {
        float vx = bx - ax;
        float vy = by - ay;
        float det = rayDirY * vx - rayDirX * vy;
        if (std::abs(det) < 1e-6f) return;

        float delX = ax - rayStartX;
        float delY = ay - rayStartY;
        float t = (delY * vx - delX * vy) / det;
        float s = (rayDirX * delY - rayDirY * delX) / det;

        if (t > 0.05f && t < result.dist && s >= 0.0f && s <= 1.0f) {
            result.hit = true;
            result.dist = t;
            result.u = s;
            result.side = (std::abs(vy) > std::abs(vx)) ? 0 : 1;
            result.texID = texID;
            result.leafIndex = 0;
        }
    };

    // Textura principal da janela de madeira com vidro quadriculado
    int mainTex = Engine::TextureManager::WINDOW_WOOD_FIXED;
    int frameTex = Engine::TextureManager::WINDOW_FRAME_WOOD;

    // Face 1: Lado externo/frontal da janela (C0 -> C1)
    testSegment(c0.x, c0.y, c1.x, c1.y, mainTex);

    // Face 2: Lado interno/traseiro da janela (C3 -> C2)
    testSegment(c3.x, c3.y, c2.x, c2.y, mainTex);

    // Face 3: Batente lateral direito do caixilho (C1 -> C2)
    testSegment(c1.x, c1.y, c2.x, c2.y, frameTex);

    // Face 4: Batente lateral esquerdo do caixilho (C0 -> C3)
    testSegment(c0.x, c0.y, c3.x, c3.y, frameTex);

    return result;
}

} // namespace DayDoom::Objetos
