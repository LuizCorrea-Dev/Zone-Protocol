#pragma once

#include <cstdint>
#include <string>
#include "../Player.hpp"
#include "../ZoneMap.hpp"
#include "../../engine/SpriteAtlas.hpp"
#include "GridInventory.hpp"

namespace DayDoom::Game::UI {

enum class VideoMenuAction {
    NONE = 0,
    OPEN_MENU,
    CLOSE_MENU,
    STEP_DOWN,        // Diminuir resolução: 1280x800 -> 960x600 -> 640x400
    STEP_UP,          // Aumentar resolução: 640x400 -> 960x600 -> 1280x800
    SELECT_640x400,   // Preset 0: 640x400
    SELECT_960x600,   // Preset 1: 960x600
    SELECT_1280x800,  // Preset 2: 1280x800
    TOGGLE_FULLSCREEN // Alternar Tela Cheia / Janela [F11]
};

struct BunkerState {
    int credits = 2450;
    int scrap = 184;
    int powerPct = 92;
    int selectedSectorIndex = 0;
    std::string statusBanner = "SELECIONE UMA ESTACAO DO BUNKER OU CLIQUE NO TERMINAL PARA INFILTRAR";
};

class HUD {
public:
    HUD(int screenWidth = 640, int screenHeight = 400);

    void setScreenDimensions(int w, int h);
    int getScreenWidth() const { return m_screenWidth; }
    int getScreenHeight() const { return m_screenHeight; }

    // Controle do Menu de Resolução (720p / Steam Deck 1280x800 / 1080p)
    bool isVideoMenuOpen() const { return m_videoMenuOpen; }
    void setVideoMenuOpen(bool open) { m_videoMenuOpen = open; }
    void toggleVideoMenu() { m_videoMenuOpen = !m_videoMenuOpen; }

    // Renderiza o menu tático de opções de resolução no Bunker/Pause
    // Retorna a ação selecionada (0=Nenhuma, 1=720p, 2=1280x800, 3=1080p, 4=Fullscreen, 5=Fechar)
    int renderVideoMenu(
        uint32_t* screenBuffer,
        int mouseX,
        int mouseY,
        float fps,
        bool isFullscreen
    );

    // Renderiza a interface tática completa na Zona: vitais, cronômetro de extração, maleta, loot e mira
    void render(
        uint32_t* screenBuffer,
        const Player& player,
        const ZoneMap& map,
        float fps,
        float remainingExtractionTime,
        int mouseX,
        int mouseY,
        bool nearDoor = false,
        bool isDoorOpen = false,
        bool nearLootCrate = false,
        const std::string& lootBanner = "",
        int mutantsKilled = 0,
        float evacuationCountdown = -1.0f
    );

    // Renderiza a Tela de Carregamento Oficial (Loading Screen.png + ui_loading_bar.png)
    void renderLoadingScreen(
        uint32_t* screenBuffer,
        const Engine::SpriteAtlas& bgLoadingAtlas,
        const Engine::SpriteAtlas& uiBarAtlas,
        float progress,
        int tipIndex,
        const std::string& sectorTitle
    );

    // Renderiza o Ecrã de Morte Desacoplado (bg_death_clean.png + ui_death_panels.png)
    // Retorna true se o botão de regressar ao Bunker estiver sob o cursor
    bool renderDeathScreen(
        uint32_t* screenBuffer,
        const Engine::SpriteAtlas& bgDeathAtlas,
        const Engine::SpriteAtlas& uiDeathPanelsAtlas,
        const Engine::SpriteAtlas* uiBriefcaseAtlas,
        float timeSurvived,
        int mutantsKilled,
        int dataPoints,
        const GridInventory& briefcase,
        int mouseX,
        int mouseY,
        float pulseTime
    );

    // Renderiza o Ecrã do Bunker e Mercado Negro Baseado em Hotspots (screen_bunker_hub.png)
    // Retorna o índice da estação clicada/sob o cursor (0=Nenhuma, 1=Infirmary, 2=Generator, 3=Workbench, 4=Terminal, 5=BlackMarket)
    int renderBunkerHub(
        uint32_t* screenBuffer,
        const Engine::SpriteAtlas& bgBunkerAtlas,
        const BunkerState& bunker,
        const Player& player,
        int mouseX,
        int mouseY,
        float time
    );

    // Primitivas públicas de texto e retângulos (usadas pelas telas dinâmicas)
    void drawRect(uint32_t* buf, int x, int y, int w, int h, uint32_t color);
    void drawBorder(uint32_t* buf, int x, int y, int w, int h, uint32_t color);
    void drawChar(uint32_t* buf, int x, int y, char c, uint32_t color, int scale = 1);
    void drawText(uint32_t* buf, int x, int y, const std::string& text, uint32_t color, int scale = 1);

private:
    int m_screenWidth;
    int m_screenHeight;
    bool m_videoMenuOpen = false;

    void drawSurvivalBars(uint32_t* screenBuffer, const Player& player, int mutantsKilled);
    void drawExtractionTimer(uint32_t* screenBuffer, float remainingTime, bool isNearExtraction, float evacuationCountdown = -1.0f);
    void drawBriefcaseStatus(uint32_t* screenBuffer, const Player& player);
    void drawCrosshair(uint32_t* screenBuffer, int mouseX, int mouseY, bool isAttacking);
    void drawSectorHeader(uint32_t* screenBuffer, const char* sectorName, float fps);
};

} // namespace DayDoom::Game::UI
