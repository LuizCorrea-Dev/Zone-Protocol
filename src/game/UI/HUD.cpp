#include "HUD.hpp"
#include "../../engine/MathDef.hpp"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <vector>

namespace DayDoom::Game::UI {

HUD::HUD(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth), m_screenHeight(screenHeight) {
}

void HUD::setScreenDimensions(int w, int h) {
    m_screenWidth = w;
    m_screenHeight = h;
}

static uint16_t getHUDGlyph(char c) {
    switch (c) {
        case '0': return 0b111110011001111;
        case '1': return 0b010010010010010;
        case '2': return 0b111001111100111;
        case '3': return 0b111001011001111;
        case '4': return 0b101101111001001;
        case '5': return 0b111100111001111;
        case '6': return 0b111100111101111;
        case '7': return 0b111001001010010;
        case '8': return 0b111101111101111;
        case '9': return 0b111101111001111;
        case ':': return 0b000010000010000;
        case '+': return 0b000010111010000;
        case '/': return 0b001001010100100;
        case '(': return 0b001010010010001;
        case ')': return 0b100010010010100;
        case '&': return 0b010101010101011;
        case 'A': return 0b010101111101101;
        case 'B': return 0b110101110101110;
        case 'C': return 0b011100100100011;
        case 'D': return 0b110101101101110;
        case 'E': return 0b111100110100111;
        case 'F': return 0b111100110100100;
        case 'G': return 0b011100101101011;
        case 'H': return 0b101101111101101;
        case 'I': return 0b111010010010111;
        case 'J': return 0b001001001101010;
        case 'K': return 0b101101110101101;
        case 'L': return 0b100100100100111;
        case 'M': return 0b101111101101101;
        case 'N': return 0b101111101101101;
        case 'O': return 0b111101101101111;
        case 'P': return 0b111101111100100;
        case 'Q': return 0b011101101011001;
        case 'R': return 0b110101110101101;
        case 'S': return 0b011100010001110;
        case 'T': return 0b111010010010010;
        case 'U': return 0b101101101101111;
        case 'V': return 0b101101101010010;
        case 'W': return 0b101101101111101;
        case 'X': return 0b101101010101101;
        case 'Y': return 0b101101010010010;
        case 'Z': return 0b111001010100111;
        case '%': return 0b101001010100101;
        case '[': return 0b011010010010011;
        case ']': return 0b110010010010110;
        case '-': return 0b000000111000000;
        case '.': return 0b000000000000010;
        case ',': return 0b000000000010100;
        case '!': return 0b010010010000010;
        case ' ': return 0;
        default:  return 0b111101101101111;
    }
}

void HUD::drawChar(uint32_t* buf, int x, int y, char c, uint32_t color, int scale) {
    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    uint16_t glyph = getHUDGlyph(c);
    if (glyph == 0) return;

    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            int bit = (row * 3) + col;
            if ((glyph >> (14 - bit)) & 1) {
                for (int sy = 0; sy < scale; ++sy) {
                    for (int sx = 0; sx < scale; ++sx) {
                        int px = x + col * scale + sx;
                        int py = y + row * scale + sy;
                        if (px >= 0 && px < m_screenWidth && py >= 0 && py < m_screenHeight) {
                            buf[py * m_screenWidth + px] = color;
                        }
                    }
                }
            }
        }
    }
}

void HUD::drawText(uint32_t* buf, int x, int y, const std::string& text, uint32_t color, int scale) {
    int curX = x;
    for (char c : text) {
        drawChar(buf, curX, y, c, color, scale);
        curX += 4 * scale;
    }
}

void HUD::drawRect(uint32_t* buf, int x, int y, int w, int h, uint32_t color) {
    int x1 = std::clamp(x, 0, m_screenWidth);
    int y1 = std::clamp(y, 0, m_screenHeight);
    int x2 = std::clamp(x + w, 0, m_screenWidth);
    int y2 = std::clamp(y + h, 0, m_screenHeight);
    uint8_t alpha = (color >> 24) & 0xFF;

    for (int py = y1; py < y2; ++py) {
        for (int px = x1; px < x2; ++px) {
            if (alpha >= 250) {
                buf[py * m_screenWidth + px] = color | 0xFF000000u;
            } else if (alpha > 0) {
                uint32_t bg = buf[py * m_screenWidth + px];
                uint32_t invA = 255 - alpha;
                uint32_t r = (((bg >> 16) & 0xFF) * invA + ((color >> 16) & 0xFF) * alpha) >> 8;
                uint32_t g = (((bg >> 8)  & 0xFF) * invA + ((color >> 8)  & 0xFF) * alpha) >> 8;
                uint32_t b = ((bg & 0xFF)         * invA + (color & 0xFF)         * alpha) >> 8;
                buf[py * m_screenWidth + px] = 0xFF000000u | (r << 16) | (g << 8) | b;
            }
        }
    }
}

