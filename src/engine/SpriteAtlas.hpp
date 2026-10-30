#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace ZoneProtocol::Engine {

struct SpriteRect {
    int x = 0;
    int y = 0;
    int w = 64;
    int h = 64;
};

// Mapeamento direto em Grade e Coordenadas Reais para as folhas geradas (2048x2048 e 2752x1536)
namespace AtlasCoords {
    inline SpriteRect fromGrid(int col, int row, int cellW, int cellH, int margin = 0) {
        return { col * cellW + margin, row * cellH + margin, cellW - margin * 2, cellH - margin * 2 };
    }

    // --- FOLHA 1: tileset_walls_floors.png (2048x2048 -> grade 8x8 de 256x256, margem interna 10px) ---
    constexpr int TILE_CELL = 256;
    constexpr int TILE_MARGIN = 10;
    inline SpriteRect tileAt(int col, int row) {
        return fromGrid(col, row, TILE_CELL, TILE_CELL, TILE_MARGIN);
    }

    // --- FOLHA 2: sprites_entities.png (2048x2048 -> células 512x512 em grade 4x4) ---
    constexpr int ENT_CELL = 512;
    const SpriteRect PLAYER_IDLE_DOWN  = { 0 * ENT_CELL, 0 * ENT_CELL, ENT_CELL, ENT_CELL };
    const SpriteRect PLAYER_IDLE_UP    = { 1 * ENT_CELL, 0 * ENT_CELL, ENT_CELL, ENT_CELL };
    const SpriteRect PLAYER_IDLE_RIGHT = { 2 * ENT_CELL, 0 * ENT_CELL, ENT_CELL, ENT_CELL };
    const SpriteRect PLAYER_IDLE_LEFT  = { 3 * ENT_CELL, 0 * ENT_CELL, ENT_CELL, ENT_CELL };

    const SpriteRect PLAYER_AIM_RIGHT  = { 0 * ENT_CELL, 1 * ENT_CELL, ENT_CELL, ENT_CELL };
    const SpriteRect PLAYER_WALK_UP    = { 1 * ENT_CELL, 1 * ENT_CELL, ENT_CELL, ENT_CELL };
    const SpriteRect PLAYER_WALK_RIGHT = { 2 * ENT_CELL, 1 * ENT_CELL, ENT_CELL, ENT_CELL };
    const SpriteRect PLAYER_AIM_LEFT   = { 3 * ENT_CELL, 1 * ENT_CELL, ENT_CELL, ENT_CELL };

    const SpriteRect CRAWLER_FRONT     = { 0 * ENT_CELL, 2 * ENT_CELL, ENT_CELL, ENT_CELL };
    const SpriteRect CRAWLER_BACK      = { 1 * ENT_CELL, 2 * ENT_CELL, ENT_CELL, ENT_CELL };
    const SpriteRect CRAWLER_RIGHT     = { 2 * ENT_CELL, 2 * ENT_CELL, ENT_CELL, ENT_CELL };
    const SpriteRect CRAWLER_LEFT      = { 3 * ENT_CELL, 2 * ENT_CELL, ENT_CELL, ENT_CELL };

    const SpriteRect BRUTE_FRONT       = { 0 * ENT_CELL, 3 * ENT_CELL, ENT_CELL, ENT_CELL };
    const SpriteRect BRUTE_BACK        = { 1 * ENT_CELL, 3 * ENT_CELL, ENT_CELL, ENT_CELL };
    const SpriteRect BRUTE_RIGHT       = { 2 * ENT_CELL, 3 * ENT_CELL, ENT_CELL, ENT_CELL };
    const SpriteRect BRUTE_LEFT        = { 3 * ENT_CELL, 3 * ENT_CELL, ENT_CELL, ENT_CELL };

