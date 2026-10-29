#pragma once

#include <cstdint>
#include <vector>
#include <array>

namespace DayDoom::Engine {

class TextureManager {
public:
    static constexpr int TEX_SIZE = 64;
    static constexpr int NUM_TEXTURES = 32;

    enum TextureID {
        BRICK_DARK = 0,       // BRICK01 - Prédios residenciais e tijolo antigo
        CONCRETE = 1,         // CONC02 - Esquadra de polícia e edifícios públicos
        STORE_FRONT = 2,      // WINCOMM - Vitrine / Janela comercial do supermercado
        WOOD_DOOR = 3,        // DOORWOOD - Porta de madeira residencial
        RUSTY_FENCE = 4,      // CORRMT1 - Chapa metálica ondulada dos armazéns
        ASPHALT = 5,          // ASPHALT - Asfalto com rachaduras e faixas gastas
        WOOD_FLOOR = 6,       // WOOD_FLOOR - Piso de madeira dos quartos
        SKY_OVERCAST = 7,     // F_SKY1 - Céu cinzento apocalíptico da Zona de Exclusão
        CONTAINER_RED = 8,    // CONTRED - Contêiner marítimo vermelho do porto
        CONTAINER_BLUE = 9,   // CONTBLU - Contêiner marítimo azul do porto
        DOOR_SLIDING = 10,    // DOORMLT - Porta corrediça de aço da armaria policial
        GATE_IRON = 11,       // GATEIRON - Portão de ferro quadriculado das docas
        FENCE_WIRE = 12,      // FENCEWI - Cerca de arame farpado perimetral
        HOSPITAL_WALL = 13,   // HOSP01 - Hospital de campanha / enfermaria
        SAND = 14,            // SAND01 - Areia da praia do Mar Báltico
        WATER = 15,           // WATER01 - Água do Mar Báltico
        CONC_FLOOR = 16,      // CONC01 - Chão de concreto da polícia e pátio
        RAILROAD = 17,        // RAILROAD - Trilhos de trem com dormentes de madeira
        HANGAR_BLUE = 18,     // HANGARBL - Galpão industrial com chapa azul ondulada
        POLICE_EMBLEM = 19,   // POLICESH - Fachada com o brasão do escudo da Polícia
        MURO_SOVIET = 20,     // MUROPO2 - Muro de placas de concreto pré-moldado soviético PO-2
        MURO_STONE = 21,      // MUROSTN - Muro de pedra e alvenaria de quintais residenciais
        DOOR_HOSPITAL = 22,   // DOORHOSP - Porta dupla branca com cruz vermelha do hospital
        DOOR_FRAME = 23,      // DOORFRM - Batente e marco lateral da porta em metal e madeira escura
        WINDOW_WOOD_FIXED = 24,   // WINFIXED - Janela fixa com caixilho de madeira e vidro quadriculado
        WINDOW_WOOD_CASEMENT = 25,// WINCASE  - Folha de abrir da janela de madeira com vidro
        WINDOW_FRAME_WOOD = 26,   // WINFRM   - Marco / batente de madeira das janelas
        STAIRS_CONCRETE = 27,     // STAIRCON - Escada de concreto e metal industrial com frisos antiderrapantes
        RUIN_BRICK_MOSS = 28,     // RUINBRK  - Tijolos vermelhos desgastados com reboco descascando e musgo
        FOREST_EARTH = 29,        // FOREART  - Terra escura de floresta com agulhas de pinheiro e musgo
        TACTICAL_PIPES = 30,      // RUSTPIPE - Tubulações industriais enferrujadas com juntas e válvulas
        WOOD_CRATE = 31           // MILCRATE - Caixa militar de suprimentos com ferragens de aço
    };

    TextureManager();

    // Inicializa as texturas procedurais estilo Doom clássico (WAD procedural)
    void initProceduralTextures();

    // Retorna o pixel (U, V) da textura selecionada (u e v entre 0 e 63)
    inline uint32_t getPixel(int texID, int u, int v) const {
        if (texID < 0 || texID >= NUM_TEXTURES) {
            texID = 0;
        }
        u = u & (TEX_SIZE - 1);
        v = v & (TEX_SIZE - 1);
        return m_textures[texID][v * TEX_SIZE + u];
    }

private:
    std::array<std::vector<uint32_t>, NUM_TEXTURES> m_textures;
};

} // namespace DayDoom::Engine
