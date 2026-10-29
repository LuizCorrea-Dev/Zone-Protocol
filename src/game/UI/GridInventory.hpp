#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "../../engine/SpriteAtlas.hpp"

namespace ZoneProtocol::Game {

constexpr int BRIEFCASE_COLS = 6;
constexpr int BRIEFCASE_ROWS = 4;
constexpr int SAFE_POCKET_COLS = 2;
constexpr int SAFE_POCKET_ROWS = 2;

enum class ItemCategory {
    WEAPON,
    AMMO,
    MEDICAL,
    ARTIFACT,
    VALUABLE,
    FOOD,
    BRIEFCASE,
    TOOL
};

enum class SynergyElement {
    NONE = 0,
    RAD_GREEN,    // +Dano corrosivo / Radiação no alvo
    ELEC_PURPLE,  // +Cadência de tiro / Ricochete elétrico
    CRYO_BLUE     // +Lentidão no mutante / Reduz recuo e calor
};

struct ItemDef {
    int id = 0;
    std::string name;
    ItemCategory category = ItemCategory::VALUABLE;
    int width = 1;  // Largura na grade (ex: AK-74 = 3)
    int height = 1; // Altura na grade (ex: AK-74 = 2)
    SynergyElement element = SynergyElement::NONE;

    // Atributos base
    float baseValue = 0.0f;      // Dano base (arma) ou Cura (médico)
    float synergyBonus = 0.0f;   // Bónus percentual por conexão (ex: 0.25f = +25%)
    int maxStack = 1;
};

struct PlacedItem {
    int instanceId = 0;
    ItemDef def;
    int gridX = 0;
    int gridY = 0;
    int stackCount = 1;
    bool rotated = false;
    bool inSafePocket = false;

    int getActualWidth() const { return rotated ? def.height : def.width; }
    int getActualHeight() const { return rotated ? def.width : def.height; }
};

// Representa um raio de energia ativo entre dois itens para desenhar na UI
struct SynergyLink {
    int fromInstanceId = 0;
    int toInstanceId = 0;
    int startGridX = 0;
    int startGridY = 0;
    int endGridX = 0;
    int endGridY = 0;
    SynergyElement element = SynergyElement::NONE;
};

// Resultado matemático consolidado das conexões (Estilo Balatro)
struct CombatModifiers {
    float damageMultiplier = 1.0f;
    float fireRateMultiplier = 1.0f;
    float medkitEfficiency = 1.0f;
    int electricRicochets = 0;
    float radiationBurnDPS = 0.0f;
    bool cryoSlowActive = false;
};

class GridInventory {
public:
    static constexpr int GRID_COLS = BRIEFCASE_COLS;
    static constexpr int GRID_ROWS = BRIEFCASE_ROWS;
    static constexpr int TOTAL_SLOTS = GRID_COLS * GRID_ROWS;

    GridInventory();

    void setupInitialLoadout();

    bool isOpen() const { return m_isOpen; }
    void toggle() { m_isOpen = !m_isOpen; if (!m_isOpen) m_selectedInstanceId = -1; }
    void setOpen(bool open) { m_isOpen = open; if (!m_isOpen) m_selectedInstanceId = -1; }

    // Tenta posicionar um item na Maleta Principal (6x4) ou no Safe Pocket (2x2)
    bool canPlaceItem(const ItemDef& def, int gx, int gy, bool rotated, bool safePocket = false, int ignoreInstanceId = -1) const;
    bool addItem(const ItemDef& def, int gx, int gy, int count = 1, bool rotated = false, bool safePocket = false);
    bool addItemAuto(const ItemDef& def, int count = 1);
    bool removeItemByInstance(int instanceId);
    bool moveItem(int instanceId, int newGx, int newGy, bool newRotated, bool toSafePocket);

    // Chamado na Tela de Morte: apaga tudo da maleta 6x4 e mantém apenas o Safe Pocket 2x2
    void clearMainBriefcaseOnDeath();

    // Recalcula todas as conexões de adjacência e atualiza os multiplicadores
    void recalculateSynergies();

    const std::vector<PlacedItem>& getItems() const { return m_items; }
    const std::vector<SynergyLink>& getSynergyLinks() const { return m_links; }
    const CombatModifiers& getModifiers() const { return m_modifiers; }

    const PlacedItem* getItemAt(int gx, int gy, bool safePocket = false) const;
    int getArtifactCount() const;
    int getSafePocketItemCount() const;

    // Interação via rato / teclado na Maleta (selecionar, mover entre 6x4 e Safe Pocket 2x2, girar com R/ Botão Direito)
    void handleMouseClick(int mouseX, int mouseY, int screenW, int screenH, bool rightClick = false);
    void rotateSelected();

    // Renderiza a Maleta 6x4 + Safe Pocket 2x2 + Raios de Sinergia usando o Atlas ui_briefcase_items.png
    void render(
        uint32_t* screenBuffer,
        int screenWidth,
        int screenHeight,
        int mouseX,
        int mouseY,
        const Engine::SpriteAtlas* uiAtlas = nullptr
    );

private:
    bool m_isOpen = false;
    int m_mainGrid[BRIEFCASE_ROWS][BRIEFCASE_COLS];     // Guarda o instanceId ou 0 se vazio
    int m_safeGrid[SAFE_POCKET_ROWS][SAFE_POCKET_COLS]; // Bolso Seguro 2x2 blindado
    int m_nextInstanceId = 1;
    int m_selectedInstanceId = -1;
    bool m_selectedPreviewRotated = false;

    std::vector<PlacedItem> m_items;
    std::vector<SynergyLink> m_links;
    CombatModifiers m_modifiers;

    void rebuildGridArrays();
    bool areItemsAdjacent(const PlacedItem& a, const PlacedItem& b, int& outAx, int& outAy, int& outBx, int& outBy) const;

    // Helpers visuais
    Engine::SpriteRect getSpriteForItemDef(int defId, ItemCategory cat, SynergyElement elem) const;
    void drawRect(uint32_t* buf, int sw, int sh, int x, int y, int w, int h, uint32_t color) const;
    void drawBorder(uint32_t* buf, int sw, int sh, int x, int y, int w, int h, uint32_t color) const;
    void drawGlowingLine(uint32_t* buf, int sw, int sh, int x0, int y0, int x1, int y1, uint32_t color) const;
    void drawChar(uint32_t* buf, int sw, int sh, int x, int y, char c, uint32_t color, int scale = 1) const;
    void drawText(uint32_t* buf, int sw, int sh, int x, int y, const std::string& text, uint32_t color, int scale = 1) const;
};

} // namespace ZoneProtocol::Game

namespace DayDoom::Game::UI {
    using ItemCategory = ZoneProtocol::Game::ItemCategory;
    using SynergyElement = ZoneProtocol::Game::SynergyElement;
    using ItemDef = ZoneProtocol::Game::ItemDef;
    using PlacedItem = ZoneProtocol::Game::PlacedItem;
    using SynergyLink = ZoneProtocol::Game::SynergyLink;
    using CombatModifiers = ZoneProtocol::Game::CombatModifiers;
    using GridInventory = ZoneProtocol::Game::GridInventory;
}
