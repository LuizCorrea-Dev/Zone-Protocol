#include "GridInventory.hpp"
#include "../../engine/MathDef.hpp"
#include "../../engine/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace ZoneProtocol::Game {

static const uint8_t INV_GLYPH_5X7[128][7] = {
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}, {0,0,0,0,0,0,0},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // 32 ' '
    {0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00}, // 33 '!'
    {0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00}, // 34 '"'
    {0x0A, 0x1F, 0x0A, 0x0A, 0x1F, 0x0A, 0x00}, // 35 '#'
    {0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04}, // 36 '$'
    {0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03}, // 37 '%'
    {0x08, 0x14, 0x14, 0x08, 0x15, 0x12, 0x0D}, // 38 '&'
    {0x04, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00}, // 39 '\''
    {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02}, // 40 '('
    {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08}, // 41 ')'
    {0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00}, // 42 '*'
    {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00}, // 43 '+'
    {0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x04}, // 44 ','
    {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}, // 45 '-'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06}, // 46 '.'
    {0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00}, // 47 '/'
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, // 48 '0'
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 49 '1'
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, // 50 '2'
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, // 51 '3'
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, // 52 '4'
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, // 53 '5'
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, // 54 '6'
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // 55 '7'
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, // 56 '8'
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, // 57 '9'
    {0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x00}, // 58 ':'
    {0x00, 0x06, 0x06, 0x00, 0x06, 0x06, 0x04}, // 59 ';'
    {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02}, // 60 '<'
    {0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00}, // 61 '='
    {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08}, // 62 '>'
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04}, // 63 '?'
    {0x0E, 0x11, 0x01, 0x0D, 0x15, 0x15, 0x0E}, // 64 '@'
    {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // 65 'A'
    {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, // 66 'B'
    {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, // 67 'C'
    {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}, // 68 'D'
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, // 69 'E'
    {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}, // 70 'F'
    {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}, // 71 'G'
    {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // 72 'H'
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 73 'I'
    {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}, // 74 'J'
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, // 75 'K'
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, // 76 'L'
    {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}, // 77 'M'
    {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}, // 78 'N'
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // 79 'O'
    {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, // 80 'P'
    {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}, // 81 'Q'
    {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, // 82 'R'
    {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}, // 83 'S'
    {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // 84 'T'
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // 85 'U'
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}, // 86 'V'
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}, // 87 'W'
    {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, // 88 'X'
    {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}, // 89 'Y'
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, // 90 'Z'
    {0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E}, // 91 '['
    {0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00}, // 92 '\\'
    {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E}, // 93 ']'
    {0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00}, // 94 '^'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}  // 95 '_'
};

GridInventory::GridInventory() {
    rebuildGridArrays();
    setupInitialLoadout();
}

void GridInventory::setupInitialLoadout() {
    m_items.clear();
    m_nextInstanceId = 1;
    rebuildGridArrays();

    // 1. Fuzil AK-74M (ocupa exatamente 4x2 slots na posição [0,0])
    ItemDef ak74 = { 1, "AK-74M", ItemCategory::WEAPON, 4, 2, SynergyElement::NONE, 25.0f, 0.0f, 1 };
    addItem(ak74, 0, 0, 1, false, false);

    // 2. Artefato Radioativo (1x1) na posição [4,0] (colado à direita da AK-74!)
    ItemDef radOrb = { 10, "ISOTOPO ZARIA", ItemCategory::ARTIFACT, 1, 1, SynergyElement::RAD_GREEN, 0.0f, 0.30f, 1 };
    addItem(radOrb, 4, 0, 1, false, false);

    // 3. Artefato Criogênico (1x1) na posição [5,0] (colado ao Isotopo Zaria)
    ItemDef cryoOrb = { 12, "CRISTAL CRIO", ItemCategory::ARTIFACT, 1, 1, SynergyElement::CRYO_BLUE, 0.0f, 0.20f, 1 };
    addItem(cryoOrb, 5, 0, 1, false, false);

    // 4. Artefato Elétrico (1x1) na posição [4,1] (colado à AK-74 E ao Isotopo Zaria -> Ressonância!)
    ItemDef elecOrb = { 11, "NUCLEO SPARK", ItemCategory::ARTIFACT, 1, 1, SynergyElement::ELEC_PURPLE, 0.0f, 0.25f, 1 };
    addItem(elecOrb, 4, 1, 1, false, false);

    // 5. Kit Médico Branco (1x1) na posição [5,1] (conectado aos artefatos!)
    ItemDef medkit = { 20, "KIT MEDICO", ItemCategory::MEDICAL, 1, 1, SynergyElement::NONE, 45.0f, 0.0f, 1 };
    addItem(medkit, 5, 1, 1, false, false);

    // 6. Caixa de Munição 5.45mm (1x1) na posição [0,2]
    ItemDef ammoBox = { 30, "5.45MM X30", ItemCategory::AMMO, 1, 1, SynergyElement::NONE, 30.0f, 0.0f, 60 };
    addItem(ammoBox, 0, 2, 30, false, false);

    // 7. Atadura Militar (1x1) na posição [1,2]
    ItemDef bandage = { 21, "ATADURA", ItemCategory::MEDICAL, 1, 1, SynergyElement::NONE, 20.0f, 0.0f, 5 };
    addItem(bandage, 1, 2, 2, false, false);

    // 8. Lata de Ração (1x1) na posição [2,2]
    ItemDef ration = { 40, "RACAO URSS", ItemCategory::FOOD, 1, 1, SynergyElement::NONE, 35.0f, 0.0f, 3 };
    addItem(ration, 2, 2, 1, false, false);

    // 9. Artefato raro / Orbe guardado no inventário (1x1) na posição [6,1]
    ItemDef safeArt = { 13, "ORBE COMPASSO", ItemCategory::ARTIFACT, 1, 1, SynergyElement::RAD_GREEN, 0.0f, 0.35f, 1 };
    addItem(safeArt, 6, 1, 1, false, false);

    char synBuf[160];
    std::snprintf(synBuf, sizeof(synBuf),
        "[GridInventory] Initial Loadout Ready -> %d items (%d in SafePocket 2x2) | Active Synergy Links: %d | DmgMult: x%.2f | FireRate: x%.2f | Ricochets: %d",
        static_cast<int>(m_items.size()), getSafePocketItemCount(), static_cast<int>(m_links.size()),
        m_modifiers.damageMultiplier, m_modifiers.fireRateMultiplier, m_modifiers.electricRicochets);
    ZoneProtocol::Engine::Logger::info(synBuf, "INVENTORY");
}

void GridInventory::rebuildGridArrays() {
    for (int y = 0; y < BRIEFCASE_ROWS; ++y)
        for (int x = 0; x < BRIEFCASE_COLS; ++x)
            m_mainGrid[y][x] = 0;

    for (int y = 0; y < SAFE_POCKET_ROWS; ++y)
        for (int x = 0; x < SAFE_POCKET_COLS; ++x)
            m_safeGrid[y][x] = 0;

    for (const auto& item : m_items) {
        int w = item.getActualWidth();
        int h = item.getActualHeight();
        for (int dy = 0; dy < h; ++dy) {
            for (int dx = 0; dx < w; ++dx) {
                if (item.inSafePocket) {
                    if (item.gridY + dy < SAFE_POCKET_ROWS && item.gridX + dx < SAFE_POCKET_COLS)
                        m_safeGrid[item.gridY + dy][item.gridX + dx] = item.instanceId;
                } else {
                    if (item.gridY + dy < BRIEFCASE_ROWS && item.gridX + dx < BRIEFCASE_COLS)
                        m_mainGrid[item.gridY + dy][item.gridX + dx] = item.instanceId;
                }
            }
        }
    }
}

bool GridInventory::canPlaceItem(const ItemDef& def, int gx, int gy, bool rotated, bool safePocket, int ignoreInstanceId) const {
    int w = rotated ? def.height : def.width;
    int h = rotated ? def.width : def.height;

    int maxCols = safePocket ? SAFE_POCKET_COLS : BRIEFCASE_COLS;
    int maxRows = safePocket ? SAFE_POCKET_ROWS : BRIEFCASE_ROWS;

    if (gx < 0 || gy < 0 || (gx + w) > maxCols || (gy + h) > maxRows) {
        return false;
    }

    for (int dy = 0; dy < h; ++dy) {
        for (int dx = 0; dx < w; ++dx) {
            int occupant = safePocket ? m_safeGrid[gy + dy][gx + dx] : m_mainGrid[gy + dy][gx + dx];
            if (occupant != 0 && occupant != ignoreInstanceId) {
                return false;
            }
        }
    }
    return true;
}

bool GridInventory::addItem(const ItemDef& def, int gx, int gy, int count, bool rotated, bool safePocket) {
    if (!canPlaceItem(def, gx, gy, rotated, safePocket)) {
        return false;
    }

    PlacedItem item;
    item.instanceId = m_nextInstanceId++;
    item.def = def;
    item.gridX = gx;
    item.gridY = gy;
    item.stackCount = std::min(count, std::max(1, def.maxStack));
    item.rotated = rotated;
    item.inSafePocket = safePocket;

    m_items.push_back(item);
    rebuildGridArrays();
    recalculateSynergies();
    return true;
}

bool GridInventory::addItemAuto(const ItemDef& def, int count) {
    // Tenta encaixar na Maleta 6x4 primeiro
    for (int r = 0; r < BRIEFCASE_ROWS; ++r) {
        for (int c = 0; c < BRIEFCASE_COLS; ++c) {
            if (addItem(def, c, r, count, false, false)) return true;
            if (addItem(def, c, r, count, true, false)) return true;
        }
    }
    // Se a Maleta 6x4 estiver cheia, tenta no Safe Pocket 2x2
    for (int r = 0; r < SAFE_POCKET_ROWS; ++r) {
        for (int c = 0; c < SAFE_POCKET_COLS; ++c) {
            if (addItem(def, c, r, count, false, true)) return true;
        }
    }
    return false;
}

bool GridInventory::removeItemByInstance(int instanceId) {
    auto it = std::remove_if(m_items.begin(), m_items.end(),
        [instanceId](const PlacedItem& item) { return item.instanceId == instanceId; });

    if (it != m_items.end()) {
        m_items.erase(it, m_items.end());
        if (m_selectedInstanceId == instanceId) m_selectedInstanceId = -1;
        rebuildGridArrays();
        recalculateSynergies();
        return true;
    }
    return false;
}

bool GridInventory::moveItem(int instanceId, int newGx, int newGy, bool newRotated, bool toSafePocket) {
    for (auto& item : m_items) {
        if (item.instanceId == instanceId) {
            if (!canPlaceItem(item.def, newGx, newGy, newRotated, toSafePocket, instanceId)) {
                return false;
            }
            item.gridX = newGx;
            item.gridY = newGy;
            item.rotated = newRotated;
            item.inSafePocket = toSafePocket;

            rebuildGridArrays();
            recalculateSynergies();
            char mvBuf[160];
            std::snprintf(mvBuf, sizeof(mvBuf),
                "[GridInventory] Moved '%s' (#%d) to %s[%d,%d] (Rot=%d) -> Active Links: %d | DmgMult: x%.2f | Ricochets: %d",
                item.def.name.c_str(), instanceId, toSafePocket ? "SafePocket" : "Briefcase6x4",
                newGx, newGy, newRotated ? 1 : 0, static_cast<int>(m_links.size()),
                m_modifiers.damageMultiplier, m_modifiers.electricRicochets);
            ZoneProtocol::Engine::Logger::info(mvBuf, "INVENTORY");
            return true;
        }
    }
    return false;
}

void GridInventory::clearMainBriefcaseOnDeath() {
    int beforeCount = static_cast<int>(m_items.size());
    m_items.erase(
        std::remove_if(m_items.begin(), m_items.end(),
            [](const PlacedItem& item) { return !item.inSafePocket; }),
        m_items.end()
    );
    m_selectedInstanceId = -1;
    rebuildGridArrays();
    recalculateSynergies();
    ZoneProtocol::Engine::Logger::warn(
        "[GridInventory] Operator K.I.A. -> Lost " + std::to_string(beforeCount - static_cast<int>(m_items.size())) +
        " items from 6x4 Briefcase; preserved " + std::to_string(m_items.size()) + " items in Safe Pocket 2x2.",
        "INVENTORY", __FILE__, __LINE__
    );
}

const PlacedItem* GridInventory::getItemAt(int gx, int gy, bool safePocket) const {
    int maxCols = safePocket ? SAFE_POCKET_COLS : BRIEFCASE_COLS;
    int maxRows = safePocket ? SAFE_POCKET_ROWS : BRIEFCASE_ROWS;
    if (gx < 0 || gy < 0 || gx >= maxCols || gy >= maxRows) return nullptr;

    int id = safePocket ? m_safeGrid[gy][gx] : m_mainGrid[gy][gx];
    if (id == 0) return nullptr;

    for (const auto& item : m_items) {
        if (item.instanceId == id) return &item;
    }
    return nullptr;
}

int GridInventory::getArtifactCount() const {
    int count = 0;
    for (const auto& item : m_items) {
        if (item.def.category == ItemCategory::ARTIFACT) count++;
    }
    return count;
}

int GridInventory::getSafePocketItemCount() const {
    int count = 0;
    for (const auto& item : m_items) {
        if (item.inSafePocket) count++;
    }
    return count;
}

bool GridInventory::areItemsAdjacent(const PlacedItem& a, const PlacedItem& b, int& outAx, int& outAy, int& outBx, int& outBy) const {
    // Itens no Bolso Seguro estão isolados por blindagem de chumbo (não geram sinergia ativa)
    if (a.inSafePocket || b.inSafePocket) return false;

    int aW = a.getActualWidth();
    int aH = a.getActualHeight();
    int bW = b.getActualWidth();
    int bH = b.getActualHeight();

    // Verifica se qualquer bloco de A toca ortogonalmente (distância de Manhattan == 1) qualquer bloco de B
    for (int ay = a.gridY; ay < a.gridY + aH; ++ay) {
        for (int ax = a.gridX; ax < a.gridX + aW; ++ax) {
            for (int by = b.gridY; by < b.gridY + bH; ++by) {
                for (int bx = b.gridX; bx < b.gridX + bW; ++bx) {
                    int dist = std::abs(ax - bx) + std::abs(ay - by);
                    if (dist == 1) {
                        outAx = ax; outAy = ay;
                        outBx = bx; outBy = by;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void GridInventory::recalculateSynergies() {
    m_links.clear();
    m_modifiers = CombatModifiers{};

    for (size_t i = 0; i < m_items.size(); ++i) {
        for (size_t j = i + 1; j < m_items.size(); ++j) {
            const auto& itemA = m_items[i];
            const auto& itemB = m_items[j];

            int ax, ay, bx, by;
            if (!areItemsAdjacent(itemA, itemB, ax, ay, bx, by)) continue;

            // Caso 1: Artefacto conectado diretamente a uma Arma
            if ((itemA.def.category == ItemCategory::ARTIFACT && itemB.def.category == ItemCategory::WEAPON) ||
                (itemB.def.category == ItemCategory::ARTIFACT && itemA.def.category == ItemCategory::WEAPON)) {

                const PlacedItem& art = (itemA.def.category == ItemCategory::ARTIFACT) ? itemA : itemB;
                m_links.push_back({ art.instanceId, (art.instanceId == itemA.instanceId ? itemB.instanceId : itemA.instanceId), ax, ay, bx, by, art.def.element });

                if (art.def.element == SynergyElement::RAD_GREEN) {
                    m_modifiers.damageMultiplier += art.def.synergyBonus;
                    m_modifiers.radiationBurnDPS += 8.0f;
                } else if (art.def.element == SynergyElement::ELEC_PURPLE) {
                    m_modifiers.fireRateMultiplier += art.def.synergyBonus;
                    m_modifiers.electricRicochets += 1;
                } else if (art.def.element == SynergyElement::CRYO_BLUE) {
                    m_modifiers.damageMultiplier += (art.def.synergyBonus * 0.5f);
                    m_modifiers.cryoSlowActive = true;
                }
            }
            // Caso 2: Dois Artefactos adjacentes entre si (Fusão / Amplificação de Nó estilo Balatro!)
            else if (itemA.def.category == ItemCategory::ARTIFACT && itemB.def.category == ItemCategory::ARTIFACT) {
                m_links.push_back({ itemA.instanceId, itemB.instanceId, ax, ay, bx, by, itemA.def.element });
                m_modifiers.damageMultiplier *= 1.15f;
                if (itemA.def.element == SynergyElement::ELEC_PURPLE || itemB.def.element == SynergyElement::ELEC_PURPLE) {
                    m_modifiers.electricRicochets += 1;
                }
                if (itemA.def.element == SynergyElement::CRYO_BLUE || itemB.def.element == SynergyElement::CRYO_BLUE) {
                    m_modifiers.cryoSlowActive = true;
                }
            }
            // Caso 3: Artefacto conectado a Suprimento Médico
            else if ((itemA.def.category == ItemCategory::ARTIFACT && itemB.def.category == ItemCategory::MEDICAL) ||
                     (itemB.def.category == ItemCategory::ARTIFACT && itemA.def.category == ItemCategory::MEDICAL)) {
                const PlacedItem& art = (itemA.def.category == ItemCategory::ARTIFACT) ? itemA : itemB;
                m_links.push_back({ itemA.instanceId, itemB.instanceId, ax, ay, bx, by, art.def.element });
                m_modifiers.medkitEfficiency += 0.35f; // +35% de cura nas ataduras/kits
            }
        }
    }
}

void GridInventory::rotateSelected() {
    if (m_selectedInstanceId != -1) {
        m_selectedPreviewRotated = !m_selectedPreviewRotated;
    }
}

namespace {

struct BriefcaseLayout {
    int chassisW;
    int chassisH;
    int panelX;
    int panelY;
    int gridStartX;
    int gridStartY;
    int cellSize;
};

BriefcaseLayout getBriefcaseLayout(int screenW, int screenH) {
    const int scale = (screenW >= 1600) ? 2 : 1;
    // Tamanho quadrado exato do slot na tela
    int cellSize = (screenW * 56) / 1280;
    if (cellSize < 36) cellSize = 36;

    // A textura UI_BRIEFCASE_CHASSIS tem 1188x920.
    // A grade 8x4 na textura mede 1040x520 (cada célula mede 130x130 px exatos).
    // X inicial da grade na textura = 68 px, Y inicial = 192 px.
    int chassisW = static_cast<int>(std::round(cellSize * (1188.0 / 130.0)));
    int chassisH = static_cast<int>(std::round(cellSize * (920.0 / 130.0)));

    int panelX = (screenW - chassisW) / 2;
    // Desloca ligeiramente para cima para acomodar o rodapé de sinergias fora da maleta
    int panelY = (screenH - chassisH) / 2 - 16 * scale;
    if (panelY < 24 * scale) panelY = 24 * scale;

    int gridStartX = panelX + static_cast<int>(std::round(chassisW * (68.0 / 1188.0)));
    int gridStartY = panelY + static_cast<int>(std::round(chassisH * (192.0 / 920.0)));

    return { chassisW, chassisH, panelX, panelY, gridStartX, gridStartY, cellSize };
}

} // namespace

void GridInventory::handleMouseClick(int mouseX, int mouseY, int screenW, int screenH, bool rightClick) {
    if (!m_isOpen) return;

    if (rightClick) {
        rotateSelected();
        return;
    }

    auto layout = getBriefcaseLayout(screenW, screenH);

    // Verifica clique na grade principal 8x4 perfeitamente alinhada aos quadrados da arte da maleta
    for (int r = 0; r < BRIEFCASE_ROWS; ++r) {
        for (int c = 0; c < BRIEFCASE_COLS; ++c) {
            int sx = layout.gridStartX + c * layout.cellSize;
            int sy = layout.gridStartY + r * layout.cellSize;
            if (mouseX >= sx && mouseX < sx + layout.cellSize && mouseY >= sy && mouseY < sy + layout.cellSize) {
                if (m_selectedInstanceId == -1) {
                    const PlacedItem* clicked = getItemAt(c, r, false);
                    if (clicked) {
                        m_selectedInstanceId = clicked->instanceId;
                        m_selectedPreviewRotated = clicked->rotated;
                    }
                } else {
                    if (moveItem(m_selectedInstanceId, c, r, m_selectedPreviewRotated, false)) {
                        m_selectedInstanceId = -1;
                    } else {
                        const PlacedItem* other = getItemAt(c, r, false);
                        if (other && other->instanceId == m_selectedInstanceId) {
                            m_selectedInstanceId = -1;
                        }
                    }
                }
                return;
            }
        }
    }

    m_selectedInstanceId = -1;
}

Engine::SpriteRect GridInventory::getSpriteForItemDef(int defId, ItemCategory cat, SynergyElement elem) const {
    using namespace Engine::AtlasCoords;
    switch (defId) {
        case 1:  return ITEM_AK74;
        case 10: return ITEM_ART_RAD_GREEN;
        case 11: return ITEM_ART_ELEC_PURPLE;
        case 12: return ITEM_ART_CRYO_BLUE;
        case 13: return ITEM_ART_COMPASS;
        case 14: return ITEM_ART_YELLOW;
        case 15: return ITEM_ART_TEAL;
        case 20: return ITEM_MEDKIT;
        case 21: return ITEM_BANDAGE;
        case 30: return ITEM_AMMO_15;
        case 40: return ITEM_CANNED_FOOD;
        default: break;
    }
    if (cat == ItemCategory::WEAPON)   return ITEM_AK74;
    if (cat == ItemCategory::MEDICAL)  return ITEM_MEDKIT;
    if (cat == ItemCategory::AMMO)     return ITEM_AMMO_10;
    if (cat == ItemCategory::FOOD)     return ITEM_RATION;
    if (cat == ItemCategory::ARTIFACT) {
        if (elem == SynergyElement::ELEC_PURPLE) return ITEM_ART_ELEC_PURPLE;
        if (elem == SynergyElement::CRYO_BLUE)   return ITEM_ART_CRYO_BLUE;
        return ITEM_ART_RAD_GREEN;
    }
    return ITEM_SUPPLY_CASE;
}

void GridInventory::drawChar(uint32_t* buf, int sw, int sh, int x, int y, char c, uint32_t color, int scale) const {
    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    uint8_t uc = static_cast<uint8_t>(c);
    if (uc >= 128) return;

    for (int row = 0; row < 7; ++row) {
        uint8_t rowBits = INV_GLYPH_5X7[uc][row];
        if (rowBits == 0) continue;

        for (int col = 0; col < 5; ++col) {
            if ((rowBits >> (4 - col)) & 1) {
                for (int sy = 0; sy < scale; ++sy) {
                    for (int sx = 0; sx < scale; ++sx) {
                        int px = x + col * scale + sx;
                        int py = y + row * scale + sy;
                        if (px >= 0 && px < sw && py >= 0 && py < sh) {
                            buf[py * sw + px] = color;
                        }
                    }
                }
            }
        }
    }
}

void GridInventory::drawText(uint32_t* buf, int sw, int sh, int x, int y, const std::string& text, uint32_t color, int scale) const {
    int curX = x;
    for (char c : text) {
        drawChar(buf, sw, sh, curX, y, c, color, scale);
        curX += 6 * scale;
    }
}

void GridInventory::drawRect(uint32_t* buf, int sw, int sh, int x, int y, int w, int h, uint32_t color) const {
    int x1 = std::clamp(x, 0, sw);
    int y1 = std::clamp(y, 0, sh);
    int x2 = std::clamp(x + w, 0, sw);
    int y2 = std::clamp(y + h, 0, sh);
    uint8_t alpha = (color >> 24) & 0xFF;

    for (int py = y1; py < y2; ++py) {
        for (int px = x1; px < x2; ++px) {
            if (alpha >= 250) {
                buf[py * sw + px] = color | 0xFF000000u;
            } else if (alpha > 0) {
                uint32_t bg = buf[py * sw + px];
                uint32_t invA = 255 - alpha;
                uint32_t r = (((bg >> 16) & 0xFF) * invA + ((color >> 16) & 0xFF) * alpha) >> 8;
                uint32_t g = (((bg >> 8)  & 0xFF) * invA + ((color >> 8)  & 0xFF) * alpha) >> 8;
                uint32_t b = ((bg & 0xFF)         * invA + (color & 0xFF)         * alpha) >> 8;
                buf[py * sw + px] = 0xFF000000u | (r << 16) | (g << 8) | b;
            }
        }
    }
}

void GridInventory::drawBorder(uint32_t* buf, int sw, int sh, int x, int y, int w, int h, uint32_t color) const {
    drawRect(buf, sw, sh, x, y, w, 1, color);
    drawRect(buf, sw, sh, x, y + h - 1, w, 1, color);
    drawRect(buf, sw, sh, x, y, 1, h, color);
    drawRect(buf, sw, sh, x + w - 1, y, 1, h, color);
}

void GridInventory::drawGlowingLine(uint32_t* buf, int sw, int sh, int x0, int y0, int x1, int y1, uint32_t color) const {
    if (!buf || sw <= 0 || sh <= 0) return;

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int maxSteps = dx + dy + 2;

    uint32_t glowOuter = (color & 0x00FFFFFFu) | 0x66000000u;
    uint32_t coreWhite = 0xFFFFFFFFu;

    for (int step = 0; step < maxSteps; ++step) {
        for (int oy = -2; oy <= 2; ++oy) {
            for (int ox = -2; ox <= 2; ++ox) {
                int px = x0 + ox;
                int py = y0 + oy;
                if (px < 0 || px >= sw || py < 0 || py >= sh) continue;
                int d = std::abs(ox) + std::abs(oy);
                if (d == 0) {
                    buf[py * sw + px] = coreWhite;
                } else if (d == 1) {
                    buf[py * sw + px] = color | 0xFF000000u;
                } else if (d == 2) {
                    drawRect(buf, sw, sh, px, py, 1, 1, glowOuter);
                }
            }
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

void GridInventory::render(
    uint32_t* screenBuffer,
    int screenWidth,
    int screenHeight,
    int mouseX,
    int mouseY,
    const Engine::SpriteAtlas* uiAtlas
) {
    if (!m_isOpen) return;

    const int scale = (screenWidth >= 1600) ? 2 : 1;
    auto layout = getBriefcaseLayout(screenWidth, screenHeight);

    // 1. TÍTULO TÁTICO DA MALETA (TOTALMENTE FORA DA IMAGEM DA MALETA, ACIMA DO CHASSI)
    drawText(screenBuffer, screenWidth, screenHeight, layout.panelX + 4 * scale, layout.panelY - 14 * scale, "MALETA DE SINERGIA [8X5] - CLIQUE P/ MOVER", 0xFF78FF96, scale);

    // 1.5. Fundo Tático dos Slots da Maleta (10% de transparência sutil, 90% opaco / 0xEE0D1311)
    // Cobre todo o vão interior da maleta para impedir que o bunker ou o chão fiquem vazando por trás dos itens
    int gridAreaW = layout.cellSize * BRIEFCASE_COLS;
    int gridAreaH = layout.cellSize * BRIEFCASE_ROWS;
    drawRect(screenBuffer, screenWidth, screenHeight, layout.gridStartX, layout.gridStartY, gridAreaW, gridAreaH, 0xEE0D1311);

    // 2. Chassi Metálico da Maleta (ui_briefcase_items.png)
    if (uiAtlas && uiAtlas->isLoaded()) {
        uiAtlas->drawSprite(
            screenBuffer, screenWidth, screenHeight,
            Engine::AtlasCoords::UI_BRIEFCASE_CHASSIS,
            layout.panelX, layout.panelY,
            layout.chassisW, layout.chassisH,
            1.0f
        );
    } else {
        drawRect(screenBuffer, screenWidth, screenHeight, layout.panelX, layout.panelY, layout.chassisW, layout.chassisH, 0xF012181B);
        drawBorder(screenBuffer, screenWidth, screenHeight, layout.panelX, layout.panelY, layout.chassisW, layout.chassisH, 0xFF3E4E42);
    }

    // 3. Hover sobre células vazias (a arte da maleta já traz a grade desenhada com acabamento de metal oxidado)
    for (int r = 0; r < BRIEFCASE_ROWS; ++r) {
        for (int c = 0; c < BRIEFCASE_COLS; ++c) {
            int slotX = layout.gridStartX + c * layout.cellSize;
            int slotY = layout.gridStartY + r * layout.cellSize;

            bool isHovered = (mouseX >= slotX && mouseX < slotX + layout.cellSize &&
                              mouseY >= slotY && mouseY < slotY + layout.cellSize);

            if (isHovered && m_mainGrid[r][c] == 0) {
                drawBorder(screenBuffer, screenWidth, screenHeight, slotX, slotY, layout.cellSize, layout.cellSize, 0xFF50FF64);
            }
        }
    }

    // 4. Desenha os Itens posicionados na Maleta 8x4 perfeitamente alinhados aos quadrados da textura
    for (const auto& item : m_items) {
        int itemScreenX = layout.gridStartX + item.gridX * layout.cellSize;
        int itemScreenY = layout.gridStartY + item.gridY * layout.cellSize;
        int itemPixelW  = item.getActualWidth() * layout.cellSize;
        int itemPixelH  = item.getActualHeight() * layout.cellSize;

        bool isSelected = (item.instanceId == m_selectedInstanceId);
        if (isSelected) {
            drawRect(screenBuffer, screenWidth, screenHeight, itemScreenX + 1, itemScreenY + 1, itemPixelW - 2, itemPixelH - 2, 0x88285838);
        }

        if (uiAtlas && uiAtlas->isLoaded()) {
            Engine::SpriteRect spriteRect = getSpriteForItemDef(item.def.id, item.def.category, item.def.element);
            int pad = 2 * scale;
            int targetW = itemPixelW - pad * 2;
            int targetH = itemPixelH - pad * 2;

            // Preserva proporção natural do sprite para evitar qualquer distorção visual!
            float srcAspect = static_cast<float>(spriteRect.w) / static_cast<float>(std::max(1, spriteRect.h));
            float dstAspect = static_cast<float>(targetW) / static_cast<float>(std::max(1, targetH));
            int drawW = targetW;
            int drawH = targetH;
            if (srcAspect > dstAspect) {
                drawH = std::max(1, static_cast<int>(targetW / srcAspect));
            } else {
                drawW = std::max(1, static_cast<int>(targetH * srcAspect));
            }
            int drawX = itemScreenX + pad + (targetW - drawW) / 2;
            int drawY = itemScreenY + pad + (targetH - drawH) / 2;

            uiAtlas->drawSprite(
                screenBuffer, screenWidth, screenHeight,
                spriteRect,
                drawX, drawY,
                drawW, drawH,
                isSelected ? 1.35f : 1.05f
            );
        } else {
            drawRect(screenBuffer, screenWidth, screenHeight, itemScreenX + 3, itemScreenY + 3, itemPixelW - 6, itemPixelH - 6, 0xFF3A5A40);
        }

        uint32_t itemBorder = isSelected ? 0xFFFFFFFF :
            (item.def.element == SynergyElement::RAD_GREEN   ? 0xFF50FF64 :
            (item.def.element == SynergyElement::ELEC_PURPLE ? 0xFFD25AFF :
            (item.def.element == SynergyElement::CRYO_BLUE   ? 0xFF46D2FF : 0xFF68786A)));

        drawBorder(screenBuffer, screenWidth, screenHeight, itemScreenX, itemScreenY, itemPixelW, itemPixelH, itemBorder);

        // Nome curto ou contador de pilha no canto inferior do slot
        if (item.stackCount > 1) {
            char stBuf[16];
            std::snprintf(stBuf, sizeof(stBuf), "X%d", item.stackCount);
            drawText(screenBuffer, screenWidth, screenHeight, itemScreenX + 3 * scale, itemScreenY + itemPixelH - 7 * scale, stBuf, 0xFFFFEE88, scale);
        } else {
            std::string shortName = item.def.name.substr(0, std::min<size_t>(item.def.name.size(), item.getActualWidth() * 6));
            drawText(screenBuffer, screenWidth, screenHeight, itemScreenX + 2 * scale, itemScreenY + itemPixelH - 7 * scale, shortName, 0xFFD8E8D8, scale);
        }
    }

    // 5. Desenha os Raios de Conexão (Synergy Links) por cima dos itens conectados!
    for (const auto& link : m_links) {
        int x0 = layout.gridStartX + link.startGridX * layout.cellSize + (layout.cellSize / 2);
        int y0 = layout.gridStartY + link.startGridY * layout.cellSize + (layout.cellSize / 2);
        int x1 = layout.gridStartX + link.endGridX   * layout.cellSize + (layout.cellSize / 2);
        int y1 = layout.gridStartY + link.endGridY   * layout.cellSize + (layout.cellSize / 2);

        uint32_t beamColor = DayDoom::Engine::makeColor(80, 255, 100); // RAD_GREEN
        if (link.element == SynergyElement::ELEC_PURPLE)
            beamColor = DayDoom::Engine::makeColor(210, 90, 255);
        else if (link.element == SynergyElement::CRYO_BLUE)
            beamColor = DayDoom::Engine::makeColor(70, 210, 255);

        drawGlowingLine(screenBuffer, screenWidth, screenHeight, x0, y0, x1, y1, beamColor);
        // Nós energéticos nas extremidades
        drawRect(screenBuffer, screenWidth, screenHeight, x0 - 2 * scale, y0 - 2 * scale, 5 * scale, 5 * scale, beamColor);
        drawRect(screenBuffer, screenWidth, screenHeight, x1 - 2 * scale, y1 - 2 * scale, 5 * scale, 5 * scale, beamColor);
    }

    // 6. Barra Inferior de Telemetria de Sinergias (TOTALMENTE FORA DA IMAGEM DA MALETA, ABAIXO DO CHASSI)
    int footerY = layout.panelY + layout.chassisH + 6 * scale;
    int footerH = 26 * scale;
    drawRect(screenBuffer, screenWidth, screenHeight, layout.panelX, footerY, layout.chassisW, footerH, 0xEE0B100E);
    drawBorder(screenBuffer, screenWidth, screenHeight, layout.panelX, footerY, layout.chassisW, footerH, 0xFF3A5842);

    char modLine1[128];
    std::snprintf(
        modLine1, sizeof(modLine1),
        "SINERGIAS ATIVAS (%d LINKS): DANO X%.2f | CADENCIA X%.2f | RICOCHETES: %d",
        static_cast<int>(m_links.size()),
        m_modifiers.damageMultiplier,
        m_modifiers.fireRateMultiplier,
        m_modifiers.electricRicochets
    );
    drawText(screenBuffer, screenWidth, screenHeight, layout.panelX + 8 * scale, footerY + 4 * scale, modLine1, 0xFF50FF64, scale);

    char modLine2[128];
    std::snprintf(
        modLine2, sizeof(modLine2),
        "RADIACAO DPS: +%.0f/S | CURA MEDKIT: +%d%% | CRIO-SLOW: %s",
        m_modifiers.radiationBurnDPS,
        static_cast<int>((m_modifiers.medkitEfficiency - 1.0f) * 100.0f + 0.5f),
        m_modifiers.cryoSlowActive ? "ATIVO" : "OFF"
    );
    drawText(screenBuffer, screenWidth, screenHeight, layout.panelX + 8 * scale, footerY + 14 * scale, modLine2, 0xFF88D8FF, scale);
}

} // namespace ZoneProtocol::Game
