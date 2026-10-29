#include "JanelaAbrir.hpp"
#include <iostream>

namespace DayDoom::Objetos {

JanelaAbrir::JanelaAbrir(int x, int y, int tipo)
    : m_x(x), m_y(y), m_tipo(tipo) {
}

void JanelaAbrir::configurarVao(
    float h1x, float h1y,
    float h2x, float h2y,
    float outDirX, float outDirY,
    bool vertical
) {
    m_hinge1X = h1x;
    m_hinge1Y = h1y;
    m_hinge2X = h2x;
    m_hinge2Y = h2y;

    float dx = h2x - h1x;
    float dy = h2y - h1y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist > 1e-4f) {
        m_baseDirX = dx / dist;
        m_baseDirY = dy / dist;
        m_comprimentoVao = dist;
    } else {
        m_baseDirX = 1.0f;
        m_baseDirY = 0.0f;
        m_comprimentoVao = 0.96f;
    }

    float outLen = std::sqrt(outDirX * outDirX + outDirY * outDirY);
    if (outLen > 1e-4f) {
        m_outDirX = outDirX / outLen;
        m_outDirY = outDirY / outLen;
    } else {
        // Por padrão, normal para a esquerda da direção base
        m_outDirX = -m_baseDirY;
        m_outDirY = m_baseDirX;
    }

    m_folhaComprimento = m_comprimentoVao * 0.495f;
    m_isVertical = vertical;
}

void JanelaAbrir::configurarPorCelula(
    int x, int y,
    bool vertical,
    float outDirX, float outDirY
) {
    m_x = x;
    m_y = y;
    m_isVertical = vertical;

    if (vertical) {
        // Vão vertical ao longo de Y (X = x + 0.5)
        configurarVao(
            static_cast<float>(x) + 0.50f, static_cast<float>(y) + 0.02f,
            static_cast<float>(x) + 0.50f, static_cast<float>(y) + 0.98f,
            outDirX, outDirY,
            true
        );
    } else {
        // Vão horizontal ao longo de X (Y = y + 0.5)
        configurarVao(
            static_cast<float>(x) + 0.02f, static_cast<float>(y) + 0.50f,
            static_cast<float>(x) + 0.98f, static_cast<float>(y) + 0.50f,
            outDirX, outDirY,
            false
        );
    }
}

void JanelaAbrir::update(float deltaTime) {
    if (m_estado == EstadoJanela::ABRINDO) {
        m_progressoAbertura += VELOCIDADE_ABERTURA * deltaTime;
        if (m_progressoAbertura >= 1.0f) {
            m_progressoAbertura = 1.0f;
            m_estado = EstadoJanela::ABERTA;
        }
    } else if (m_estado == EstadoJanela::FECHANDO) {
        m_progressoAbertura -= VELOCIDADE_ABERTURA * deltaTime;
        if (m_progressoAbertura <= 0.0f) {
            m_progressoAbertura = 0.0f;
            m_estado = EstadoJanela::FECHADA;
        }
    }
    m_anguloAtual = m_progressoAbertura * ANGULO_MAXIMO;
}

void JanelaAbrir::toggle() {
    if (m_estado == EstadoJanela::FECHADA || m_estado == EstadoJanela::FECHANDO) {
        abrir();
    } else {
        fechar();
    }
}

void JanelaAbrir::abrir() {
    m_estado = EstadoJanela::ABRINDO;
}

void JanelaAbrir::fechar() {
    m_estado = EstadoJanela::FECHANDO;
}

void JanelaAbrir::receberDano(float dano, int folha) {
    if (folha == 1 || folha == 0) {
        m_saudeFolha1 = std::max(0.0f, m_saudeFolha1 - dano);
        if (m_saudeFolha1 <= 0.0f && !m_vidro1Quebrado) {
            quebrarVidro(1);
        }
    }
    if (folha == 2 || folha == 0) {
        m_saudeFolha2 = std::max(0.0f, m_saudeFolha2 - dano);
        if (m_saudeFolha2 <= 0.0f && !m_vidro2Quebrado) {
            quebrarVidro(2);
        }
    }
}

void JanelaAbrir::quebrarVidro(int folha) {
    if (folha == 1 || folha == 0) {
        m_vidro1Quebrado = true;
        m_saudeFolha1 = 0.0f;
    }
    if (folha == 2 || folha == 0) {
        m_vidro2Quebrado = true;
        m_saudeFolha2 = 0.0f;
    }
    std::cout << "[JANELA] Vidro da folha " << (folha == 0 ? "1 e 2" : (folha == 1 ? "1" : "2"))
              << " quebrado na janela (" << m_x << ", " << m_y << ")!" << std::endl;
}

