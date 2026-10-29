#pragma once

#include <cstdint>

namespace DayDoom::Game {

// ==============================================================================
// 1. DIMENSÕES OFICIAIS DA MALHA DA BACIA DE ZARIA (64x64)
// ==============================================================================

constexpr int MAP_GRID_COLS = 64;
constexpr int MAP_GRID_ROWS = 64;

// ==============================================================================
// 2. ELEMENTOS ESTRUTURAIS DA ZONA DE EXCLUSÃO (PAREDES, MUROS, PORTAS)
// ==============================================================================

enum UrbanElementType {
    TYPE_EMPTY = 0,             // Terreno livre (Floresta / Ruas / Clareiras)
    // --- PAREDES DE RUÍNAS E INSTALAÇÕES ---
    TYPE_WALL_BRICK = 1,        // Ruínas residenciais e alvenaria de Krastov
    TYPE_WALL_CONCRETE = 2,     // Concreto de instalações militares e búnqueres
    TYPE_WALL_METAL = 3,        // Armazéns industriais e galpões de Novo-Bryansk
    TYPE_CONTAINER_RED = 4,     // Contêiner de transporte vermelho
    TYPE_CONTAINER_BLU = 5,     // Contêiner de suprimentos azul
    TYPE_WINDOW_GLASS = 10,     // Janelas reforçadas de observação
    TYPE_WALL_HOSPITAL = 11,    // Ala médica / Enfermaria de quarentena
    TYPE_WATER_BARRIER = 12,    // Limite pantanoso da bacia
    TYPE_HANGAR_BLUE = 13,      // Galpão de maquinário pesado
    TYPE_POLICE_EMBLEM = 14,    // Posto de comando e controle perimetral

    // --- MUROS DIVISÓRIOS E PERIMETRAIS DA ZONA ---
    TYPE_MURO_SOVIET = 15,      // Muro de contenção perimetral PO-2
    TYPE_MURO_STONE = 16,       // Muro de alvenaria e rocha
    TYPE_FENCE_WIRE = 9,        // Cerca perimetral de arame farpado

    // --- PORTAS E PORTÕES TÁTICOS (Interação com 'F') ---
    TYPE_DOOR_WOOD = 6,         // Porta de madeira de abrigo
    TYPE_DOOR_SLIDING = 7,      // Porta corrediça de aço blindado
    TYPE_DOOR_HOSPITAL = 17,    // Porta dupla de contenção médica
    TYPE_GATE_LARGE = 8,        // Portão de ferro maciço da instalação

    // --- JANELAS DE TIRO E OBSERVAÇÃO ---
    TYPE_WINDOW_FIXED = 18,     // Janela fixa blindada
    TYPE_WINDOW_OPENABLE = 19,  // Janela tática de abertura dupla

    // --- ACESSOS VERTICAIS ---
    TYPE_STAIRS_CONC = 20       // Escada de concreto para subsolo / telhado
};

// Tipos de terreno da Bacia de Zaria
enum FloorType {
    FLOOR_ASPHALT = 0,          // Estradas e pistas da Zona
    FLOOR_SAND = 1,             // Solo orgânico da floresta (terra/musgo)
    FLOOR_WATER = 2,            // Pântano tóxico da bacia
    FLOOR_CONCRETE = 3,         // Lajes de concreto e pátios industriais
    FLOOR_WOOD = 4,             // Assoalho de abrigos de madeira
    FLOOR_RAILROAD = 5,         // Linha férrea de transporte militar
    FLOOR_STAIRS_UP = 6,        // Acesso ao nível superior
    FLOOR_STAIRS_DOWN = 7,      // Acesso ao búnquer subterrâneo
    FLOOR_EMPTY = 8             // Vazio
};

// Funções de categorização rápida
inline bool isStairsType(int type) {
    return (type == TYPE_STAIRS_CONC);
}

inline bool isDoorType(int type) {
    return (type == TYPE_DOOR_WOOD || type == TYPE_DOOR_SLIDING ||
            type == TYPE_DOOR_HOSPITAL || type == TYPE_GATE_LARGE);
}

inline bool isWindowType(int type) {
    return (type == TYPE_WINDOW_GLASS || type == TYPE_WINDOW_FIXED || type == TYPE_WINDOW_OPENABLE);
}

inline bool isMuroType(int type) {
    return (type == TYPE_MURO_SOVIET || type == TYPE_MURO_STONE || type == TYPE_FENCE_WIRE);
}

inline bool isWallType(int type) {
    return (type == TYPE_WALL_BRICK || type == TYPE_WALL_CONCRETE || type == TYPE_WALL_METAL ||
            type == TYPE_CONTAINER_RED || type == TYPE_CONTAINER_BLU || type == TYPE_WINDOW_GLASS ||
            type == TYPE_WALL_HOSPITAL || type == TYPE_HANGAR_BLUE || type == TYPE_POLICE_EMBLEM ||
            type == TYPE_WINDOW_FIXED);
}

// ==============================================================================
// 3. SETORES OFICIAIS DA BACIA DE ZARIA (ZARIA EXCLUSION ZONE)
// ==============================================================================

struct ZoneSectorInfo {
    int id;
    const char* code;
    const char* name;
    int threatLevel; // 1: Baixo, 2: Médio, 3: Extremo (Marco Zero)
};

constexpr ZoneSectorInfo g_ZariaSectors[] = {
    { 1, "SEC-04", "SETOR 04: PERIFERIA DE KRASTOV", 1 },
    { 2, "SEC-02", "SETOR 02: COMPLEXO INDUSTRIAL NOVO-BRYANSK", 2 },
    { 3, "SEC-07", "SETOR 07: INSTALACAO AEREA 11-B", 2 },
    { 4, "SEC-05", "SETOR 05: VILAREJO RESIDENCIAL DE KRASNOYE", 2 },
    { 5, "SEC-00", "MARCO ZERO: INSTALACAO SUBTERRANEA 'OBELISK'", 3 }
};

inline const char* getSectorNameByTier(int tier) {
    switch (tier) {
        case 1: return "SETOR 04: PERIFERIA DE KRASTOV";
        case 2: return "SETOR 02: COMPLEXO INDUSTRIAL NOVO-BRYANSK";
        case 3: return "MARCO ZERO: INSTALACAO SUBTERRANEA 'OBELISK'";
        default: return "BACIA DE ZARIA: ZONA DE EXCLUSAO";
    }
}

constexpr int16_t TAG_ARMORY_ACCESS_DOOR = 101;
constexpr int16_t TAG_EVAC_EXTRACTION_GATE = 102;

} // namespace DayDoom::Game