void HUD::drawBorder(uint32_t* buf, int x, int y, int w, int h, uint32_t color) {
    drawRect(buf, x, y, w, 1, color);
    drawRect(buf, x, y + h - 1, w, 1, color);
    drawRect(buf, x, y, 1, h, color);
    drawRect(buf, x + w - 1, y, 1, h, color);
}

void HUD::render(
    uint32_t* screenBuffer,
    const Player& player,
    const ZoneMap& map,
    float fps,
    float remainingExtractionTime,
    int mouseX,
    int mouseY,
    bool nearDoor,
    bool isDoorOpen,
    bool nearLootCrate,
    const std::string& lootBanner,
    int mutantsKilled
) {
    const int s = (m_screenWidth >= 1000) ? 2 : 1;

    // 1. Logo Militar Estilizado "ZONE PROTOCOL" e Cabeçalho de Setor no canto superior esquerdo
    const char* sectorName = map.getSectorName(player.getCamera().pos.x, player.getCamera().pos.y);
    drawSectorHeader(screenBuffer, sectorName, fps);

    // 2. Cronômetro de Extração Tática Militar no canto superior direito (EXTRACTION TIMER: MM:SS)
    bool isNearExt = map.isNearExtraction(player.getCamera().pos.x, player.getCamera().pos.y);
    drawExtractionTimer(screenBuffer, remainingExtractionTime, isNearExt);

    // 3. Status da Maleta de Extração abaixo do cabeçalho
    drawBriefcaseStatus(screenBuffer, player);

    // 4. Painel de Telemetria e Vitais de Sobrevivência (HP, ST, AMMO, RADS) no canto inferior direito
    drawSurvivalBars(screenBuffer, player, mutantsKilled);

    // 5. Prompt de Interação com Portas ou Caixas de Loot
    if (nearLootCrate) {
        std::string prompt = "[F] SAQUEAR CAIXA MILITAR";
        int pw = static_cast<int>(prompt.length()) * 4 * s;
        int px = (m_screenWidth - pw) / 2;
        drawRect(screenBuffer, px - 6 * s, m_screenHeight - 72 * s, pw + 12 * s, 13 * s, 0xD8142212);
        drawBorder(screenBuffer, px - 6 * s, m_screenHeight - 72 * s, pw + 12 * s, 13 * s, 0xFF50FF64);
        drawText(screenBuffer, px, m_screenHeight - 68 * s, prompt, 0xFF88FF99, s);
    } else if (nearDoor) {
        std::string prompt = isDoorOpen ? "[F] FECHAR PORTA" : "[F] ABRIR PORTA";
        int pw = static_cast<int>(prompt.length()) * 4 * s;
        int px = (m_screenWidth - pw) / 2;
        drawRect(screenBuffer, px - 6 * s, m_screenHeight - 56 * s, pw + 12 * s, 13 * s, 0xD00F1722);
        drawBorder(screenBuffer, px - 6 * s, m_screenHeight - 56 * s, pw + 12 * s, 13 * s, 0xFF00FF88);
        drawText(screenBuffer, px, m_screenHeight - 52 * s, prompt, 0xFFFFFFFF, s);
    }

    // Banner temporário de Loot saqueado
    if (!lootBanner.empty()) {
        int bw = static_cast<int>(lootBanner.length()) * 4 * s;
        int bx = (m_screenWidth - bw) / 2;
        drawRect(screenBuffer, bx - 8 * s, 60 * s, bw + 16 * s, 14 * s, 0xE60C1E12);
        drawBorder(screenBuffer, bx - 8 * s, 60 * s, bw + 16 * s, 14 * s, 0xFF50FF64);
        drawText(screenBuffer, bx, 64 * s, lootBanner, 0xFF78FF96, s);
    }

    // 6. Retículo Tático / Crosshair de Alta Precisão centralizado no cursor do mouse
    drawCrosshair(screenBuffer, mouseX, mouseY, player.isAttacking());
}

