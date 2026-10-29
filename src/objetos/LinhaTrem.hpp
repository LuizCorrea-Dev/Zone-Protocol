#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include "../engine/MathDef.hpp"
#include "../game/ZoneLocationsDef.hpp"

namespace DayDoom::Objetos {

// Representa um vagão cargueiro ou obstáculo sobre os trilhos da ferrovia
struct VagaoLinhaTrem {
    int x = 0;
    int tipo = Game::TYPE_CONTAINER_RED;
};

using TrainCar = VagaoLinhaTrem;

class LinhaTrem {
public:
    // Elevação canônica do leito e blocos da linha férrea = 10 unidades
    static constexpr int ELEVACAO_PADRAO = 10;
    static constexpr int Y_PADRAO = 20;

    LinhaTrem(int trackY = Y_PADRAO, int elevacao = ELEVACAO_PADRAO);

    // Eixo longitudinal da ferrovia
    int getY() const { return m_trackY; }
    void setY(int y) { m_trackY = y; }

    int getElevacao() const { return m_elevacao; }
    int getElevation() const { return m_elevacao; }
    void setElevacao(int elev) { m_elevacao = elev; }
    void setElevation(int elev) { m_elevacao = elev; }

    int getStartX() const { return m_startX; }
    int getEndX() const { return m_endX; }
    void setExtensao(int startX, int endX) { m_startX = startX; m_endX = endX; }

    // Consultas espaciais
    bool pertenceALinha(int x, int y) const;
    bool isRailroadCell(int x, int y) const { return pertenceALinha(x, y); }
    int getElevationAt(int x, int y) const;

    // Constrói e aplica a ferrovia (piso FLOOR_RAILROAD, elevação de 10 e composição de vagões) na malha urbana
    void construirNaMalha(
        int* grid,
        int* floorGrid,
        int* elevationGrid,
        int mapWidth,
        int mapHeight
    ) const;

    // Gerenciamento dos vagões e composições da linha férrea
    const std::vector<VagaoLinhaTrem>& getVagoes() const { return m_vagoes; }
    void adicionarVagao(int x, int tipo);
    void limparVagoes() { m_vagoes.clear(); }
    void redefinirVagoesPadrao();

private:
    int m_trackY = Y_PADRAO;
    int m_elevacao = ELEVACAO_PADRAO;
    int m_startX = 0;
    int m_endX = 64;
    std::vector<VagaoLinhaTrem> m_vagoes;
};

using LinhaDoTrem = LinhaTrem;
using Railroad = LinhaTrem;
using RailroadTrack = LinhaTrem;

} // namespace DayDoom::Objetos

namespace DayDoom::Game {
    using LinhaTrem = Objetos::LinhaTrem;
    using LinhaDoTrem = Objetos::LinhaDoTrem;
    using Railroad = Objetos::Railroad;
    using RailroadTrack = Objetos::RailroadTrack;
    using VagaoLinhaTrem = Objetos::VagaoLinhaTrem;
    using TrainCar = Objetos::TrainCar;
}
