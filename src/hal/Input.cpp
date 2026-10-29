#include "Input.hpp"
#include "../engine/Logger.hpp"

namespace DayDoom::HAL {

Input::Input() {
    m_keystates = SDL_GetKeyboardState(nullptr);
    ZP_LOG_SCRIPT("[KeybindsManager] Registered bind: MoveForward (W / UP)");
    ZP_LOG_SCRIPT("[KeybindsManager] Registered bind: MoveBackward (S / DOWN)");
    ZP_LOG_SCRIPT("[KeybindsManager] Registered bind: StrafeLeft (A) | StrafeRight (D)");
    ZP_LOG_SCRIPT("[KeybindsManager] Registered bind: SprintTactical (LSHIFT / RSHIFT)");
    ZP_LOG_SCRIPT("[KeybindsManager] Registered bind: InteractLootOrDoor (F / SPACE)");
    ZP_LOG_SCRIPT("[KeybindsManager] Registered bind: ToggleSynergyBriefcase6x4 (TAB / I)");
    ZP_LOG_SCRIPT("[KeybindsManager] Registered bind: RotateBriefcaseItem (R / MOUSE_RIGHT)");
    ZP_LOG_SCRIPT("[KeybindsManager] Registered bind: ExtractAtObelisk (E) | BunkerHub (B)");
    ZP_LOG_SCRIPT("[KeybindsManager] Registered bind: PrimaryWeaponFire (MOUSE_LEFT)");
}

void Input::beginFrame() {
    for (size_t i = 0; i < 16; ++i) {
        m_prevActionStates[i] = m_actionStates[i];
    }
    for (size_t i = 0; i < 8; ++i) {
        m_prevMouseButtons[i] = m_mouseButtons[i];
        m_mouseJustPressed[i] = false;
        m_mouseJustReleased[i] = false;
    }

    // Transfere o movimento do mouse acumulado durante os eventos deste frame
    m_mouseDeltaX = m_accumMouseDeltaX;
    m_mouseDeltaY = m_accumMouseDeltaY;
    m_accumMouseDeltaX = 0;
    m_accumMouseDeltaY = 0;

    m_mouseWheel = m_accumMouseWheel;
    m_accumMouseWheel = 0;
}

void Input::update(int logicalMouseX, int logicalMouseY) {
    // Captura da posição do cursor do mouse em coordenadas absolutas
    uint32_t mouseState = SDL_GetMouseState(&m_rawMouseX, &m_rawMouseY);

    if (logicalMouseX >= 0 && logicalMouseY >= 0) {
        m_mouseX = logicalMouseX;
        m_mouseY = logicalMouseY;
    } else {
        m_mouseX = m_rawMouseX;
        m_mouseY = m_rawMouseY;
    }

    // Atualiza teclas do PC
    if (m_keystates) {
        m_actionStates[static_cast<size_t>(Action::MOVE_FORWARD)]     = m_keystates[SDL_SCANCODE_W] || m_keystates[SDL_SCANCODE_UP];
        m_actionStates[static_cast<size_t>(Action::MOVE_BACKWARD)]    = m_keystates[SDL_SCANCODE_S] || m_keystates[SDL_SCANCODE_DOWN];
        m_actionStates[static_cast<size_t>(Action::STRAFE_LEFT)]      = m_keystates[SDL_SCANCODE_A];
        m_actionStates[static_cast<size_t>(Action::STRAFE_RIGHT)]     = m_keystates[SDL_SCANCODE_D];
        m_actionStates[static_cast<size_t>(Action::TURN_LEFT)]        = m_keystates[SDL_SCANCODE_LEFT] || m_keystates[SDL_SCANCODE_Q];
        m_actionStates[static_cast<size_t>(Action::TURN_RIGHT)]       = m_keystates[SDL_SCANCODE_RIGHT] || m_keystates[SDL_SCANCODE_E];
        m_actionStates[static_cast<size_t>(Action::RUN)]              = m_keystates[SDL_SCANCODE_LSHIFT] || m_keystates[SDL_SCANCODE_RSHIFT];
        m_actionStates[static_cast<size_t>(Action::INTERACT_LOOT)]    = m_keystates[SDL_SCANCODE_F] || m_keystates[SDL_SCANCODE_SPACE];
        m_actionStates[static_cast<size_t>(Action::USE_BANDAGE)]      = m_keystates[SDL_SCANCODE_1];
        m_actionStates[static_cast<size_t>(Action::USE_FOOD)]         = m_keystates[SDL_SCANCODE_2];
        m_actionStates[static_cast<size_t>(Action::RELOAD)]           = m_keystates[SDL_SCANCODE_R];
        m_actionStates[static_cast<size_t>(Action::TOGGLE_MAP)]       = m_keystates[SDL_SCANCODE_M];
        m_actionStates[static_cast<size_t>(Action::TOGGLE_INVENTORY)] = m_keystates[SDL_SCANCODE_TAB] || m_keystates[SDL_SCANCODE_I];
    }

    // Consulta do estado atual dos botões físicos
    m_mouseButtons[SDL_BUTTON_LEFT]   = (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    m_mouseButtons[SDL_BUTTON_MIDDLE] = (mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    m_mouseButtons[SDL_BUTTON_RIGHT]  = (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;

    bool isLeftDown = m_mouseButtons[SDL_BUTTON_LEFT] || m_mouseAttackClicked;
    m_actionStates[static_cast<size_t>(Action::ATTACK)] = isLeftDown;
    m_mouseAttackClicked = false;
}

void Input::handleEvent(const SDL_Event& event, int logicalMouseX, int logicalMouseY) {
    if (logicalMouseX >= 0 && logicalMouseY >= 0) {
        m_mouseX = logicalMouseX;
        m_mouseY = logicalMouseY;
    }

    if (event.type == SDL_MOUSEMOTION) {
        m_accumMouseDeltaX += event.motion.xrel;
        m_accumMouseDeltaY += event.motion.yrel;
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (event.button.button < 8) {
            m_mouseButtons[event.button.button] = true;
            m_mouseJustPressed[event.button.button] = true;
        }
        if (event.button.button == SDL_BUTTON_LEFT) {
            m_mouseAttackClicked = true;
        }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
        if (event.button.button < 8) {
            m_mouseButtons[event.button.button] = false;
            m_mouseJustReleased[event.button.button] = true;
        }
    } else if (event.type == SDL_MOUSEWHEEL) {
        m_accumMouseWheel += event.wheel.y;
    }

    // Tratamento de toque multi-touch no Android
    if (event.type == SDL_FINGERDOWN || event.type == SDL_FINGERMOTION) {
        if (event.tfinger.x < 0.5f) {
            m_leftStickTouch.id = event.tfinger.fingerId;
            m_leftStickTouch.x = event.tfinger.x;
            m_leftStickTouch.y = event.tfinger.y;
            m_leftStickTouch.active = true;
        } else {
            m_rightLookTouch.id = event.tfinger.fingerId;
            m_rightLookTouch.x = event.tfinger.x;
            m_rightLookTouch.y = event.tfinger.y;
            m_rightLookTouch.active = true;
        }
    } else if (event.type == SDL_FINGERUP) {
        if (event.tfinger.fingerId == m_leftStickTouch.id) {
            m_leftStickTouch.active = false;
            m_virtualStickX = 0.0f;
            m_virtualStickY = 0.0f;
        }
        if (event.tfinger.fingerId == m_rightLookTouch.id) {
            m_rightLookTouch.active = false;
        }
    }
}

bool Input::isActionPressed(Action action) const {
    size_t idx = static_cast<size_t>(action);
    return idx < 16 ? m_actionStates[idx] : false;
}

bool Input::isActionJustPressed(Action action) const {
    size_t idx = static_cast<size_t>(action);
    return idx < 16 ? (m_actionStates[idx] && !m_prevActionStates[idx]) : false;
}

bool Input::isMouseButtonDown(int button) const {
    if (button >= 0 && button < 8) return m_mouseButtons[button];
    return false;
}

bool Input::isMouseButtonJustPressed(int button) const {
    if (button >= 0 && button < 8) {
        return m_mouseJustPressed[button] || (m_mouseButtons[button] && !m_prevMouseButtons[button]);
    }
    return false;
}

bool Input::isMouseButtonJustReleased(int button) const {
    if (button >= 0 && button < 8) {
        return m_mouseJustReleased[button] || (!m_mouseButtons[button] && m_prevMouseButtons[button]);
    }
    return false;
}

float Input::getMoveAxisX() const {
    float axis = 0.0f;
    if (isActionPressed(Action::STRAFE_RIGHT)) axis += 1.0f;
    if (isActionPressed(Action::STRAFE_LEFT))  axis -= 1.0f;
    if (m_virtualStickX != 0.0f) axis = m_virtualStickX;
    return axis;
}

float Input::getMoveAxisY() const {
    float axis = 0.0f;
    if (isActionPressed(Action::MOVE_FORWARD))  axis += 1.0f;
    if (isActionPressed(Action::MOVE_BACKWARD)) axis -= 1.0f;
    if (m_virtualStickY != 0.0f) axis = m_virtualStickY;
    return axis;
}

float Input::getTurnAxis() const {
    float axis = 0.0f;
    if (isActionPressed(Action::TURN_RIGHT)) axis += 1.0f;
    if (isActionPressed(Action::TURN_LEFT))  axis -= 1.0f;
    return axis;
}

void Input::setVirtualButton(Action action, bool pressed) {
    size_t idx = static_cast<size_t>(action);
    if (idx < 16) {
        m_actionStates[idx] = pressed;
    }
}

} // namespace DayDoom::HAL
