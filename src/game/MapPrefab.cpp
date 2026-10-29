#include "MapPrefab.hpp"
#include "ZoneLocationsDef.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace DayDoom::Game {

const char* getPropTypeName(PropType type) {
    switch (type) {
        case PropType::LOOT_CHEST:     return "Bau de Loot";
        case PropType::WATER_BARREL:   return "Barril de Agua";
        case PropType::BOOKSHELF:      return "Estante Livros";
        case PropType::MEDKIT_BOX:     return "Caixa Medkit";
        case PropType::ZOMBIE_SPAWN:   return "Spawn de Afetados";
        case PropType::VEHICLE_WRECK:  return "Carcaca Veiculo";
        case PropType::CAMPFIRE:       return "Fogueira";
        default:                       return "Desconhecido";
    }
}

bool PrefabTemplate::saveToFile(const std::string& filepath) const {
    std::ofstream out(filepath);
    if (!out.is_open()) {
        std::cerr << "[PREFAB] Erro ao salvar prefab em: " << filepath << std::endl;
        return false;
    }

    out << "# DAYDOOM PREFAB v1\n";
    out << "NAME=" << name << "\n";
    out << "WIDTH=" << width << "\n";
    out << "HEIGHT=" << height << "\n";

    out << "[PROPS]\n";
    for (const auto& p : props) {
        out << p.floor << "," << p.x << "," << p.y << "," << p.yaw << "," << static_cast<int>(p.type) << "\n";
    }

    out << "[WALLS]\n";
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            out << wallGrid[y * width + x] << (x + 1 < width ? "," : "");
        }
        out << "\n";
    }

    out << "[FLOORS]\n";
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            out << floorGrid[y * width + x] << (x + 1 < width ? "," : "");
        }
        out << "\n";
    }

    return true;
}

bool PrefabTemplate::loadFromFile(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open()) {
        return false;
    }

    std::string line;
    std::string currentSection = "";

    wallGrid.clear();
    floorGrid.clear();
    props.clear();

    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[') {
            size_t endPos = line.find(']');
            if (endPos != std::string::npos) {
                currentSection = line.substr(1, endPos - 1);
            }
            continue;
        }

        if (currentSection.empty()) {
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string key = line.substr(0, eqPos);
                std::string val = line.substr(eqPos + 1);
                if (key == "NAME") name = val;
                else if (key == "WIDTH") width = std::stoi(val);
                else if (key == "HEIGHT") height = std::stoi(val);
            }
        } else if (currentSection == "PROPS") {
            std::stringstream ss(line);
            std::string item;
            std::vector<std::string> parts;
            while (std::getline(ss, item, ',')) {
                parts.push_back(item);
            }
            if (parts.size() >= 5) {
                MapProp p;
                p.floor = std::stoi(parts[0]);
                p.x = std::stoi(parts[1]);
                p.y = std::stoi(parts[2]);
                p.yaw = std::stof(parts[3]);
                p.type = static_cast<PropType>(std::stoi(parts[4]));
                props.push_back(p);
            }
        } else if (currentSection == "WALLS") {
            std::stringstream ss(line);
            std::string item;
            while (std::getline(ss, item, ',')) {
                if (!item.empty()) {
                    wallGrid.push_back(std::stoi(item));
                }
            }
        } else if (currentSection == "FLOORS") {
            std::stringstream ss(line);
            std::string item;
            while (std::getline(ss, item, ',')) {
                if (!item.empty()) {
                    floorGrid.push_back(std::stoi(item));
                }
            }
        }
    }

    return (width > 0 && height > 0 && !wallGrid.empty());
}

PrefabTemplate PrefabTemplate::createPoliceStation() {
    PrefabTemplate p;
    p.name = "Esquadra Policia";
    p.width = 10;
    p.height = 8;
    p.wallGrid.resize(p.width * p.height, TYPE_EMPTY);
    p.floorGrid.resize(p.width * p.height, FLOOR_WOOD);

    for (int y = 0; y < p.height; ++y) {
        for (int x = 0; x < p.width; ++x) {
            bool border = (x == 0 || x == p.width - 1 || y == 0 || y == p.height - 1);
            if (border) {
                p.wallGrid[y * p.width + x] = TYPE_WALL_CONCRETE;
            }
        }
    }
    // Brasão policial na frente
    p.wallGrid[0 * p.width + 4] = TYPE_POLICE_EMBLEM;
    p.wallGrid[0 * p.width + 5] = TYPE_POLICE_EMBLEM;
    // Porta de entrada
    p.wallGrid[(p.height - 1) * p.width + 4] = TYPE_DOOR_SLIDING;

    // Props internos
    MapProp chest;
    chest.floor = 0; chest.x = 2; chest.y = 2; chest.type = PropType::LOOT_CHEST;
    p.props.push_back(chest);

    MapProp shelf;
    shelf.floor = 0; shelf.x = 8; shelf.y = 2; shelf.type = PropType::BOOKSHELF;
    p.props.push_back(shelf);

    return p;
}