    // --- FOLHAS DE ANIMAÇÃO FLUIDA HD (2048x2048 -> Células 512x512 ou 256x256) ---
    // sprites_player_anim.png:
    // Linha 0: Caminhada / Corrida com Arma AK-74
    // Linha 1: Mira e Passada Tática com Arma
    // Linha 2: Caminhada / Corrida Desarmado (Bunker / Fuga)
    // Linha 3: Hit Stagger / Sofrer Dano (Impacto e Sangue)
    inline SpriteRect playerAnim(int frame, int row, int cell = ENT_CELL) {
        return fromGrid(frame % 4, std::clamp(row, 0, 3), cell, cell);
    }

    // sprites_crawler_anim.png:
    // Linhas 0-1: Deslocamento furtivo e corrida rápida (4 direções)
    // Linha 2: Perseguição agressiva
    // Linha 3: Bote / Ataque garras e mandíbula + Reação a dano
    inline SpriteRect crawlerAnim(int frame, int row, int cell = ENT_CELL) {
        return fromGrid(frame % 4, std::clamp(row, 0, 3), cell, cell);
    }

    // sprites_brute_anim.png:
    // Linhas 0-1: Passada pesada de patrulha e investida (Charge)
    // Linha 2: Golpe duplo de garras (Melee Smash)
    // Linha 3: Reação a dano / Rugido
    inline SpriteRect bruteAnim(int frame, int row, int cell = ENT_CELL) {
        return fromGrid(frame % 4, std::clamp(row, 0, 3), cell, cell);
    }

    // --- FOLHA 3: sprites_map_objects.png (2048x2048) ---
    const SpriteRect OBJ_CRATE_OPEN      = { 30, 58, 202, 200 };
    const SpriteRect OBJ_CRATE_CLOSED    = { 34, 316, 192, 194 };
    const SpriteRect OBJ_CRATE_MOSS      = { 278, 314, 230, 236 };
    const SpriteRect OBJ_CRATE_MILITARY  = { 548, 314, 196, 196 };
    const SpriteRect OBJ_BARREL_GREEN    = { 1072, 26, 136, 230 };
    const SpriteRect OBJ_BARREL_HAZARD   = { 1072, 288, 136, 230 };
    const SpriteRect OBJ_BARREL_RAD      = { 1592, 288, 136, 230 };
    const SpriteRect OBJ_BARREL_SPILL    = { 1785, 360, 220, 165 };
    const SpriteRect OBJ_PANEL_ELEC      = { 50, 594, 308, 352 };
    const SpriteRect OBJ_GENERATOR       = { 650, 602, 362, 314 };
    const SpriteRect OBJ_LOCKER_DOUBLE   = { 1080, 624, 304, 540 };
    const SpriteRect OBJ_LOCKER_OPEN     = { 1500, 624, 240, 550 };
    const SpriteRect OBJ_PINE_TREE_1     = { 14, 1230, 284, 588 };
    const SpriteRect OBJ_PINE_TREE_2     = { 310, 1208, 284, 590 };
    const SpriteRect OBJ_BUSH_1          = { 1036, 1310, 190, 180 };
    const SpriteRect OBJ_RUBBLE_1        = { 1290, 1272, 338, 226 };
    const SpriteRect OBJ_RUBBLE_2        = { 1676, 1270, 340, 226 };

