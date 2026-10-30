# ☢️ ZONE PROTOCOL — Release v0.1.5 (v015)

**Data:** 30 de Outubro de 2026  
**Status:** Produção Estável — Geração Procedural 5-Pass, Áudio Dinâmico com Crossfade, AK-47 Realista & Evacuação Automática  
**Tag:** `v0.1.5` / `v015`  
**Binário:** `ZoneProtocol_v015.exe` (x64 Windows)

---

## 🎯 Destaques da Versão `v0.1.5`

### 1. 🚁 Correção da Extração Automática na LZ & Fim de Turno
* **Evacuação Tática por Permanência:** Ao alcançar a Zona de Extração (`[H] EXTRACTION ZONE`), o HUD tático aciona uma contagem regressiva de 4 segundos:
  `[ ZONA DE EXTRACAO ATIVA - EVACUANDO EM 4.0s | PRESSIONE [E] ]`.
  Ao zerar a contagem dentro da LZ, o resgate aéreo é acionado automaticamente, concedendo `+300 Credits & +25 Scrap` e retornando com segurança ao Bunker Hub.
* **Extração Imediata:** A tecla `[E]` continua operacional para extração instantânea sem espera caso o operador deseje evacuar rapidamente.
* **Término do Tempo da Incursão:**
  * **Dentro da LZ:** Caso o cronômetro geral de missão zere dentro da zona de resgate, o helicóptero decola de emergência com o operador a bordo.
  * **Fora da LZ:** Caso o tempo zere fora da LZ, o setor entra em colapso por radiação extrema, eliminando o operador com tela de morte (`DEATH_SCREEN`).

---

### 2. 🎚️ Transição Suave de Áudio (Crossfade Linear de 1.2s)
* **Zero Cortes Secos:** A transição entre a música exterior da Zona (`background.wav`) e os interiores/bunkers de missão (`dungeon002.ogg`) agora ocorre de forma suave e cinematográfica.
* **Algoritmo de Interpolação:** A faixa anterior tem seu volume reduzido gradualmente enquanto a nova faixa surge progressivamente sem cliques sonoros, pops ou quebra de fase no mixer SDL2.

---

### 3. 🔫 Áudio Dinâmico da AK-47 (Tiro Único, Rajada Contínua e Última Bala)
* **Tiro Único (Tap-fire):** Um clique rápido dispara uma bala isolada com o estrondo completo e reverberação de eco longa (`ak47_1tiro.wav`).
* **Rajada Contínua (Full-auto):** Ao segurar o gatilho, cada projétil intermediário da rajada emite o som seco e rápido de ciclo mecânico (`ak47_1tiro_da_rajada.wav`).
* **Última Bala com Reverb:** Ao soltar o gatilho após uma rajada de múltiplos tiros, a última bala disparada ecoa com a reverberação completa de disparo (`ak47_1tiro.wav`).

---

### 4. 🗺️ Geração Procedural de 5 Passos (Fase 1 do Roadmap)
* **Passo 1 (BSP Rooms):** Salas temáticas (Arsenal, Laboratório, Quarentena, Armazém) com espaçamento de segurança.
* **Passo 2 (Prédios Periféricos e Edifícios em U):** Estruturas com portas registradas e paredes duplas de concreto.
* **Passo 3 (Coberturas Táticas & Barricadas):** 25 a 35 caixotes, sacos de areia e barreiras espalhados proceduralmente com proteção de 3x3 tiles em frente a cada porta.
* **Passo 4 (Autotiling 4-bit):** Seleção contínua de paredes conectadas em 16 direções cardeais a partir de `tileset_walls_floors.png`.
* **Passo 5 (Flood Fill BFS & Garantia de Acessibilidade):** Verificação de caminho 100% livre e desobstruído do Spawn `(X, Y)` até o ponto de Extração `(X, Y)`.

---

### 5. 🐺 IA de Mutantes & Áudio Espacial 360°
* **Grito Aterrorizante no Escuro:** Mutantes fora da visão da lanterna DDA emitem gritos aterrorizantes (`grito do mostros.wav`) com atenuação por distância e pan estereofônico em tempo real baseado na posição relativa ao operador.

---

## 📦 Conteúdo do Pacote de Distribuição
* `ZoneProtocol_v015.exe` — Executável otimizado nativo Win64
* `SDL2.dll` — Biblioteca dinâmica SDL2 oficial
* `assets/` — Spritesheets HD, tilesets, prefabs e trilhas sonoras completas
* `run_pc.bat` / `run_widescreen_16_9.bat` / `run_1280x800_16_10.bat` — Scripts de inicialização rápida
