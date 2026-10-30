#include "AudioSystem.hpp"
#include "Logger.hpp"
#include "VFS.hpp"
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <fstream>

extern "C" int stb_vorbis_decode_filename(const char* filename, int* channels, int* sample_rate, short** output);

namespace ZoneProtocol::Engine {

namespace {
    bool fileExistsOnDisk(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        return f.good();
    }

    std::string extractFileName(const std::string& path) {
        size_t pos = path.find_last_of("/\\");
        if (pos == std::string::npos) return path;
        return path.substr(pos + 1);
    }
}

AudioSystem::AudioSystem() = default;

AudioSystem::~AudioSystem() {
    shutdown();
}

std::string AudioSystem::resolveAudioPath(const std::string& rawPath) const {
    std::string fname = extractFileName(rawPath);

    const std::string candidates[] = {
        rawPath,
        "../" + rawPath,
        "../../" + rawPath,
        "assets/music/" + fname,
        "../assets/music/" + fname,
        "../../assets/music/" + fname,
        "Zone Protocol/assets/music/" + fname,
        "referencia/music/" + fname,
        "../referencia/music/" + fname,
        "../../referencia/music/" + fname
    };

    for (const auto& c : candidates) {
        if (fileExistsOnDisk(c)) {
            return c;
        }
        std::string vfsPath = DayDoom::Engine::VFS::get().resolve(c);
        if (fileExistsOnDisk(vfsPath)) {
            return vfsPath;
        }
    }
    return rawPath;
}

bool AudioSystem::init() {
    if (m_initialized) return true;

    if ((SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO) == 0) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
            ZP_LOG_WARN("AUDIO", std::string("Falha ao inicializar subsistema SDL_INIT_AUDIO: ") + SDL_GetError());
            return false;
        }
    }

    SDL_AudioSpec desired{};
    desired.freq     = 44100;
    desired.format   = AUDIO_S16SYS;
    desired.channels = 2;
    desired.samples  = 2048;
    desired.callback = &AudioSystem::audioCallback;
    desired.userdata = this;

    m_deviceId = SDL_OpenAudioDevice(nullptr, 0, &desired, &m_deviceSpec, 0);
    if (m_deviceId == 0) {
        ZP_LOG_WARN("AUDIO", std::string("Nao foi possivel abrir dispositivo de audio SDL2: ") + SDL_GetError());
        return false;
    }

    m_initialized = true;
    ZoneProtocol::Engine::Logger::info(
        "[AudioSystem] Dispositivo SDL2 inicializado -> " +
        std::to_string(m_deviceSpec.freq) + " Hz, " +
        std::to_string(static_cast<int>(m_deviceSpec.channels)) + " canais (S16SYS)",
        "AUDIO"
    );

    // Pre-carrega as trilhas e efeitos sonoros oficiais solicitados
    loadTrackWAV(AudioTrack::ZONE_BACKGROUND,  "referencia/music/background.wav", true);
    loadTrackOGG(AudioTrack::MISSION_BUNKER,   "referencia/music/dungeon002.ogg", true);
    loadTrackWAV(AudioTrack::EXTRACTION_SOUND, "referencia/music/Stalked In The Woods (reversed).wav", true);
    loadTrackWAV(AudioTrack::DEATH_SCREEN,     "referencia/music/tela de morte.wav", false);
    loadTrackWAV(AudioTrack::MONSTER_SCREAM,   "referencia/music/grito do mostros.wav", false);
    loadTrackWAV(AudioTrack::AK47_SINGLE,      "referencia/music/ak47_1tiro.wav", false);
    loadTrackWAV(AudioTrack::AK47_BURST,       "referencia/music/ak47_1tiro_da_rajada.wav", false);

    // Libera o mixer da SDL2 para processar o callback
    SDL_PauseAudioDevice(m_deviceId, 0);
    return true;
}