    // --- FOLHA 4: ui_briefcase_items.png (2048x2048) ---
    const SpriteRect UI_BRIEFCASE_CHASSIS  = { 132, 54, 1188, 920 };
    const SpriteRect UI_SAFEPOCKET_CHASSIS = { 1374, 346, 574, 460 };
    const SpriteRect ITEM_AK74             = { 70, 1074, 708, 228 };
    const SpriteRect ITEM_BULLETS          = { 860, 1078, 126, 150 };
    const SpriteRect ITEM_AMMO_10          = { 1048, 1096, 152, 134 };
    const SpriteRect ITEM_AMMO_15          = { 1258, 1084, 174, 146 };
    const SpriteRect ITEM_SUPPLY_CASE      = { 1494, 1078, 234, 158 };
    const SpriteRect ITEM_METAL_TIN        = { 1780, 1106, 178, 120 };
    const SpriteRect ITEM_BANDAGE          = { 80, 1370, 146, 154 };
    const SpriteRect ITEM_MEDKIT           = { 286, 1348, 206, 184 };
    const SpriteRect ITEM_GASMASK          = { 550, 1324, 228, 236 };
    const SpriteRect ITEM_CANNED_FOOD      = { 842, 1364, 110, 162 };
    const SpriteRect ITEM_RATION           = { 1002, 1354, 150, 178 };
    const SpriteRect ITEM_STIM_SYRINGE     = { 1198, 1380, 112, 138 };
    const SpriteRect ITEM_RAD_PACK         = { 94, 1614, 106, 156 };
    const SpriteRect ITEM_ART_YELLOW       = { 270, 1600, 140, 174 };
    const SpriteRect ITEM_ART_COMPASS      = { 468, 1620, 150, 152 };
    const SpriteRect ITEM_ART_RAD_GREEN    = { 666, 1630, 144, 136 };
    const SpriteRect ITEM_ART_CRYO_BLUE    = { 852, 1614, 148, 162 };
    const SpriteRect ITEM_ART_ELEC_PURPLE  = { 472, 1818, 142, 146 };
    const SpriteRect ITEM_ART_TEAL         = { 662, 1810, 152, 158 };
    const SpriteRect ITEM_ART_SHARD_BLUE   = { 858, 1814, 134, 150 };

    // --- FOLHA 5: ui_loading_bar.png (2752x1536) ---
    const SpriteRect LOADING_EMPTY_BAR     = { 172, 178, 2406, 422 };
    const SpriteRect LOADING_GREEN_BLOCK   = { 896, 1386, 20, 48 };

    // --- FOLHA 6: ui_death_panels.png (2752x1536) ---
    const SpriteRect DEATH_CRT_PANEL       = { 586, 144, 1580, 766 };
    const SpriteRect DEATH_RETURN_BUTTON   = { 850, 1038, 1052, 318 };
}

class SpriteAtlas {
public:
    SpriteAtlas() = default;
    ~SpriteAtlas() = default;

    // Carrega PNG/JPG e remove automaticamente o fundo escuro da IA (removeDarkBg = true)
    bool loadFromFile(const std::string& filepath, bool removeDarkBg = true);

    bool isLoaded() const { return !m_pixels.empty() && m_width > 0 && m_height > 0; }

    // Desenha um sprite recortado escalando para (dstW x dstH) no screenBuffer com suporte a Fog/Luz
    void drawSprite(
        uint32_t* screenBuffer, int screenW, int screenH,
        const SpriteRect& src,
        int dstX, int dstY, int dstW, int dstH,
        float lightFactor = 1.0f, // 1.0 = 100% iluminado, 0.15 = Penumbra
        bool flipH = false
    ) const;

    // Desenha a imagem inteira ocupando uma região (ex: Telas de Loading, Morte e Bunker 1280x800)
    void drawFullScreen(
        uint32_t* screenBuffer, int screenW, int screenH,
        float brightness = 1.0f
    ) const;

    // Amostragem direta de UV (0.0 a 1.0) dentro de um tile para o renderizador de chão/paredes
    uint32_t sampleTileUV(const SpriteRect& tileRect, float u, float v) const;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const std::vector<uint32_t>& getPixels() const { return m_pixels; }

private:
    int m_width = 0;
    int m_height = 0;
    std::vector<uint32_t> m_pixels;
};

} // namespace ZoneProtocol::Engine

namespace DayDoom::Engine {
    using SpriteRect = ZoneProtocol::Engine::SpriteRect;
    using SpriteAtlas = ZoneProtocol::Engine::SpriteAtlas;
    namespace AtlasCoords = ZoneProtocol::Engine::AtlasCoords;
}

