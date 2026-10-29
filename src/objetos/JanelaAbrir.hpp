#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>
#include "../engine/MathDef.hpp"
#include "../engine/Texture.hpp"
#include "../game/ZoneLocationsDef.hpp"
#include "JanelaFixa.hpp"

namespace DayDoom::Objetos {

enum class EstadoJanela {
    FECHADA = 0,
    ABRINDO = 1,
    ABERTA = 2,
    FECHANDO = 3
};

using WindowState = EstadoJanela;

/**
 * @brief Janela de abrir interativa de 2 folhas com vidro quadriculado e abertura para fora.
 * 
 * Funciona de forma análoga à Porta (interagível com 'F' / 'Espaço'), composta por duas
 * folhas com esquadria de madeira e quadrículos de vidro. As duas folhas possuem dobradiças
 * nas extremidades do vão e giram sincronizadas para o exterior do edifício (para fora).
 */
class JanelaAbrir {
public:
    static constexpr float ESPESSURA = 0.05f;           // Espessura de cada folha de madeira = 5cm
    static constexpr float THICKNESS = ESPESSURA;
    static constexpr float VELOCIDADE_ABERTURA = 1.8f;   // Abertura completa de 0° a 90° em ~0.55s
    static constexpr float ANGULO_MAXIMO = 1.57079632679f; // 90 graus em radianos

    JanelaAbrir() = default;
    JanelaAbrir(int x, int y, int tipo = Game::TYPE_WINDOW_OPENABLE);

    // Configuração geométrica do vão e dobradiças das duas folhas
    void configurarVao(
        float h1x, float h1y,
        float h2x, float h2y,
        float outDirX, float outDirY,
        bool vertical = false
    );

    // Configuração simplificada com base nas coordenadas de célula e orientação
    void configurarPorCelula(
        int x, int y,
        bool vertical,
        float outDirX, float outDirY
    );

    // Ciclo de atualização contínua e animação suave
    void update(float deltaTime);
    void toggle();
    void abrir();
    void fechar();

    void open() { abrir(); }
    void close() { fechar(); }

    // Consultas de estado
    bool isAberta() const { return m_progressoAbertura > 0.5f; }
    bool isOpen() const { return isAberta(); }

    bool isFechada() const { return m_progressoAbertura <= 0.02f; }
    bool isClosed() const { return isFechada(); }

    bool isPassavel() const { return m_progressoAbertura > 0.60f; }
    bool isWalkable() const { return isPassavel(); }

    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getTipo() const { return m_tipo; }
    int getType() const { return m_tipo; }
    void setTipo(int tipo) { m_tipo = tipo; }
    void setType(int tipo) { m_tipo = tipo; }

    EstadoJanela getEstado() const { return m_estado; }
    WindowState getState() const { return m_estado; }
    void setEstado(EstadoJanela estado) { m_estado = estado; }
    void setState(WindowState estado) { m_estado = estado; }

    float getProgressoAbertura() const { return m_progressoAbertura; }
    float getOpenProgress() const { return m_progressoAbertura; }

    float getAnguloAtual() const { return m_anguloAtual; }
    float getCurrentAngle() const { return m_anguloAtual; }

    bool isVertical() const { return m_isVertical; }
    float getCentroX() const { return 0.5f * (m_hinge1X + m_hinge2X); }
    float getCentroY() const { return 0.5f * (m_hinge1Y + m_hinge2Y); }

    float getHinge1X() const { return m_hinge1X; }
    float getHinge1Y() const { return m_hinge1Y; }
    float getHinge2X() const { return m_hinge2X; }
    float getHinge2Y() const { return m_hinge2Y; }
    float getOutDirX() const { return m_outDirX; }
    float getOutDirY() const { return m_outDirY; }

    float getComprimentoVao() const { return m_comprimentoVao; }
    float getFolhaComprimento() const { return m_folhaComprimento; }

    // Sistema de integridade balística / quebra dos vidros quadriculados
    void receberDano(float dano, int folha = 0);
    void quebrarVidro(int folha = 0);
    bool isVidroFolha1Quebrado() const { return m_vidro1Quebrado; }
    bool isVidroFolha2Quebrado() const { return m_vidro2Quebrado; }

    // Geometria 3D: Obtém os 4 vértices de cada uma das duas folhas sólidas
    void getVerticesFolha1(
        Engine::Vec2& c0,
        Engine::Vec2& c1,
        Engine::Vec2& c2,
        Engine::Vec2& c3
    ) const;

    void getVerticesFolha2(
        Engine::Vec2& c0,
        Engine::Vec2& c1,
        Engine::Vec2& c2,
        Engine::Vec2& c3
    ) const;

    // Teste de interseção de raio contra as faces das 2 folhas giratórias
    JanelaRayHit intersectRay(
        float rayStartX, float rayStartY,
        float rayDirX, float rayDirY
    ) const;

private:
    int m_x = 0;
    int m_y = 0;
    int m_tipo = Game::TYPE_WINDOW_OPENABLE;
    EstadoJanela m_estado = EstadoJanela::FECHADA;

    float m_progressoAbertura = 0.0f; // [0.0f, 1.0f]
    float m_anguloAtual = 0.0f;       // [0.0f, PI/2]
    bool m_isVertical = false;

    // Dobradiça 1 (Folha Esquerda / Início do Vão)
    float m_hinge1X = 0.0f;
    float m_hinge1Y = 0.0f;

    // Dobradiça 2 (Folha Direita / Fim do Vão)
    float m_hinge2X = 1.0f;
    float m_hinge2Y = 0.0f;

    // Vetores unitários do vão e projeção para fora
    float m_baseDirX = 1.0f;
    float m_baseDirY = 0.0f;
    float m_outDirX = 0.0f;
    float m_outDirY = 1.0f;

    float m_comprimentoVao = 0.96f;
    float m_folhaComprimento = 0.475f;

    // Integridade do vidro de cada folha
    bool m_vidro1Quebrado = false;
    bool m_vidro2Quebrado = false;
    float m_saudeFolha1 = 60.0f;
    float m_saudeFolha2 = 60.0f;
};

using JanelaDupla = JanelaAbrir;
using CasementWindow = JanelaAbrir;
using OpenableWindow = JanelaAbrir;

} // namespace DayDoom::Objetos

namespace DayDoom::Game {
    using EstadoJanela = Objetos::EstadoJanela;
    using WindowState = Objetos::WindowState;
    using JanelaAbrir = Objetos::JanelaAbrir;
    using JanelaDupla = Objetos::JanelaDupla;
    using CasementWindow = Objetos::CasementWindow;
    using OpenableWindow = Objetos::OpenableWindow;
}
