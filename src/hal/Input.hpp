#pragma once

#include <cstdint>
#include <unordered_map>

#if defined(__ANDROID__)
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

namespace DayDoom::HAL {

enum class Action {
    MOVE_FORWARD,
    MOVE_BACKWARD,
    STRAFE_LEFT,
    STRAFE_RIGHT,
    TURN_LEFT,
    TURN_RIGHT,
    RUN,
    ATTACK,
    INTERACT_LOOT,
    USE_BANDAGE,
    USE_FOOD,
    RELOAD,
    TOGGLE_MAP,
    TOGGLE_INVENTORY
};

struct TouchFinger {
    int64_t id = -1;
    float x = 0.0f; // 0.0 a 1.0 (normalizado)
    float y = 0.0f;
    bool active = false;
};

class Input {
public:
    Input();

    // Inicia um novo frame: prepara os acumuladores e zera os eventos de clique único
    void beginFrame();

    void update(int logicalMouseX = -1, int logicalMouseY = -1);
    void handleEvent(const SDL_Event& event, int logicalMouseX = -1, int logicalMouseY = -1);

    // Estado das ações de jogabilidade
    bool isActionPressed(Action action) const;
    bool isActionJustPressed(Action action) const;

    // Eixos analógicos (-1.0f a 1.0f)
    float getMoveAxisX() const; // -1 (esquerda), +1 (direita)
    float getMoveAxisY() const; // -1 (trás), +1 (frente)
    float getTurnAxis() const;  // -1 (virar esquerda), +1 (virar direita)

    // Entrada por toque no celular
    const TouchFinger& getLeftStickTouch() const { return m_leftStickTouch; }
    const TouchFinger& getRightLookTouch() const { return m_rightLookTouch; }
    void setVirtualStickOutput(float x, float y) { m_virtualStickX = x; m_virtualStickY = y; }
    void setVirtualButton(Action action, bool pressed);

    // Mouse: Posição Absoluta (Lógica e Raw), Deltas, Botões e Roda de Scroll
    int getMouseX() const { return m_mouseX; }
    int getMouseY() const { return m_mouseY; }
    int getRawMouseX() const { return m_rawMouseX; }
    int getRawMouseY() const { return m_rawMouseY; }
    void getAbsoluteMouse(int& outX, int& outY) const { outX = m_mouseX; outY = m_mouseY; }

    int getMouseDeltaX() const { return m_mouseDeltaX; }
    int getMouseDeltaY() const { return m_mouseDeltaY; }
    bool isMouseButtonDown(int button) const;
    bool isMouseButtonJustPressed(int button) const;
    bool isMouseButtonJustReleased(int button) const;
    int getMouseWheel() const { return m_mouseWheel; }

    void setMousePosition(int x, int y) { m_mouseX = x; m_mouseY = y; }

private:
    const uint8_t* m_keystates = nullptr;
    bool m_actionStates[16] = {false};
    bool m_prevActionStates[16] = {false};

    int m_mouseX = 0;
    int m_mouseY = 0;
    int m_rawMouseX = 0;
    int m_rawMouseY = 0;
    int m_mouseDeltaX = 0;
    int m_mouseDeltaY = 0;
    int m_accumMouseDeltaX = 0;
    int m_accumMouseDeltaY = 0;
    int m_mouseWheel = 0;
    int m_accumMouseWheel = 0;

    bool m_mouseButtons[8] = {false};
    bool m_prevMouseButtons[8] = {false};
    bool m_mouseJustPressed[8] = {false};
    bool m_mouseJustReleased[8] = {false};
    bool m_mouseAttackClicked = false;

    float m_virtualStickX = 0.0f;
    float m_virtualStickY = 0.0f;

    TouchFinger m_leftStickTouch;
    TouchFinger m_rightLookTouch;
};

} // namespace DayDoom::HAL
