#pragma once

#include <cstdint>
#include <string>
#include <vector>

#if defined(__ANDROID__)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

namespace DayDoom::HAL {

// Perfis de Resolução e Proporção de Tela
enum class DisplayProfile {
    WIDESCREEN_16_10,  // 640x400 (janela 1280x800): Proporção 16:10 PC e Steam Deck
    WIDESCREEN_16_9,   // 480x270: Proporção 16:9
    RETRO_4_3,         // 320x240: Proporção Retro CRT 4:3
    CLASSIC_DOOM_13H   // 320x200: Modo 13h MS-DOS
};

class Platform {
public:
    Platform();
    ~Platform();

    // Inicialização com perfil de tela adaptável (Padrão 1280x800 16:10)
    bool init(
        const std::string& title,
        DisplayProfile profile = DisplayProfile::WIDESCREEN_16_10,
        int windowWidth = 1280,
        int windowHeight = 800,
        bool fullscreen = false
    );

    void shutdown();

    bool processEvents();
    void beginFrame();
    void endFrame();

    // Alternar tela cheia dinamicamente
    void toggleFullscreen();

    // Controle de captura do mouse (Modo Relativo para FPS)
    void setRelativeMouseMode(bool enabled) { SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE); }
    bool getRelativeMouseMode() const { return SDL_GetRelativeMouseMode() == SDL_TRUE; }

    // Modo Editor em Alta Resolução (1280x720)
    void setEditorMode(bool enabled);
    bool isEditorMode() const { return m_isEditorMode; }
    uint32_t* getEditorBuffer() { return m_editorBuffer.data(); }
    int getEditorWidth() const { return 1280; }
    int getEditorHeight() const { return 720; }

    // Acesso direto ao buffer de pixels da CPU (Jogabilidade Canônica)
    uint32_t* getScreenBuffer() { return m_screenBuffer.data(); }
    void clearScreen(uint32_t color = 0xFF000000);

    // Converte coordenadas da janela física para a resolução lógica ativa
    void windowToLogical(int windowX, int windowY, int& outLogicalX, int& outLogicalY) const;

    // Tempo e desempenho
    float getDeltaTime() const { return m_deltaTime; }
    float getFPS() const { return m_fps; }
    bool isRunning() const { return m_running; }

    // Resoluções canônicas ativas
    int getScreenWidth() const { return m_canonicalWidth; }
    int getScreenHeight() const { return m_canonicalHeight; }
    DisplayProfile getProfile() const { return m_profile; }

    SDL_Window* getSDLWindow() const { return m_window; }
    SDL_Renderer* getSDLRenderer() const { return m_renderer; }

private:
    DisplayProfile m_profile = DisplayProfile::WIDESCREEN_16_9;
    int m_canonicalWidth = 480;
    int m_canonicalHeight = 270;

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture* m_framebufferTexture = nullptr;

    std::vector<uint32_t> m_screenBuffer;

    // Alta Resolução (1280x720) para o DayDoom Editor Mode
    bool m_isEditorMode = false;
    SDL_Texture* m_editorTexture = nullptr;
    std::vector<uint32_t> m_editorBuffer;

    bool m_running = false;
    bool m_isFullscreen = false;
    uint64_t m_lastTime = 0;
    float m_deltaTime = 0.016f;
    float m_fps = 60.0f;
    int m_frameCount = 0;
    float m_fpsTimer = 0.0f;
};

} // namespace DayDoom::HAL
