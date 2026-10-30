# ☣️ ZONE PROTOCOL — Release v0.1.3 (v013)

**Data:** 29 de Outubro de 2026  
**Status:** Produção Estável • Pronto para Steam Deck e PC Desktop  
**Tag:** `v0.1.3` / `v013`  
**Binário:** `ZoneProtocol_v013.exe` (x64 Windows)

---

## 🎯 Destaques da Versão v0.1.3

### 1. Resoluções Canônicas Padronizadas (Exclusivas)
Todas as resoluções legadas não padronizadas foram removidas do pipeline. O jogo agora opera exclusivamente com três modos de exibição de alto desempenho e aspecto preservado:

| Modo | Resolução | Formato | Foco Primário |
|---|---|---|---|
| **HD 720p** | `1280 x 720` | **16:9** | Notebooks e PCs de entrada com alta taxa de quadros |
| **Steam Deck** | `1280 x 800` | **16:10** | Valve Steam Deck nativo (pixel-perfect 1:1) |
| **FHD 1080p** | `1920 x 1080` | **16:9** | Monitores Full HD / QHD / 4K com máxima nitidez |

---

### 2. Auto-Detecção da Resolução Máxima & Tela Cheia Nativa
* O jogo detecta automaticamente a resolução máxima suportada pelo display principal (`SDL_GetDesktopDisplayMode`).
* Inicializa por padrão em **Tela Cheia Exclusiva / Desktop (`SDL_WINDOW_FULLSCREEN_DESKTOP`)** a **1080p**, preenchendo 100% da tela sem barras pretas em displays 16:9 convencionais.
* O redimensionamento do framebuffer de iluminação DDA e inventário respeita escala lógica proporcional com filtragem limpa.

---

### 3. Menu de Vídeo Aprimorado (Sem Poluir a HUD de Jogo)
* **HUD Limpa:** As opções de resolução foram completamente retiradas da interface fixa de gameplay na Zona, garantindo imersão total.
* **Menu Modal Dedicado:** Pressione **`[O]`** ou **`[F10]`** a qualquer momento (no Bunker Hub ou em jogo) para abrir a janela modal de vídeo com opções interativas via clique ou teclado:
  * `[1]` Ativar **720p (1280x720)**
  * `[2]` Ativar **Steam Deck (1280x800)**
  * `[3]` Ativar **1080p FHD (1920x1080)**
  * `[F11]` Alternar **Tela Cheia (Fullscreen)** / Janela
  * `[ESC]` Fechar Menu de Vídeo
* Atalhos rápidos **`[-]`** e **`[+]`** permanecem disponíveis para ciclar as resoluções de forma instantânea.

---

### 4. Nova Fonte Tipográfica Bitmap 5x7 (Legibilidade Superior)
* A matriz antiga e compacta de 3x5 foi substituída por um conjunto completo de caracteres **5x7** com avanço de 6 pixels por caractere e kerning consistente.
* Textos na HUD (coordenadas, FPS, relógio de extração, botões, modais) e nas dicas flutuantes da Maleta Tática 6x4 agora são perfeitamente legíveis mesmo em telas de alta densidade de pixels (FHD/4K).

---

## 🛠️ Arquivos Modificados & Engenharia
* [`Platform.hpp`](file:///C:/Users/luizc/Documents/ROOTSZERA/Projetos/GAME/ZONE%20PROTOCOL/Zone%20Protocol/src/hal/Platform.hpp) & [`Platform.cpp`](file:///C:/Users/luizc/Documents/ROOTSZERA/Projetos/GAME/ZONE%20PROTOCOL/Zone%20Protocol/src/hal/Platform.cpp): Enum `DisplayProfile` atualizado para `{ HD_720P, STEAM_DECK_800P, FHD_1080P }`, inicialização padrão em 1080p Fullscreen e mapeamento de presets.
* [`HUD.hpp`](file:///C:/Users/luizc/Documents/ROOTSZERA/Projetos/GAME/ZONE%20PROTOCOL/Zone%20Protocol/src/game/UI/HUD.hpp) & [`HUD.cpp`](file:///C:/Users/luizc/Documents/ROOTSZERA/Projetos/GAME/ZONE%20PROTOCOL/Zone%20Protocol/src/game/UI/HUD.cpp): Implementação da matriz de glifos 5x7 e renderizador de menu modal de vídeo.
* [`GridInventory.cpp`](file:///C:/Users/luizc/Documents/ROOTSZERA/Projetos/GAME/ZONE%20PROTOCOL/Zone%20Protocol/src/game/UI/GridInventory.cpp): Fonte 5x7 integrada e espaçamento dinâmico proporcional nas células do inventário.
* [`WorldView2D.cpp`](file:///C:/Users/luizc/Documents/ROOTSZERA/Projetos/GAME/ZONE%20PROTOCOL/Zone%20Protocol/src/game/UI/WorldView2D.cpp): Ajustes de passo de iluminação DDA e tamanho dinâmico de tiles para 720p/800p/1080p.
* [`main.cpp`](file:///C:/Users/luizc/Documents/ROOTSZERA/Projetos/GAME/ZONE%20PROTOCOL/Zone%20Protocol/src/main.cpp): Integração das ações de vídeo disparadas pelo menu e atalhos globais.
* [`CMakeLists.txt`](file:///C:/Users/luizc/Documents/ROOTSZERA/Projetos/GAME/ZONE%20PROTOCOL/Zone%20Protocol/CMakeLists.txt): Geração automatizada do executável versionado `ZoneProtocol_v013.exe`.

