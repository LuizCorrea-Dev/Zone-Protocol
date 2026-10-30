#include "Player.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace DayDoom::Game {

Player::Player(float startX, float startY)
    : m_camera(startX, startY, -1.5707963f, 20.0f) {
}

void Player::update(float deltaTime, const HAL::Input& input, ZoneMap& map, int screenWidth, int screenHeight) {
    // 1. Mira tática em 360° direcionada pelo cursor do mouse
    int mouseX = input.getMouseX();
    int mouseY = input.getMouseY();

    float centerX = screenWidth * 0.5f;
    float centerY = screenHeight * 0.5f;
    float deltaAimX = static_cast<float>(mouseX) - centerX;
    float deltaAimY = static_cast<float>(mouseY) - centerY;

    if (deltaAimX * deltaAimX + deltaAimY * deltaAimY > 9.0f) {
        float targetAngle = std::atan2(deltaAimY, deltaAimX);
        m_camera.setAngle(targetAngle);
    } else {
        // Suporte opcional a rotação via teclado (QE ou Setas)
        float turnInput = input.getTurnAxis();
        if (turnInput != 0.0f) {
            m_camera.rotate(turnInput * m_turnSpeed * deltaTime);
        }
    }

    // 2. Velocidade e Stamina
    bool isRunning = input.isActionPressed(HAL::Action::RUN) && m_stamina > 10.0f;
    float currentSpeed = isRunning ? m_runSpeed : m_walkSpeed;

    // 3. Vetor de Movimento WASD Top-Down (W = Cima/Norte, S = Baixo/Sul, A = Esquerda/Oeste, D = Direita/Leste)
    float moveX = 0.0f;
    float moveY = 0.0f;

    if (input.isActionPressed(HAL::Action::MOVE_FORWARD))  moveY -= 1.0f;
    if (input.isActionPressed(HAL::Action::MOVE_BACKWARD)) moveY += 1.0f;
    if (input.isActionPressed(HAL::Action::STRAFE_LEFT))   moveX -= 1.0f;
    if (input.isActionPressed(HAL::Action::STRAFE_RIGHT))  moveX += 1.0f;

    // Consumo / recuperação de Stamina
    if (isRunning && (moveX != 0.0f || moveY != 0.0f)) {
        m_stamina = std::max(0.0f, m_stamina - 22.0f * deltaTime);
        m_hunger = std::max(0.0f, m_hunger - 0.7f * deltaTime);
    } else {
        m_stamina = std::min(100.0f, m_stamina + 16.0f * deltaTime);
        m_hunger = std::max(0.0f, m_hunger - 0.15f * deltaTime);
    }

    // Efeitos de Fome e Sangramento
    if (m_hunger <= 0.0f) {
        m_health = std::max(0.0f, m_health - 2.5f * deltaTime);
    }
    if (m_bleeding) {
        m_health = std::max(0.0f, m_health - 3.0f * deltaTime);
    }

    // Normaliza o vetor para velocidade constante em diagonais
    float moveLenSq = moveX * moveX + moveY * moveY;
    m_isMoving = (moveLenSq > 0.0001f);
    m_isRunning = (m_isMoving && isRunning && m_stamina > 0.0f);

    if (m_isMoving) {
        float len = std::sqrt(moveLenSq);
        moveX = (moveX / len) * currentSpeed * deltaTime;
        moveY = (moveY / len) * currentSpeed * deltaTime;
        m_animTimer += deltaTime;
    } else {
        m_animTimer = 0.0f;
    }

    if (m_hitStaggerTimer > 0.0f) {
        m_hitStaggerTimer = std::max(0.0f, m_hitStaggerTimer - deltaTime);
    }

    // 4. Colisão com paredes com deslizamento físico (Wall Sliding)
    // Eixo X
    float newPosX = m_camera.pos.x + moveX;
    if (map.isWalkable(newPosX, m_camera.pos.y, m_radius)) {
        m_camera.pos.x = newPosX;
    }

    // Eixo Y
    float newPosY = m_camera.pos.y + moveY;
    if (map.isWalkable(m_camera.pos.x, newPosY, m_radius)) {
        m_camera.pos.y = newPosY;
    }

    // 5. Interação com portas (tecla F ou Espaço)
    if (input.isActionJustPressed(HAL::Action::INTERACT_LOOT)) {
        // Procura porta à frente da mira
        float interactDist = 1.35f;
        int targetX = static_cast<int>(std::floor(m_camera.pos.x + m_camera.dir.x * interactDist));
        int targetY = static_cast<int>(std::floor(m_camera.pos.y + m_camera.dir.y * interactDist));
        if (map.isDoor(targetX, targetY)) {
            map.toggleDoor(targetX, targetY);
        }
    }

    // 6. Atualização de Ataque / Combate
    if (m_attackCooldown > 0.0f) {
        m_attackCooldown = std::max(0.0f, m_attackCooldown - deltaTime);
    }

    if (input.isActionJustPressed(HAL::Action::ATTACK) && m_attackCooldown <= 0.0f) {
        performAttack(&map);
    }
}

void Player::takeDamage(float amount) {
    m_health = std::max(0.0f, m_health - amount);
    m_hitStaggerTimer = 0.28f; // Ativa visual de recuo e sangue (Hit Stagger)
    if (amount > 15.0f && (std::rand() % 100 < 40)) {
        m_bleeding = true;
    }
}

void Player::applyBandage() {
    if (m_bleeding) {
        m_bleeding = false;
        m_health = std::min(100.0f, m_health + 10.0f);
    }
}

void Player::eatFood(float nutrition) {
    m_hunger = std::min(100.0f, m_hunger + nutrition);
    m_health = std::min(100.0f, m_health + 5.0f);
}

void Player::performAttack(ZoneMap* map) {
    m_attackCooldown = 0.22f; // Cooldown visual de disparo tático
}

} // namespace DayDoom::Game
