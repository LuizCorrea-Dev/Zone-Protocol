# ☣️ ZONE PROTOCOL — Release v0.1.4 (v014)

**Data:** 30 de Outubro de 2026  
**Status:** Produção Estável • Áudio Dinâmico Contextual & Maleta 8x5  
**Tag:** `v0.1.4` / `v014`  
**Binário:** `ZoneProtocol_v014.exe` (x64 Windows)

---

## 🎯 Destaques da Versão `v0.1.4`

### 1. 🔊 Sistema de Áudio Dinâmico Contextual (`AudioSystem` — WAV + OGG Vorbis)
* **Trilha de Background da Zona em Loop (`referencia/music/background.wav`):** Reprodução contínua sem engasgos durante toda a incursão ao ar livre na Zona (`IN_ZONE`).
* **Ambiência Interna de Bunkers na Missão (`referencia/music/dungeon002.ogg`):** Decodificação nativa de Ogg Vorbis via `stb_vorbis` integrada ao mixer SDL2. Ao atravessar a porta e entrar em qualquer bunker ou estrutura interna na missão (`ZoneMap::isInsideMissionBunker`), a trilha transita automaticamente para `dungeon002.ogg` (distinta do Bunker Hub do jogador).
* **Som de Extração (`referencia/music/Stalked In The Woods (reversed).wav`):** Ativado automaticamente quando o Operador alcança o perímetro de extração, encerrando imediatamente ao extrair para o Bunker Hub (`BUNKER_HUB`) ou morrer.
* **Áudio do Ecrã de Morte (`referencia/music/tela de morte.wav`):** Ao tombar na Zona (`DEATH_SCREEN`), todo o áudio da missão é interrompido e a trilha de morte é executada exatamente **1 única vez** (`loop = false`) em sincronia com o monitor CRT.

---

### 2. 💼 Maleta de Sinergia Expandida `8x5` (40 Slots) + Fundo Tático
* **5ª Linha Desbloqueada (`8x5 = 40 Slots`):** Todos os 40 compartimentos desenhados no chassi `UI_BRIEFCASE_CHASSIS` estão 100% operacionais.
* **Fundo Tático com 10% de Transparência (`0xEE0D1311`):** Preenchimento chumbo escuro sob a grade metálica oxidada para impedir interferência visual do cenário do Bunker atrás dos itens.

---

### 3. 👾 Animações de Caminhada & Silhuetas no Escuro Calibradas
* **Ciclos de Passada Ativos:** Mutantes caminham com animação fluida (`sprites_crawler_anim.png` e `sprites_brute_anim.png`) dentro do feixe de luz DDA.
* **Calibração de Faixas Y e `flipH`:** Eliminação completa de cortes horizontais ao caminhar para Leste/Oeste (`Y = 580..790` no Crawler e `Y = 400..690` no Brute).
* **Silhuetas Sombrias (`sprites_shadows_anim.png`):** 3 variantes de silhuetas negras com olhos vermelhos acesos fora do cone da lanterna.