void AudioSystem::shutdown() {
    if (m_deviceId != 0) {
        SDL_LockAudioDevice(m_deviceId);
        m_isPlaying = false;
        m_currentTrack = AudioTrack::NONE;
        m_fadeTrack = AudioTrack::NONE;
        m_isCrossfading = false;
        m_fadeProgress = 1.0f;
        for (auto& sfx : m_activeSFX) {
            sfx.active = false;
        }
        SDL_UnlockAudioDevice(m_deviceId);
        SDL_CloseAudioDevice(m_deviceId);
        m_deviceId = 0;
    }
    for (auto& t : m_tracks) {
        t.pcmData.clear();
        t.loaded = false;
        t.playCursor = 0;
    }
    m_initialized = false;
}

bool AudioSystem::loadTrackWAV(AudioTrack track, const std::string& virtualPath, bool loop) {
    if (!m_initialized) return false;

    std::string resolved = resolveAudioPath(virtualPath);
    SDL_AudioSpec wavSpec{};
    Uint8* wavBuf = nullptr;
    Uint32 wavLen = 0;

    if (SDL_LoadWAV(resolved.c_str(), &wavSpec, &wavBuf, &wavLen) == nullptr) {
        ZP_LOG_WARN("AUDIO", "Falha ao carregar WAV '" + resolved + "': " + SDL_GetError());
        return false;
    }

    SDL_AudioStream* stream = SDL_NewAudioStream(
        wavSpec.format, wavSpec.channels, wavSpec.freq,
        m_deviceSpec.format, m_deviceSpec.channels, m_deviceSpec.freq
    );

    if (!stream) {
        SDL_FreeWAV(wavBuf);
        return false;
    }

    if (SDL_AudioStreamPut(stream, wavBuf, static_cast<int>(wavLen)) != 0 ||
        SDL_AudioStreamFlush(stream) != 0) {
        SDL_FreeAudioStream(stream);
        SDL_FreeWAV(wavBuf);
        return false;
    }

    int avail = SDL_AudioStreamAvailable(stream);
    auto& slot = m_tracks[static_cast<size_t>(track)];
    slot.pcmData.resize(static_cast<size_t>( std::max(0, avail) ));
    if (avail > 0) {
        int readBytes = SDL_AudioStreamGet(stream, slot.pcmData.data(), avail);
        if (readBytes > 0) {
            slot.pcmData.resize(static_cast<size_t>(readBytes));
        }
    }

    SDL_FreeAudioStream(stream);
    SDL_FreeWAV(wavBuf);

    slot.playCursor = 0;
    slot.loop       = loop;
    slot.loaded     = !slot.pcmData.empty();
    slot.sourcePath = resolved;

    Logger::info(
        "[AudioSystem] Loaded WAV Track '" + resolved + "' (" +
        std::to_string(slot.pcmData.size() / 1024) + " KB PCM, Loop=" + (loop ? "ON" : "ONCE") + ")",
        "AUDIO"
    );
    return slot.loaded;
}

bool AudioSystem::loadTrackOGG(AudioTrack track, const std::string& virtualPath, bool loop) {
    if (!m_initialized) return false;

    std::string resolved = resolveAudioPath(virtualPath);
    int channels = 0;
    int sampleRate = 0;
    short* decodedPcm = nullptr;

    int samplesPerChannel = stb_vorbis_decode_filename(resolved.c_str(), &channels, &sampleRate, &decodedPcm);
    if (samplesPerChannel <= 0 || !decodedPcm) {
        ZP_LOG_WARN("AUDIO", "Falha ao decodificar OGG '" + resolved + "' via stb_vorbis.");
        if (decodedPcm) std::free(decodedPcm);
        return false;
    }

    int rawBytes = samplesPerChannel * channels * static_cast<int>(sizeof(short));

    SDL_AudioStream* stream = SDL_NewAudioStream(
        AUDIO_S16SYS, static_cast<Uint8>(channels), sampleRate,
        m_deviceSpec.format, m_deviceSpec.channels, m_deviceSpec.freq
    );

    if (!stream) {
        std::free(decodedPcm);
        return false;
    }

    if (SDL_AudioStreamPut(stream, decodedPcm, rawBytes) != 0 ||
        SDL_AudioStreamFlush(stream) != 0) {
        SDL_FreeAudioStream(stream);
        std::free(decodedPcm);
        return false;
    }

    int avail = SDL_AudioStreamAvailable(stream);
    auto& slot = m_tracks[static_cast<size_t>(track)];
    slot.pcmData.resize(static_cast<size_t>(std::max(0, avail)));
    if (avail > 0) {
        int readBytes = SDL_AudioStreamGet(stream, slot.pcmData.data(), avail);
        if (readBytes > 0) {
            slot.pcmData.resize(static_cast<size_t>(readBytes));
        }
    }

    SDL_FreeAudioStream(stream);
    std::free(decodedPcm);

    slot.playCursor = 0;
    slot.loop       = loop;
    slot.loaded     = !slot.pcmData.empty();
    slot.sourcePath = resolved;

    Logger::info(
        "[AudioSystem] Loaded OGG Track '" + resolved + "' (" +
        std::to_string(slot.pcmData.size() / 1024) + " KB PCM, Loop=" + (loop ? "ON" : "ONCE") + ")",
        "AUDIO"
    );
    return slot.loaded;
}

