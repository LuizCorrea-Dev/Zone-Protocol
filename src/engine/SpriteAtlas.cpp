#include "SpriteAtlas.hpp"
#include "MathDef.hpp"
#include "Logger.hpp"
#include "VFS.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <filesystem>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")
#endif

namespace ZoneProtocol::Engine {

static std::string resolveSpritePath(const std::string& filepath) {
    namespace fs = std::filesystem;
    if (fs::exists(filepath)) return filepath;

    std::string vfsPath = DayDoom::Engine::VFS::get().resolve(filepath);
    if (fs::exists(vfsPath)) return vfsPath;

    fs::path p(filepath);
    std::string filename = p.filename().string();

    const std::vector<std::string> candidates = {
        "assets/sprites/" + filename,
        "../assets/sprites/" + filename,
        "referencia/" + filename,
        "../referencia/" + filename,
        "../../referencia/" + filename,
        "Zone Protocol/assets/sprites/" + filename
    };

    for (const auto& c : candidates) {
        if (fs::exists(c)) return c;
    }
    return filepath;
}

bool SpriteAtlas::loadFromFile(const std::string& filepath, bool removeDarkBg) {
    std::string resolved = resolveSpritePath(filepath);

#if defined(_WIN32)
    static bool comInitialized = false;
    if (!comInitialized) {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        comInitialized = true;
    }

    IWICImagingFactory* factory = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
    );
    if (FAILED(hr) || !factory) {
        ZoneProtocol::Engine::Logger::registerResourceLoad("SpriteAtlas", resolved, 0, 0, false);
        ZP_LOG_ERROR("RESOURCES", "Falha ao inicializar WIC Imaging Factory para: " + resolved);
        return false;
    }

    int wlen = MultiByteToWideChar(CP_UTF8, 0, resolved.c_str(), -1, nullptr, 0);
    std::wstring wpath(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, resolved.c_str(), -1, wpath.data(), wlen);

    IWICBitmapDecoder* decoder = nullptr;
    hr = factory->CreateDecoderFromFilename(
        wpath.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );

    if (FAILED(hr) || !decoder) {
        // Fallback ANSI codepage para caminhos com acento no Windows
        int alen = MultiByteToWideChar(CP_ACP, 0, resolved.c_str(), -1, nullptr, 0);
        std::wstring wpathAcp(alen, L'\0');
        MultiByteToWideChar(CP_ACP, 0, resolved.c_str(), -1, wpathAcp.data(), alen);
        hr = factory->CreateDecoderFromFilename(
            wpathAcp.c_str(),
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            &decoder
        );
    }

    if (FAILED(hr) || !decoder) {
        factory->Release();
        ZoneProtocol::Engine::Logger::registerResourceLoad("SpriteAtlas", resolved, 0, 0, false);
        ZP_LOG_WARN("RESOURCES", "Arquivo de SpriteAtlas nao encontrado: " + resolved);
        return false;
    }

    IWICBitmapFrameDecode* frame = nullptr;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr) || !frame) {
        decoder->Release();
        factory->Release();
        return false;
    }

    UINT width = 0, height = 0;
    frame->GetSize(&width, &height);
    m_width = static_cast<int>(width);
    m_height = static_cast<int>(height);

    IWICFormatConverter* converter = nullptr;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr) || !converter) {
        frame->Release();
        decoder->Release();
        factory->Release();
        return false;
    }

    hr = converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0f,
        WICBitmapPaletteTypeCustom
    );
    if (FAILED(hr)) {
        converter->Release();
        frame->Release();
        decoder->Release();
        factory->Release();
        return false;
    }

    std::vector<uint8_t> rawBgra(m_width * m_height * 4);
    hr = converter->CopyPixels(
        nullptr,
        static_cast<UINT>(m_width * 4),
        static_cast<UINT>(rawBgra.size()),
        rawBgra.data()
    );

    converter->Release();
    frame->Release();
    decoder->Release();
    factory->Release();

    if (FAILED(hr)) {
        return false;
    }

    m_pixels.resize(m_width * m_height);

    // Amostra o canto (4,4) da folha como cor de referência do fundo da IA
    int sampleIdx = (std::min(4, m_height - 1) * m_width + std::min(4, m_width - 1)) * 4;
    int bgRefB = rawBgra[sampleIdx + 0];
    int bgRefG = rawBgra[sampleIdx + 1];
    int bgRefR = rawBgra[sampleIdx + 2];

    for (int i = 0; i < m_width * m_height; ++i) {
        uint8_t b = rawBgra[i * 4 + 0];
        uint8_t g = rawBgra[i * 4 + 1];
        uint8_t r = rawBgra[i * 4 + 2];
        uint8_t a = rawBgra[i * 4 + 3];

        if (removeDarkBg) {
            int diffR = std::abs(static_cast<int>(r) - bgRefR);
            int diffG = std::abs(static_cast<int>(g) - bgRefG);
            int diffB = std::abs(static_cast<int>(b) - bgRefB);

            // Recorta qualquer cor de fundo uniforme (amostrada no canto) sem apagar as silhuetas negras
            if (diffR < 18 && diffG < 18 && diffB < 18) {
                a = 0;
            }
        }

        m_pixels[i] = (static_cast<uint32_t>(a) << 24) |
                      (static_cast<uint32_t>(r) << 16) |
                      (static_cast<uint32_t>(g) << 8)  |
                      (static_cast<uint32_t>(b));
    }

    ZoneProtocol::Engine::Logger::registerResourceLoad("SpriteAtlas", resolved, m_width, m_height, true);
    ZoneProtocol::Engine::Logger::info("Loaded SpriteAtlas @\"" + resolved + "\" (" +
        std::to_string(m_width) + "x" + std::to_string(m_height) + ", ChromaKey=" + (removeDarkBg ? "ON" : "OFF") + ")", "RESOURCES");
    return true;