void HUD::drawSectorHeader(uint32_t* screenBuffer, const char* sectorName, float fps) {
    const int s = (m_screenWidth >= 1000) ? 2 : 1;
    drawRect(screenBuffer, 0, 0, m_screenWidth, 15 * s, 0xD8080C10);
    drawRect(screenBuffer, 0, 15 * s, m_screenWidth, 1 * s, 0xFF182430);

    drawText(screenBuffer, 10 * s, 4 * s, "ZONE PROTOCOL [v0.1.2]", 0xFF7D9E74, s);

    if (sectorName) {
        drawText(screenBuffer, 115 * s, 4 * s, sectorName, 0xFF4DEEEA, s);
    }

    drawText(screenBuffer, 285 * s, 4 * s, "[TAB] MALETA  [B] BUNKER", 0xFF98B098, s);

    char fpsBuf[16];
    std::snprintf(fpsBuf, sizeof(fpsBuf), "%d FPS", static_cast<int>(fps));
    int fpsX = m_screenWidth - static_cast<int>(std::string(fpsBuf).length()) * 4 * s - 10 * s;
    drawText(screenBuffer, fpsX, 4 * s, fpsBuf, 0xFF00FF88, s);
}

void HUD::drawExtractionTimer(uint32_t* screenBuffer, float remainingTime, bool isNearExtraction) {
    const int s = (m_screenWidth >= 1000) ? 2 : 1;
    int totalSec = std::max(0, static_cast<int>(remainingTime));
    int minutes = totalSec / 60;
    int seconds = totalSec % 60;

    char timerStr[48];
    std::snprintf(timerStr, sizeof(timerStr), "EXTRACTION TIMER: %02d:%02d", minutes, seconds);

    int textLen = static_cast<int>(std::string(timerStr).length()) * 4 * s;
    int boxW = textLen + 16 * s;
    int boxH = 18 * s;
    int boxX = m_screenWidth - boxW - 10 * s;
    int boxY = 20 * s;

    uint32_t timerColor = (totalSec <= 60) ? 0xFFFF3333 : 0xFF00FF88;
    uint32_t borderColor = (totalSec <= 60) ? 0xFFFF4444 : 0xFF2A5038;

    drawRect(screenBuffer, boxX, boxY, boxW, boxH, 0xD80A120E);
    drawBorder(screenBuffer, boxX, boxY, boxW, boxH, borderColor);
    drawBorder(screenBuffer, boxX + 1 * s, boxY + 1 * s, boxW - 2 * s, boxH - 2 * s, 0xFF14241A);
    drawText(screenBuffer, boxX + 8 * s, boxY + 6 * s, timerStr, timerColor, s);

    if (isNearExtraction) {
        std::string alert = "[ ZONA DE EXTRACAO ATIVA - PRESSIONE [E] OU [B] PARA EXTRAIR AO BUNKER ]";
        int alertW = static_cast<int>(alert.length()) * 4 * s;
        int alertX = (m_screenWidth - alertW) / 2;
        drawRect(screenBuffer, alertX - 6 * s, 42 * s, alertW + 12 * s, 13 * s, 0xD8003A12);
        drawBorder(screenBuffer, alertX - 6 * s, 42 * s, alertW + 12 * s, 13 * s, 0xFF00FF66);
        drawText(screenBuffer, alertX, 46 * s, alert, 0xFFFFFFFF, s);
    }
}

void HUD::drawBriefcaseStatus(uint32_t* screenBuffer, const Player& player) {
    const int s = (m_screenWidth >= 1000) ? 2 : 1;
    int boxW = 94 * s;
    int boxH = 16 * s;
    int boxX = 10 * s;
    int boxY = 20 * s;

    bool hasCase = player.hasExtractionCase();
    uint32_t caseColor = hasCase ? 0xFF00E5FF : 0xFF888888;
    std::string text = hasCase ? "MALETA 6X4: OK" : "MALETA: PERDIDA";

    drawRect(screenBuffer, boxX, boxY, boxW, boxH, 0xD80C141E);
    drawBorder(screenBuffer, boxX, boxY, boxW, boxH, 0xFF1C344A);

    drawRect(screenBuffer, boxX + 5 * s, boxY + 5 * s, 8 * s, 6 * s, caseColor);
    drawRect(screenBuffer, boxX + 7 * s, boxY + 3 * s, 4 * s, 2 * s, caseColor);
    drawRect(screenBuffer, boxX + 8 * s, boxY + 4 * s, 2 * s, 1 * s, 0xFF0C141E);

    drawText(screenBuffer, boxX + 18 * s, boxY + 5 * s, text, caseColor, s);
}