void JanelaAbrir::getVerticesFolha1(
    Engine::Vec2& c0,
    Engine::Vec2& c1,
    Engine::Vec2& c2,
    Engine::Vec2& c3
) const {
    float cosA = std::cos(m_anguloAtual);
    float sinA = std::sin(m_anguloAtual);

    // Folha 1 (Esquerda): gira da base (+baseDir) em direção ao exterior (+outDir)
    float v1x = cosA * m_baseDirX + sinA * m_outDirX;
    float v1y = cosA * m_baseDirY + sinA * m_outDirY;

    // Normal perpendicular à Folha 1
    float m1x = -sinA * m_baseDirX + cosA * m_outDirX;
    float m1y = -sinA * m_baseDirY + cosA * m_outDirY;

    float T = ESPESSURA;
    float hT = 0.5f * T;
    float L = m_folhaComprimento;

    c0.x = m_hinge1X - hT * m1x;
    c0.y = m_hinge1Y - hT * m1y;

    c1.x = c0.x + L * v1x;
    c1.y = c0.y + L * v1y;

    c2.x = c1.x + T * m1x;
    c2.y = c1.y + T * m1y;

    c3.x = c0.x + T * m1x;
    c3.y = c0.y + T * m1y;
}

void JanelaAbrir::getVerticesFolha2(
    Engine::Vec2& c0,
    Engine::Vec2& c1,
    Engine::Vec2& c2,
    Engine::Vec2& c3
) const {
    float cosA = std::cos(m_anguloAtual);
    float sinA = std::sin(m_anguloAtual);

    // Folha 2 (Direita): gira do centro (-baseDir) em direção ao exterior (+outDir)
    float v2x = -cosA * m_baseDirX + sinA * m_outDirX;
    float v2y = -cosA * m_baseDirY + sinA * m_outDirY;

    // Normal perpendicular à Folha 2
    float m2x = sinA * m_baseDirX + cosA * m_outDirX;
    float m2y = sinA * m_baseDirY + cosA * m_outDirY;

    float T = ESPESSURA;
    float hT = 0.5f * T;
    float L = m_folhaComprimento;

    c0.x = m_hinge2X - hT * m2x;
    c0.y = m_hinge2Y - hT * m2y;

    c1.x = c0.x + L * v2x;
    c1.y = c0.y + L * v2y;

    c2.x = c1.x + T * m2x;
    c2.y = c1.y + T * m2y;

    c3.x = c0.x + T * m2x;
    c3.y = c0.y + T * m2y;
}

JanelaRayHit JanelaAbrir::intersectRay(
    float rayStartX, float rayStartY,
    float rayDirX, float rayDirY
) const {
    JanelaRayHit result;

    int casementTex = Engine::TextureManager::WINDOW_WOOD_CASEMENT;
    int frameTex = Engine::TextureManager::WINDOW_FRAME_WOOD;

    auto testLeafSegment = [&](float ax, float ay, float bx, float by, int texID, int leafIdx) {
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
            result.leafIndex = leafIdx;
        }
    };

    // 1. Testa as 4 faces da Folha 1 (Esquerda)
    Engine::Vec2 f1_c0, f1_c1, f1_c2, f1_c3;
    getVerticesFolha1(f1_c0, f1_c1, f1_c2, f1_c3);

    testLeafSegment(f1_c0.x, f1_c0.y, f1_c1.x, f1_c1.y, casementTex, 1); // Face frontal
    testLeafSegment(f1_c3.x, f1_c3.y, f1_c2.x, f1_c2.y, casementTex, 1); // Face traseira
    testLeafSegment(f1_c1.x, f1_c1.y, f1_c2.x, f1_c2.y, frameTex, 1);    // Ponta da folha
    testLeafSegment(f1_c0.x, f1_c0.y, f1_c3.x, f1_c3.y, frameTex, 1);    // Dobradiça

    // 2. Testa as 4 faces da Folha 2 (Direita)
    Engine::Vec2 f2_c0, f2_c1, f2_c2, f2_c3;
    getVerticesFolha2(f2_c0, f2_c1, f2_c2, f2_c3);

    testLeafSegment(f2_c0.x, f2_c0.y, f2_c1.x, f2_c1.y, casementTex, 2); // Face frontal
    testLeafSegment(f2_c3.x, f2_c3.y, f2_c2.x, f2_c2.y, casementTex, 2); // Face traseira
    testLeafSegment(f2_c1.x, f2_c1.y, f2_c2.x, f2_c2.y, frameTex, 2);    // Ponta da folha
    testLeafSegment(f2_c0.x, f2_c0.y, f2_c3.x, f2_c3.y, frameTex, 2);    // Dobradiça

    return result;
}

} // namespace DayDoom::Objetos