void AudioSystem::playTrack(AudioTrack track, bool resetCursor, float crossfadeDuration) {
    if (!m_initialized || m_deviceId == 0) return;

    if (track == AudioTrack::NONE) {
        stopAll();
        return;
    }

    size_t idx = static_cast<size_t>(track);
    if (idx >= m_tracks.size() || !m_tracks[idx].loaded) return;

    SDL_LockAudioDevice(m_deviceId);

    // Se já estiver tocando esta mesma trilha e não for reset de cursor
    if (m_currentTrack == track && m_isPlaying && !resetCursor) {
        SDL_UnlockAudioDevice(m_deviceId);
        return;
    }

    // Se o jogador estiver alternando de volta para a trilha que estava saindo no crossfade
    if (m_isCrossfading && m_fadeTrack == track) {
        m_fadeTrack = m_currentTrack;
        m_currentTrack = track;
        m_fadeProgress = std::clamp(1.0f - m_fadeProgress, 0.0f, 1.0f);
        m_fadeDuration = std::max(0.1f, crossfadeDuration);
        SDL_UnlockAudioDevice(m_deviceId);
        return;
    }

    // Transição imediata: se resetCursor explícito, tela de morte, ou se o áudio não estava ativo
    if (resetCursor || track == AudioTrack::DEATH_SCREEN || m_currentTrack == AudioTrack::NONE || !m_isPlaying) {
        if (resetCursor || track == AudioTrack::DEATH_SCREEN) {
            m_tracks[idx].playCursor = 0;
        } else if (m_tracks[idx].playCursor >= m_tracks[idx].pcmData.size()) {
            m_tracks[idx].playCursor = 0;
        }
        m_currentTrack = track;
        m_fadeTrack = AudioTrack::NONE;
        m_isCrossfading = false;
        m_fadeProgress = 1.0f;
        m_isPlaying = true;
        SDL_UnlockAudioDevice(m_deviceId);
        return;
    }

    // Crossfade Suave entre ambientes (ex: Exterior <-> Bunker / Estruturas)
    m_fadeTrack = m_currentTrack;
    m_currentTrack = track;
    m_fadeProgress = 0.0f;
    m_fadeDuration = std::max(0.1f, crossfadeDuration);
    m_isCrossfading = true;
    m_isPlaying = true;

    if (m_tracks[idx].playCursor >= m_tracks[idx].pcmData.size()) {
        m_tracks[idx].playCursor = 0;
    }

    SDL_UnlockAudioDevice(m_deviceId);
}

void AudioSystem::stopAll() {
    if (!m_initialized || m_deviceId == 0) return;

    SDL_LockAudioDevice(m_deviceId);
    m_isPlaying = false;
    m_currentTrack = AudioTrack::NONE;
    m_fadeTrack = AudioTrack::NONE;
    m_isCrossfading = false;
    m_fadeProgress = 1.0f;
    for (auto& sfx : m_activeSFX) {
        sfx.active = false;
    }
    SDL_UnlockAudioDevice(m_deviceId);
}

