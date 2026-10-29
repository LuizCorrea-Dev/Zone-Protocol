# 🚀 ZONE PROTOCOL v0.1.2 (v012) — Tactical Atlas Overhaul, 6x4 Synergy Briefcase, Bunker Hub & 3-File Telemetry

**Tag da Release:** `v0.1.2` (`v012`)  
**Título da Release no GitHub:** `☣️ ZONE PROTOCOL v0.1.2 (v012) — Tactical 2D Atlas, 6x4 Synergy Briefcase, Bunker Hub & Diagnostic Telemetry`

---

## 📋 Descrição da Release (Copie para a página de Release do GitHub)

**ZONE PROTOCOL v0.1.2 (`v012`)** é um marco de evolução visual, mecânica e de engenharia do nosso roguelike tático de extração top-down em **C++17 / SDL2**, otimizado para **Steam Deck (1280x800 16:10)** e **PC Widescreen (16:9 @ 60 FPS)**.

---

### ✨ Novidades e Destaques da Versão `v0.1.2` (`v012`)

#### 1. 🎨 Novo Pipeline Visual (`SpriteAtlas` 64x64 + Iluminação DDA 2D)
* **Folhas de Texturas e Sprites HD Integrados (`assets/sprites/`):**
  * `tileset_walls_floors.png`: Lajes de concreto rachado, terra batida com musgo, grelhas metálicas enferrujadas, blocos soviéticos e paredes com tubulações industriais.
  * `sprites_entities.png`: Operador com traje Hazmat amarelo-oliva e fuzil AK-74 em múltiplas direções, além das criaturas mutantes (*Rastejantes Pálidos* e *Brutamontes de Carne*).
  * `sprites_map_objects.png`: Caixas militares saqueáveis (fechadas e abertas), tambores radioativos, geradores enferrujados e obstáculos táticos.
* **Olhos Vermelhos no Escuro (*Fog of War*):** Criaturas fora do cone de luz da lanterna DDA permanecem ocultas na escuridão, exibindo apenas os dois olhos vermelhos brilhantes e reagindo ao som dos disparos.
* **Proporção Nativa 16:10 (Steam Deck 1280x800):** Exibição nítida de **20x12.5 blocos** na tela com escalonamento *Nearest-Neighbor* sem borrão.

#### 2. 💼 Maleta Tática de Sinergias 6x4 + Safe Pocket 2x2 (`GridInventory`)
* **Matriz Energizada 6x4 (`ui_briefcase_items.png`):** Posicione artefatos radioativos (*Isótopo Zaria*, *Núcleo Spark*, *Cristal Crio*, *Orbe Compasso*) adjacentes ao fuzil **AK-74M** ou a suprimentos médicos para ativar feixes luminosos de energia em tempo real:
  * 🟢 **Sinergia Radioativa (Verde):** Multiplicador de dano balístico + queimadura radioativa contínua (DPS) nos mutantes.
  * 🟣 **Sinergia Elétrica (Roxa):** Aumento de cadência de tiro + **ricochete elétrico em cadeia** que salta entre inimigos próximos.
  * 🔵 **Sinergia Criogênica (Azul):** Redução de velocidade (*Cryo-Slow*) nos mutantes atingidos.
* **Bolso Seguro Blindado (`Safe Pocket 2x2`):** Qualquer artefato ou item guardado no compartimento blindado 2x2 é preservado mesmo se o Operador tombar na Zona.

#### 3. 🏚️ Loop Completo de Incursão: Loading Dinâmico, Ecrã de Morte e Bunker Hub
* **Tela de Carregamento Dinâmica (`Loading Screen.png` + `ui_loading_bar.png`):** Barra de progresso segmentada em néon verde acompanhando as etapas reais de geração procedural e dicas táticas de sobrevivência.
* **Ecrã de Morte (`bg_death_clean.png` + `ui_death_panels.png`):** Relatório pós-incursão com tempo sobrevivido, mutantes abatidos, Pontos de Dados obtidos e visualização dos itens salvos no *Safe Pocket 2x2*.
* **Abrigo Subterrâneo & Mercado Negro (`screen_bunker_hub.png`):**
  * `[1] Enfermaria`: Descontaminação e restauração completa de HP.
  * `[2] Gerador`: Conversão de sucata (*Scrap*) em energia e fabricação de munição `5.45mm`.
  * `[3] Bancada de Armas`: Organização tática da Maleta 6x4 antes da incursão.
  * `[4] Terminal de Infiltração`: Seleção de setor e lançamento de nova incursão procedural.
  * `[5] Mercado Negro`: Venda de artefatos extraídos e compra de novos núcleos anômalos.

#### 4. 🛠️ Engenharia, Estabilidade e Telemetria Tripla (`ZoneProtocol::Engine::Logger`)
* **Sistema de Diagnóstico Triplo Automático (`logs/`):**
  1. `ZoneProtocol_x64_YYYY-MM-DD_HH-MM-SS.RPT`: Relatório mestre de motor, hardware (CPU/RAM/GPU D3D11), auditoria de recursos e sub-sistemas.
  2. `error_YYYY-MM-DD_HH-MM-SS.log`: Registro dedicado de alertas `(W)`, erros `(E)` e falhas críticas `(C)` com arquivo e linha exatos (`@"src/...,linha"`), *Live Game State Snapshot* e *Call Stack* simbólica via `DbgHelp.dll` + MiniDump `.dmp`.
  3. `script_YYYY-MM-DD_HH-MM-SS.log`: Rastreamento de eventos de missão, *KeybindsManager*, geração procedural, combate e sinergias da maleta.
* **Correções de Estabilidade:**
  * Correção de *bounds clamping* vertical/horizontal na projeção de paredes 2.5D (`WorldView2D::render`) ao caminhar para o Norte próximo a entidades.
  * Remoção de arquivos e sistemas legados não utilizados (`chernogorsk.ddmap`, `ModManager`).

---

### 📦 Arquivos da Release (Assets para Download)
* **`ZoneProtocol_v012_Win64_Portable.zip`**: Pacote portátil completo para Windows 10/11 (64-bit) contendo `ZoneProtocol_v012.exe`, `SDL2.dll`, `assets/sprites/` e lançadores `.bat`.
* **`ZoneProtocol_v012.exe`**: Executável avulso da versão `v0.1.2`.

---

### 🕹️ Como Jogar (Versão Portátil)
1. Baixe e extraia `ZoneProtocol_v012_Win64_Portable.zip`.
2. Execute `ZoneProtocol_v012.exe` (ou `run_1280x800_16_10.bat`).
3. Use **WASD** para mover, **Mouse** para mirar e atirar, **F** para saquear caixas/abrir portas, **TAB/I** para abrir a Maleta 6x4, **R/Botão Direito** para girar itens e **B** para visitar o Bunker Subterrâneo.