#else
    (void)removeDarkBg;
    return false;
#endif
}

void SpriteAtlas::drawSprite(
    uint32_t* screenBuffer, int screenW, int screenH,
    const SpriteRect& src,
    int dstX, int dstY, int dstW, int dstH,
    float lightFactor,
    bool flipH
) const {
    if (!screenBuffer || m_pixels.empty() || dstW <= 0 || dstH <= 0 || src.w <= 0 || src.h <= 0) return;

    // Se o Atlas foi carregado com resolução diferente de 2048x2048 e as coordenadas ultrapassam a dimensão física,
    // normaliza dinamicamente as coordenadas do src para a resolução física real carregada!
    SpriteRect actualSrc = src;
    if (m_width > 0 && m_height > 0 && (m_width != 2048 || m_height != 2048)) {
        if (src.x + src.w > m_width || src.y + src.h > m_height) {
            float scaleX = static_cast<float>(m_width) / 2048.0f;
            float scaleY = static_cast<float>(m_height) / 2048.0f;
            actualSrc.x = static_cast<int>(std::round(src.x * scaleX));
            actualSrc.y = static_cast<int>(std::round(src.y * scaleY));
            actualSrc.w = std::max(1, static_cast<int>(std::round(src.w * scaleX)));
            actualSrc.h = std::max(1, static_cast<int>(std::round(src.h * scaleY)));
        }
    }

    int startX = std::max(0, dstX);
    int startY = std::max(0, dstY);
    int endX   = std::min(screenW, dstX + dstW);
    int endY   = std::min(screenH, dstY + dstH);
    if (startX >= endX || startY >= endY) return;

    int lightFixed = static_cast<int>(std::clamp(lightFactor, 0.0f, 3.0f) * 256.0f);

    for (int sy = startY; sy < endY; ++sy) {
        int srcY = actualSrc.y + ((sy - dstY) * actualSrc.h) / dstH;
        if (srcY < 0 || srcY >= m_height) continue;

        const uint32_t* srcRow = &m_pixels[srcY * m_width];
        uint32_t* dstRow = &screenBuffer[sy * screenW];

        for (int sx = startX; sx < endX; ++sx) {
            int relX = ((sx - dstX) * actualSrc.w) / dstW;
            int srcX = flipH ? (actualSrc.x + actualSrc.w - 1 - relX) : (actualSrc.x + relX);
            if (srcX < 0 || srcX >= m_width) continue;

            uint32_t pixel = srcRow[srcX];
            uint8_t alpha = (pixel >> 24) & 0xFF;
            if (alpha < 128) continue;

            int r = (((pixel >> 16) & 0xFF) * lightFixed) >> 8;
            int g = (((pixel >> 8)  & 0xFF) * lightFixed) >> 8;
            int b = ((pixel & 0xFF)         * lightFixed) >> 8;

            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;

            dstRow[sx] = 0xFF000000u | (static_cast<uint32_t>(r) << 16) |
                                       (static_cast<uint32_t>(g) << 8)  |
                                       static_cast<uint32_t>(b);
        }
    }
}

void SpriteAtlas::drawFullScreen(
    uint32_t* screenBuffer, int screenW, int screenH,
    float brightness
) const {
    if (!screenBuffer || m_pixels.empty() || screenW <= 0 || screenH <= 0 || m_width <= 0 || m_height <= 0) return;

    int brightFixed = static_cast<int>(std::clamp(brightness, 0.0f, 3.0f) * 256.0f);

    for (int dy = 0; dy < screenH; ++dy) {
        int sy = (dy * m_height) / screenH;
        if (sy < 0 || sy >= m_height) continue;
        const uint32_t* srcRow = &m_pixels[sy * m_width];
        uint32_t* dstRow = &screenBuffer[dy * screenW];

        for (int dx = 0; dx < screenW; ++dx) {
            int sx = (dx * m_width) / screenW;
            if (sx < 0 || sx >= m_width) continue;
            uint32_t c = srcRow[sx];

            if (brightFixed != 256) {
                int r = (((c >> 16) & 0xFF) * brightFixed) >> 8;
                int g = (((c >> 8)  & 0xFF) * brightFixed) >> 8;
                int b = ((c & 0xFF)         * brightFixed) >> 8;
                r = std::min(255, r);
                g = std::min(255, g);
                b = std::min(255, b);
                c = (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
            }
            dstRow[dx] = 0xFF000000u | (c & 0x00FFFFFFu);
        }
    }
}

uint32_t SpriteAtlas::sampleTileUV(const SpriteRect& tileRect, float u, float v) const {
    if (m_pixels.empty()) return 0xFF202020;
    int tx = tileRect.x + static_cast<int>(std::clamp(u, 0.0f, 0.999f) * tileRect.w);
    int ty = tileRect.y + static_cast<int>(std::clamp(v, 0.0f, 0.999f) * tileRect.h);
    tx = std::clamp(tx, 0, m_width - 1);
    ty = std::clamp(ty, 0, m_height - 1);
    return m_pixels[ty * m_width + tx];
}

} // namespace ZoneProtocol::Engine