void HUD::drawSurvivalBars(uint32_t* screenBuffer, const Player& player, int mutantsKilled) {
    const int s = (m_screenWidth >= 1000) ? 2 : 1;
    int panelW = 172 * s;
    int panelH = 52 * s;
    int panelX = m_screenWidth - panelW - 10 * s;
    int panelY = m_screenHeight - panelH - 10 * s;

    drawRect(screenBuffer, panelX, panelY, panelW, panelH, 0xD80A0E14);
    drawBorder(screenBuffer, panelX, panelY, panelW, panelH, 0xFF2A3644);
    drawBorder(screenBuffer, panelX + 1 * s, panelY + 1 * s, panelW - 2 * s, panelH - 2 * s, 0xFF121A22);

    int barStartX = panelX + 24 * s;
    int barW = 84 * s;
    int barH = 6 * s;

    // 1. Barra de Vida (HP)
    int hpY = panelY + 6 * s;
    drawText(screenBuffer, panelX + 6 * s, hpY + 1 * s, "HP", 0xFFE0E0E0, s);
    drawRect(screenBuffer, barStartX, hpY, barW, barH, 0xFF141820);

    float hpPct = std::clamp(player.getHealth() / 100.0f, 0.0f, 1.0f);
    int filledW = static_cast<int>(barW * hpPct);
    uint32_t hpColor = (hpPct > 0.4f) ? 0xFFE04434 : 0xFFFF2020;
    drawRect(screenBuffer, barStartX, hpY, filledW, barH, hpColor);

    for (int seg = 1; seg < 10; ++seg) {
        int segX = barStartX + (barW * seg) / 10;
        drawRect(screenBuffer, segX, hpY, 1 * s, barH, 0xFF0A0E14);
    }
    drawBorder(screenBuffer, barStartX, hpY, barW, barH, 0xFF4A5565);

    drawText(screenBuffer, barStartX + barW + 6 * s, hpY + 1 * s, "RADS  1", 0xFFD47050, s);

    // 2. Barra de Stamina (ST)
    int staY = hpY + barH + 5 * s;
    drawText(screenBuffer, panelX + 6 * s, staY + 1 * s, "ST", 0xFFE0E0E0, s);
    drawRect(screenBuffer, barStartX, staY, barW, barH, 0xFF141820);

    float staPct = std::clamp(player.getStamina() / 100.0f, 0.0f, 1.0f);
    int staW = static_cast<int>(barW * staPct);
    drawRect(screenBuffer, barStartX, staY, staW, barH, 0xFF4AE066);
    drawBorder(screenBuffer, barStartX, staY, barW, barH, 0xFF35553E);

    drawText(screenBuffer, barStartX + barW + 6 * s, staY + 1 * s, "AMMO 30X", 0xFFD8B040, s);

    // 3. Indicador de Armadura / Kills
    int radY = staY + barH + 5 * s;
    drawText(screenBuffer, panelX + 6 * s, radY + 1 * s, "AH", 0xFF8A9A8A, s);
    drawRect(screenBuffer, barStartX, radY, barW, barH, 0xFF141820);
    drawRect(screenBuffer, barStartX, radY, barW * 3 / 4, barH, 0xFF8A7840);
    drawBorder(screenBuffer, barStartX, radY, barW, barH, 0xFF4A4530);

    char killBuf[24];
    std::snprintf(killBuf, sizeof(killBuf), "KILLS %d", mutantsKilled);
    drawText(screenBuffer, barStartX + barW + 6 * s, radY + 1 * s, killBuf, 0xFFD45040, s);

    int ammoGaugeX = panelX + panelW - 10 * s;
    int ammoGaugeY = panelY + 6 * s;
    int ammoGaugeH = panelH - 12 * s;
    drawRect(screenBuffer, ammoGaugeX, ammoGaugeY, 4 * s, ammoGaugeH, 0xFF242010);
    drawRect(screenBuffer, ammoGaugeX, ammoGaugeY + 4 * s, 4 * s, ammoGaugeH - 4 * s, 0xFFD8B030);
    drawBorder(screenBuffer, ammoGaugeX, ammoGaugeY, 4 * s, ammoGaugeH, 0xFF554820);
}

