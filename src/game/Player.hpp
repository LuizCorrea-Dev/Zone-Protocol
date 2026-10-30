#pragma once

#include "../engine/Camera.hpp"
#include "../hal/Input.hpp"
#include "ZoneMap.hpp"
#include <string>

namespace DayDoom::Game {

class Player {
public:
    Player(float startX = 12.5f, float startY = 54.5f);

    // Atualização com movimentação WASD Top-Down e mira 360° via mouse
    void update(float deltaTime, const HAL::Input& input, ZoneMap& map, int screenWidth = 480, int screenHeight = 270);

    const Engine::Camera& getCamera() const { return m_camera; }
    Engine::Camera& getCamera() { return m_camera; }

    // Status vitais de sobrevivência na Zona
    float getHealth() const { return m_health; }
    float getHunger() const { return m_hunger; }
    float getStamina() const { return m_stamina; }
    bool isBleeding() const { return m_bleeding; }
    void setBleeding(bool bleeding) { m_bleeding = bleeding; }
    float getElevation() const { return m_elevation; }

    void takeDamage(float amount);
    void applyBandage();
    void eatFood(float nutrition = 35.0f);
    void healFull() {
        m_health = 100.0f;
        m_stamina = 100.0f;
        m_hunger = 100.0f;
        m_bleeding = false;
    }

    // Sistema de combate / disparo tático
    void performAttack(ZoneMap* map = nullptr);
    bool isAttacking() const { return m_attackCooldown > 0.0f; }
    float getAttackCooldown() const { return m_attackCooldown; }

    float getMouseSensitivity() const { return m_mouseSensitivity; }
    void setMouseSensitivity(float s) { m_mouseSensitivity = s; }

    // Gestão de Andares e Posição
    int getFloor() const { return m_floor; }
    void setFloor(int floor) { m_floor = floor; }

    float getAimAngle() const { return m_camera.flashlightAngle; }
    float getRadius() const { return m_radius; }

    // Extração / Maleta
    bool hasExtractionCase() const { return m_hasExtractionCase; }
    void setExtractionCase(bool hasCase) { m_hasExtractionCase = hasCase; }

    // Animação e Estados Visuais Fluidos
    bool isMoving() const { return m_isMoving; }
    bool isRunning() const { return m_isRunning; }
    bool hasWeaponEquipped() const { return m_hasWeaponEquipped; }
    void setWeaponEquipped(bool equipped) { m_hasWeaponEquipped = equipped; }
    float getHitStaggerTimer() const { return m_hitStaggerTimer; }
    float getAnimTimer() const { return m_animTimer; }

private:
    Engine::Camera m_camera;

    float m_walkSpeed = 3.8f;
    float m_runSpeed = 6.0f;
    float m_turnSpeed = 3.5f;
    float m_mouseSensitivity = 1.0f;
    float m_radius = 0.28f; // Raio de colisão físico com paredes

    // Status vitais
    float m_health = 100.0f;
    float m_hunger = 100.0f;
    float m_stamina = 100.0f;
    bool m_bleeding = false;
    float m_elevation = 0.0f;
    float m_attackCooldown = 0.0f;

    // Animação fluida
    bool m_isMoving = false;
    bool m_isRunning = false;
    bool m_hasWeaponEquipped = true;
    float m_hitStaggerTimer = 0.0f;
    float m_animTimer = 0.0f;

    int m_floor = 0;
    bool m_hasExtractionCase = true; // Agente infiltrou na Zona com a Maleta de Extração
};

} // namespace DayDoom::Game
