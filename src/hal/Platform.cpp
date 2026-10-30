#include "Platform.hpp"
#include "../engine/Logger.hpp"
#include <iostream>
#include <algorithm>

namespace DayDoom::HAL {

Platform::Platform() {
}

Platform::~Platform() {
    shutdown();
}

bool Platform::init(
    const std::string& title,
    DisplayProfile profile,
    int windowWidth,
    int windowHeight,
    bool fullscreen
) {
    m_profile = profile;

    // 1. Configuração da Resolução Canônica baseada no Perfil Selecionado
    switch (m_profile) {
        case DisplayProfile::HD_720P:
            m_canonicalWidth = 1280;
            m_canonicalHeight = 720;
            break;

        case DisplayProfile::STEAM_DECK_800P:
            m_canonicalWidth = 1280;
            m_canonicalHeight = 800;
            break;

        case DisplayProfile::FHD_1080P:
        default:
            m_canonicalWidth = 1920;
            m_canonicalHeight = 1080;
            break;
    }

    // 2. Travar a orientação do Celular (Android/iOS) obrigatoriamente em Paisagem (Horizontal)
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");

    // 3. Inicialização dos subsistemas da SDL2
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        ZP_LOG_ERROR("ENGINE", std::string("Falha ao inicializar SDL: ") + SDL_GetError());
        return false;
    }

    // Detecção da resolução máxima da tela do monitor para Fullscreen nativo
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
        windowWidth = dm.w;
        windowHeight = dm.h;
        ZoneProtocol::Engine::Logger::info(
            "Display Monitor Detectado -> Resolucao Nativa: " + std::to_string(dm.w) + "x" + std::to_string(dm.h) +
            " @" + std::to_string(dm.refresh_rate) + "Hz", "RENDER"
        );
    }

    // Configuração das flags da janela (Fullscreen desktop automático com redimensionamento total)
    uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;

    #if defined(__ANDROID__)
    windowFlags |= SDL_WINDOW_FULLSCREEN;
    m_isFullscreen = true;
    #else
    if (fullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        m_isFullscreen = true;
    }
    #endif

    m_window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        windowFlags
    );

    if (!m_window) {
        ZP_LOG_ERROR("ENGINE", std::string("Falha ao criar janela SDL: ") + SDL_GetError());
        return false;
    }

    // 4. Criação do Renderer com VSync
    m_renderer = SDL_CreateRenderer(
        m_window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!m_renderer) {
        ZP_LOG_WARN("RENDER", std::string("Fallback para renderizador por software: ") + SDL_GetError());
        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_SOFTWARE);
    }

    if (!m_renderer) {
        ZP_LOG_ERROR("RENDER", std::string("Erro fatal ao criar SDL_Renderer: ") + SDL_GetError());
        return false;
    }

    SDL_RendererInfo rInfo{};
    if (SDL_GetRendererInfo(m_renderer, &rInfo) == 0) {
        ZoneProtocol::Engine::Logger::info(
            std::string("GPU Renderer Backend: '") + (rInfo.name ? rInfo.name : "unknown") +
            "' | MaxTextureSize: " + std::to_string(rInfo.max_texture_width) + "x" + std::to_string(rInfo.max_texture_height) +
            " | VSync: ON", "RENDER"
        );
    }

    // 5. Adaptação Dinâmica à Tela com Filtro Nearest-Neighbor (Pixel Art Nítido 16:10)
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // 0 = Nearest Neighbor limpo sem borrão
    SDL_RenderSetLogicalSize(m_renderer, m_canonicalWidth, m_canonicalHeight);

    // 6. Alocação do Buffer de Memória Canônico da CPU (640x400 para 16:10)
    m_screenBuffer.resize(m_canonicalWidth * m_canonicalHeight, 0xFF000000);

    // 7. Textura de Streaming Canônica na GPU com escala nítida
    m_framebufferTexture = SDL_CreateTexture(
        m_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        m_canonicalWidth,
        m_canonicalHeight
    );

    if (!m_framebufferTexture) {
        std::cerr << "[HAL] Falha ao criar SDL_Texture de streaming: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_SetTextureScaleMode(m_framebufferTexture, SDL_ScaleModeNearest);

    // 8. Pré-alocação do Buffer de Alta Resolução (1280x720) para o Editor
    m_editorBuffer.resize(1280 * 720, 0xFF181C24);
    m_editorTexture = SDL_CreateTexture(
        m_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        1280,
        720
    );

    m_running = true;
    m_lastTime = SDL_GetPerformanceCounter();

#if !defined(__ANDROID__)
    setRelativeMouseMode(false); // No Roguelike Top-Down 2D, o cursor do mouse se move livremente pela janela
#endif

    ZoneProtocol::Engine::Logger::info(
        "Display Profile Active -> Window: " + std::to_string(windowWidth) + "x" + std::to_string(windowHeight) +
        " | Canonical Framebuffer: " + std::to_string(m_canonicalWidth) + "x" + std::to_string(m_canonicalHeight) +
        " (Nearest-Neighbor Pixel Scaling)", "ENGINE"
    );

    return true;
}

