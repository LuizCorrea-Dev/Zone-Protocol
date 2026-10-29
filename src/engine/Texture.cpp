#include "Texture.hpp"
#include "SpriteAtlas.hpp"
#include "MathDef.hpp"
#include <cmath>
#include <algorithm>

namespace DayDoom::Engine {

TextureManager::TextureManager() {
    for (auto& tex : m_textures) {
        tex.resize(TEX_SIZE * TEX_SIZE, 0xFF000000);
    }
    initProceduralTextures();
}

void TextureManager::initProceduralTextures() {
    // =========================================================================
    // 0. BRICK_DARK (BRICK01): Tijolos clássicos estilo Doom com bisel 3D e argamassa
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        int rowIndex = y / 8;
        int rowY = y % 8;
        bool isHorizMortar = (rowY == 0 || rowY == 7);

        for (int x = 0; x < TEX_SIZE; ++x) {
            int shiftX = (rowIndex % 2 == 1) ? 8 : 0;
            int brickX = (x + shiftX) % 16;
            bool isVertMortar = (brickX == 0 || brickX == 15);

            if (isHorizMortar || isVertMortar) {
                // Argamassa escura de cimento com grãos
                int grain = ((x * 13) ^ (y * 17)) % 10;
                m_textures[BRICK_DARK][y * TEX_SIZE + x] = makeColor(30 + grain, 26 + grain, 24 + grain);
            } else {
                // Corpo do tijolo com variação de cor por tijolo (terracota, carmesim e marrom escuro)
                int brickID = ((x + shiftX) / 16) + rowIndex * 5;
                int baseRed = 135 + ((brickID * 37) % 40) - 20;
                int baseGreen = 55 + ((brickID * 19) % 25) - 10;
                int baseBlue = 40 + ((brickID * 23) % 20) - 10;

                // Relevo 3D: topo e esquerda recebem luz (bisel claro), base e direita sombra profunda
                bool isHighlight = (rowY == 1 || brickX == 1);
                bool isShadow = (rowY == 6 || brickX == 14);

                int noise = ((x * 7 + y * 11) % 18) - 9;

                if (isHighlight) {
                    m_textures[BRICK_DARK][y * TEX_SIZE + x] = makeColor(
                        std::clamp(baseRed + 45 + noise, 0, 255),
                        std::clamp(baseGreen + 30 + noise, 0, 255),
                        std::clamp(baseBlue + 25 + noise, 0, 255)
                    );
                } else if (isShadow) {
                    m_textures[BRICK_DARK][y * TEX_SIZE + x] = makeColor(
                        std::clamp(baseRed - 50 + noise, 0, 255),
                        std::clamp(baseGreen - 30 + noise, 0, 255),
                        std::clamp(baseBlue - 25 + noise, 0, 255)
                    );
                } else {
                    m_textures[BRICK_DARK][y * TEX_SIZE + x] = makeColor(
                        std::clamp(baseRed + noise, 0, 255),
                        std::clamp(baseGreen + noise / 2, 0, 255),
                        std::clamp(baseBlue + noise / 2, 0, 255)
                    );
                }
            }
        }
    }

    // =========================================================================
    // 1. CONCRETE (CONC02): Concreto soviético com juntas, parafusos e escorrimento
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isJointX = (x == 31 || x == 32);
            bool isJointY = (y == 31 || y == 32);
            bool isBorder = (x == 0 || x == 63 || y == 0 || y == 63);

            // Furos de amarração de cofragem com arruela de aço (Tie-rod holes)
            bool isBolt = ((x >= 8 && x <= 11 && y >= 8 && y <= 11) ||
                           (x >= 52 && x <= 55 && y >= 8 && y <= 11) ||
                           (x >= 8 && x <= 11 && y >= 52 && y <= 55) ||
                           (x >= 52 && x <= 55 && y >= 52 && y <= 55));

            int noise = ((x ^ y) * 11 + (x * 7) ^ (y * 13)) % 25;
            int drip = (y > 32 && (x % 16 == 5 || x % 16 == 12)) ? ((64 - y) / 3) : 0;

