#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace DayDoom::Game {

enum class PropType {
    LOOT_CHEST = 1,      // Baú / Caixa de Munição
    WATER_BARREL = 2,    // Barril de Água (interagível)
    BOOKSHELF = 3,       // Estante de livros (obstáculo sólido)
    MEDKIT_BOX = 4,      // Caixa de primeiros socorros
    ZOMBIE_SPAWN = 5,    // Ponto de spawn de infectados
    VEHICLE_WRECK = 6,   // Carcaça de veículo urbano
    CAMPFIRE = 7         // Fogueira de sobrevivente
};

const char* getPropTypeName(PropType type);

struct MapProp {
    int floor = 0;       // Andar (0: Térreo, 1: 1º Andar, 2: Telhado)
    int x = 0;           // Coordenada Grid X (0..63)
    int y = 0;           // Coordenada Grid Y (0..63)
    float yaw = 0.0f;    // Rotação em graus (0, 90, 180, 270)
    PropType type = PropType::LOOT_CHEST;
};

struct PrefabTemplate {
    std::string name;
    int width = 0;
    int height = 0;
    std::vector<int> wallGrid;   // width * height (IDs de parede)
    std::vector<int> floorGrid;  // width * height (IDs de piso)
    std::vector<MapProp> props;  // Props internos do prefab

    bool saveToFile(const std::string& filepath) const;
    bool loadFromFile(const std::string& filepath);

    // Helpers para criar prefabs canônicos
    static PrefabTemplate createPoliceStation();
    static PrefabTemplate createHospital();
    static PrefabTemplate createBlueHangar();
    static PrefabTemplate createUShapeHouse();
    static PrefabTemplate createGuardPost();
};

} // namespace DayDoom::Game
