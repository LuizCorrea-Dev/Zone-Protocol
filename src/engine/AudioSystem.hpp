#pragma once

#include <SDL.h>
#include <string>
#include <vector>
#include <cstdint>
#include <array>

namespace ZoneProtocol::Engine {

enum class AudioTrack {
    NONE = 0,
    ZONE_BACKGROUND = 1,  // referencia/music/background.wav (Loop continuo na Zona)
    MISSION_BUNKER  = 2,  // referencia/music/dungeon002.ogg (Loop dentro de bunkers/estruturas na missao)
    EXTRACTION_SOUND = 3, // referencia/music/Stalked In The Woods (reversed).wav (Barulho da extracao)
    DEATH_SCREEN    = 4,  // referencia/music/tela de morte.wav (1 unica vez na tela de morte)
    MONSTER_SCREAM  = 5,  // referencia/music/grito do mostros.wav (Grito aterrorizante dos monstros no escuro)
    AK47_SINGLE     = 6,  // referencia/music/ak47_1tiro.wav (Tiro unico / ultima bala da rajada com reverb)
    AK47_BURST      = 7,  // referencia/music/ak47_1tiro_da_rajada.wav (Ciclo continuo de tiros na rajada)
    COUNT           = 8
};

struct DecodedAudioBuffer {
    std::vector<uint8_t> pcmData; // Formato padronizado: 44100 Hz, AUDIO_S16SYS, 2 Canais (Stereo)
    size_t playCursor = 0;
    bool loop = true;
    bool loaded = false;
    std::string sourcePath;
};

class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();

    // Inicializa o dispositivo SDL2 Audio e pre-carrega as 4 trilhas oficiais
    bool init();

    // Encerra o dispositivo de audio e libera buffers
    void shutdown();

    // Ativa uma trilha especifica (se resetCursor=true, reinicia do segundo 0:00). Aplica crossfade suave caso troque de ambiente
    void playTrack(AudioTrack track, bool resetCursor = false, float crossfadeDuration = 1.2f);

    // Dispara um efeito sonoro pontual (SFX) com suporte a áudio posicional 2D (pan e volume por distância)
    void playSFX(AudioTrack sfxTrack, float pan = 0.0f, float volume = 1.0f);

    // Finaliza imediatamente qualquer som em execucao (ex: ao extrair para o Bunker Hub)
    void stopAll();

    // Reinicia os cursores de missao ao iniciar nova incursao na Zona
    void resetMissionCursors();

    AudioTrack getCurrentTrack() const { return m_currentTrack; }
    bool isPlaying() const { return m_isPlaying; }
    void setMasterVolume(float volume); // 0.0f a 1.0f

private:
    SDL_AudioDeviceID m_deviceId = 0;
    SDL_AudioSpec m_deviceSpec{};
    bool m_initialized = false;

    AudioTrack m_currentTrack = AudioTrack::NONE;
    AudioTrack m_fadeTrack = AudioTrack::NONE;
    bool m_isCrossfading = false;
    float m_fadeProgress = 1.0f; // 0.0f a 1.0f
    float m_fadeDuration = 1.2f; // Segundos para transição completa
    bool m_isPlaying = false;
    int m_volumeSDL = SDL_MIX_MAXVOLUME; // 0..128

    struct ActiveSFX {
        AudioTrack track = AudioTrack::NONE;
        size_t cursor = 0;
        float pan = 0.0f;     // -1.0 (Esquerda) .. 0.0 (Centro) .. +1.0 (Direita)
        float volume = 1.0f;  // 0.0 .. 1.0
        bool active = false;
    };
    static constexpr size_t MAX_SIMULTANEOUS_SFX = 8;
    std::array<ActiveSFX, MAX_SIMULTANEOUS_SFX> m_activeSFX{};

    std::array<DecodedAudioBuffer, static_cast<size_t>(AudioTrack::COUNT)> m_tracks;

    bool loadTrackWAV(AudioTrack track, const std::string& virtualPath, bool loop);
    bool loadTrackOGG(AudioTrack track, const std::string& virtualPath, bool loop);
    std::string resolveAudioPath(const std::string& rawPath) const;

    static void SDLCALL audioCallback(void* userdata, Uint8* stream, int len);
};

} // namespace ZoneProtocol::Engine

