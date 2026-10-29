#include "LinhaTrem.hpp"

namespace DayDoom::Objetos {

LinhaTrem::LinhaTrem(int trackY, int elevacao)
    : m_trackY(trackY), m_elevacao(elevacao), m_startX(0), m_endX(64) {
    redefinirVagoesPadrao();
}

void LinhaTrem::redefinirVagoesPadrao() {
    m_vagoes.clear();
    // Composição oficial da ferrovia de Chernogorsk ao longo de Y = 20:
    // Contêineres cargueiros e seção industrial metálica
    m_vagoes.push_back({10, Game::TYPE_CONTAINER_RED});
    m_vagoes.push_back({11, Game::TYPE_CONTAINER_RED});
    m_vagoes.push_back({32, Game::TYPE_CONTAINER_BLU});
    m_vagoes.push_back({50, Game::TYPE_WALL_METAL});
}

void LinhaTrem::adicionarVagao(int x, int tipo) {
    m_vagoes.push_back({x, tipo});
}

bool LinhaTrem::pertenceALinha(int x, int y) const {
    return (y == m_trackY && x >= m_startX && x < m_endX);
}

int LinhaTrem::getElevationAt(int x, int y) const {
    if (pertenceALinha(x, y)) {
        return m_elevacao;
    }
    return 0;
}

void LinhaTrem::construirNaMalha(
    int* grid,
    int* floorGrid,
    int* elevationGrid,
    int mapWidth,
    int mapHeight
) const {
    if (m_trackY < 0 || m_trackY >= mapHeight) return;

    int minX = std::max(0, m_startX);
    int maxX = std::min(mapWidth, m_endX);

    // 1. Assenta o leito contínuo da ferrovia: piso FLOOR_RAILROAD, elevação de 10 e espaço livre
    for (int x = minX; x < maxX; ++x) {
        int idx = m_trackY * mapWidth + x;
        if (floorGrid) {
            floorGrid[idx] = Game::FLOOR_RAILROAD;
        }
        if (grid) {
            grid[idx] = Game::TYPE_EMPTY;
        }
        if (elevationGrid) {
            elevationGrid[idx] = m_elevacao; // Elevação 10
        }
    }

    // 2. Posiciona os vagões de carga e obstáculos na composição da ferrovia
    for (const auto& vagao : m_vagoes) {
        if (vagao.x >= 0 && vagao.x < mapWidth) {
            int idx = m_trackY * mapWidth + vagao.x;
            if (grid) {
                grid[idx] = vagao.tipo;
            }
        }
    }
}

} // namespace DayDoom::Objetos