void AudioSystem::playSFX(AudioTrack sfxTrack, float pan, float volume) {
    if (!m_initialized || m_deviceId == 0 || sfxTrack == AudioTrack::NONE) return;

    size_t trackIdx = static_cast<size_t>(sfxTrack);
    if (trackIdx >= m_tracks.size() || !m_tracks[trackIdx].loaded || m_tracks[trackIdx].pcmData.empty()) {
        return;
    }

    SDL_LockAudioDevice(m_deviceId);
    // Encontra um canal de SFX livre ou o primeiro
    int freeIdx = -1;
    for (size_t i = 0; i < m_activeSFX.size(); ++i) {
        if (!m_activeSFX[i].active) {
            freeIdx = static_cast<int>(i);
            break;
        }
    }
    if (freeIdx == -1) {
        freeIdx = 0;
    }

    m_activeSFX[freeIdx].track = sfxTrack;
    m_activeSFX[freeIdx].cursor = 0;
    m_activeSFX[freeIdx].pan = std::clamp(pan, -1.0f, 1.0f);
    m_activeSFX[freeIdx].volume = std::clamp(volume, 0.0f, 1.0f);
    m_activeSFX[freeIdx].active = true;
    SDL_UnlockAudioDevice(m_deviceId);
}

void AudioSystem::resetMissionCursors() {
    if (!m_initialized || m_deviceId == 0) return;

    SDL_LockAudioDevice(m_deviceId);
    for (auto& t : m_tracks) {
        t.playCursor = 0;
    }
    SDL_UnlockAudioDevice(m_deviceId);
}

void AudioSystem::setMasterVolume(float volume) {
    float clamped = std::clamp(volume, 0.0f, 1.0f);
    m_volumeSDL = static_cast<int>(clamped * static_cast<float>(SDL_MIX_MAXVOLUME));
}