void HUD::drawCrosshair(uint32_t* screenBuffer, int mouseX, int mouseY, bool isAttacking) {
    if (mouseX < 0 || mouseX >= m_screenWidth || mouseY < 0 || mouseY >= m_screenHeight) return;

    const int s = (m_screenWidth >= 1000) ? 2 : 1;
    uint32_t color = isAttacking ? 0xFFFF3333 : 0xFF00FF88;

    int gap = 3 * s;
    int len = 4 * s;

    for (int i = gap; i <= gap + len; ++i) {
        if (mouseX - i >= 0) screenBuffer[mouseY * m_screenWidth + (mouseX - i)] = color;
        if (mouseX + i < m_screenWidth) screenBuffer[mouseY * m_screenWidth + (mouseX + i)] = color;
        if (mouseY - i >= 0) screenBuffer[(mouseY - i) * m_screenWidth + mouseX] = color;
        if (mouseY + i < m_screenHeight) screenBuffer[(mouseY + i) * m_screenWidth + mouseX] = color;
    }

    screenBuffer[mouseY * m_screenWidth + mouseX] = color;
}

// =============================================================================
// TELA DE CARREGAMENTO DINÂMICA (Loading Screen.png + ui_loading_bar.png)
// =============================================================================
void HUD::renderLoadingScreen(
    uint32_t* screenBuffer,
    const Engine::SpriteAtlas& bgLoadingAtlas,
    const Engine::SpriteAtlas& uiBarAtlas,
    float progress,
    int tipIndex,
    const std::string& sectorTitle
) {
    const int s = (m_screenWidth >= 1000) ? 2 : 1;
    progress = std::clamp(progress, 0.0f, 1.0f);

    // 1. Desenha a tela de fundo oficial (Loading Screen.png)
    if (bgLoadingAtlas.isLoaded()) {
        bgLoadingAtlas.drawFullScreen(screenBuffer, m_screenWidth, m_screenHeight, 1.0f);
    } else {
        drawRect(screenBuffer, 0, 0, m_screenWidth, m_screenHeight, 0xFF0C1014);
    }

    // 2. Cobre a barra estática e a linha de texto antiga na base do piso do bunker
    int coverX = (m_screenWidth * 28) / 100;
    int coverY = (m_screenHeight * 86) / 100;
    int coverW = (m_screenWidth * 44) / 100;
    int coverH = (m_screenHeight * 13) / 100;
    drawRect(screenBuffer, coverX, coverY, coverW, coverH, Engine::makeColor(14, 18, 20));

    // 3. Posiciona a Moldura Metálica Vazia (ui_loading_bar.png) e os 24 blocos verdes néon
    int barW = 242 * s;
    int barH = 24 * s;
    int barX = (m_screenWidth - barW) / 2;
    int barY = (m_screenHeight * 87) / 100;

    if (uiBarAtlas.isLoaded()) {
        uiBarAtlas.drawSprite(
            screenBuffer, m_screenWidth, m_screenHeight,
            Engine::AtlasCoords::LOADING_EMPTY_BAR,
            barX, barY, barW, barH, 1.0f
        );
    } else {
        drawRect(screenBuffer, barX, barY, barW, barH, 0xFF181A1E);
        drawBorder(screenBuffer, barX, barY, barW, barH, 0xFF5A6858);
    }

    constexpr int MAX_BLOCKS = 24;
    int activeBlocks = static_cast<int>(progress * MAX_BLOCKS + 0.5f);
    int startX = barX + 10 * s;
    int startY = barY + 5 * s;
    int blockW = 7 * s;
    int blockH = 14 * s;
    int gap    = 2 * s;

    for (int i = 0; i < activeBlocks; ++i) {
        int bx = startX + i * (blockW + gap);
        if (uiBarAtlas.isLoaded()) {
            uiBarAtlas.drawSprite(
                screenBuffer, m_screenWidth, m_screenHeight,
                Engine::AtlasCoords::LOADING_GREEN_BLOCK,
                bx, startY, blockW, blockH, 1.15f
            );
        } else {
            drawRect(screenBuffer, bx, startY, blockW, blockH, Engine::makeColor(80, 255, 100));
        }
    }

    // 4. Etapa real do gerador procedural (Estilo Terminal Soviético)
    std::string stageText = "25%: SEEDING RNG & SECTOR LAYOUT...";
    if (progress >= 0.75f)      stageText = "100%: INITIALIZING DDA LIGHTCASTER...";
    else if (progress >= 0.50f) stageText = "75%: SPAWNING MUTANTS & LOOT CRATES...";
    else if (progress >= 0.25f) stageText = "50%: STAMPING PREFAB BUILDINGS & ROADS...";

    int stW = static_cast<int>(stageText.length()) * 4 * s;
    drawText(screenBuffer, (m_screenWidth - stW) / 2, barY + barH + 5 * s, stageText, Engine::makeColor(80, 255, 100), s);

    // 5. Dica Tática de Sobrevivência sorteada acima da barra
    static const std::vector<std::string> TIPS = {
        "DICA: CONECTAR ARTEFATOS ELETRICOS A AK-74 CAUSA RICOCHETE E DANO EM CADEIA.",
        "DICA: ITENS NO SAFE POCKET 2X2 SAO PRESERVADOS MESMO SE VOCE MORRER NA ZONA.",
        "DICA: MUTANTES FORA DO CONE DE LUZ EXIBEM APENAS OS OLHOS VERMELHOS NO ESCURO.",
        "DICA: DOIS ARTEFATOS ADJACENTES AMPLIFICAM O MULTIPLICADOR GLOBAL EM +15%."
    };
    const std::string& tip = TIPS[ std::abs(tipIndex) % TIPS.size() ];
    int tipW = static_cast<int>(tip.length()) * 4 * s;
    int tipBoxX = (m_screenWidth - tipW) / 2 - 6 * s;
    int tipBoxY = barY - 26 * s;
    drawRect(screenBuffer, tipBoxX, tipBoxY, tipW + 12 * s, 20 * s, 0xD80A100E);
    drawBorder(screenBuffer, tipBoxX, tipBoxY, tipW + 12 * s, 20 * s, 0xFF2E4A36);

    int secW = static_cast<int>(sectorTitle.length()) * 4 * s;
    drawText(screenBuffer, (m_screenWidth - secW) / 2, tipBoxY + 3 * s, sectorTitle, 0xFFFFD055, s);
    drawText(screenBuffer, (m_screenWidth - tipW) / 2, tipBoxY + 11 * s, tip, 0xFF98E8AA, s);
}

