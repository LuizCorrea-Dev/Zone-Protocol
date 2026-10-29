#define SDL_MAIN_HANDLED
#include "hal/Platform.hpp"
#include "hal/Input.hpp"
#include "engine/Texture.hpp"
#include "engine/SpriteAtlas.hpp"
#include "engine/LightCaster2D.hpp"
#include "engine/VFS.hpp"
#include "engine/Logger.hpp"
#include "game/ZoneMap.hpp"
#include "game/Player.hpp"
#include "game/CombatSystem.hpp"
#include "game/UI/HUD.hpp"
#include "game/UI/WorldView2D.hpp"
#include "game/UI/GridInventory.hpp"
#include <iostream>
#include <string>
#include <exception>
#include <algorithm>
#include <cmath>

#if defined(__ANDROID__)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

enum class GameLoopState {
    LOADING_SECTOR,
    IN_ZONE,
    DEATH_SCREEN,
    BUNKER_HUB
};

int main(int argc, char* argv[]) {
    SDL_SetMainReady();

    ZoneProtocol::Engine::Logger::init(argc, argv);

    try {
        DayDoom::Engine::VFS::get().init(".");

        DayDoom::HAL::DisplayProfile profile = DayDoom::HAL::DisplayProfile::WIDESCREEN_16_10;
        int initialWinW = 1280;
        int initialWinH = 800;

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--1610" || arg == "--800p") {
                profile = DayDoom::HAL::DisplayProfile::WIDESCREEN_16_10;
                initialWinW = 1280;
                initialWinH = 800;
            } else if (arg == "--hd" || arg == "--720p") {
                profile = DayDoom::HAL::DisplayProfile::WIDESCREEN_16_9;
                initialWinW = 1280;
                initialWinH = 720;
            } else if (arg == "--169" || arg == "--480p") {
                profile = DayDoom::HAL::DisplayProfile::WIDESCREEN_16_9;
                initialWinW = 960;
                initialWinH = 540;
            }
        }

        // 1. Camada 0: HAL (Platform e Input)
        DayDoom::HAL::Platform platform;
        if (!platform.init("ZONE PROTOCOL [v0.1.2] - Roguelike Tatico Top-Down 2D", profile, initialWinW, initialWinH, false)) {
            ZoneProtocol::Engine::Logger::critical("Falha critica ao inicializar a plataforma SDL2.", "PLATFORM");
            return 1;
        }

        DayDoom::HAL::Input input;

        // 2. Camada 1: Engine (Texturas, Iluminação DDA 2D e Folhas de SpriteAtlas)
        DayDoom::Engine::TextureManager textures;
        DayDoom::Engine::LightCaster2D lightCaster(platform.getScreenWidth(), platform.getScreenHeight());

        ZoneProtocol::Engine::SpriteAtlas entityAtlas;
        entityAtlas.loadFromFile("assets/sprites/sprites_entities.png", true);

        ZoneProtocol::Engine::SpriteAtlas objectsAtlas;
        objectsAtlas.loadFromFile("assets/sprites/sprites_map_objects.png", true);

        ZoneProtocol::Engine::SpriteAtlas uiBriefcaseAtlas;
        uiBriefcaseAtlas.loadFromFile("assets/sprites/ui_briefcase_items.png", true);

        ZoneProtocol::Engine::SpriteAtlas bgLoadingAtlas;
        bgLoadingAtlas.loadFromFile("assets/sprites/Loading Screen.png", false);

        ZoneProtocol::Engine::SpriteAtlas uiLoadingBarAtlas;
        uiLoadingBarAtlas.loadFromFile("assets/sprites/ui_loading_bar.png", true);

        ZoneProtocol::Engine::SpriteAtlas bgDeathAtlas;
        bgDeathAtlas.loadFromFile("assets/sprites/bg_death_clean.png", false);

        ZoneProtocol::Engine::SpriteAtlas uiDeathPanelsAtlas;
        uiDeathPanelsAtlas.loadFromFile("assets/sprites/ui_death_panels.png", true);

        ZoneProtocol::Engine::SpriteAtlas bgBunkerAtlas;
        bgBunkerAtlas.loadFromFile("assets/sprites/screen_bunker_hub.png", false);

        // 3. Camada 2: Game e UI (ZoneMap, Player, CombatSystem, WorldView2D, HUD, GridInventory)
        DayDoom::Game::ZoneMap zoneMap;
        DayDoom::Game::Player player(zoneMap.getSpawnPosition().x, zoneMap.getSpawnPosition().y);
        player.getCamera().setAngle(zoneMap.getSpawnAngle());

        DayDoom::Game::UI::WorldView2D worldView;
        DayDoom::Game::UI::HUD hud(platform.getScreenWidth(), platform.getScreenHeight());
        ZoneProtocol::Game::GridInventory briefcase;
        ZoneProtocol::Game::CombatSystem combat;
        combat.spawnMutantsForZone(zoneMap, 1337, 25);

        DayDoom::Game::UI::BunkerState bunker;
        GameLoopState gameState = GameLoopState::LOADING_SECTOR;
        float loadingTimer = 0.0f;
        float loadingDuration = 1.85f;
        int currentTipIndex = 0;
        uint32_t currentSeed = 1337;
        std::string currentSectorTitle = "GENERATING SECTOR 04: KRASTOV PERIPHERY...";

        float totalTime = 0.0f;
        float runTimeSurvived = 0.0f;
        float remainingExtractionTime = 300.0f;
        std::string lootBanner = "";
        float lootBannerTimer = 0.0f;

        auto startNewDeployment = [&](int sectorIdx) {
            ZP_LOG_SCRIPT("LoadingScreen::ShowEx - Start");
            ZP_LOG_SCRIPT("LoadingScreen::SetBackgroundImage - Background: assets/sprites/Loading Screen.png");
            currentSeed += 733u;
            DayDoom::Game::ZoneConfig cfg{ (sectorIdx % 3) + 1, 0.55f, false };
            zoneMap.generateProceduralZone(currentSeed, cfg);
            player.healFull();
            player.getCamera().pos = zoneMap.getSpawnPosition();
            player.getCamera().setAngle(zoneMap.getSpawnAngle());
            combat.spawnMutantsForZone(zoneMap, currentSeed, 25 + (sectorIdx % 3) * 6);
            remainingExtractionTime = 300.0f;
            runTimeSurvived = 0.0f;
            loadingTimer = 0.0f;
            currentTipIndex++;
            if (sectorIdx % 3 == 0)      currentSectorTitle = "GENERATING SECTOR 04: KRASTOV PERIPHERY...";
            else if (sectorIdx % 3 == 1) currentSectorTitle = "GENERATING SECTOR 02: NOVO-BRYANSK COMPLEX...";
            else                         currentSectorTitle = "GENERATING SECTOR 07: ZARIA GROUND ZERO...";
            gameState = GameLoopState::LOADING_SECTOR;
            ZP_LOG_SCRIPT("[MissionGameplay] Deployment initialized -> " + currentSectorTitle + " (Seed=" + std::to_string(currentSeed) + ")");
            ZP_LOG_SCRIPT("LoadingScreen::ShowEx - End");
        };

        ZP_LOG_SCRIPT("LoadingScreen::ShowEx - Start");
        ZP_LOG_SCRIPT("LoadingScreen::SetBackgroundImage - Background: assets/sprites/Loading Screen.png");
        ZP_LOG_SCRIPT("LoadingScreen::ShowEx - End");
        ZP_LOG_SCRIPT("[MissionGameplay] OnInit - Client Ready");

        ZoneProtocol::Engine::Logger::info("Controles: MOUSE Mira & Tiro | WASD Movimento | [F] Saquear/Porta | [TAB/I] Maleta 6x4 | [R] Girar Item | [B] Bunker Hub | [K] Testar Tela de Morte", "GAME");

        // 4. Loop Principal de Jogo a 60 FPS
        while (platform.isRunning()) {
            platform.beginFrame();
            float deltaTime = platform.getDeltaTime();
            totalTime += deltaTime;

            if (lootBannerTimer > 0.0f) {
                lootBannerTimer -= deltaTime;
                if (lootBannerTimer <= 0.0f) lootBanner.clear();
            }

            input.beginFrame();

            int rawMx = 0, rawMy = 0;
            SDL_GetMouseState(&rawMx, &rawMy);
            int logMx = 0, logMy = 0;
            platform.windowToLogical(rawMx, rawMy, logMx, logMy);

            int targetLogicalW = platform.getScreenWidth();
            int targetLogicalH = platform.getScreenHeight();

            bool clickedLeft = false;
            bool clickedRight = false;
            bool pressedEnter = false;
            int pressedStationNum = 0;

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT ||
                    (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)) {
                    platform.shutdown();
                    ZoneProtocol::Engine::Logger::shutdown();
                    return 0;
                }
                if (event.type == SDL_KEYDOWN) {
                    SDL_Keycode sym = event.key.keysym.sym;
                    if (sym == SDLK_ESCAPE) {
                        if (briefcase.isOpen()) {
                            briefcase.setOpen(false);
                        } else if (gameState == GameLoopState::BUNKER_HUB) {
                            gameState = GameLoopState::IN_ZONE;
                        } else {
                            platform.shutdown();
                            ZoneProtocol::Engine::Logger::shutdown();
                            return 0;
                        }
                    } else if (sym == SDLK_F11) {
                        platform.toggleFullscreen();
                    } else if (sym == SDLK_TAB || sym == SDLK_i) {
                        if (gameState == GameLoopState::IN_ZONE || gameState == GameLoopState::BUNKER_HUB) {
                            briefcase.toggle();
                        }
                    } else if (sym == SDLK_r) {
                        if (briefcase.isOpen()) {
                            briefcase.rotateSelected();
                        }
                    } else if (sym == SDLK_b) {
                        briefcase.setOpen(false);
                        if (gameState == GameLoopState::IN_ZONE) {
                            bunker.credits += combat.getDataPointsEarned();
                            bunker.statusBanner = "OPERADOR RETORNOU AO BUNKER COM SUCESSO!";
                            gameState = GameLoopState::BUNKER_HUB;
                        } else if (gameState == GameLoopState::BUNKER_HUB) {
                            gameState = GameLoopState::IN_ZONE;
                        }
                    } else if (sym == SDLK_e) {
                        if (gameState == GameLoopState::IN_ZONE &&
                            zoneMap.isNearExtraction(player.getCamera().pos.x, player.getCamera().pos.y)) {
                            bunker.credits += 300 + combat.getDataPointsEarned();
                            bunker.scrap += 25;
                            bunker.statusBanner = "EXTRACAO COMPLETA! +300 CREDITS & +25 SCRAP";
                            gameState = GameLoopState::BUNKER_HUB;
                        }
                    } else if (sym == SDLK_k) {
                        if (gameState == GameLoopState::IN_ZONE) {
                            player.takeDamage(999.0f);
                        }
                    } else if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
                        pressedEnter = true;
                    } else if (sym >= SDLK_1 && sym <= SDLK_5) {
                        pressedStationNum = (sym - SDLK_1) + 1;
                    }
                }

                int eventLogX = logMx, eventLogY = logMy;
                if (event.type == SDL_MOUSEMOTION) {
                    eventLogX = std::clamp(event.motion.x, 0, targetLogicalW - 1);
                    eventLogY = std::clamp(event.motion.y, 0, targetLogicalH - 1);
                    logMx = eventLogX;
                    logMy = eventLogY;
                } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                    eventLogX = std::clamp(event.button.x, 0, targetLogicalW - 1);
                    eventLogY = std::clamp(event.button.y, 0, targetLogicalH - 1);
                    logMx = eventLogX;
                    logMy = eventLogY;
                    if (event.button.button == SDL_BUTTON_LEFT)  clickedLeft = true;
                    if (event.button.button == SDL_BUTTON_RIGHT) clickedRight = true;
                } else if (event.type == SDL_MOUSEBUTTONUP) {
                    eventLogX = std::clamp(event.button.x, 0, targetLogicalW - 1);
                    eventLogY = std::clamp(event.button.y, 0, targetLogicalH - 1);
                    logMx = eventLogX;
                    logMy = eventLogY;
                }

                input.handleEvent(event, eventLogX, eventLogY);
            }

            input.update(logMx, logMy);

            uint32_t* screenBuffer = platform.getScreenBuffer();
            int screenW = platform.getScreenWidth();
            int screenH = platform.getScreenHeight();

            // Atualiza telemetria em tempo real para captura imediata em qualquer erro/crash
            {
                ZoneProtocol::Engine::GameStateDiagnosticSnapshot snap;
                snap.gameLoopState = (gameState == GameLoopState::LOADING_SECTOR) ? "LOADING_SECTOR" :
                                     (gameState == GameLoopState::IN_ZONE)        ? "IN_ZONE" :
                                     (gameState == GameLoopState::DEATH_SCREEN)   ? "DEATH_SCREEN" : "BUNKER_HUB";
                snap.sectorTitle       = currentSectorTitle;
                snap.sectorSeed        = currentSeed;
                snap.playerX           = player.getCamera().pos.x;
                snap.playerY           = player.getCamera().pos.y;
                snap.playerDirX        = player.getCamera().dir.x;
                snap.playerDirY        = player.getCamera().dir.y;
                snap.playerHP          = player.getHealth();
                snap.playerStamina     = player.getStamina();
                snap.mutantsAlive      = static_cast<int>(combat.getMutants().size());
                snap.mutantsKilled     = combat.getKilledCount();
                snap.activeProjectiles = combat.getActiveProjectilesCount();
                snap.briefcaseOpen     = briefcase.isOpen();
                snap.briefcaseItems    = static_cast<int>(briefcase.getItems().size());
                snap.synergyLinks      = static_cast<int>(briefcase.getSynergyLinks().size());
                snap.mouseLogicalX     = logMx;
                snap.mouseLogicalY     = logMy;
                snap.screenW           = screenW;
                snap.screenH           = screenH;
                snap.fps               = platform.getFPS();
                snap.uptimeSeconds     = totalTime;

                float bestDist = 99999.0f;
                for (const auto& m : combat.getMutants()) {
                    float d = std::hypot(m.pos.x - snap.playerX, m.pos.y - snap.playerY);
                    if (d < bestDist) {
                        bestDist = d;
                        snap.nearestMutantId   = m.id;
                        snap.nearestMutantType = (m.type == ZoneProtocol::Game::MutantType::BRUTE_FLESH) ? "BRUTE_FLESH" : "CRAWLER_PALE";
                        snap.nearestMutantAI   = (m.state == ZoneProtocol::Game::AIState::IDLE) ? "IDLE" :
                                                 (m.state == ZoneProtocol::Game::AIState::ALERT_SOUND ? "ALERT_SOUND" : "CHASE_VISIBLE");
                        snap.nearestMutantDist = d;
                        snap.nearestMutantX    = m.pos.x;
                        snap.nearestMutantY    = m.pos.y;
                    }
                }
                ZoneProtocol::Engine::Logger::updateGameSnapshot(snap);
            }

            // =====================================================================
            // ESTADO 1: TELA DE CARREGAMENTO DINÂMICA (Loading Screen.png + Barra)
            // =====================================================================
            if (gameState == GameLoopState::LOADING_SECTOR) {
                loadingTimer += deltaTime;
                float progress = std::clamp(loadingTimer / loadingDuration, 0.0f, 1.0f);

                hud.renderLoadingScreen(
                    screenBuffer,
                    bgLoadingAtlas,
                    uiLoadingBarAtlas,
                    progress,
                    currentTipIndex,
                    currentSectorTitle
                );

                if (loadingTimer >= loadingDuration || pressedEnter || clickedLeft) {
                    ZP_LOG_SCRIPT("LoadingScreen::Hide - Start");
                    ZP_LOG_SCRIPT("LoadingScreen::EndLoading - Sector deployment active (IN_ZONE)");
                    ZP_LOG_SCRIPT("LoadingScreen::Hide - End");
                    gameState = GameLoopState::IN_ZONE;
                }

                platform.endFrame();
                continue;
            }

            // =====================================================================
            // ESTADO 2: ECRÃ DE MORTE (bg_death_clean.png + ui_death_panels.png)
            // =====================================================================
            if (gameState == GameLoopState::DEATH_SCREEN) {
                bool btnHovered = hud.renderDeathScreen(
                    screenBuffer,
                    bgDeathAtlas,
                    uiDeathPanelsAtlas,
                    &uiBriefcaseAtlas,
                    runTimeSurvived,
                    combat.getKilledCount(),
                    combat.getDataPointsEarned(),
                    briefcase,
                    logMx,
                    logMy,
                    totalTime
                );

                if ((btnHovered && clickedLeft) || pressedEnter) {
                    bunker.credits += combat.getDataPointsEarned();
                    bunker.statusBanner = "OPERADOR EVACUADO DE EMERGENCIA. ITENS DO SAFE POCKET SALVOS!";
                    player.healFull();
                    gameState = GameLoopState::BUNKER_HUB;
                }

                platform.endFrame();
                continue;
            }

            // =====================================================================
            // ESTADO 3: ECRÃ DO BUNKER E MERCADO NEGRO (screen_bunker_hub.png)
            // =====================================================================
            if (gameState == GameLoopState::BUNKER_HUB) {
                int hoveredStation = hud.renderBunkerHub(
                    screenBuffer,
                    bgBunkerAtlas,
                    bunker,
                    player,
                    logMx,
                    logMy,
                    totalTime
                );

                if (briefcase.isOpen()) {
                    if (clickedLeft || clickedRight) {
                        briefcase.handleMouseClick(logMx, logMy, screenW, screenH, clickedRight);
                    }
                    briefcase.render(screenBuffer, screenW, screenH, logMx, logMy, &uiBriefcaseAtlas);
                } else {
                    int triggeredStation = (clickedLeft && hoveredStation > 0) ? hoveredStation : pressedStationNum;
                    if (pressedEnter) triggeredStation = 4;

                    if (triggeredStation == 1) {
                        // ENFERMARIA
                        if (bunker.credits >= 25) {
                            bunker.credits -= 25;
                            player.healFull();
                            bunker.statusBanner = "ENFERMARIA: SAUDE RESTAURADA A 100% E RADIACAO REMOVIDA!";
                        } else {
                            bunker.statusBanner = "CREDITOS INSUFICIENTES PARA DESCONTAMINACAO!";
                        }
                    } else if (triggeredStation == 2) {
                        // GERADOR
                        if (bunker.scrap >= 15) {
                            bunker.scrap -= 15;
                            bunker.powerPct = 100;
                            ZoneProtocol::Game::ItemDef ammo = { 30, "5.45MM X30", ZoneProtocol::Game::ItemCategory::AMMO, 1, 1, ZoneProtocol::Game::SynergyElement::NONE, 30.0f, 0.0f, 60 };
                            briefcase.addItemAuto(ammo, 30);
                            bunker.statusBanner = "GERADOR: ENERGIA 100% & +30 MUNICOES FABRICADAS NA MALETA!";
                        } else {
                            bunker.statusBanner = "SUCATA (SCRAP) INSUFICIENTE!";
                        }
                    } else if (triggeredStation == 3) {
                        // BANCADA DE ARMAS & MALETA
                        briefcase.setOpen(true);
                        bunker.statusBanner = "BANCADA: ORGANIZE OS ARTEFATOS E ARMAS NA MALETA 6X4!";
                    } else if (triggeredStation == 4) {
                        // TERMINAL DE INFILTRAÇÃO
                        bunker.selectedSectorIndex = (bunker.selectedSectorIndex + 1) % 3;
                        startNewDeployment(bunker.selectedSectorIndex);
                    } else if (triggeredStation == 5) {
                        // MERCADO NEGRO
                        bool soldAny = false;
                        for (const auto& item : briefcase.getItems()) {
                            if (item.def.category == ZoneProtocol::Game::ItemCategory::ARTIFACT && !item.inSafePocket) {
                                briefcase.removeItemByInstance(item.instanceId);
                                bunker.credits += 250;
                                bunker.scrap += 40;
                                bunker.statusBanner = "MERCADO NEGRO: ARTEFATO VENDIDO POR +250 CREDITS E +40 SCRAP!";
                                soldAny = true;
                                break;
                            }
                        }
                        if (!soldAny && bunker.credits >= 200) {
                            bunker.credits -= 200;
                            ZoneProtocol::Game::ItemDef newOrb = {
                                11, "NUCLEO SPARK", ZoneProtocol::Game::ItemCategory::ARTIFACT,
                                1, 1, ZoneProtocol::Game::SynergyElement::ELEC_PURPLE, 0.0f, 0.25f, 1
                            };
                            briefcase.addItemAuto(newOrb, 1);
                            bunker.statusBanner = "MERCADO NEGRO: ARTEFATO ELETRICO ADQUIRIDO (-200 CREDITS)!";
                        }
                    }
                }

                platform.endFrame();
                continue;
            }

            // =====================================================================
            // ESTADO 4: INFILTRAÇÃO ATIVA NA ZONA (IN_ZONE)
            // =====================================================================
            runTimeSurvived += deltaTime;
            if (remainingExtractionTime > 0.0f) {
                remainingExtractionTime -= deltaTime;
            }

            zoneMap.update(deltaTime);

            if (briefcase.isOpen()) {
                if (clickedLeft || clickedRight) {
                    briefcase.handleMouseClick(logMx, logMy, screenW, screenH, clickedRight);
                }
            } else {
                // Saque de Caixas Militares com a tecla F
                if (input.isActionJustPressed(DayDoom::HAL::Action::INTERACT_LOOT)) {
                    std::string msg;
                    if (combat.tryInteractLootCrate(player.getCamera().pos, briefcase, msg)) {
                        lootBanner = msg;
                        lootBannerTimer = 3.2f;
                    }
                }

                player.update(deltaTime, input, zoneMap, screenW, screenH);

                // Disparo de Projéteis com Sinergias da Maleta no clique esquerdo do rato
                if (input.isMouseButtonDown(SDL_BUTTON_LEFT)) {
                    combat.fireWeapon(player.getCamera().pos, player.getCamera().dir, briefcase.getModifiers());
                    player.performAttack(&zoneMap);
                }

                combat.update(deltaTime, zoneMap, player, briefcase.getModifiers());

                // Verifica se o Operador morreu na Zona
                if (player.getHealth() <= 0.0f) {
                    briefcase.clearMainBriefcaseOnDeath();
                    ZoneProtocol::Engine::Logger::warn("Operador abatido na Zona! Exibindo Ecra de Morte.", "GAME");
                    gameState = GameLoopState::DEATH_SCREEN;
                    platform.endFrame();
                    continue;
                }
            }

            float lookX = player.getCamera().pos.x + player.getCamera().dir.x * 1.35f;
            float lookY = player.getCamera().pos.y + player.getCamera().dir.y * 1.35f;
            int targetCellX = static_cast<int>(std::floor(lookX));
            int targetCellY = static_cast<int>(std::floor(lookY));
            bool nearDoor = zoneMap.isDoor(targetCellX, targetCellY);
            bool isDoorOpen = nearDoor && zoneMap.isDoorOpen(targetCellX, targetCellY);
            bool nearCrate = combat.isNearClosedCrate(player.getCamera().pos);

            // 1. Renderizador Principal do Mundo (Pisos e Paredes do tileset_walls_floors.png + Player do sprites_entities.png)
            worldView.render(
                screenBuffer,
                screenW,
                screenH,
                zoneMap,
                player,
                textures,
                totalTime,
                &entityAtlas,
                &objectsAtlas
            );

            // 2. Renderizador de Combate, Mutantes, Olhos Vermelhos no Escuro, Caixas de Loot e Projéteis
            combat.render(
                screenBuffer,
                screenW,
                screenH,
                zoneMap,
                player,
                entityAtlas,
                &objectsAtlas,
                worldView.getTileSize(screenW)
            );

            // 3. HUD de Sobrevivência e Cronômetro de Extração
            hud.render(
                screenBuffer,
                player,
                zoneMap,
                platform.getFPS(),
                remainingExtractionTime,
                logMx,
                logMy,
                nearDoor,
                isDoorOpen,
                nearCrate,
                lootBanner,
                combat.getKilledCount()
            );

            // 4. Maleta Tática 6x4 + Safe Pocket 2x2 + Raios de Sinergia (se aberta com TAB ou I)
            if (briefcase.isOpen()) {
                briefcase.render(
                    screenBuffer,
                    screenW,
                    screenH,
                    logMx,
                    logMy,
                    &uiBriefcaseAtlas
                );
            }

            platform.endFrame();
        }

        platform.shutdown();
        ZoneProtocol::Engine::Logger::info("Sessao de Zone Protocol v0.1.2 finalizada com sucesso.", "MAIN");
        ZoneProtocol::Engine::Logger::shutdown();
        return 0;

    } catch (const std::exception& e) {
        ZoneProtocol::Engine::Logger::critical(std::string("Excecao fatal: ") + e.what(), "FATAL");
        return 1;
    } catch (...) {
        ZoneProtocol::Engine::Logger::critical("Excecao fatal desconhecida!", "FATAL");
        return 1;
    }
}
