# ☣️ ZONE PROTOCOL — Top-Down Tactical Extraction Roguelike & Inventory Grid Management

**Um roguelike tático de extração e sobrevivência em visão top-down, combinando iluminação dinâmica DDA, gerenciamento de inventário em grade por sinergia energética e desenvolvimento de base em abrigo subterrâneo.**

[Visão Geral](#vis%C3%A3o-geral) • [A Promessa do Game](#a-promessa-do-game) • [Lore & Ambientação](#lore--ambienta%C3%A7%C3%A3o) • [Pilares de Gameplay](#pilares-de-gameplay) • [Arquitetura & Engenharia](#arquitetura-t%C3%A9cnica--engenharia) • [Estrutura do Repositório](#estrutura-do-reposit%C3%B3rio) • [Compilação](#como-compilar-e-executar) • [Controles](#controles) • [Steamworks](#conte%C3%BAdo-para-a-p%C3%A1gina-coming-soon-na-steam-steamworks)

---

## 📖 Visão Geral

**ZONE PROTOCOL** é um jogo indie de extração e ação tática em Pixel Art 2D, desenvolvido em **C++17** e **SDL2** com foco primário em alta rejogabilidade, resposta tática rápida e verificação nativa para **Steam Deck (16:10 1280x800 & 16:9 @ 60 FPS)**.

O jogo une a tensão sufocante de exploração no escuro com lanternas e *Fog of War* à dopamina matemática de otimização de espaço, onde artefatos radioativos se conectam fisicamente a armas e suprimentos em uma maleta tática energizada.

---

## 🎯 A Promessa do Game

1. **Extração de Alto Risco sem Frustração Cega:** Incursões intensas contra o relógio (*Extraction Timer*). Se você morrer, não perde todo o progresso: o **Container Blindado (Safe Pocket 2x2)** garante os itens mais valiosos, enquanto os Pontos de Dados da Zona financiam evoluções permanentes no seu Bunker.
2. **Maleta de Sinergias (Grade Tática Energizada):** O inventário não é apenas espaço de armazenamento. É uma matriz de conexões onde a posição dos artefatos altera o comportamento das armas (ex: evoluções por fusão de fuzis com eletricidade, criogenia ou radiação).
3. **Loop Duplo Viciante:**  
  * **Na Zona (Ação/Risco):** Sobreviva ao escuro, gerencie recursos escassos e extraia antes do fechamento do setor.
  * **No Bunker (Gestão/Evolução):** Venda artefatos no mercado negro, recarregue o gerador, desacontamine na enfermaria, expanda a bancada de armas e infiltre em novos setores.
4. **Desempenho e Portabilidade Absoluta:** Motor leve com renderização customizada por software escalada via GPU, garantindo **60 FPS cravados no Steam Deck e PCs de entrada**.

---

## ☣️ Lore & Ambientação

| Parâmetro        | Detalhe                                                                                       |
| ---------------- | --------------------------------------------------------------------------------------------- |
| **Ano**          | **2029** (Anos após o Grande Colapso Anômalo)                                                 |
| **Localização**  | **Setor 04: Periferia de Krastov** (Zona de Exclusão de Krastov, Leste Europeu)               |
| **Protagonista** | **O Operador / Stalker** (Recrutado no Bunker Subterrâneo)                                    |
| **Condições**    | Escuridão extrema, névoa radioativa ascensional, mutações agressivas e tempestades magnéticas |

### A História

Após o acidente e fechamento definitivo do Complexo Energético de Krastov, a região circundante foi isolada sob quarentena militar estrita. Fenômenos conhecidos apenas como **Anomalias** passaram a sintetizar estruturas cristalinas radioativas de alto valor científico e financeiro — os **Artefatos**.

Como um **Operador** operando a partir de um abrigo subterrâneo clandestino (o Bunker), você aceita contratos de incursão para explorar ruínas industriais, galpões e complexos militares abandonados. Cada Operador recrutado possui **Traits (Traços Únicos)** genéticos ou psicológicos (ex: *Paranoico*, *Batedor Leve*, *Hemofílico*). Se o seu sobrevivente tombar na Zona, o sinal é perdido, e o próximo recruta assume o legado e os recursos acumulados na base.

---

## 🎮 Pilares de Gameplay

* **Iluminação DDA & Som no Escuro:** O feixe da lanterna revela terrenos e ameaças em tempo real usando algoritmo de varredura DDA 2D. Criaturas fora do cone de visão exibem apenas olhos vermelhos brilhantes na escuridão (*Fog of War*) e reagem a disparos.
* **Grade de Sinergias de Artefatos:** Conecte nós energéticos luminosos (linhas verdes, azuis e roxas) entre artefatos e armas na maleta 6x4 para ativar multiplicadores de dano, cadência, ricochete elétrico em cadeia e dano radioativo por segundo.
* **Geração Procedural Híbrida (*Hybrid Chunk Grid*):** O mapa de 64x64 da Zona combina blocos estruturados (prédios, hangares, muros soviéticos e caixas de suprimentos militares) com terrenos contaminados, garantindo um layout tático novo a cada run.
* **Controles Twin-Stick:** Movimentação precisa via WASD/Analógico esquerdo com mira livre em 360° guiada pelo cursor do mouse ou analógico direito.

---

## ⚙️ Arquitetura Técnica & Engenharia

O projeto foi construído sobre uma arquitetura desacoplada em **3 Camadas Estritas** em C++17:

* **Camada 2: O Jogo (Zone Protocol):** `WorldView2D`, `ZoneMap`, `CombatSystem`, `GridInventory`, `Player`, `HUD`.
* **Camada 1: Engine 2D & Telemetria:** `LightCaster2D` (DDA 2D), `SpriteAtlas`, `TextureManager`, `Logger` (`.RPT`, `error.log`, `script.log` + MiniDump `.dmp`), `VFS`, `MathDef`, `Camera`.
* **Camada 0: HAL (SDL2 Platform):** `Platform Wrapper` (640x400 / 1280x800 16:10 & 480x270 16:9 framebuffer), `Input Mapper` (WASD + Mouse 360°).

### Especificações Técnicas

* **Linguagem:** C++17 Nativo.
* **API Gráfica:** SDL2 (Renderização por Software em `uint32_t screenBuffer` linear escalada via GPU com filtro *Nearest-Neighbor*).
* **Resolução Canônica:** 640x400 escalado para 1280x800 Widescreen (16:10 Steam Deck) e 480x270 / 720p / 1080p (16:9) sem perda de proporção.
* **Sistemas de Build:** CMake 3.16+ cross-platform.

---

## 📁 Estrutura do Repositório

```
ZONE_PROTOCOL/
├── CMakeLists.txt              # Build unificado CMake em C++17
├── LICENSE                     # Licença Proprietária (All Rights Reserved)
├── README.md                   # Documentação do projeto
├── STEAMWORKS.md               # Textos, BBCode, Tags e Requisitos para a página Steam
├── .gitignore                  # Regras de build, logs e binários
├── run_1280x800_16_10.bat      # Lançador rápido 1280x800 (16:10 Steam Deck / PC)
├── run_widescreen_16_9.bat     # Lançador rápido PC Widescreen (16:9)
├── assets/
│   └── sprites/                # Folhas de SpriteAtlas 2D, UI da Maleta 6x4, Bunker e Loading Screen
└── src/
    ├── main.cpp                # Loop principal (60 FPS cravados) e Máquina de Estados
    ├── hal/                    # Camada 0: Abstração de Hardware
    │   ├── Platform.hpp/.cpp   # Gerenciador de janela SDL2, VSync e framebuffer
    │   └── Input.hpp/.cpp      # Mapeamento de WASD e posição 360° do mouse
    ├── engine/                 # Camada 1: Engine 2D, Iluminação & Diagnóstico
    │   ├── LightCaster2D.hpp/.cpp # Algoritmo DDA para iluminação da lanterna e sombras
    │   ├── SpriteAtlas.hpp/.cpp   # Carregador WIC de folhas de sprites com Chroma Key
    │   ├── Logger.hpp/.cpp        # Telemetria tripla (.RPT, error.log, script.log) e Crash Handler
    │   ├── VFS.hpp/.cpp           # Sistema Virtual de Arquivos
    │   ├── MathDef.hpp            # Vetores 2D e funções de iluminação/névoa
    │   ├── Texture.hpp/.cpp       # Texturas procedurais e mapeamento de superfícies
    │   └── Camera.hpp             # Câmera Top-Down e direção do feixe de luz
    ├── objetos/                # Entidades Físicas
    │   └── Porta.hpp/.cpp      # Portas giratórias com colisão e bloqueio de luz
    └── game/                   # Camada 2: Gameplay do Zone Protocol
        ├── ZoneMap.hpp/.cpp       # Gerador de mapa procedural por Chunks 16x16
        ├── Player.hpp/.cpp        # Sobrevivência, movimentação e colisão com deslizamento
        ├── CombatSystem.hpp/.cpp  # Mutantes, projéteis, caixas de loot e arcos elétricos
        └── UI/
            ├── GridInventory.hpp/.cpp # Maleta Tática 6x4, Safe Pocket 2x2 e sinergias
            ├── WorldView2D.hpp/.cpp   # Renderizador principal do mundo Top-Down
            └── HUD.hpp/.cpp           # Painel militar, Loading Screen, Tela de Morte e Bunker Hub
```

---

## 💻 Como Compilar e Executar

### Pré-requisitos

* Compilador C++17 (MSVC no Windows, GCC 9+ ou Clang 10+ no Linux/macOS).
* CMake 3.16 ou superior.
* SDL2 (incluída em `external/SDL2` ou baixada automaticamente via CMake `FetchContent`).

### Windows (Visual Studio / MSBuild)

```powershell
# 1. Obtenha o repositório
cd "Zone Protocol"

# 2. Gere o projeto com CMake
cmake -B build -S .

# 3. Compile em modo Release
cmake --build build --config Release

# 4. Execute
./ZoneProtocol_v013.exe
```

### Linux / macOS

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/ZoneProtocol
```

---

## 🕹️ Controles

| Comando                     | Ação                                                      |
| --------------------------- | --------------------------------------------------------- |
| **W / A / S / D**           | Movimentação em 8 direções (Norte, Sul, Leste, Oeste)     |
| **Movimento do Mouse**      | Mirar lanterna e arma em 360°                             |
| **Botão Esquerdo do Mouse** | Disparar arma equipada / Interagir com botões da UI       |
| **Shift Esquerdo**          | Correr (Consome Stamina)                                  |
| **F / Espaço**              | Interagir / Saquear Caixa Militar / Abrir e fechar portas |
| **I / Tab**                 | Abrir/Fechar Maleta de Inventário Tático (6x4 + 2x2)      |
| **R / Botão Direito**       | Girar item selecionado na Maleta 6x4                      |
| **O / F10**                 | Abrir Menu de Opções de Resolução (720p / 1280x800 / 1080p)|
| **- / + (ou PgDn / PgUp)**  | Alternar Resolução em tempo real                          |
| **E**                       | Extrair no Ponto de Extração                              |
| **B**                       | Alternar acesso rápido ao Bunker Subterrâneo              |
| **F11**                     | Alternar Tela Cheia (*Fullscreen*)                        |
| **ESC**                     | Fechar Menu / Fechar Maleta / Sair do Jogo                |

---

## 🛒 Conteúdo para a Página "Coming Soon" na Steam (Steamworks)

### A. Descrição Curta (*Short Description* — limite de 300 caracteres)

> **Zone Protocol** é um roguelike tático de extração top-down. Enfrente a escuridão da Zona de Krastov com iluminação DDA, organize uma maleta de sinergias energéticas, extraia antes do tempo acabar e evolua seu bunker subterrâneo.

---

### B. Sobre Este Jogo (*About This Game* — Formatado em BBCode para a Steam)

```bbcode
[h2]SOBRE A ZONA DE KRASTOV[/h2]
Ano de 2029. Após o colapso do Complexo Energético de Krastov, a região tornou-se uma Zona de Exclusão tomada por escuridão profunda, mutações e anomalias radioativas que sintetizam artefatos de alto valor. No comando de um abrigo subterrâneo clandestino, você recruta Operadores para incursões táticas de alto risco.

[h2]PRINCIPAIS CARACTERÍSTICAS[/h2]

[list]
[*][b]Iluminação Tática e Som no Escuro:[/b] Use sua lanterna para cortar a escuridão com cálculo de luz DDA em tempo real. Cuidado: inimigos fora do seu cone de visão geram ondas sonoras vermelhas e letais.
[*][b]Maleta Tática de Sinergias Energéticas:[/b] Gerencie um inventário em grade 6x4 onde artefatos radioativos se conectam fisicamente a armas e itens médicos por nós de energia, liberando tiros elementares e evoluções por fusão.
[*][b]Incursões Contra o Relógio & Bolso Seguro:[/b] Corra contra o cronômetro de extração antes que o setor seja selado. Se morrer, seu contêiner blindado (Safe Pocket 2x2) garante que seus itens mais valiosos e Pontos de Dados cheguem em segurança à base.
[*][b]O Loop do Bunker Subterrâneo:[/b] Venda artefatos no mercado negro, construa bancadas de armas, geradores de energia e recrute Operadores com traços únicos (Traits) para as próximas partidas.
[*][b]Otimizado para Steam Deck e PCs de Entrada:[/b] Desenvolvido em C++17 nativo, oferecendo 60 FPS cravados em resolução 16:10 / 16:9 Widescreen com suporte total a controles Twin-Stick.
[/list]
```

---

### C. Tags da Loja (*Store Tags* — em ordem de relevância para o algoritmo)

1. **Extraction Roguelike**
2. **Top-Down**
3. **Inventory Management**
4. **Tactical**
5. **Survival**
6. **Pixel Art**
7. **Post-apocalyptic**
8. **Twin-Stick Shooter**
9. **Action Roguelike**
10. **Dark**

---

### D. Requisitos de Sistema (*System Requirements*)

**MÍNIMOS:**

* **SO:** Windows 10/11 (64-bit)
* **Processador:** Intel Core i3-2100 / AMD FX-6300 ou equivalente
* **Memória:** 2 GB de RAM
* **Placa de vídeo:** Placa integrada compatível com OpenGL 3.3 / Direct3D 11
* **Armazenamento:** 500 MB de espaço disponível
* **Outras notas:** Suporte nativo a controles (Xbox, PlayStation, Steam Deck)

**RECOMENDADOS:**

* **SO:** Windows 10/11 (64-bit)
* **Processador:** Intel Core i5-4460 / AMD Ryzen 3 1200
* **Memória:** 4 GB de RAM
* **Placa de vídeo:** NVIDIA GeForce GTX 750 Ti / AMD Radeon R7 260X (1 GB VRAM)
* **Armazenamento:** 1 GB de espaço disponível

---

## 👤 Autor & Licença

**Luiz Corrêa**

Copyright (c) 2026 Luiz Corrêa. **All Rights Reserved.** Veja o arquivo [LICENSE](LICENSE) para detalhes.