// =============================================================================
// ECRÃ DE MORTE DESACOPLADO (bg_death_clean.png + ui_death_panels.png)
// =============================================================================
bool HUD::renderDeathScreen(
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
) {
    const int s = (m_screenWidth >= 1000) ? 2 : 1;

    // 1. Fundo Limpo do Ecrã de Morte com leve pulsação atmosférica
    float pulse = 0.92f + 0.08f * std::sin(pulseTime * 3.0f);
    if (bgDeathAtlas.isLoaded()) {
        bgDeathAtlas.drawFullScreen(screenBuffer, m_screenWidth, m_screenHeight, pulse);
    } else {
        drawRect(screenBuffer, 0, 0, m_screenWidth, m_screenHeight, 0xFF120808);
    }

    // 2. Painel CRT Superior Recortado (ui_death_panels.png)
    int crtW = 320 * s;
    int crtH = 155 * s;
    int crtX = (m_screenWidth - crtW) / 2;
    int crtY = 24 * s;

    if (uiDeathPanelsAtlas.isLoaded()) {
        uiDeathPanelsAtlas.drawSprite(
            screenBuffer, m_screenWidth, m_screenHeight,
            Engine::AtlasCoords::DEATH_CRT_PANEL,
            crtX, crtY, crtW, crtH, 1.05f
        );
    } else {
        drawRect(screenBuffer, crtX, crtY, crtW, crtH, 0xEE101C14);
        drawBorder(screenBuffer, crtX, crtY, crtW, crtH, 0xFF4A6850);
    }

    // Estatísticas dinâmicas escritas dentro do visor CRT
    std::string header = "K.I.A. - OPERADOR ABATIDO NA ZONA";
    drawText(screenBuffer, crtX + 36 * s, crtY + 26 * s, header, 0xFFFF4444, s);

    int mins = static_cast<int>(timeSurvived) / 60;
    int secs = static_cast<int>(timeSurvived) % 60;
    char line1[64], line2[64], line3[64], line4[64];
    std::snprintf(line1, sizeof(line1), "TIME SURVIVED:      %02d:%02d", mins, secs);
    std::snprintf(line2, sizeof(line2), "MUTANTS KILLED:     %d", mutantsKilled);
    std::snprintf(line3, sizeof(line3), "DATA POINTS EARNED: +%d PTS", dataPoints);
    std::snprintf(line4, sizeof(line4), "SAFE POCKET SAVED:  %d ITEMS", briefcase.getSafePocketItemCount());

    drawText(screenBuffer, crtX + 36 * s, crtY + 46 * s, line1, 0xFF88FF99, s);
    drawText(screenBuffer, crtX + 36 * s, crtY + 62 * s, line2, 0xFF88FF99, s);
    drawText(screenBuffer, crtX + 36 * s, crtY + 78 * s, line3, 0xFFFFD866, s);
    drawText(screenBuffer, crtX + 36 * s, crtY + 94 * s, line4, 0xFF66E0FF, s);

    // Ícones dos itens salvos no Safe Pocket 2x2
    int iconX = crtX + 36 * s;
    int iconY = crtY + 110 * s;
    for (const auto& item : briefcase.getItems()) {
        if (!item.inSafePocket) continue;
        drawRect(screenBuffer, iconX, iconY, 22 * s, 22 * s, 0xFF18241C);
        if (uiBriefcaseAtlas && uiBriefcaseAtlas->isLoaded()) {
            uiBriefcaseAtlas->drawSprite(
                screenBuffer, m_screenWidth, m_screenHeight,
                Engine::AtlasCoords::ITEM_ART_COMPASS,
                iconX + 2 * s, iconY + 2 * s, 18 * s, 18 * s, 1.1f
            );
        }
        drawBorder(screenBuffer, iconX, iconY, 22 * s, 22 * s, 0xFFFFCC44);
        iconX += 26 * s;
    }

    // 3. Botão Inferior Recortado para Regressar ao Bunker
    int btnW = 220 * s;
    int btnH = 54 * s;
    int btnX = (m_screenWidth - btnW) / 2;
    int btnY = m_screenHeight - 78 * s;

    bool hovered = (mouseX >= btnX && mouseX < btnX + btnW &&
                    mouseY >= btnY && mouseY < btnY + btnH);

    if (uiDeathPanelsAtlas.isLoaded()) {
        uiDeathPanelsAtlas.drawSprite(
            screenBuffer, m_screenWidth, m_screenHeight,
            Engine::AtlasCoords::DEATH_RETURN_BUTTON,
            btnX, btnY, btnW, btnH, hovered ? 1.35f : 1.0f
        );
    } else {
        drawRect(screenBuffer, btnX, btnY, btnW, btnH, hovered ? 0xFF24422C : 0xFF16281B);
        drawBorder(screenBuffer, btnX, btnY, btnW, btnH, 0xFF50FF64);
    }

    std::string btnLabel = "REGRESSAR AO BUNKER [ENTER / CLIQUE]";
    int lblW = static_cast<int>(btnLabel.length()) * 4 * s;
    drawText(screenBuffer, (m_screenWidth - lblW) / 2, btnY + 24 * s, btnLabel, hovered ? 0xFFFFFFFF : 0xFF80FF98, s);

    drawCrosshair(screenBuffer, mouseX, mouseY, false);
    return hovered;
}

