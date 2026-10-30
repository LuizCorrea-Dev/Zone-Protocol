#pragma once

#include <vector>
#include <string>
#include "../engine/MathDef.hpp"
#include "../engine/SpriteAtlas.hpp"
#include "ZoneMap.hpp"
#include "Player.hpp"
#include "UI/GridInventory.hpp"

namespace ZoneProtocol::Engine {
    class AudioSystem;
}

namespace ZoneProtocol::Game {

enum class MutantType {
    CRAWLER_PALE,  // Rápido, baixo HP, ataca em bando (Linha 2 do Atlas)
    BRUTE_FLESH    // Lento, alto HP, causa sangramento pesado (Linha 3 do Atlas)
};

enum class AIState {
    IDLE,          // Parado no escuro
    ALERT_SOUND,   // Ouviu tiro do jogador e caminha até a origem do som
    CHASE_VISIBLE  // Viu o jogador (linha de visão limpa) e corre para atacar
};

struct Mutant {
    int id = 0;
    MutantType type = MutantType::CRAWLER_PALE;
    AIState state = AIState::IDLE;
    DayDoom::Engine::Vec2 pos{0.0f, 0.0f};
    DayDoom::Engine::Vec2 vel{0.0f, 0.0f};
    DayDoom::Engine::Vec2 lastHeardTarget{0.0f, 0.0f};

    float hp = 60.0f;
    float maxHp = 60.0f;
    float speed = 2.8f;
    float radius = 0.32f;

    // Efeitos de status aplicados pelas sinergias da Maleta!
    float radBurnTimer = 0.0f;
    float radDPS = 0.0f;
    float cryoSlowTimer = 0.0f;
    float hitFlashTimer = 0.0f; // Pisca branco ao levar tiro
    float attackCooldown = 0.0f;

    // Animação fluida HD
    float animTimer = 0.0f;
    float attackAnimTimer = 0.0f;
    float screamCooldown = 0.0f; // Cooldown para o grito no escuro
};

struct Projectile {
    DayDoom::Engine::Vec2 pos{0.0f, 0.0f};
    DayDoom::Engine::Vec2 dir{0.0f, 0.0f};
    float speed = 18.0f;
    float damage = 25.0f;
    int ricochetsLeft = 0;
    float radBurnDPS = 0.0f;
    bool cryoSlow = false;
    bool active = true;
};

// Arco elétrico visual quando a sinergia roxa salta entre inimigos
struct ElectricArcFX {
    DayDoom::Engine::Vec2 startPos;
    DayDoom::Engine::Vec2 endPos;
    float timer = 0.15f;
};

// Caixa de Loot saqueável na Zona (renderizada com sprites_map_objects.png)
struct LootCrate {
    DayDoom::Engine::Vec2 pos{0.0f, 0.0f};
    bool opened = false;
    ItemDef rewardItem;
};

class CombatSystem {
public:
    CombatSystem() = default;

    // Gera mutantes e caixas de loot proceduralmente longe do Spawn do Player
    void spawnMutantsForZone(const DayDoom::Game::ZoneMap& map, uint32_t seed, int count = 25);

    // Dispara um tiro aplicando os modificadores em tempo real da Maleta de Sinergia e sons de tiro único vs rajada da AK-47
    void fireWeapon(
        const DayDoom::Engine::Vec2& playerPos,
        const DayDoom::Engine::Vec2& aimDir,
        const CombatModifiers& synergyMods,
        ZoneProtocol::Engine::AudioSystem* audioSystem = nullptr
    );

    // Notifica quando o operador soltou o gatilho da arma (ao cessar a rajada, a última bala ressoa com o som de reverb)
    void notifyTriggerReleased(ZoneProtocol::Engine::AudioSystem* audioSystem = nullptr);

    // Tenta abrir uma caixa de loot próxima ao jogador (Tecla F)
    bool tryInteractLootCrate(
        const DayDoom::Engine::Vec2& playerPos,
        GridInventory& inventory,
        std::string& outLootMessage
    );

    bool isNearClosedCrate(const DayDoom::Engine::Vec2& playerPos) const;

    // Atualiza física dos tiros, ricochetes, IA dos mutantes e dano
    void update(
        float dt,
        const DayDoom::Game::ZoneMap& map,
        DayDoom::Game::Player& player,
        const CombatModifiers& synergyMods,
        ZoneProtocol::Engine::AudioSystem* audioSystem = nullptr
    );

    // Renderiza caixas de loot, mutantes (ou apenas olhos vermelhos no escuro!), tiros e raios
    void render(
        uint32_t* screenBuffer, int screenW, int screenH,
        const DayDoom::Game::ZoneMap& map,
        const DayDoom::Game::Player& player,
        const Engine::SpriteAtlas& entityAtlas,
        const Engine::SpriteAtlas* objectsAtlas,
        float tileSize,
        const Engine::SpriteAtlas* crawlerAnimAtlas = nullptr,
        const Engine::SpriteAtlas* bruteAnimAtlas = nullptr,
        const Engine::SpriteAtlas* shadowAtlas = nullptr
    ) const;

    int getKilledCount() const { return m_mutantsKilled; }
    int getDataPointsEarned() const { return m_dataPointsEarned; }
    int getActiveProjectilesCount() const { return static_cast<int>(m_projectiles.size()); }
    const std::vector<Mutant>& getMutants() const { return m_mutants; }
    const std::vector<LootCrate>& getLootCrates() const { return m_crates; }

    // Raycast 2D rápido para checar se há parede entre 2 pontos
    bool hasLineOfSight(const DayDoom::Game::ZoneMap& map, DayDoom::Engine::Vec2 a, DayDoom::Engine::Vec2 b) const;

private:
    std::vector<Mutant> m_mutants;
    std::vector<Projectile> m_projectiles;
    std::vector<ElectricArcFX> m_arcs;
    std::vector<LootCrate> m_crates;
    int m_nextMutantId = 1;
    int m_mutantsKilled = 0;
    int m_dataPointsEarned = 0;
    float m_fireCooldown = 0.0f;
    int m_burstShotsFired = 0;

    void drawLine(uint32_t* buf, int sw, int sh, int x0, int y0, int x1, int y1, uint32_t color) const;
};

} // namespace ZoneProtocol::Game

