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

static const uint8_t GLYPH_5X7[128][7] = {
    // 0-31: whitespace/null
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    // 32 ' '
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // 33 '!'
    {0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00},
    // 34 '"'
    {0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00},
    // 35 '#'
    {0x0A, 0x1F, 0x0A, 0x0A, 0x1F, 0x0A, 0x00},
    // 36 '$'
    {0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04},
    // 37 '%'
    {0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03},
    // 38 '&'
    {0x08, 0x14, 0x14, 0x08, 0x15, 0x12, 0x0D},
    // 39 '\''
    {0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00},
    // 40 '('
    {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02},
    // 41 ')'
    {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08},
    // 42 '*'
    {0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00},
    // 43 '+'
    {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00},
    // 44 ','
    {0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x04},
    // 45 '-'
    {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00},
    // 46 '.'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06},
    // 47 '/'
    {0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00},
    // 48-57 '0'-'9'
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, // 0
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 1
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, // 2
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, // 3
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, // 4
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, // 5
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, // 6
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // 7
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, // 8
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, // 9
    // 58 ':'
    {0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00},
    // 59 ';'
    {0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x04},
    // 60 '<'
    {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02},
    // 61 '='
    {0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00},
    // 62 '>'
    {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08},
    // 63 '?'
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04},
    // 64 '@'
    {0x0E, 0x11, 0x01, 0x0D, 0x15, 0x15, 0x0E},
    // 65-90 'A'-'Z'
    {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // A
    {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, // B
    {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, // C
    {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}, // D
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, // E
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}, // F
    {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}, // G
    {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // H
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, // I
    {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}, // J
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, // K
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, // L
    {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}, // M
    {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}, // N
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // O
    {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, // P
    {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}, // Q
    {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, // R
    {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}, // S
    {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // T
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // U
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}, // V
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}, // W
    {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, // X
    {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}, // Y
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, // Z
    // 91 '['
    {0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E},
    // 92 '\\'
    {0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00},
    // 93 ']'
    {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E},
    // 94 '^'
    {0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00},
    // 95 '_'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}
};

void HUD::drawChar(uint32_t* buf, int x, int y, char c, uint32_t color, int scale) {
    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    uint8_t uc = static_cast<uint8_t>(c);
    if (uc >= 128) return;

    for (int row = 0; row < 7; ++row) {
        uint8_t rowBits = GLYPH_5X7[uc][row];
        if (rowBits == 0) continue;

        for (int col = 0; col < 5; ++col) {
            if ((rowBits >> (4 - col)) & 1) {
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
        curX += 6 * scale; // 5 colunas + 1 de espaçamento para leitura perfeita
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
    int mutantsKilled,
    float evacuationCountdown
) {
    const int s = (m_screenWidth >= 1600) ? 2 : 1;

    // 1. Logo Militar Estilizado "ZONE PROTOCOL" e Cabeçalho de Setor no canto superior esquerdo
    const char* sectorName = map.getSectorName(player.getCamera().pos.x, player.getCamera().pos.y);
    drawSectorHeader(screenBuffer, sectorName, fps);

    // 2. Status da Maleta de Extração no canto superior esquerdo
    drawBriefcaseStatus(screenBuffer, player);

    // 3. Painel Integrado Tático Militar no canto inferior direito:
    // Bloco Superior Acoplado: EXTRACTION TIMER: MM:SS
    // Bloco Inferior: HP, ST, AH, RADS, AMMO 30x, KILLS e Medidores Verticais
    bool isNearExtraction = map.isNearExtraction(player.getCamera().pos.x, player.getCamera().pos.y, 4.5f);
    drawExtractionTimer(screenBuffer, remainingExtractionTime, isNearExtraction, evacuationCountdown);
    drawSurvivalBars(screenBuffer, player, mutantsKilled);

    // 4. Prompt de Interação com Portas ou Caixas de Loot
    if (nearLootCrate) {
        std::string prompt = "[F] SAQUEAR CAIXA MILITAR";
        int pw = static_cast<int>(prompt.length()) * 6 * s;
        int px = (m_screenWidth - pw) / 2;
        drawRect(screenBuffer, px - 8 * s, m_screenHeight - 72 * s, pw + 16 * s, 16 * s, 0xD8142212);
        drawBorder(screenBuffer, px - 8 * s, m_screenHeight - 72 * s, pw + 16 * s, 16 * s, 0xFF50FF64);
        drawText(screenBuffer, px, m_screenHeight - 68 * s, prompt, 0xFF88FF99, s);
    } else if (nearDoor) {
        std::string prompt = isDoorOpen ? "[F] FECHAR PORTA" : "[F] ABRIR PORTA";
        int pw = static_cast<int>(prompt.length()) * 6 * s;
        int px = (m_screenWidth - pw) / 2;
        drawRect(screenBuffer, px - 8 * s, m_screenHeight - 56 * s, pw + 16 * s, 16 * s, 0xD00F1722);
        drawBorder(screenBuffer, px - 8 * s, m_screenHeight - 56 * s, pw + 16 * s, 16 * s, 0xFF00FF88);
        drawText(screenBuffer, px, m_screenHeight - 52 * s, prompt, 0xFFFFFFFF, s);
    }

    // Banner temporário de Loot ou mudança de resolução
    if (!lootBanner.empty()) {
        int bw = static_cast<int>(lootBanner.length()) * 6 * s;
        int bx = (m_screenWidth - bw) / 2;
        drawRect(screenBuffer, bx - 10 * s, 50 * s, bw + 20 * s, 18 * s, 0xE60C1E12);
        drawBorder(screenBuffer, bx - 10 * s, 50 * s, bw + 20 * s, 18 * s, 0xFF50FF64);
        drawText(screenBuffer, bx, 54 * s, lootBanner, 0xFF78FF96, s);
    }

    // 6. Retículo Tático / Crosshair de Alta Precisão centralizado no cursor do mouse
    drawCrosshair(screenBuffer, mouseX, mouseY, player.isAttacking());
}

void HUD::drawSectorHeader(uint32_t* screenBuffer, const char* sectorName, float fps) {
    const int s = (m_screenWidth >= 1600) ? 2 : 1;
    drawRect(screenBuffer, 0, 0, m_screenWidth, 18 * s, 0xD8080C10);
    drawRect(screenBuffer, 0, 18 * s, m_screenWidth, 1 * s, 0xFF182430);

    drawText(screenBuffer, 10 * s, 5 * s, "ZONE PROTOCOL [v0.1.3]", 0xFF7D9E74, s);

    if (sectorName) {
        drawText(screenBuffer, 160 * s, 5 * s, sectorName, 0xFF4DEEEA, s);
    }

    drawText(screenBuffer, 380 * s, 5 * s, "[TAB] MALETA  [B] BUNKER  [O] VIDEO", 0xFF98B098, s);

    char fpsBuf[16];
    std::snprintf(fpsBuf, sizeof(fpsBuf), "%d FPS", static_cast<int>(fps));
    int fpsX = m_screenWidth - static_cast<int>(std::string(fpsBuf).length()) * 6 * s - 10 * s;
    drawText(screenBuffer, fpsX, 5 * s, fpsBuf, 0xFF00FF88, s);
}

void HUD::drawExtractionTimer(uint32_t* screenBuffer, float remainingTime, bool isNearExtraction, float evacuationCountdown) {
    const int s = (m_screenWidth >= 1000) ? 2 : 1;
    int totalSec = std::max(0, static_cast<int>(remainingTime));
    int minutes = totalSec / 60;
    int seconds = totalSec % 60;

    // Conforme o layout de referência (media_1793306523730_4b704918.png):
    // O painel do Timer fica acoplado imediatamente acima do bloco de vitais no canto inferior direito!
    int statsPanelW = 290 * s;
    int statsPanelH = 60 * s;
    int panelMargin = 10 * s;
    int statsPanelX = m_screenWidth - statsPanelW - panelMargin;
    int statsPanelY = m_screenHeight - statsPanelH - panelMargin;

    int timerBoxW = 196 * s;
    int timerBoxH = 22 * s;
    int timerBoxX = statsPanelX + statsPanelW - timerBoxW;
    int timerBoxY = statsPanelY - timerBoxH + (2 * s); // Sobrepõe a borda superior para conectar os chassis

    // Chassi Metálico Enferrujado do Timer
    drawRect(screenBuffer, timerBoxX, timerBoxY, timerBoxW, timerBoxH, 0xD80D1410);
    drawBorder(screenBuffer, timerBoxX, timerBoxY, timerBoxW, timerBoxH, 0xFF4D4032);
    drawBorder(screenBuffer, timerBoxX + 1 * s, timerBoxY + 1 * s, timerBoxW - 2 * s, timerBoxH - 2 * s, 0xFF243022);

    // Texto: "EXTRACTION TIMER: " em Verde Oliva / Tático
    drawText(screenBuffer, timerBoxX + 12 * s, timerBoxY + 7 * s, "EXTRACTION TIMER:", 0xFF719357, s);

    // Dígitos "MM:SS" em Laranja / Âmbar Retro
    char timeDigits[16];
    std::snprintf(timeDigits, sizeof(timeDigits), "%02d:%02d", minutes, seconds);
    uint32_t digitsColor = (totalSec <= 60) ? 0xFFFF3333 : 0xFFC1A25C;
    drawText(screenBuffer, timerBoxX + timerBoxW - 46 * s, timerBoxY + 7 * s, timeDigits, digitsColor, s);

    if (isNearExtraction) {
        char alertBuf[128];
        if (evacuationCountdown >= 0.0f) {
            std::snprintf(alertBuf, sizeof(alertBuf), "[ ZONA DE EXTRACAO ATIVA - EVACUANDO EM %.1fs | PRESSIONE [E] ]", evacuationCountdown);
        } else {
            std::snprintf(alertBuf, sizeof(alertBuf), "[ ZONA DE EXTRACAO ATIVA - PRESSIONE [E] OU [B] PARA EXTRAIR ]");
        }
        std::string alert(alertBuf);
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
    int boxY = 24 * s;

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

    // Dimensões fiéis ao layout de referência (media_1793306523730_4b704918.png)
    int panelW = 290 * s;
    int panelH = 60 * s;
    int panelMargin = 10 * s;
    int panelX = m_screenWidth - panelW - panelMargin;
    int panelY = m_screenHeight - panelH - panelMargin;

    // Chassi Metálico Estilo Painel Tático com borda dupla
    drawRect(screenBuffer, panelX, panelY, panelW, panelH, 0xEB0A0E10);
    drawBorder(screenBuffer, panelX, panelY, panelW, panelH, 0xFF4D4032);
    drawBorder(screenBuffer, panelX + 1 * s, panelY + 1 * s, panelW - 2 * s, panelH - 2 * s, 0xFF1F2922);

    int barStartX = panelX + 30 * s;
    int barW = 110 * s;
    int barH = 9 * s;

    // 1. Barra de Vida (HP): Vermelha contínua com gradiente suave
    int hpY = panelY + 7 * s;
    drawText(screenBuffer, panelX + 10 * s, hpY + 1 * s, "HP", 0xFFB6834E, s);
    drawRect(screenBuffer, barStartX, hpY, barW, barH, 0xFF181515);

    float hpPct = std::clamp(player.getHealth() / 100.0f, 0.0f, 1.0f);
    int filledHpW = static_cast<int>(barW * hpPct);
    uint32_t hpColor = (hpPct > 0.35f) ? 0xFFC63E32 : 0xFFFF2020;
    drawRect(screenBuffer, barStartX, hpY, filledHpW, barH, hpColor);
    drawBorder(screenBuffer, barStartX, hpY, barW, barH, 0xFF2A2020);

    // Texto RADS à direita da linha de HP em Vermelho/Alaranjado
    drawText(screenBuffer, barStartX + barW + 12 * s, hpY + 1 * s, "RADS", 0xFFD85040, s);
    drawText(screenBuffer, barStartX + barW + 52 * s, hpY + 1 * s, "1", 0xFFD85040, s);

    // 2. Barra de Stamina (ST): Verde contínua
    int staY = hpY + barH + 6 * s;
    drawText(screenBuffer, panelX + 10 * s, staY + 1 * s, "ST", 0xFF718A62, s);
    drawRect(screenBuffer, barStartX, staY, barW, barH, 0xFF121B14);

    float staPct = std::clamp(player.getStamina() / 100.0f, 0.0f, 1.0f);
    int filledStaW = static_cast<int>(barW * staPct);
    drawRect(screenBuffer, barStartX, staY, filledStaW, barH, 0xFF5F993D);
    drawBorder(screenBuffer, barStartX, staY, barW, barH, 0xFF243628);

    // Texto AMMO 30x em Verde Tático
    drawText(screenBuffer, barStartX + barW + 12 * s, staY + 1 * s, "AMMO 30X", 0xFF749462, s);

    // 3. Barra de Armadura (AH): Segmentada verticalmente em traços amarelos / dourados
    int ahY = staY + barH + 6 * s;
    drawText(screenBuffer, panelX + 10 * s, ahY + 1 * s, "AH", 0xFFA08656, s);
    drawRect(screenBuffer, barStartX, ahY, barW, barH, 0xFF1A1710);

    constexpr int AH_TOTAL_TICKS = 18;
    int activeTicks = 12; // Valor de integridade de blindagem
    int tickW = 4 * s;
    int tickGap = 2 * s;
    for (int t = 0; t < AH_TOTAL_TICKS; ++t) {
        int tx = barStartX + 2 * s + t * (tickW + tickGap);
        if (tx + tickW > barStartX + barW) break;
        uint32_t tCol = (t < activeTicks) ? 0xFFB39D5A : 0xFF2A2416;
        drawRect(screenBuffer, tx, ahY + 1 * s, tickW, barH - 2 * s, tCol);
    }
    drawBorder(screenBuffer, barStartX, ahY, barW, barH, 0xFF3D3422);

    // Texto RADS 3 / KILLS em Âmbar
    drawText(screenBuffer, barStartX + barW + 12 * s, ahY + 1 * s, "RADS", 0xFFC1A25C, s);
    drawText(screenBuffer, barStartX + barW + 52 * s, ahY + 1 * s, "3", 0xFFC1A25C, s);

    // 4. Medidores Verticais no canto direito do painel
    // Medidor 1: Barra de Nível Amarela/Dourada com segmentos
    int gauge1X = panelX + panelW - 40 * s;
    int gaugeY = panelY + 7 * s;
    int gaugeW = 11 * s;
    int gaugeH = 46 * s;

    drawRect(screenBuffer, gauge1X, gaugeY, gaugeW, gaugeH, 0xFF0D120E);
    drawBorder(screenBuffer, gauge1X, gaugeY, gaugeW, gaugeH, 0xFF2F3D2C);

    // Preenchimento de 9 segmentos amarelos
    constexpr int GAUGE_SEGS = 12;
    int activeSegs = 7;
    int segH = 2 * s;
    int segGap = 1 * s;
    for (int gs = 0; gs < activeSegs; ++gs) {
        int sy = gaugeY + gaugeH - 4 * s - (gs + 1) * (segH + segGap);
        drawRect(screenBuffer, gauge1X + 2 * s, sy, gaugeW - 4 * s, segH, 0xFFE2AC2C);
    }

    // Medidor 2: Barra vertical fina com ícone de biohazard/símbolo na base
    int gauge2X = panelX + panelW - 22 * s;
    drawRect(screenBuffer, gauge2X, gaugeY, gaugeW, gaugeH, 0xFF0D120E);
    drawBorder(screenBuffer, gauge2X, gaugeY, gaugeW, gaugeH, 0xFF2F3D2C);
    drawRect(screenBuffer, gauge2X + 3 * s, gaugeY + gaugeH - 10 * s, gaugeW - 6 * s, 6 * s, 0xFF5D4E3A);
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

    // Estatísticas dinâmicas escritas dentro do visor CRT com alinhamento tabular preciso
    std::string header = "K.I.A. - OPERADOR ABATIDO NA ZONA";
    int headerW = static_cast<int>(header.length()) * 6 * s;
    drawText(screenBuffer, crtX + (crtW - headerW) / 2, crtY + 22 * s, header, 0xFFFF3333, s);

    int mins = static_cast<int>(timeSurvived) / 60;
    int secs = static_cast<int>(timeSurvived) % 60;

    char val1[16];
    std::snprintf(val1, sizeof(val1), "%02d:%02d", mins, secs);
    std::string val2 = std::to_string(mutantsKilled);
    std::string val3 = "+" + std::to_string(dataPoints) + " PTS";
    int savedCount = briefcase.getSafePocketItemCount();
    std::string val4 = std::to_string(savedCount) + " ITEMS";

    int maxValChars = static_cast<int>(std::max({
        std::strlen(val1), val2.length(), val3.length(), val4.length()
    }));

    // O rótulo mais longo é "DATA POINTS EARNED:" com 19 caracteres
    const int labelChars = 19;
    const int gapChars = 4; // Espaçamento harmônico entre o rótulo e o valor
    int totalTableChars = labelChars + gapChars + maxValChars;
    int tableW = totalTableChars * 6 * s;

    // Centraliza o bloco tabular horizontalmente com precisão simétrica dentro do visor CRT (crtW)
    int colLabelX = crtX + (crtW - tableW) / 2;
    int colValX   = colLabelX + (labelChars + gapChars) * 6 * s;

    // Linha 1: Tempo Sobrevivido
    int row1Y = crtY + 44 * s;
    drawText(screenBuffer, colLabelX, row1Y, "TIME SURVIVED:", 0xFF749462, s);
    drawText(screenBuffer, colValX, row1Y, val1, 0xFF98E8AA, s);

    // Linha 2: Mutantes Eliminados
    int row2Y = crtY + 60 * s;
    drawText(screenBuffer, colLabelX, row2Y, "MUTANTS KILLED:", 0xFF749462, s);
    drawText(screenBuffer, colValX, row2Y, val2, 0xFF98E8AA, s);

    // Linha 3: Pontos de Dados Conquistados
    int row3Y = crtY + 76 * s;
    drawText(screenBuffer, colLabelX, row3Y, "DATA POINTS EARNED:", 0xFFB39D5A, s);
    drawText(screenBuffer, colValX, row3Y, val3, 0xFFFFD466, s);

    // Linha 4: Itens Salvos no Bolso Blindado
    int row4Y = crtY + 92 * s;
    drawText(screenBuffer, colLabelX, row4Y, "SAFE POCKET SAVED:", 0xFF4EB8D4, s);
    drawText(screenBuffer, colValX, row4Y, val4, 0xFF66E0FF, s);

    // Ícones dos itens salvos no Safe Pocket 2x2 com moldura e sprite real alinhados à tabela
    int iconX = colLabelX;
    int iconY = crtY + 110 * s;
    int iconSlotSize = 24 * s;
    for (const auto& item : briefcase.getItems()) {
        if (!item.inSafePocket) continue;
        drawRect(screenBuffer, iconX, iconY, iconSlotSize, iconSlotSize, 0xFF142018);
        if (uiBriefcaseAtlas && uiBriefcaseAtlas->isLoaded()) {
            Engine::SpriteRect spriteRect = briefcase.getSpriteForItemDef(item.def.id, item.def.category, item.def.element);
            uiBriefcaseAtlas->drawSprite(
                screenBuffer, m_screenWidth, m_screenHeight,
                spriteRect,
                iconX + 2 * s, iconY + 2 * s, iconSlotSize - 4 * s, iconSlotSize - 4 * s, 1.25f
            );
        }
        drawBorder(screenBuffer, iconX, iconY, iconSlotSize, iconSlotSize, 0xFFFFCC44);
        iconX += iconSlotSize + 6 * s;
    }

    // 3. Botão Inferior Recortado para Regressar ao Bunker
    int btnW = 240 * s;
    int btnH = 50 * s;
    int btnX = (m_screenWidth - btnW) / 2;
    int btnY = m_screenHeight - 72 * s;

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

    std::string btnLabel = "REGRESSAR [ENTER / CLIQUE]";
    int lblW = static_cast<int>(btnLabel.length()) * 6 * s; // 6px por caractere na nova fonte 5x7!
    int lblX = (m_screenWidth - lblW) / 2;
    int lblY = btnY + (btnH - 7 * s) / 2; // Perfeitamente centralizado na vertical
    drawText(screenBuffer, lblX, lblY, btnLabel, hovered ? 0xFFFFFFFF : 0xFF78FF96, s);

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
    const int s = (m_screenWidth >= 1200) ? 2 : 1;

    // 1. Renderiza a imagem de fundo completa do Bunker (screen_bunker_hub.png) sem cortes
    if (bgBunkerAtlas.isLoaded()) {
        bgBunkerAtlas.drawFullScreen(screenBuffer, m_screenWidth, m_screenHeight, 1.0f);
    } else {
        drawRect(screenBuffer, 0, 0, m_screenWidth, m_screenHeight, 0xFF101416);
    }

    // 2. Atualiza os indicadores dinâmicos no topo dentro das 4 caixas dedicadas da arte original (screen_bunker_hub.png)
    int topY = static_cast<int>(m_screenHeight * 0.038f);

    // Caixa 1 (Esquerda): Título da Base
    int titleX = static_cast<int>(m_screenWidth * 0.052f);
    drawText(screenBuffer, titleX, topY, "BUNKER BASE DE OPERATIONS", 0xFF98E8AA, s);

    // Caixa 2 (Centro-Esquerda): Moeda / Créditos (ao lado do ícone da moeda na arte)
    int credX = static_cast<int>(m_screenWidth * 0.395f);
    char credBuf[32];
    if (bunker.credits >= 1000) {
        std::snprintf(credBuf, sizeof(credBuf), "CURRENCY: %d,%03d", bunker.credits / 1000, bunker.credits % 1000);
    } else {
        std::snprintf(credBuf, sizeof(credBuf), "CURRENCY: %d", bunker.credits);
    }
    drawText(screenBuffer, credX, topY, credBuf, 0xFFFFCC55, s);

    // Caixa 3 (Centro-Direita): Sucata / Scrap (ao lado do ícone de engrenagem na arte)
    int scrapX = static_cast<int>(m_screenWidth * 0.595f);
    char scrapBuf[32];
    std::snprintf(scrapBuf, sizeof(scrapBuf), "SCRAP: %d", bunker.scrap);
    drawText(screenBuffer, scrapX, topY, scrapBuf, 0xFFD8E0D8, s);

    // Caixa 4 (Direita): Energia / Power (ao lado do ícone de raio na arte) + Barra segmentada verde
    int powerX = static_cast<int>(m_screenWidth * 0.768f);
    char powerBuf[32];
    std::snprintf(powerBuf, sizeof(powerBuf), "POWER: %d%%", bunker.powerPct);
    drawText(screenBuffer, powerX, topY, powerBuf, 0xFFFFCC55, s);

    // Barra de segmentos verdes de energia à direita do texto POWER
    int barStartX = powerX + static_cast<int>(std::strlen(powerBuf)) * 6 * s + 8 * s;
    int pwrBarY = topY + 1 * s;
    int barH = 7 * s;
    int totalSegments = 16;
    int filledSegments = std::clamp((bunker.powerPct * totalSegments) / 100, 0, totalSegments);
    int segW = 3 * s;
    int segGap = 1 * s;

    for (int seg = 0; seg < totalSegments; ++seg) {
        int sx = barStartX + seg * (segW + segGap);
        uint32_t segColor = (seg < filledSegments) ? 0xFF33EE55 : 0xFF142418;
        for (int py = pwrBarY; py < pwrBarY + barH; ++py) {
            if (py < 0 || py >= m_screenHeight) continue;
            for (int px = sx; px < sx + segW; ++px) {
                if (px < 0 || px >= m_screenWidth) continue;
                screenBuffer[py * m_screenWidth + px] = segColor;
            }
        }
    }

    // 3. Definição dos 5 Hotspots Interativos das Estações do Bunker alinhados à arte original
    struct StationHotspot {
        int id;
        int x, y, w, h;
        const char* title;
        const char* actionDesc;
    };

    const StationHotspot stations[5] = {
        { 1, static_cast<int>(m_screenWidth * 0.02f), static_cast<int>(m_screenHeight * 0.38f), static_cast<int>(m_screenWidth * 0.22f), static_cast<int>(m_screenHeight * 0.48f), "[1] INFIRMARY",    "CURAR 100% HP & SANGRAMENTO (-25 CREDITS)" },
        { 2, static_cast<int>(m_screenWidth * 0.26f), static_cast<int>(m_screenHeight * 0.46f), static_cast<int>(m_screenWidth * 0.18f), static_cast<int>(m_screenHeight * 0.40f), "[2] GENERATOR",    "REABASTECER GERADOR & +30 MUNICAO (-15 SCRAP)" },
        { 3, static_cast<int>(m_screenWidth * 0.46f), static_cast<int>(m_screenHeight * 0.42f), static_cast<int>(m_screenWidth * 0.15f), static_cast<int>(m_screenHeight * 0.40f), "[3] WORKBENCH",    "ABRIR MALETA 8X4 & FUNDIR SINERGIAS [TAB]" },
        { 4, static_cast<int>(m_screenWidth * 0.62f), static_cast<int>(m_screenHeight * 0.46f), static_cast<int>(m_screenWidth * 0.13f), static_cast<int>(m_screenHeight * 0.40f), "[4] TERMINAL",     "INFILTRAR NA ZONA ( INICIAR MISSAO! )" },
        { 5, static_cast<int>(m_screenWidth * 0.76f), static_cast<int>(m_screenHeight * 0.35f), static_cast<int>(m_screenWidth * 0.22f), static_cast<int>(m_screenHeight * 0.50f), "[5] BLACK MARKET", "VENDER ARTEFATO (+250 CREDITS) / COMPRAR ORBE" }
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
            drawRect(screenBuffer, st.x, st.y - 18 * s, st.w, 16 * s, 0xE60B1810);
            drawBorder(screenBuffer, st.x, st.y - 18 * s, st.w, 16 * s, 0xFF50FF64);
            drawText(screenBuffer, st.x + 4 * s, st.y - 14 * s, st.title, 0xFF78FF96, s);
        } else {
            // Etiquetas visíveis permanentes com títulos das estações do Bunker!
            drawBorder(screenBuffer, st.x, st.y, st.w, st.h, 0x88285838);
            drawRect(screenBuffer, st.x, st.y - 14 * s, st.w, 13 * s, 0xCC09120C);
            drawBorder(screenBuffer, st.x, st.y - 14 * s, st.w, 13 * s, 0xFF2F5438);
            drawText(screenBuffer, st.x + 4 * s, st.y - 11 * s, st.title, 0xFF98E8AA, s);
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
        "PROXIMA MISSAO: %s | HP: %d%% | [ENTER] INFILTRAR | [O] OPCOES DE VIDEO",
        curSector, static_cast<int>(player.getHealth())
    );
    drawText(screenBuffer, 20 * s, barY + 16 * s, bottomLine, 0xFFFFD466, s);

    drawCrosshair(screenBuffer, mouseX, mouseY, false);
    return hoveredStation;
}

// =============================================================================
// MENU EXCLUSIVO DE RESOLUÇÃO (720p / Steam Deck 1280x800 / 1080p)
// =============================================================================
int HUD::renderVideoMenu(
    uint32_t* screenBuffer,
    int mouseX,
    int mouseY,
    float fps,
    bool isFullscreen
) {
    if (!m_videoMenuOpen) return 0;

    const int s = (m_screenWidth >= 1600) ? 2 : 1;
    auto scX = [this](int v) { return (v * m_screenWidth) / 1280; };
    auto scY = [this](int v) { return (v * m_screenHeight) / 800; };

    int selectedAction = 0;

    // Fundo escurecido semi-transparente
    drawRect(screenBuffer, 0, 0, m_screenWidth, m_screenHeight, 0xCC050A08);

    // Painel Central Estilo Terminal Tático
    int panelW = scX(560);
    int panelH = scY(420);
    int panelX = (m_screenWidth - panelW) / 2;
    int panelY = (m_screenHeight - panelH) / 2;

    drawRect(screenBuffer, panelX, panelY, panelW, panelH, 0xF60E1812);
    drawBorder(screenBuffer, panelX, panelY, panelW, panelH, 0xFF50FF64);
    drawBorder(screenBuffer, panelX + 2 * s, panelY + 2 * s, panelW - 4 * s, panelH - 4 * s, 0xFF1C3A24);

    // Cabeçalho
    int headH = scY(32);
    drawRect(screenBuffer, panelX + 4 * s, panelY + 4 * s, panelW - 8 * s, headH, 0xFF14281A);
    drawBorder(screenBuffer, panelX + 4 * s, panelY + 4 * s, panelW - 8 * s, headH, 0xFF386A44);
    drawText(screenBuffer, panelX + scX(16), panelY + scY(12), "CONFIGURACOES DE RESOLUCAO & TELA", 0xFF78FF96, s);

    // Telemetria da tela ativa
    char curInfo[128];
    std::snprintf(
        curInfo, sizeof(curInfo),
        "RESOLUCAO ATUAL: %dX%d | TAXA: %.0f FPS | MODO: %s",
        m_screenWidth, m_screenHeight, fps, isFullscreen ? "TELA CHEIA" : "JANELA"
    );
    drawText(screenBuffer, panelX + scX(18), panelY + scY(46), curInfo, 0xFFFFCC55, s);

    // Estrutura dos 3 Padrões Oficiais
    struct VideoOpt {
        int id;
        int w;
        int h;
        const char* title;
        const char* desc;
    };

    const VideoOpt opts[3] = {
        { 1, 1280, 720,  "[1] 720P PADRAO (1280 X 720 - 16:9)",        "PERFORMANCE ELEVADA / COMPATIBILIDADE UNIVERSAL" },
        { 2, 1280, 800,  "[2] STEAM DECK NATIVO (1280 X 800 - 16:10)", "RESOLUCAO NATIVA DO STEAM DECK / TELAS 16:10" },
        { 3, 1920, 1080, "[3] 1080P FULL HD (1920 X 1080 - 16:9)",     "RESOLUCAO MAXIMA DE ALTA DEFINICAO / MONITORES FHD" }
    };

    int cardStartY = panelY + scY(76);
    int cardW = panelW - scX(32);
    int cardH = scY(62);
    int cardGap = scY(10);

    for (int i = 0; i < 3; ++i) {
        int cx = panelX + scX(16);
        int cy = cardStartY + i * (cardH + cardGap);

        bool isCurrent = (m_screenWidth == opts[i].w && m_screenHeight == opts[i].h);
        bool isHover = (mouseX >= cx && mouseX < cx + cardW && mouseY >= cy && mouseY < cy + cardH);

        if (isHover) {
            selectedAction = opts[i].id;
        }

        uint32_t bg = isCurrent ? 0xFF1C4226 : (isHover ? 0xFF183022 : 0xFF101C14);
        uint32_t border = isCurrent ? 0xFF50FF64 : (isHover ? 0xFF4DEEEA : 0xFF2A4A32);

        drawRect(screenBuffer, cx, cy, cardW, cardH, bg);
        drawBorder(screenBuffer, cx, cy, cardW, cardH, border);

        std::string tag = isCurrent ? ">> ATIVO: " : "   ";
        drawText(screenBuffer, cx + scX(12), cy + scY(12), tag + opts[i].title, isCurrent ? 0xFF50FF64 : 0xFFE0F4E8, s);
        drawText(screenBuffer, cx + scX(12), cy + scY(34), opts[i].desc, 0xFF88C89C, s);
    }

    // Botões inferiores: Alternar Tela Cheia [F11] e Fechar Menu [ESC / O]
    int btnY = panelY + panelH - scY(46);
    int btnW1 = scX(255);
    int btnW2 = panelW - scX(40) - btnW1;
    int btnX1 = panelX + scX(16);
    int btnX2 = btnX1 + btnW1 + scX(8);
    int btnH = scY(32);

    bool hovFs = (mouseX >= btnX1 && mouseX < btnX1 + btnW1 && mouseY >= btnY && mouseY < btnY + btnH);
    bool hovClose = (mouseX >= btnX2 && mouseX < btnX2 + btnW2 && mouseY >= btnY && mouseY < btnY + btnH);

    if (hovFs) selectedAction = 4;
    if (hovClose) selectedAction = 5;

    drawRect(screenBuffer, btnX1, btnY, btnW1, btnH, hovFs ? 0xFF20444C : 0xFF12262E);
    drawBorder(screenBuffer, btnX1, btnY, btnW1, btnH, hovFs ? 0xFF4DEEEA : 0xFF2A525E);
    std::string fsText = isFullscreen ? "[F11] TELA CHEIA (ATIVA)" : "[F11] MODO JANELA";
    drawText(screenBuffer, btnX1 + scX(12), btnY + scY(10), fsText, 0xFF88E8FF, s);

    drawRect(screenBuffer, btnX2, btnY, btnW2, btnH, hovClose ? 0xFF502222 : 0xFF281212);
    drawBorder(screenBuffer, btnX2, btnY, btnW2, btnH, hovClose ? 0xFFFF5555 : 0xFF662A2A);
    drawText(screenBuffer, btnX2 + scX(12), btnY + scY(10), "FECHAR MENU [O / ESC]", 0xFFFFCCCC, s);

    drawCrosshair(screenBuffer, mouseX, mouseY, false);
    return selectedAction;
}

} // namespace DayDoom::Game::UI