void SDLCALL AudioSystem::audioCallback(void* userdata, Uint8* stream, int len) {
    std::memset(stream, 0, static_cast<size_t>(len));

    auto* self = static_cast<AudioSystem*>(userdata);
    if (!self) return;

    // 1. Processa a trilha principal (BGM) com suporte a Crossfade Suave
    if (self->m_isPlaying && self->m_currentTrack != AudioTrack::NONE) {
        size_t curIdx = static_cast<size_t>(self->m_currentTrack);
        auto* curTrack = (curIdx < self->m_tracks.size() && self->m_tracks[curIdx].loaded) ? &self->m_tracks[curIdx] : nullptr;

        size_t fadeIdx = static_cast<size_t>(self->m_fadeTrack);
        auto* fadeTrack = (self->m_isCrossfading && fadeIdx < self->m_tracks.size() && self->m_tracks[fadeIdx].loaded) ? &self->m_tracks[fadeIdx] : nullptr;

        float masterGain = static_cast<float>(self->m_volumeSDL) / static_cast<float>(SDL_MIX_MAXVOLUME);
        auto* dstSamples = reinterpret_cast<int16_t*>(stream);
        int totalFrames = len / (static_cast<int>(sizeof(int16_t)) * 2);

        float fadeStep = (self->m_isCrossfading && self->m_fadeDuration > 0.001f)
            ? (1.0f / (44100.0f * self->m_fadeDuration))
            : 0.0f;

        for (int f = 0; f < totalFrames; ++f) {
            float alpha = std::clamp(self->m_fadeProgress, 0.0f, 1.0f);
            float gainIn = (self->m_isCrossfading ? alpha : 1.0f) * masterGain;
            float gainOut = (self->m_isCrossfading ? (1.0f - alpha) : 0.0f) * masterGain;

            float sampleL = 0.0f;
            float sampleR = 0.0f;

            // Amostra da trilha atual (entrando com fade-in ou em 100%)
            if (curTrack && !curTrack->pcmData.empty()) {
                if (curTrack->playCursor + sizeof(int16_t) * 2 > curTrack->pcmData.size()) {
                    if (curTrack->loop) {
                        curTrack->playCursor = 0;
                    } else {
                        self->m_isPlaying = false;
                        curTrack = nullptr;
                    }
                }
                if (curTrack && curTrack->playCursor + sizeof(int16_t) * 2 <= curTrack->pcmData.size()) {
                    const auto* s = reinterpret_cast<const int16_t*>(curTrack->pcmData.data() + curTrack->playCursor);
                    sampleL += static_cast<float>(s[0]) * gainIn;
                    sampleR += static_cast<float>(s[1]) * gainIn;
                    curTrack->playCursor += sizeof(int16_t) * 2;
                }
            }

            // Amostra da trilha anterior (saindo suavemente com fade-out)
            if (fadeTrack && !fadeTrack->pcmData.empty() && gainOut > 0.0001f) {
                if (fadeTrack->playCursor + sizeof(int16_t) * 2 > fadeTrack->pcmData.size()) {
                    if (fadeTrack->loop) {
                        fadeTrack->playCursor = 0;
                    } else {
                        fadeTrack = nullptr;
                    }
                }
                if (fadeTrack && fadeTrack->playCursor + sizeof(int16_t) * 2 <= fadeTrack->pcmData.size()) {
                    const auto* s = reinterpret_cast<const int16_t*>(fadeTrack->pcmData.data() + fadeTrack->playCursor);
                    sampleL += static_cast<float>(s[0]) * gainOut;
                    sampleR += static_cast<float>(s[1]) * gainOut;
                    fadeTrack->playCursor += sizeof(int16_t) * 2;
                }
            }

            dstSamples[f * 2]     = static_cast<int16_t>(std::clamp(static_cast<int>(sampleL), -32768, 32767));
            dstSamples[f * 2 + 1] = static_cast<int16_t>(std::clamp(static_cast<int>(sampleR), -32768, 32767));

            if (self->m_isCrossfading) {
                self->m_fadeProgress += fadeStep;
                if (self->m_fadeProgress >= 1.0f) {
                    self->m_isCrossfading = false;
                    self->m_fadeTrack = AudioTrack::NONE;
                    self->m_fadeProgress = 1.0f;
                }
            }
        }
    }

    // 2. Mixa os canais ativos de SFX com Pan e Volume Posicional
    for (auto& sfx : self->m_activeSFX) {
        if (!sfx.active || sfx.track == AudioTrack::NONE) continue;
        size_t sfxIdx = static_cast<size_t>(sfx.track);
        if (sfxIdx >= self->m_tracks.size()) continue;

        auto& track = self->m_tracks[sfxIdx];
        if (!track.loaded || track.pcmData.empty() || sfx.cursor >= track.pcmData.size()) {
            sfx.active = false;
            continue;
        }

        // Calcula ganhos esquerdo e direito pelo pan (-1.0 = 100% L, 0.0 = 100% L/R, +1.0 = 100% R)
        float leftGain = (sfx.pan <= 0.0f) ? 1.0f : (1.0f - sfx.pan);
        float rightGain = (sfx.pan >= 0.0f) ? 1.0f : (1.0f + sfx.pan);
        leftGain *= sfx.volume;
        rightGain *= sfx.volume;

        int numSamples = len / static_cast<int>(sizeof(int16_t)); // Amostras estéreo intercaladas L, R
        auto* dstSamples = reinterpret_cast<int16_t*>(stream);
        const auto* srcSamples = reinterpret_cast<const int16_t*>(track.pcmData.data() + sfx.cursor);
        size_t srcRemainingBytes = track.pcmData.size() - sfx.cursor;
        int maxSrcSamples = static_cast<int>(srcRemainingBytes / sizeof(int16_t));
        int samplesToMix = std::min(numSamples, maxSrcSamples);

        for (int i = 0; i < samplesToMix; i += 2) {
            float sL = static_cast<float>(srcSamples[i]) * leftGain;
            float sR = static_cast<float>(srcSamples[i + 1]) * rightGain;

            int mixedL = static_cast<int>(dstSamples[i]) + static_cast<int>(sL);
            int mixedR = static_cast<int>(dstSamples[i + 1]) + static_cast<int>(sR);

            dstSamples[i]     = static_cast<int16_t>(std::clamp(mixedL, -32768, 32767));
            dstSamples[i + 1] = static_cast<int16_t>(std::clamp(mixedR, -32768, 32767));
        }

        sfx.cursor += static_cast<size_t>(samplesToMix * static_cast<int>(sizeof(int16_t)));
        if (sfx.cursor >= track.pcmData.size()) {
            sfx.active = false;
        }
    }
}

} // namespace ZoneProtocol::Engine