void Platform::shutdown() {
    if (!m_running && !m_window && !m_renderer && !m_framebufferTexture && !m_editorTexture) {
        return;
    }
    if (m_editorTexture) {
        SDL_DestroyTexture(m_editorTexture);
        m_editorTexture = nullptr;
    }
    if (m_framebufferTexture) {
        SDL_DestroyTexture(m_framebufferTexture);
        m_framebufferTexture = nullptr;
    }
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    SDL_Quit();
    m_running = false;
}

void Platform::toggleFullscreen() {
    m_isFullscreen = !m_isFullscreen;
    uint32_t flags = m_isFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
    SDL_SetWindowFullscreen(m_window, flags);
    ZoneProtocol::Engine::Logger::info(
        std::string("[Platform] Display Mode toggled -> ") + (m_isFullscreen ? "FULLSCREEN_DESKTOP" : "WINDOWED"),
        "RENDER"
    );
}

bool Platform::setCanonicalResolution(int width, int height) {
    if (width <= 0 || height <= 0) return false;
    if (width == m_canonicalWidth && height == m_canonicalHeight && m_framebufferTexture != nullptr) {
        return true;
    }

    m_canonicalWidth = width;
    m_canonicalHeight = height;
    m_screenBuffer.assign(static_cast<size_t>(m_canonicalWidth * m_canonicalHeight), 0xFF000000);

    if (m_renderer) {
        if (m_framebufferTexture) {
            SDL_DestroyTexture(m_framebufferTexture);
            m_framebufferTexture = nullptr;
        }

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
        SDL_RenderSetLogicalSize(m_renderer, m_canonicalWidth, m_canonicalHeight);

        m_framebufferTexture = SDL_CreateTexture(
            m_renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            m_canonicalWidth,
            m_canonicalHeight
        );

        if (!m_framebufferTexture) {
            ZP_LOG_ERROR("RENDER", std::string("Falha ao recriar SDL_Texture para resolucao ") +
                         std::to_string(width) + "x" + std::to_string(height) + ": " + SDL_GetError());
            return false;
        }
        SDL_SetTextureScaleMode(m_framebufferTexture, SDL_ScaleModeNearest);
    }

    ZoneProtocol::Engine::Logger::info(
        "[Platform] Canonical Resolution changed dynamically -> " +
        std::to_string(m_canonicalWidth) + "x" + std::to_string(m_canonicalHeight) +
        " (" + std::to_string((m_canonicalWidth * m_canonicalHeight) / 1000) + "k pixels, 16:10 Nearest-Neighbor)",
        "RENDER"
    );
    ZP_LOG_SCRIPT("[VideoSettings] SetCanonicalResolution(" + std::to_string(m_canonicalWidth) + "x" + std::to_string(m_canonicalHeight) + ")");
    return true;
}

int Platform::getResolutionPresetIndex() const {
    if (m_canonicalWidth >= 1920) return 2; // 1080p (1920x1080)
    if (m_canonicalHeight >= 800) return 1; // Steam Deck (1280x800)
    return 0;                               // 720p (1280x720)
}

void Platform::setResolutionPresetIndex(int presetIdx) {
    int clamped = std::clamp(presetIdx, 0, 2);
    if (clamped == 0) {
        setCanonicalResolution(1280, 720);
    } else if (clamped == 1) {
        setCanonicalResolution(1280, 800);
    } else {
        setCanonicalResolution(1920, 1080);
    }
}

void Platform::stepResolutionPreset(int delta) {
    int nextIdx = std::clamp(getResolutionPresetIndex() + delta, 0, 2);
    setResolutionPresetIndex(nextIdx);
}