// =============================================================================
// ECRÃ DO BUNKER E MERCADO NEGRO (screen_bunker_hub.png - 5 HOTSPOTS INTERATIVOS)
// =============================================================================
int HUD::renderBunkerHub(
    uint32_t* screenBuffer,
    const Engine::SpriteAtlas& bgBunkerAtlas,
    const BunkerState& bunker,
    const Player& player,
    int mouseX,
    int mouseY,
    float time
) {
    const int s = (m_screenWidth >= 1000) ? 2 : 1;

    // 1. Renderiza a imagem de fundo completa do Bunker (screen_bunker_hub.png)
    if (bgBunkerAtlas.isLoaded()) {
        bgBunkerAtlas.drawFullScreen(screenBuffer, m_screenWidth, m_screenHeight, 1.0f);
    } else {
        drawRect(screenBuffer, 0, 0, m_screenWidth, m_screenHeight, 0xFF101416);
    }

    // 2. Atualiza os indicadores dinâmicos no topo (CREDITS, SCRAP, POWER)
    drawRect(screenBuffer, 232 * s, 6 * s, 110 * s, 18 * s, 0xFF161A18);
    char credBuf[32];
    std::snprintf(credBuf, sizeof(credBuf), "CREDITS: %d", bunker.credits);
    drawText(screenBuffer, 238 * s, 12 * s, credBuf, 0xFFFFCC55, s);

    drawRect(screenBuffer, 358 * s, 6 * s, 96 * s, 18 * s, 0xFF161A18);
    char scrapBuf[32];
    std::snprintf(scrapBuf, sizeof(scrapBuf), "SCRAP: %d", bunker.scrap);
    drawText(screenBuffer, 364 * s, 12 * s, scrapBuf, 0xFFD8E0D8, s);

    // 3. Definição dos 5 Hotspots Interativos das Estações do Bunker
    struct StationHotspot {
        int id;
        int x, y, w, h;
        const char* title;
        const char* actionDesc;
    };

    const StationHotspot stations[5] = {
        { 1,  18 * s, 135 * s, 130 * s, 145 * s, "[1] INFIRMARY",    "CURAR 100% HP & SANGRAMENTO (-25 CREDITS)" },
        { 2, 168 * s, 180 * s, 115 * s, 130 * s, "[2] GENERATOR",    "REABASTECER GERADOR & +30 MUNICAO (-15 SCRAP)" },
        { 3, 290 * s, 160 * s, 108 * s, 140 * s, "[3] WORKBENCH",    "ABRIR MALETA 6X4 & FUNDIR SINERGIAS [TAB]" },
        { 4, 404 * s, 175 * s,  88 * s, 145 * s, "[4] TERMINAL",     "INFILTRAR NA ZONA ( INICIAR MISSAO! )" },
        { 5, 500 * s, 130 * s, 128 * s, 165 * s, "[5] BLACK MARKET", "VENDER ARTEFATO (+250 CREDITS) / COMPRAR ORBE" }
    };

    int hoveredStation = 0;
    float pulse = 0.6f + 0.4f * std::sin(time * 5.0f);
    uint32_t neonGreen = Engine::makeColor(static_cast<uint8_t>(60 * pulse), 255, static_cast<uint8_t>(90 * pulse));

    for (const auto& st : stations) {
        bool isHover = (mouseX >= st.x && mouseX < st.x + st.w &&
                        mouseY >= st.y && mouseY < st.y + st.h);
        if (isHover) {
            hoveredStation = st.id;
            drawBorder(screenBuffer, st.x, st.y, st.w, st.h, neonGreen);
            drawBorder(screenBuffer, st.x + 1 * s, st.y + 1 * s, st.w - 2 * s, st.h - 2 * s, 0xFF50FF64);
            drawRect(screenBuffer, st.x, st.y - 16 * s, st.w, 14 * s, 0xE60B1810);
            drawBorder(screenBuffer, st.x, st.y - 16 * s, st.w, 14 * s, 0xFF50FF64);
            drawText(screenBuffer, st.x + 4 * s, st.y - 12 * s, st.title, 0xFF78FF96, s);
        } else {
            drawBorder(screenBuffer, st.x, st.y, st.w, st.h, 0x66284830);
        }
    }

    // 4. Rodapé Tático do Bunker com Status da Estação Selecionada
    int barY = m_screenHeight - 36 * s;
    drawRect(screenBuffer, 12 * s, barY, m_screenWidth - 24 * s, 28 * s, 0xEB0A100E);
    drawBorder(screenBuffer, 12 * s, barY, m_screenWidth - 24 * s, 28 * s, 0xFF3E6848);

    static const char* SECTOR_NAMES[3] = {
        "SETOR 04: KRASTOV PERIPHERY (RADS: BAIXO)",
        "SETOR 02: COMPLEXO NOVO-BRYANSK (RADS: MEDIO)",
        "SETOR 07: MARCO ZERO DE ZARIA (RADS: EXTREMO)"
    };
    const char* curSector = SECTOR_NAMES[bunker.selectedSectorIndex % 3];

    if (hoveredStation > 0) {
        const auto& st = stations[hoveredStation - 1];
        drawText(screenBuffer, 20 * s, barY + 5 * s, std::string(st.title) + " -> " + st.actionDesc, 0xFF66FF88, s);
    } else {
        drawText(screenBuffer, 20 * s, barY + 5 * s, bunker.statusBanner, 0xFF98E8AA, s);
    }

    char bottomLine[160];
    std::snprintf(
        bottomLine, sizeof(bottomLine),
        "PROXIMA MISSAO: %s | HP: %d%% | [ENTER] INFILTRAR | [1-5] ESTACOES",
        curSector, static_cast<int>(player.getHealth())
    );
    drawText(screenBuffer, 20 * s, barY + 16 * s, bottomLine, 0xFFFFD466, s);

    drawCrosshair(screenBuffer, mouseX, mouseY, false);
    return hoveredStation;
}

} // namespace DayDoom::Game::UI