PrefabTemplate PrefabTemplate::createHospital() {
    PrefabTemplate p;
    p.name = "Hospital Central";
    p.width = 12;
    p.height = 10;
    p.wallGrid.resize(p.width * p.height, TYPE_EMPTY);
    p.floorGrid.resize(p.width * p.height, FLOOR_CONCRETE);

    for (int y = 0; y < p.height; ++y) {
        for (int x = 0; x < p.width; ++x) {
            bool border = (x == 0 || x == p.width - 1 || y == 0 || y == p.height - 1);
            if (border) {
                p.wallGrid[y * p.width + x] = TYPE_WALL_HOSPITAL;
            }
        }
    }
    // Porta dupla de emergência
    p.wallGrid[(p.height - 1) * p.width + 5] = TYPE_DOOR_HOSPITAL;
    p.wallGrid[(p.height - 1) * p.width + 6] = TYPE_DOOR_HOSPITAL;

    MapProp med;
    med.floor = 0; med.x = 3; med.y = 3; med.type = PropType::MEDKIT_BOX;
    p.props.push_back(med);

    MapProp barrel;
    barrel.floor = 0; barrel.x = 8; barrel.y = 3; barrel.type = PropType::WATER_BARREL;
    p.props.push_back(barrel);

    return p;
}

PrefabTemplate PrefabTemplate::createBlueHangar() {
    PrefabTemplate p;
    p.name = "Galpao Azul";
    p.width = 10;
    p.height = 12;
    p.wallGrid.resize(p.width * p.height, TYPE_EMPTY);
    p.floorGrid.resize(p.width * p.height, FLOOR_CONCRETE);

    for (int y = 0; y < p.height; ++y) {
        for (int x = 0; x < p.width; ++x) {
            bool border = (x == 0 || x == p.width - 1 || y == 0 || y == p.height - 1);
            if (border) {
                p.wallGrid[y * p.width + x] = TYPE_HANGAR_BLUE;
            }
        }
    }
    p.wallGrid[(p.height - 1) * p.width + 4] = TYPE_GATE_LARGE;
    p.wallGrid[(p.height - 1) * p.width + 5] = TYPE_GATE_LARGE;

    MapProp chest;
    chest.floor = 0; chest.x = 2; chest.y = 2; chest.type = PropType::LOOT_CHEST;
    p.props.push_back(chest);

    return p;
}

PrefabTemplate PrefabTemplate::createUShapeHouse() {
    PrefabTemplate p;
    p.name = "Casa em U";
    p.width = 8;
    p.height = 8;
    p.wallGrid.resize(p.width * p.height, TYPE_EMPTY);
    p.floorGrid.resize(p.width * p.height, FLOOR_WOOD);

    for (int y = 0; y < p.height; ++y) {
        for (int x = 0; x < p.width; ++x) {
            // U virado para o sul (aberto em y == p.height - 1 e centro)
            bool border = (x == 0 || x == p.width - 1 || y == 0);
            if (border) {
                p.wallGrid[y * p.width + x] = TYPE_WALL_BRICK;
            }
        }
    }
    // Entrada com porta de madeira
    p.wallGrid[0 * p.width + 4] = TYPE_DOOR_WOOD;

    MapProp barrel;
    barrel.floor = 0; barrel.x = 2; barrel.y = 2; barrel.type = PropType::WATER_BARREL;
    p.props.push_back(barrel);

    return p;
}

PrefabTemplate PrefabTemplate::createGuardPost() {
    PrefabTemplate p;
    p.name = "Guarita / Posto Guarda";
    p.width = 4;
    p.height = 4;
    p.wallGrid.resize(p.width * p.height, TYPE_EMPTY);
    p.floorGrid.resize(p.width * p.height, FLOOR_CONCRETE);

    for (int y = 0; y < p.height; ++y) {
        for (int x = 0; x < p.width; ++x) {
            bool border = (x == 0 || x == p.width - 1 || y == 0 || y == p.height - 1);
            if (border) {
                p.wallGrid[y * p.width + x] = TYPE_MURO_SOVIET;
            }
        }
    }
    p.wallGrid[(p.height - 1) * p.width + 1] = TYPE_DOOR_WOOD;

    MapProp med;
    med.floor = 0; med.x = 2; med.y = 2; med.type = PropType::MEDKIT_BOX;
    p.props.push_back(med);

    return p;
}

} // namespace DayDoom::Game