void Platform::setEditorMode(bool enabled) {
    m_isEditorMode = enabled;

    if (m_isEditorMode) {
        // Altera a resolução lógica da SDL2 para HD (1280x720)
        SDL_RenderSetLogicalSize(m_renderer, 1280, 720);
        setRelativeMouseMode(false);
        SDL_ShowCursor(SDL_ENABLE);

        // Se não estiver em fullscreen e a janela for menor que 1280x720, expande para 1280x720
        if (!m_isFullscreen && m_window) {
            int curW = 0, curH = 0;
            SDL_GetWindowSize(m_window, &curW, &curH);
            if (curW < 1280 || curH < 720) {
                SDL_SetWindowSize(m_window, 1280, 720);
                SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            }
        }
        std::cout << "[HAL] Modo DayDoom Editor ativado em Alta Resolucao (1280x720)!" << std::endl;
    } else {
        // Restaura a resolução lógica canônica do gameplay retrô
        SDL_RenderSetLogicalSize(m_renderer, m_canonicalWidth, m_canonicalHeight);
        setRelativeMouseMode(true);
        SDL_ShowCursor(SDL_DISABLE);
        std::cout << "[HAL] Modo Jogo restaurado na resolucao canonica (" 
                  << m_canonicalWidth << "x" << m_canonicalHeight << ")!" << std::endl;
    }
}

void Platform::windowToLogical(int windowX, int windowY, int& outLogicalX, int& outLogicalY) const {
    if (!m_renderer) {
        outLogicalX = windowX;
        outLogicalY = windowY;
        return;
    }

    float lx = 0.0f, ly = 0.0f;
    SDL_RenderWindowToLogical(m_renderer, windowX, windowY, &lx, &ly);

    int targetW = m_isEditorMode ? 1280 : m_canonicalWidth;
    int targetH = m_isEditorMode ? 720 : m_canonicalHeight;

    outLogicalX = std::clamp(static_cast<int>(lx), 0, targetW - 1);
    outLogicalY = std::clamp(static_cast<int>(ly), 0, targetH - 1);
}

bool Platform::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_running = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE && !m_isEditorMode) {
                m_running = false;
            } else if (event.key.keysym.sym == SDLK_F11) {
                toggleFullscreen();
            }
        }

        #if defined(__ANDROID__)
        if (event.type == SDL_APP_WILLENTERBACKGROUND) {
            std::cout << "[HAL] Celular minimizado / tela bloqueada" << std::endl;
        } else if (event.type == SDL_APP_DIDENTERFOREGROUND) {
            std::cout << "[HAL] Celular reativado no jogo" << std::endl;
        }
        #endif
    }
    return m_running;
}

void Platform::beginFrame() {
    uint64_t currentTime = SDL_GetPerformanceCounter();
    uint64_t frequency = SDL_GetPerformanceFrequency();
    m_deltaTime = static_cast<float>(currentTime - m_lastTime) / static_cast<float>(frequency);
    m_lastTime = currentTime;

    if (m_deltaTime > 0.1f) {
        m_deltaTime = 0.1f;
    }

    m_frameCount++;
    m_fpsTimer += m_deltaTime;
    if (m_fpsTimer >= 1.0f) {
        m_fps = static_cast<float>(m_frameCount) / m_fpsTimer;
        m_frameCount = 0;
        m_fpsTimer = 0.0f;
    }
}

void Platform::endFrame() {
    if (m_isEditorMode) {
        // Atualiza a textura HD de 1280x720 com o buffer do editor
        SDL_UpdateTexture(
            m_editorTexture,
            nullptr,
            m_editorBuffer.data(),
            1280 * sizeof(uint32_t)
        );

        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_editorTexture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
    } else {
        // Atualiza a textura canônica (480x270) com o buffer do jogo
        SDL_UpdateTexture(
            m_framebufferTexture,
            nullptr,
            m_screenBuffer.data(),
            m_canonicalWidth * sizeof(uint32_t)
        );

        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_framebufferTexture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
    }
}

void Platform::clearScreen(uint32_t color) {
    if (m_isEditorMode) {
        std::fill(m_editorBuffer.begin(), m_editorBuffer.end(), color);
    } else {
        std::fill(m_screenBuffer.begin(), m_screenBuffer.end(), color);
    }
}

} // namespace DayDoom::HAL