            if (isBolt) {
                // Parafuso / cavidade circular metálica
                m_textures[CONCRETE][y * TEX_SIZE + x] = makeColor(35, 38, 42);
            } else if (isBorder || isJointX || isJointY) {
                // Juntas de dilatação em reentrância escura
                m_textures[CONCRETE][y * TEX_SIZE + x] = makeColor(40, 42, 45);
            } else {
                int c = 115 + noise - drip;
                if (y > 48 && ((x + y) % 5 == 0)) {
                    // Manchas de musgo nos cantos inferiores
                    m_textures[CONCRETE][y * TEX_SIZE + x] = makeColor(c - 30, c + 15, c - 20);
                } else {
                    m_textures[CONCRETE][y * TEX_SIZE + x] = makeColor(c, c + 2, c - 3);
                }
            }
        }
    }

    // =========================================================================
    // 2. STORE_FRONT (WINCOMM): Vitrine com vidro quebrado e tábuas de barricada
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isFrame = (x < 3 || x > 60 || y < 3 || y > 60);
            // Tábuas de madeira pregadas diagonal e horizontalmente
            bool isBoard1 = (y >= 18 && y <= 26);
            bool isBoard2 = (y >= 38 && y <= 46);
            bool isBoardDiag = (x - y >= -5 && x - y <= 5);

            if (isFrame) {
                // Caixilho de alumínio industrial anodizado escuro com bisel
                uint8_t c = (x == 1 || y == 1) ? 90 : 40;
                m_textures[STORE_FRONT][y * TEX_SIZE + x] = makeColor(c, c + 5, c + 10);
            } else if (isBoard1 || isBoard2 || isBoardDiag) {
                // Tábuas de madeira desgastada com veio e pregos
                int grain = (x * 7 + y * 3) % 18;
                bool isNail = ((x == 6 || x == 57) && (y == 22 || y == 42));
                if (isNail) {
                    m_textures[STORE_FRONT][y * TEX_SIZE + x] = makeColor(220, 220, 230); // Cabeça do prego brilhante
                } else {
                    m_textures[STORE_FRONT][y * TEX_SIZE + x] = makeColor(140 + grain, 90 + grain, 50 + grain);
                }
            } else {
                // Vidro escuro reflexivo com trincas e rachaduras brancas
                bool isCrack = (x == y || x + y == 45 || (x == 32 && y > 20));
                if (isCrack) {
                    m_textures[STORE_FRONT][y * TEX_SIZE + x] = makeColor(200, 220, 235); // Trinca iluminada
                } else {
                    int glassShade = 20 + ((x + y) % 15);
                    m_textures[STORE_FRONT][y * TEX_SIZE + x] = makeColor(15, 25 + glassShade / 2, 40 + glassShade);
                }
            }
        }
    }

    // =========================================================================
    // 3. WOOD_DOOR (DOORWOOD): Porta residencial de carvalho com ripas e ferragens
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isOuterFrame = (x < 2 || x > 61 || y < 2 || y > 61);
            int plank = x / 16;
            int plankX = x % 16;
            bool isPlankSeam = (plankX == 0 || plankX == 15);

            // Ferragens: 2 dobradicas de ferro forjado preto com pontas triangulares na lateral da dobradica (x <= 26)
            bool isHingeTop = (y >= 10 && y <= 16 && x <= 26);
            bool isHingeBot = (y >= 46 && y <= 52 && x <= 26);
            bool isRivet = ((isHingeTop || isHingeBot) && (x == 5 || x == 12 || x == 20));

            // Espelho e macaneta de bronze dourado na lateral da ponta da porta (x >= 46)
            bool isPlate = (x >= 49 && x <= 55 && y >= 26 && y <= 38);
            bool isHandle = (x >= 46 && x <= 58 && y >= 29 && y <= 32);
            bool isKeyhole = (x >= 51 && x <= 53 && y >= 34 && y <= 36);

            if (isKeyhole) {
                m_textures[WOOD_DOOR][y * TEX_SIZE + x] = makeColor(15, 10, 5);
            } else if (isHandle) {
                m_textures[WOOD_DOOR][y * TEX_SIZE + x] = makeColor(240, 205, 55); // Ouro brilhante
            } else if (isPlate) {
                m_textures[WOOD_DOOR][y * TEX_SIZE + x] = makeColor(190, 155, 45); // Placa de bronze
            } else if (isRivet) {
                m_textures[WOOD_DOOR][y * TEX_SIZE + x] = makeColor(120, 125, 135); // Rebite de ferro
            } else if (isHingeTop || isHingeBot) {
                m_textures[WOOD_DOOR][y * TEX_SIZE + x] = makeColor(35, 38, 42); // Ferro forjado
            } else if (isOuterFrame || isPlankSeam) {
                m_textures[WOOD_DOOR][y * TEX_SIZE + x] = makeColor(40, 24, 15); // Ranhura escura
            } else {
                int grain = (y * 9 + x * 3 + plank * 7) % 22;
                bool isPlankLight = (plankX == 1);
                bool isPlankDark = (plankX == 14);

                if (isPlankLight) {
                    m_textures[WOOD_DOOR][y * TEX_SIZE + x] = makeColor(150 + grain, 95 + grain, 50 + grain);
                } else if (isPlankDark) {
                    m_textures[WOOD_DOOR][y * TEX_SIZE + x] = makeColor(85 + grain, 50 + grain, 25 + grain);
                } else {
                    m_textures[WOOD_DOOR][y * TEX_SIZE + x] = makeColor(115 + grain, 70 + grain, 35 + grain);
                }
            }
        }
    }

    // =========================================================================
    // 4. RUSTY_FENCE (CORRMT1): Chapa metálica ondulada com relevo e ferrugem
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            int waveX = x % 4;
            int ridge = 0;
            if (waveX == 0) ridge = -30; // Vale na sombra
            if (waveX == 1) ridge = 0;
            if (waveX == 2) ridge = 45;  // Crista com brilho metálico especular
            if (waveX == 3) ridge = 15;

            int rustPattern = ((x * 19 + y * 23) ^ (x * 7)) % 40;
            bool isRust = (rustPattern > 26);

            if (isRust) {
                int rVal = std::clamp(130 + rustPattern, 0, 255);
                m_textures[RUSTY_FENCE][y * TEX_SIZE + x] = makeColor(rVal, 50 + rustPattern / 2, 20);
            } else {
                int mVal = std::clamp(95 + ridge, 0, 255);
                m_textures[RUSTY_FENCE][y * TEX_SIZE + x] = makeColor(mVal, mVal + 2, mVal + 6);
            }
        }
    }

    // =========================================================================
    // 5. ASPHALT: Asfalto com rachaduras, pedras britadas e faixa amarela
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            int stone = ((x * 17) ^ (y * 23)) % 30;
            bool isCrack = (x == (30 + ((y * 7) % 7) - 3)) && (y > 10 && y < 50);
            bool isYellowLine = (x >= 29 && x <= 34) && (y % 20 < 12);

            if (isCrack) {
                m_textures[ASPHALT][y * TEX_SIZE + x] = makeColor(15, 15, 16);
            } else if (isYellowLine && stone < 24) {
                m_textures[ASPHALT][y * TEX_SIZE + x] = makeColor(210 + stone, 175 + stone, 35);
            } else {
                uint8_t a = 42 + stone;
                m_textures[ASPHALT][y * TEX_SIZE + x] = makeColor(a, a + 1, a + 2);
            }
        }
    }

    // =========================================================================
    // 6. WOOD_FLOOR: Tábuas corridas de madeira interna
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isPlankEdge = (y % 16 == 0);
            bool isHeader = (y % 16 != 0) && (((y / 16) % 2 == 0 && x % 32 == 0) || ((y / 16) % 2 == 1 && (x + 16) % 32 == 0));

            if (isPlankEdge || isHeader) {
                m_textures[WOOD_FLOOR][y * TEX_SIZE + x] = makeColor(28, 18, 10);
            } else {
                int g = (x * 3 + y * 7) % 20;
                m_textures[WOOD_FLOOR][y * TEX_SIZE + x] = makeColor(135 + g, 90 + g, 55 + g);
            }
        }
    }

    // =========================================================================
    // 7. SKY_OVERCAST (F_SKY1): Céu nublado da Zona de Exclusão
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            int cloud = ((x * 5 + y * 11) ^ (x * 3)) % 25;
            uint8_t gray = 45 + (y / 2) + cloud;
            m_textures[SKY_OVERCAST][y * TEX_SIZE + x] = makeColor(gray + 5, gray, gray - 5);
        }
    }

    // =========================================================================
    // 8. CONTAINER_RED (CONTRED): Contêiner marítimo vermelho com relevo 3D
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isFrame = (x < 3 || x > 60 || y < 3 || y > 60);
            int rib = x % 6;
            int ridgeShade = 0;
            if (rib == 0) ridgeShade = -40; // Sombra do friso
            if (rib == 1) ridgeShade = 0;
            if (rib == 2 || rib == 3) ridgeShade = 40; // Crista frontal com luz
            if (rib == 4) ridgeShade = 15;
            if (rib == 5) ridgeShade = -25;

            if (isFrame) {
                m_textures[CONTAINER_RED][y * TEX_SIZE + x] = makeColor(55, 20, 18);
            } else {
                int r = std::clamp(155 + ridgeShade, 0, 255);
                m_textures[CONTAINER_RED][y * TEX_SIZE + x] = makeColor(r, 35, 30);
            }
        }
    }

    // =========================================================================
    // 9. CONTAINER_BLUE (CONTBLU): Contêiner marítimo azul com relevo 3D
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isFrame = (x < 3 || x > 60 || y < 3 || y > 60);
            int rib = x % 6;
            int ridgeShade = 0;
            if (rib == 0) ridgeShade = -40;
            if (rib == 1) ridgeShade = 0;
            if (rib == 2 || rib == 3) ridgeShade = 45;
            if (rib == 4) ridgeShade = 15;
            if (rib == 5) ridgeShade = -25;

            if (isFrame) {
                m_textures[CONTAINER_BLUE][y * TEX_SIZE + x] = makeColor(18, 30, 55);
            } else {
                int b = std::clamp(155 + ridgeShade, 0, 255);
                int g = std::clamp(75 + ridgeShade / 2, 0, 255);
                m_textures[CONTAINER_BLUE][y * TEX_SIZE + x] = makeColor(25, g, b);
            }
        }
    }

    // =========================================================================
    // 10. DOOR_SLIDING (DOORMLT): Porta blindada com faixas zebradas de perigo
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isHazard = (y >= 50 && y <= 60);
            bool isWindow = (x >= 24 && x <= 40 && y >= 14 && y <= 26);
            bool isFrame = (x < 3 || x > 60 || y < 3 || y > 60);
            bool isKeypad = (x >= 50 && x <= 56 && y >= 30 && y <= 38);

            if (isKeypad) {
                // Teclado de acesso eletrônico da polícia
                bool isLed = (y == 32 && x == 53);
                m_textures[DOOR_SLIDING][y * TEX_SIZE + x] = isLed ? makeColor(40, 255, 60) : makeColor(20, 22, 28);
            } else if (isWindow) {
                // Visor blindado com malha aramada
                bool isWire = (x % 4 == 0 || y % 4 == 0);
                m_textures[DOOR_SLIDING][y * TEX_SIZE + x] = isWire ? makeColor(180, 190, 205) : makeColor(25, 35, 50);
            } else if (isHazard) {
                // Faixa zebrada amarela e preta
                bool isYellow = ((x + y) % 12 < 6);
                m_textures[DOOR_SLIDING][y * TEX_SIZE + x] = isYellow ? makeColor(240, 210, 25) : makeColor(30, 30, 32);
            } else if (isFrame) {
                m_textures[DOOR_SLIDING][y * TEX_SIZE + x] = makeColor(45, 50, 58);
            } else {
                // Aço usinado escuro com painel rebaixado
                bool isRecess = (x >= 8 && x <= 56 && y >= 8 && y <= 48);
                int steelNoise = (x * 3 + y * 7) % 15;
                if (isRecess && (x == 8 || y == 8)) {
                    m_textures[DOOR_SLIDING][y * TEX_SIZE + x] = makeColor(40, 45, 52); // Sombra do rebaixo
                } else if (isRecess && (x == 56 || y == 48)) {
                    m_textures[DOOR_SLIDING][y * TEX_SIZE + x] = makeColor(140, 150, 165); // Brilho do rebaixo
                } else {
                    uint8_t s = 95 + steelNoise;
                    m_textures[DOOR_SLIDING][y * TEX_SIZE + x] = makeColor(s, s + 3, s + 8);
                }
            }
        }
    }

    // =========================================================================
    // 11. GATE_IRON (GATEIRON): Portão de segurança de ferro forjado
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isFrame = (x < 4 || x > 59 || y < 4 || y > 59);
            bool isVerticalBar = (x % 6 <= 1);
            bool isHorizontalRail = (y == 8 || y == 9 || y == 32 || y == 33 || y == 54 || y == 55);
            bool isPadlock = (x >= 48 && x <= 56 && y >= 28 && y <= 36);

            if (isPadlock) {
                m_textures[GATE_IRON][y * TEX_SIZE + x] = makeColor(210, 175, 45); // Cadeado dourado
            } else if (isFrame || isVerticalBar || isHorizontalRail) {
                int iron = (x * 5 + y * 7) % 15;
                m_textures[GATE_IRON][y * TEX_SIZE + x] = makeColor(65 + iron, 70 + iron, 78 + iron);
            } else {
                m_textures[GATE_IRON][y * TEX_SIZE + x] = makeColor(12, 14, 18); // Fundo vazado
            }
        }
    }

    // =========================================================================
    // 12. FENCE_WIRE (FENCEWI): Cerca de arame entrelaçado
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isPost = (x % 32 < 3);
            bool isMesh = ((x + y) % 8 == 0) || ((x - y + 64) % 8 == 0);
            bool isBarbedTop = (y < 6) && ((x % 6) == 0);

            if (isPost) {
                m_textures[FENCE_WIRE][y * TEX_SIZE + x] = makeColor(85, 90, 98);
            } else if (isBarbedTop || isMesh) {
                m_textures[FENCE_WIRE][y * TEX_SIZE + x] = makeColor(165, 170, 180);
            } else {
                m_textures[FENCE_WIRE][y * TEX_SIZE + x] = makeColor(15, 18, 22);
            }
        }
    }

    // =========================================================================
    // 13. HOSPITAL_WALL (HOSP01): Azulejos brancos e grande cruz vermelha 3D
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        int tileY = y % 8;
        bool isGroutY = (tileY == 0);

        for (int x = 0; x < TEX_SIZE; ++x) {
            int shiftX = ((y / 8) % 2 == 1) ? 8 : 0;
            int tileX = (x + shiftX) % 16;
            bool isGroutX = (tileX == 0);

            bool isCross = (x >= 25 && x <= 38 && y >= 14 && y <= 49) ||
                           (x >= 14 && x <= 49 && y >= 25 && y <= 38);

            if (isCross) {
                // Cruz Vermelha com relevo chanfrado
                bool isCrossBorder = (x == 25 || x == 38 || y == 14 || y == 49 ||
                                      x == 14 || x == 49 || y == 25 || y == 38);
                if (isCrossBorder) {
                    m_textures[HOSPITAL_WALL][y * TEX_SIZE + x] = makeColor(135, 15, 15);
                } else {
                    m_textures[HOSPITAL_WALL][y * TEX_SIZE + x] = makeColor(225, 30, 30);
                }
            } else if (isGroutX || isGroutY) {
                m_textures[HOSPITAL_WALL][y * TEX_SIZE + x] = makeColor(120, 125, 128);
            } else {
                // Azulejo branco esmaltado com brilho
                bool isTileLight = (tileX == 1 || tileY == 1);
                uint8_t w = isTileLight ? 230 : 205;
                m_textures[HOSPITAL_WALL][y * TEX_SIZE + x] = makeColor(w, w + 2, w + 3);
            }
        }
    }

    // =========================================================================
    // 14. SAND (SAND01): Areia da praia do Mar Báltico
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            int grain = ((x * 19) ^ (y * 23)) % 25;
            int ripple = static_cast<int>(std::sin((x + y * 0.4f) * 0.45f) * 8.0f);
            uint8_t r = std::clamp(185 + grain + ripple, 0, 255);
            uint8_t g = std::clamp(155 + grain / 2 + ripple, 0, 255);
            uint8_t b = std::clamp(95 + grain / 3, 0, 255);
            m_textures[SAND][y * TEX_SIZE + x] = makeColor(r, g, b);
        }
    }

    // =========================================================================
    // 15. WATER (WATER01): Água azul do Báltico com ondas e reflexos cáusticos
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            int wave = static_cast<int>(std::sin(x * 0.35f + y * 0.2f) * 22.0f);
            int foam = ((x * 17 + y * 13) % 35 > 31) ? 65 : 0;
            uint8_t r = 25 + foam / 2;
            uint8_t g = std::clamp(85 + wave + foam, 0, 255);
            uint8_t b = std::clamp(170 + wave + foam, 0, 255);
            m_textures[WATER][y * TEX_SIZE + x] = makeColor(r, g, b);
        }
    }

    // =========================================================================
    // 16. CONC_FLOOR (CONC01): Concreto polido com juntas quadradas
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isJoint = (x % 32 == 0 || y % 32 == 0);
            int grain = ((x * 11) ^ (y * 13)) % 18;
            if (isJoint) {
                m_textures[CONC_FLOOR][y * TEX_SIZE + x] = makeColor(50, 52, 55);
            } else {
                uint8_t c = 120 + grain;
                m_textures[CONC_FLOOR][y * TEX_SIZE + x] = makeColor(c, c + 1, c - 2);
            }
        }
    }

    // =========================================================================
    // 17. RAILROAD: Ferrovia girada a 90° alinhada ao eixo longitudinal (X)
    // Trilhos de aço paralelos ao eixo horizontal e dormentes perpendiculares (Y)
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        bool isRail = (y >= 12 && y <= 15) || (y >= 48 && y <= 51);
        bool isRailGlint = (y == 13 || y == 49);
        bool isRailShadow = (y == 15 || y == 51);
        bool isTiePlateZone = (y >= 10 && y <= 17) || (y >= 46 && y <= 53);

        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isTie = (x % 16 >= 4 && x % 16 <= 11) && (y >= 4 && y <= 59);

            if (isRailGlint) {
                // Crista de aço brilhante polida pelas rodas dos vagões
                m_textures[RAILROAD][y * TEX_SIZE + x] = makeColor(240, 245, 255);
            } else if (isRailShadow) {
                // Chanfro inferior do perfil do trilho
                m_textures[RAILROAD][y * TEX_SIZE + x] = makeColor(105, 110, 120);
            } else if (isRail) {
                // Corpo do trilho de aço
                m_textures[RAILROAD][y * TEX_SIZE + x] = makeColor(150, 155, 165);
            } else if (isTie && isTiePlateZone) {
                // Placa metálica de fixação do dormente (tirefões de ferro fundido)
                int plateRust = (x + y) % 3;
                m_textures[RAILROAD][y * TEX_SIZE + x] = makeColor(65 + plateRust, 62 + plateRust, 60 + plateRust);
            } else if (isTie) {
                // Dormente de madeira escura tratada com creosoto
                int wood = (y * 7 + x * 3) % 20;
                m_textures[RAILROAD][y * TEX_SIZE + x] = makeColor(72 + wood, 46 + wood, 26 + wood);
            } else if (y <= 2) {
                // Chanfro superior iluminado do talude elevado da ferrovia (+10 de elevação)
                int gravel = ((x * 17) ^ (y * 19)) % 20;
                m_textures[RAILROAD][y * TEX_SIZE + x] = makeColor(90 + gravel, 88 + gravel, 84 + gravel);
            } else if (y >= 61) {
                // Sombra do degrau inferior do talude da ferrovia (+10 de elevação)
                int gravel = ((x * 17) ^ (y * 19)) % 15;
                m_textures[RAILROAD][y * TEX_SIZE + x] = makeColor(30 + gravel, 28 + gravel, 26 + gravel);
            } else {
                // Brita de lastro ferroviário escuro
                int gravel = ((x * 17) ^ (y * 19)) % 25;
                m_textures[RAILROAD][y * TEX_SIZE + x] = makeColor(55 + gravel, 52 + gravel, 50 + gravel);
            }
        }
    }

    // =========================================================================
    // 18. HANGAR_BLUE (HANGARBL): Galpão industrial com chapa metálica azul e rebites
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        bool isBeam = (y >= 28 && y <= 34); // Viga de sustentação horizontal

        for (int x = 0; x < TEX_SIZE; ++x) {
            int waveX = x % 4;
            int ridge = (waveX == 2) ? 45 : ((waveX == 0) ? -35 : 0);

            if (y == 31 && (x % 16 == 8)) {
                m_textures[HANGAR_BLUE][y * TEX_SIZE + x] = makeColor(220, 230, 245); // Rebite
            } else if (isBeam) {
                m_textures[HANGAR_BLUE][y * TEX_SIZE + x] = (y == 28) ? makeColor(110, 155, 215) : makeColor(35, 60, 100);
            } else {
                int b = std::clamp(145 + ridge, 0, 255);
                int g = std::clamp(95 + ridge / 2, 0, 255);
                m_textures[HANGAR_BLUE][y * TEX_SIZE + x] = makeColor(30, g, b);
            }
        }
    }

    // =========================================================================
    // 19. POLICE_EMBLEM (POLICESH): Escudo com brasão da Polícia de Chernogorsk
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            // Escudo policial clássico centrado
            int dx = std::abs(x - 31);
            int dy = y - 10;
            bool isInsideShield = (y >= 10 && y <= 54 && dx <= 20 && (y < 36 || (y - 36) * 1.2f + dx <= 20));
            bool isShieldBorder = isInsideShield && (dx >= 18 || y <= 12 || ((y - 36) * 1.2f + dx >= 18));
            bool isStar = (std::abs(x - 31) + std::abs(y - 28) <= 6) ||
                          (std::abs(x - 31) <= 1 && y >= 20 && y <= 36) ||
                          (std::abs(y - 28) <= 1 && x >= 23 && x <= 39);

            if (isStar) {
                m_textures[POLICE_EMBLEM][y * TEX_SIZE + x] = makeColor(255, 225, 50); // Estrela dourada
            } else if (isShieldBorder) {
                m_textures[POLICE_EMBLEM][y * TEX_SIZE + x] = makeColor(220, 185, 40); // Moldura de ouro
            } else if (isInsideShield) {
                m_textures[POLICE_EMBLEM][y * TEX_SIZE + x] = makeColor(25, 60, 140);  // Azul marinho policial
            } else {
                // Pedra de granito cinzento com bisel
                int granite = ((x * 13) ^ (y * 17)) % 25;
                m_textures[POLICE_EMBLEM][y * TEX_SIZE + x] = makeColor(90 + granite, 92 + granite, 95 + granite);
            }
        }
    }

    // =========================================================================
    // 20. MURO_SOVIET (MUROPO2): Muro Soviético PO-2 com pirâmides em diamante 3D
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        bool isFrameTop = (y < 4);
        bool isFrameBot = (y > 59);

        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isPost = (x < 4 || x > 59);
            if (isFrameTop || isFrameBot || isPost) {
                // Moldura de concreto estrutural
                uint8_t f = (y == 1 || x == 1) ? 165 : 85;
                m_textures[MURO_SOVIET][y * TEX_SIZE + x] = makeColor(f, f + 2, f + 3);
            } else {
                // Grelha de 4x4 pirâmides em relevo de diamante ("Забор ПО-2")
                int cellX = (x - 4) % 14;
                int cellY = (y - 4) % 14;
                int cx = cellX - 7;
                int cy = cellY - 7;

                // 4 facetas da pirâmide com iluminação directional solar superior
                if (cy < 0 && std::abs(cx) <= std::abs(cy)) {
                    // Faceta superior voltada para o céu: LUZ MÁXIMA
                    m_textures[MURO_SOVIET][y * TEX_SIZE + x] = makeColor(185, 190, 195);
                } else if (cx < 0 && std::abs(cy) <= std::abs(cx)) {
                    // Faceta esquerda: MEIA-LUZ
                    m_textures[MURO_SOVIET][y * TEX_SIZE + x] = makeColor(140, 145, 150);
                } else if (cx > 0 && std::abs(cy) <= std::abs(cx)) {
                    // Faceta direita: SOMBRA SUAVE
                    m_textures[MURO_SOVIET][y * TEX_SIZE + x] = makeColor(105, 110, 115);
                } else {
                    // Faceta inferior: SOMBRA PROFUNDA
                    m_textures[MURO_SOVIET][y * TEX_SIZE + x] = makeColor(65, 70, 75);
                }
            }
        }
    }

    // =========================================================================
    // 21. MURO_STONE (MUROSTN): Muro de pedra rústica com musgo
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isTopCap = (y <= 4);
            int stonePattern = ((x / 12) * 7 + (y / 8) * 11) % 30;
            bool isMortar = (x % 12 == 0) || (y % 8 == 0);

            if (isTopCap) {
                m_textures[MURO_STONE][y * TEX_SIZE + x] = (y == 1) ? makeColor(170, 175, 170) : makeColor(100, 105, 100);
            } else if (isMortar) {
                m_textures[MURO_STONE][y * TEX_SIZE + x] = makeColor(45, 48, 42);
            } else {
                int stoneColor = 110 + stonePattern;
                if ((x + y) % 7 == 0) {
                    m_textures[MURO_STONE][y * TEX_SIZE + x] = makeColor(stoneColor - 25, stoneColor + 15, stoneColor - 20); // Musgo
                } else {
                    m_textures[MURO_STONE][y * TEX_SIZE + x] = makeColor(stoneColor, stoneColor - 5, stoneColor - 10);
                }
            }
        }
    }

    // =========================================================================
    // 22. DOOR_HOSPITAL (DOORHOSP): Porta dupla de hospital com vidros e barra
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isCenterSeam = (x == 31 || x == 32);
            bool isKickPlate = (y >= 48 && y <= 62);
            bool isPushBar = (y >= 35 && y <= 38 && x >= 4 && x <= 59 && !isCenterSeam);
            bool isWindow = ((x >= 8 && x <= 26) || (x >= 37 && x <= 55)) && (y >= 8 && y <= 30);

            if (isPushBar) {
                // Barra antipânico de aço inoxidável com reflexo especular
                m_textures[DOOR_HOSPITAL][y * TEX_SIZE + x] = (y == 36) ? makeColor(250, 252, 255) : makeColor(170, 175, 185);
            } else if (isKickPlate) {
                // Chapa de proteção de inox na base
                int brush = (x * 11) % 15;
                m_textures[DOOR_HOSPITAL][y * TEX_SIZE + x] = makeColor(160 + brush, 165 + brush, 175 + brush);
            } else if (isWindow) {
                // Vidro aramado com decalque de cruz vermelha
                bool isCross = ((x >= 15 && x <= 19 && y >= 14 && y <= 24) || (x >= 12 && x <= 22 && y >= 17 && y <= 21)) ||
                               ((x >= 44 && x <= 48 && y >= 14 && y <= 24) || (x >= 41 && x <= 51 && y >= 17 && y <= 21));
                if (isCross) {
                    m_textures[DOOR_HOSPITAL][y * TEX_SIZE + x] = makeColor(220, 30, 30);
                } else {
                    bool isWire = (x % 4 == 0 || y % 4 == 0);
                    m_textures[DOOR_HOSPITAL][y * TEX_SIZE + x] = isWire ? makeColor(170, 190, 210) : makeColor(50, 75, 95);
                }
            } else if (isCenterSeam) {
                m_textures[DOOR_HOSPITAL][y * TEX_SIZE + x] = makeColor(40, 42, 45);
            } else {
                // Painel de fórmica hospitalar branca
                m_textures[DOOR_HOSPITAL][y * TEX_SIZE + x] = makeColor(220, 225, 228);
            }
        }
    }

    // =========================================================================
    // 23. DOOR_FRAME (DOORFRM): Batente e marco lateral da porta em metal/madeira
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isJambLip = (x >= 28 && x <= 36); // Batente rebaixado de encosto
            bool isStrikePlate = (x >= 26 && x <= 38 && y >= 28 && y <= 36); // Chapa da trava

            if (isStrikePlate) {
                bool isHole = (x >= 30 && x <= 34 && y >= 31 && y <= 33);
                m_textures[DOOR_FRAME][y * TEX_SIZE + x] = isHole ? makeColor(15, 15, 18) : makeColor(200, 170, 50); // Latão
            } else if (isJambLip) {
                m_textures[DOOR_FRAME][y * TEX_SIZE + x] = makeColor(35, 38, 42); // Sombra interna do rebaixo
            } else {
                uint8_t c = (x < 16) ? 75 : 60;
                m_textures[DOOR_FRAME][y * TEX_SIZE + x] = makeColor(c, c + 2, c + 5);
            }
        }
    }

    // =========================================================================
    // 24. WINDOW_WOOD_FIXED (WINFIXED): Janela fixa com caixilho e vidro quadriculado
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isOuterFrame = (x < 4 || x > 59 || y < 4 || y > 59);
            // Divisórias de madeira (muntins / travessas) formando grade 3x3 de vidros
            bool isVertMullion = ((x >= 22 && x <= 24) || (x >= 39 && x <= 41));
            bool isHorizMullion = ((y >= 22 && y <= 24) || (y >= 39 && y <= 41));
            bool isWoodGrid = isOuterFrame || isVertMullion || isHorizMullion;

            if (isWoodGrid) {
                // Madeira de carvalho envelhecida com veio e bisel
                int grain = ((x * 5 + y * 13) % 20);
                int shadow = (x == 3 || y == 3 || x == 24 || y == 24 || x == 41 || y == 41) ? -22 : 0;
                int highlight = (x == 4 || y == 4 || x == 22 || y == 22 || x == 39 || y == 39) ? 18 : 0;

                int r = std::clamp(130 + grain + shadow + highlight, 40, 240);
                int g = std::clamp(80 + (grain * 3) / 4 + shadow + highlight, 25, 200);
                int b = std::clamp(45 + grain / 2 + shadow + highlight, 15, 160);
                m_textures[WINDOW_WOOD_FIXED][y * TEX_SIZE + x] = makeColor(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b));
            } else {
                // Vidro reflexivo com reflexo especular diagonal do céu e poeira
                bool isDiagGleam = ((x + y * 2) >= 38 && (x + y * 2) <= 45) || ((x + y * 2) >= 88 && (x + y * 2) <= 94);
                bool isCornerDust = (x % 17 <= 2 && y % 17 <= 2);

                if (isDiagGleam) {
                    m_textures[WINDOW_WOOD_FIXED][y * TEX_SIZE + x] = makeColor(140, 180, 215); // Brilho de reflexo
                } else if (isCornerDust) {
                    m_textures[WINDOW_WOOD_FIXED][y * TEX_SIZE + x] = makeColor(40, 48, 45);    // Poeira acumulada
                } else {
                    int glassTone = 25 + ((x * 3 + y * 7) % 12);
                    m_textures[WINDOW_WOOD_FIXED][y * TEX_SIZE + x] = makeColor(22, 35 + glassTone / 2, 55 + glassTone);
                }
            }
        }
    }

    // =========================================================================
    // 25. WINDOW_WOOD_CASEMENT (WINCASE): Folha móvel de abrir com vidro quadriculado e cremona
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isOuterFrame = (x < 4 || x > 59 || y < 4 || y > 59);
            // Travessas internas formando grade 2x3 de vidros por folha
            bool isVertMullion = (x >= 30 && x <= 33);
            bool isHorizMullion = ((y >= 22 && y <= 24) || (y >= 40 && y <= 42));
            bool isWoodGrid = isOuterFrame || isVertMullion || isHorizMullion;

            // Ferragens: Fecho cremona / trinco de latão no bordo central (x >= 55)
            bool isCremonePlate = (x >= 56 && x <= 59 && y >= 25 && y <= 39);
            bool isCremoneHandle = (x >= 52 && x <= 58 && y >= 31 && y <= 33);
            bool isCremoneRod = (x == 57 && (y < 25 || y > 39));

            // Dobradiças de ferro forjado no bordo da dobradiça (x <= 6)
            bool isHingeTop = (x <= 5 && y >= 8 && y <= 14);
            bool isHingeBot = (x <= 5 && y >= 50 && y <= 56);

            if (isCremoneHandle || isCremonePlate) {
                // Latão envelhecido dourado
                m_textures[WINDOW_WOOD_CASEMENT][y * TEX_SIZE + x] = (y == 32) ? makeColor(235, 205, 75) : makeColor(170, 140, 45);
            } else if (isCremoneRod) {
                // Haste vertical de ferro escuro da cremona
                m_textures[WINDOW_WOOD_CASEMENT][y * TEX_SIZE + x] = makeColor(45, 45, 50);
            } else if (isHingeTop || isHingeBot) {
                // Dobradiça de ferro preto
                m_textures[WINDOW_WOOD_CASEMENT][y * TEX_SIZE + x] = makeColor(30, 32, 35);
            } else if (isWoodGrid) {
                // Caixilho de madeira nobre
                int grain = ((x * 7 + y * 11) % 18);
                int r = std::clamp(135 + grain, 50, 230);
                int g = std::clamp(85 + (grain * 3) / 4, 30, 190);
                int b = std::clamp(50 + grain / 2, 20, 150);
                m_textures[WINDOW_WOOD_CASEMENT][y * TEX_SIZE + x] = makeColor(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b));
            } else {
                // Vidro quadriculado com reflexo
                bool isDiagGleam = ((x + y * 2) >= 42 && (x + y * 2) <= 49);
                if (isDiagGleam) {
                    m_textures[WINDOW_WOOD_CASEMENT][y * TEX_SIZE + x] = makeColor(155, 195, 225);
                } else {
                    int glassTone = 22 + ((x * 2 + y * 5) % 15);
                    m_textures[WINDOW_WOOD_CASEMENT][y * TEX_SIZE + x] = makeColor(20, 32 + glassTone / 2, 50 + glassTone);
                }
            }
        }
    }

    // =========================================================================
    // 26. WINDOW_FRAME_WOOD (WINFRM): Marco lateral e peitoril de madeira
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            bool isSillBevel = (y >= 54); // Peitoril chanfrado inferior
            int grain = ((x * 9 + y * 3) % 22);

            if (isSillBevel) {
                // Peitoril de madeira mais escura com sombra projetada
                m_textures[WINDOW_FRAME_WOOD][y * TEX_SIZE + x] = makeColor(75 + grain / 2, 45 + grain / 3, 25 + grain / 4);
            } else {
                // Batente e marco de madeira maciça
                int bevel = (x < 8 || x > 56) ? -18 : (x == 8 ? 16 : 0);
                int r = std::clamp(115 + grain + bevel, 35, 220);
                int g = std::clamp(72 + grain / 2 + bevel, 25, 180);
                int b = std::clamp(42 + grain / 3 + bevel, 15, 140);
                m_textures[WINDOW_FRAME_WOOD][y * TEX_SIZE + x] = makeColor(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b));
            }
        }
    }

    // =========================================================================
    // 27. STAIRS_CONCRETE (STAIRCON): Escada de concreto e metal industrial
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        int stepY = y % 12; // 5 degraus largos com perfil profundo

        for (int x = 0; x < TEX_SIZE; ++x) {
            // Longarinas metálicas / vigas de aço laterais nos bordos (x <= 4 ou x >= 59)
            bool isStringer = (x <= 4 || x >= 59);
            bool isRivet = ((x == 2 || x == 61) && (y % 16 == 8));

            if (isStringer) {
                if (isRivet) {
                    m_textures[STAIRS_CONCRETE][y * TEX_SIZE + x] = makeColor(215, 215, 225); // Rebite metálico brilhante
                } else {
                    int stGrain = ((x * 5 + y * 7) % 15);
                    m_textures[STAIRS_CONCRETE][y * TEX_SIZE + x] = makeColor(52 + stGrain, 56 + stGrain, 62 + stGrain); // Viga de ferro escura
                }
                continue;
            }

            // Perfil e degraus de concreto antiderrapante com cantoneira metálica:
            if (stepY == 0) {
                // Quina de aço iluminada / reflexo metálico do topo do degrau
                bool isGripNotch = (x % 4 == 0);
                uint8_t metalTone = isGripNotch ? 120 : 210;
                m_textures[STAIRS_CONCRETE][y * TEX_SIZE + x] = makeColor(metalTone, metalTone + 5, metalTone + 15);
            } else if (stepY == 1) {
                // Faixa amarela de segurança industrial soviética desgastada
                int wear = ((x * 13 ^ y * 7) % 35);
                uint8_t r = std::clamp(210 + wear, 0, 255);
                uint8_t g = std::clamp(175 + wear / 2, 0, 255);
                uint8_t b = std::clamp(40 + wear / 3, 0, 255);
                m_textures[STAIRS_CONCRETE][y * TEX_SIZE + x] = makeColor(r, g, b);
            } else if (stepY == 11) {
                // Sombra projetada profunda abaixo do degrau seguinte
                int shadow = (x * 3) % 12;
                m_textures[STAIRS_CONCRETE][y * TEX_SIZE + x] = makeColor(28 + shadow, 30 + shadow, 35 + shadow);
            } else if (stepY == 10) {
                // Penumbra
                int shadow = (x * 3) % 14;
                m_textures[STAIRS_CONCRETE][y * TEX_SIZE + x] = makeColor(48 + shadow, 50 + shadow, 55 + shadow);
            } else {
                // Piso de concreto antiderrapante com agregados minerais e ranhuras
                int grain = ((x * 11) ^ (y * 17)) % 25;
                bool isTreadGroove = (stepY == 4 || stepY == 7);
                int grooveShade = isTreadGroove ? -25 : 0;
                int c = std::clamp(120 + grain + grooveShade, 40, 210);
                m_textures[STAIRS_CONCRETE][y * TEX_SIZE + x] = makeColor(c, c + 2, c + 4);
            }
        }
    }

    // =========================================================================
    // 28. RUIN_BRICK_MOSS (RUINBRK): Tijolos de ruína com argamassa e musgo
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        int rowIndex = y / 8;
        int rowY = y % 8;
        bool isHorizMortar = (rowY == 0 || rowY == 7);

        for (int x = 0; x < TEX_SIZE; ++x) {
            int shiftX = (rowIndex % 2 == 1) ? 8 : 0;
            int brickX = (x + shiftX) % 16;
            bool isVertMortar = (brickX == 0 || brickX == 15);

            int noise = ((x * 19) ^ (y * 23)) % 30;
            int plasterNoise = ((x * 7 + y * 13 + rowIndex * 17) % 100);

            if (isHorizMortar || isVertMortar) {
                // Argamassa desgastada e musgo nas frestas
                if (plasterNoise < 35) {
                    m_textures[RUIN_BRICK_MOSS][y * TEX_SIZE + x] = makeColor(32 + noise / 2, 48 + noise, 24 + noise / 2); // Musgo
                } else {
                    m_textures[RUIN_BRICK_MOSS][y * TEX_SIZE + x] = makeColor(28 + noise / 2, 25 + noise / 2, 22 + noise / 2); // Argamassa
                }
            } else {
                if (plasterNoise > 78) {
                    // Reboco antigo descascando (cor concreto/areia)
                    m_textures[RUIN_BRICK_MOSS][y * TEX_SIZE + x] = makeColor(145 + noise / 2, 138 + noise / 2, 125 + noise / 3);
                } else {
                    // Corpo de tijolo vermelho queimado
                    int brickID = ((x + shiftX) / 16) + rowIndex * 4;
                    int baseR = 120 + ((brickID * 29) % 35) - 15;
                    int baseG = 45 + ((brickID * 17) % 20) - 10;
                    int baseB = 32 + ((brickID * 19) % 15) - 8;

                    // Sombreamento de relevo
                    bool isBevelTop = (rowY == 1 || brickX == 1);
                    bool isBevelBot = (rowY == 6 || brickX == 14);

                    int r = baseR + noise;
                    int g = baseG + noise / 3;
                    int b = baseB + noise / 4;

                    if (isBevelTop) { r += 35; g += 20; b += 15; }
                    if (isBevelBot) { r -= 40; g -= 20; b -= 15; }

                    // Manchas de limo verde nos tijolos inferiores
                    if (y > 42 && (noise % 5 == 0)) {
                        m_textures[RUIN_BRICK_MOSS][y * TEX_SIZE + x] = makeColor(r / 2, g + 25, b / 2);
                    } else {
                        m_textures[RUIN_BRICK_MOSS][y * TEX_SIZE + x] = makeColor(
                            std::clamp(r, 10, 255),
                            std::clamp(g, 10, 255),
                            std::clamp(b, 10, 255)
                        );
                    }
                }
            }
        }
    }

    // =========================================================================
    // 29. FOREST_EARTH (FOREART): Solo orgânico escuro com agulhas e musgo
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            int n1 = ((x * 13) ^ (y * 29)) % 40;
            int n2 = ((x * 37) + (y * 17)) % 100;

            int r = 24 + n1 / 4;
            int g = 32 + n1 / 3;
            int b = 18 + n1 / 5;

            // Agulhas de pinheiro caídas (marrom alaranjado fino)
            if ((x + y * 2) % 19 == 0 || (x * 2 - y) % 23 == 0) {
                r = 65 + n1 / 2;
                g = 42 + n1 / 3;
                b = 22 + n1 / 4;
            }
            // Manchas aveludadas de musgo denso da floresta
            else if (n2 < 38) {
                r = 28 + n1 / 4;
                g = 55 + n1 / 2;
                b = 22 + n1 / 4;
            }
            // Terra batida escura
            else if (n2 > 85) {
                r = 38 + n1 / 3;
                g = 32 + n1 / 4;
                b = 25 + n1 / 5;
            }

            m_textures[FOREST_EARTH][y * TEX_SIZE + x] = makeColor(
                std::clamp(r, 8, 255),
                std::clamp(g, 12, 255),
                std::clamp(b, 6, 255)
            );
        }
    }

    // =========================================================================
    // 30. TACTICAL_PIPES (RUSTPIPE): Tubulação industrial com juntas e válvulas
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            int grain = ((x * 17) ^ (y * 19)) % 25;
            // Parede de fundo da ruína
            int r = 45 + grain;
            int g = 42 + grain;
            int b = 40 + grain;

            // Tubo industrial horizontal correndo no centro (y de 20 a 44)
            if (y >= 20 && y <= 44) {
                int pipeY = y - 20;
                // Gradiente cilíndrico de metal com ferrugem
                float t = static_cast<float>(pipeY) / 24.0f;
                float highlight = std::sin(t * 3.14159f);

                int metal = static_cast<int>(60.0f + highlight * 110.0f);
                int rust = ((x * 7 + y * 11) % 40) - 20;

                r = std::clamp(metal + 40 + rust, 30, 240);
                g = std::clamp(metal + 15 + rust / 2, 20, 200);
                b = std::clamp(metal - 10 + rust / 3, 15, 160);

                // Flanges metálicas e anéis de retenção em x == 12..15 e x == 48..51
                bool isFlange = (x >= 12 && x <= 15) || (x >= 48 && x <= 51);
                if (isFlange) {
                    r = std::clamp(r + 30, 0, 255);
                    g = std::clamp(g + 20, 0, 255);
                    b = std::clamp(b + 20, 0, 255);
                }

                // Válvula vermelha central em x == 28..35
                if (x >= 28 && x <= 35 && y >= 24 && y <= 40) {
                    int distFromCenter = std::abs(x - 31) + std::abs(y - 32);
                    if (distFromCenter <= 6) {
                        r = 175 + grain;
                        g = 35 + grain / 3;
                        b = 30 + grain / 4;
                    }
                }
            }

            m_textures[TACTICAL_PIPES][y * TEX_SIZE + x] = makeColor(r, g, b);
        }
    }

    // =========================================================================
    // 31. WOOD_CRATE (MILCRATE): Caixa de suprimentos militar de madeira e aço
    // =========================================================================
    for (int y = 0; y < TEX_SIZE; ++y) {
        for (int x = 0; x < TEX_SIZE; ++x) {
            int grain = ((x * 11) ^ (y * 13)) % 20;
            // Prancha de madeira base
            int r = 100 + grain;
            int g = 72 + grain / 2;
            int b = 42 + grain / 3;

            // Frestas entre tábuas
            if (y % 16 == 0 || y % 16 == 1) {
                r = 35; g = 25; b = 15;
            }

            // Reforços de cantoneira metálica nas bordas
            bool isBorder = (x < 6 || x >= 58 || y < 6 || y >= 58);
            // Cinta de reforço diagonal
            bool isDiag1 = std::abs(x - y) <= 3;
            bool isDiag2 = std::abs(x - (63 - y)) <= 3;

            if (isBorder || isDiag1 || isDiag2) {
                // Aço escurecido militar
                int metalGrain = (x * 7 + y * 5) % 15;
                r = 45 + metalGrain;
                g = 48 + metalGrain;
                b = 55 + metalGrain;

                // Rebites prateados nos cantos
                bool isRivet = ((x == 3 || x == 60) && (y == 3 || y == 60 || y == 31)) ||
                               ((y == 3 || y == 60) && (x == 31));
                if (isRivet) {
                    r = 200; g = 205; b = 215;
                }
            }

            m_textures[WOOD_CRATE][y * TEX_SIZE + x] = makeColor(r, g, b);
        }
    }

    // =========================================================================
    // 32. SOBRESCRITA AUTOMÁTICA COM O ATLAS OFICIAL (tileset_walls_floors.png)
    // =========================================================================
    SpriteAtlas tilesetAtlas;
    if (tilesetAtlas.loadFromFile("assets/sprites/tileset_walls_floors.png", false)) {
        auto copyTileToTex = [&](int texID, const SpriteRect& rect) {
            for (int y = 0; y < TEX_SIZE; ++y) {
                float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(TEX_SIZE);
                for (int x = 0; x < TEX_SIZE; ++x) {
                    float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(TEX_SIZE);
                    m_textures[texID][y * TEX_SIZE + x] = tilesetAtlas.sampleTileUV(rect, u, v) | 0xFF000000u;
                }
            }
        };

        // Chão: Lajes de concreto rachado, terra batida com musgo e grelhas metálicas de bunker
        copyTileToTex(CONC_FLOOR,      AtlasCoords::tileAt(0, 0)); // Concreto rachado esverdeado
        copyTileToTex(ASPHALT,         AtlasCoords::tileAt(1, 0)); // Laje de concreto com trincas profundas
        copyTileToTex(FOREST_EARTH,    AtlasCoords::tileAt(4, 0)); // Terra batida escura com musgo invadindo
        copyTileToTex(WOOD_FLOOR,      AtlasCoords::tileAt(6, 0)); // Grelha metálica enferrujada de bunker
        copyTileToTex(SAND,            AtlasCoords::tileAt(0, 2)); // Laje de concreto desgastada bege

        // Paredes: Blocos estruturais de concreto soviético, tijolos vermelhos quebrados e tubulações
        copyTileToTex(CONCRETE,        AtlasCoords::tileAt(1, 3)); // Bloco estrutural de concreto soviético
        copyTileToTex(MURO_SOVIET,     AtlasCoords::tileAt(2, 3)); // Concreto soviético desgastado
        copyTileToTex(RUIN_BRICK_MOSS, AtlasCoords::tileAt(4, 4)); // Parede de tijolos vermelhos com reboco
        copyTileToTex(BRICK_DARK,      AtlasCoords::tileAt(5, 4)); // Tijolos vermelhos de ruína
        copyTileToTex(TACTICAL_PIPES,  AtlasCoords::tileAt(1, 6)); // Segmento de parede com tubulações acopladas
    }

    SpriteAtlas objectsAtlas;
    if (objectsAtlas.loadFromFile("assets/sprites/sprites_map_objects.png", true)) {
        for (int y = 0; y < TEX_SIZE; ++y) {
            float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(TEX_SIZE);
            for (int x = 0; x < TEX_SIZE; ++x) {
                float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(TEX_SIZE);
                uint32_t p = objectsAtlas.sampleTileUV(AtlasCoords::OBJ_CRATE_CLOSED, u, v);
                if (((p >> 24) & 0xFF) > 128) {
                    m_textures[WOOD_CRATE][y * TEX_SIZE + x] = p | 0xFF000000u;
                }
            }
        }
    }
}

} // namespace DayDoom::Engine
