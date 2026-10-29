#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <deque>
#include <functional>

namespace ZoneProtocol::Engine {

enum class LogLevel {
    INFO,
    WARN,
    ERR,
    CRITICAL
};

// Categorias de subsistema para os arquivos de diagnóstico (.RPT / error.log / script.log)
enum class LogCategory {
    DEFAULT,
    ENGINE,
    RESOURCES,
    GUI,
    RENDER,
    WORLD,
    ENTITY,
    INPUTS,
    COMBAT,
    INVENTORY,
    SCRIPT,
    PROFILING
};

// Snapshot em tempo real do estado do jogo para inclusão automática em erros e crashes
struct GameStateDiagnosticSnapshot {
    std::string gameLoopState = "BOOT";
    std::string sectorTitle   = "NONE";
    uint32_t    sectorSeed    = 0;
    float       playerX       = 0.0f;
    float       playerY       = 0.0f;
    float       playerDirX    = 0.0f;
    float       playerDirY    = 0.0f;
    float       playerHP      = 100.0f;
    float       playerStamina = 100.0f;
    int         mutantsAlive  = 0;
    int         mutantsKilled = 0;
    int         nearestMutantId   = -1;
    std::string nearestMutantType = "NONE";
    std::string nearestMutantAI   = "NONE";
    float       nearestMutantDist = -1.0f;
    float       nearestMutantX    = 0.0f;
    float       nearestMutantY    = 0.0f;
    int         activeProjectiles = 0;
    bool        briefcaseOpen     = false;
    int         briefcaseItems    = 0;
    int         synergyLinks      = 0;
    int         mouseLogicalX     = 0;
    int         mouseLogicalY     = 0;
    int         screenW           = 640;
    int         screenH           = 400;
    float       fps               = 60.0f;
    float       uptimeSeconds     = 0.0f;
};

class Logger {
public:
    // Inicializa os 3 arquivos de telemetria da sessão:
    // 1. logs/ZoneProtocol_x64_YYYY-MM-DD_HH-MM-SS.RPT  (Relatório Mestre de Engine, Hardware, Recursos e Render)
    // 2. logs/error_YYYY-MM-DD_HH-MM-SS.log             (Warnings (W), Errors (E) e Crash Stack/Dump dedicados)
    // 3. logs/script_YYYY-MM-DD_HH-MM-SS.log            (Rastreamento de Gameplay, Missão, FSM, Combate, UI e Maleta)
    static void init(int argc = 0, char* argv[] = nullptr);
    static void shutdown();

    static void info(const std::string& msg, const std::string& tag = "ENGINE");
    static void warn(const std::string& msg, const std::string& tag = "ENGINE", const char* srcFile = nullptr, int srcLine = 0);
    static void error(const std::string& msg, const std::string& tag = "ENGINE", const char* srcFile = nullptr, int srcLine = 0);
    static void critical(const std::string& msg, const std::string& tag = "ENGINE", const char* srcFile = nullptr, int srcLine = 0);

    // Log direcionado para o script_*.log e para o relatório mestre .RPT
    static void script(const std::string& msg, LogLevel level = LogLevel::INFO, const char* srcFile = nullptr, int srcLine = 0);

    // Log estruturado completo com categoria de subsistema e arquivo/linha de origem opcional
    static void log(LogLevel level, const std::string& tag, const std::string& msg, const char* srcFile = nullptr, int srcLine = 0);

    // Controle de indentação hierárquica para escopos aninhados
    static void pushIndent();
    static void popIndent();

    struct ScopedIndent {
        ScopedIndent()  { Logger::pushIndent(); }
        ~ScopedIndent() { Logger::popIndent(); }
    };

    // Atualiza o snapshot de estado do jogo a cada frame para diagnóstico instantâneo
    static void updateGameSnapshot(const GameStateDiagnosticSnapshot& snapshot);
    static GameStateDiagnosticSnapshot getGameSnapshot();
    static std::string formatGameSnapshotReport();

    // Registra contador de recursos carregados para auditoria no encerramento
    static void registerResourceLoad(const std::string& category, const std::string& resourcePath, int width = 0, int height = 0, bool success = true);

    static void installCrashHandler();

    static const std::string& getRptPath();
    static const std::string& getErrorLogPath();
    static const std::string& getScriptLogPath();
    static const std::string& getLogPath() { return getRptPath(); }

    // Formatos de data/hora:
    // getTimeOnly()       -> "HH:MM:SS.mmm"
    // getTimestamp()      -> "YYYY-MM-DD HH:MM:SS.mmm"
    // getDateTimeString() -> "YYYY-MM-DD_HH-MM-SS" (para nomes de arquivos)
    static std::string getTimeOnly();
    static std::string getTimestamp();
    static std::string getDateTimeString();

    static std::deque<std::string> getRecentLogs();
    static std::deque<std::string> getRecentScriptLogs();

private:
    static std::ofstream s_rptFile;
    static std::ofstream s_errorFile;
    static std::ofstream s_scriptFile;
    static std::recursive_mutex s_mutex;

    static std::string s_logsDir;
    static std::string s_rptFilePath;
    static std::string s_errorFilePath;
    static std::string s_scriptFilePath;
    static std::string s_sessionTimestamp;
    static int s_indentLevel;

    static std::deque<std::string> s_recentLogs;
    static std::deque<std::string> s_recentScriptLogs;
    static GameStateDiagnosticSnapshot s_snapshot;
    static bool s_initialized;

    static void pushRecentLog(const std::string& line);
    static void pushRecentScriptLog(const std::string& line);
    static std::string normalizeSubsystemColumn(const std::string& rawTag);
    static std::string formatSourceRef(const char* srcFile, int srcLine);
};

} // namespace ZoneProtocol::Engine

// Macros de diagnóstico rápido com captura automática de arquivo e linha (@"src/arquivo.cpp,123")
#define ZP_LOG_INFO(tag, msg)     ::ZoneProtocol::Engine::Logger::info((msg), (tag))
#define ZP_LOG_WARN(tag, msg)     ::ZoneProtocol::Engine::Logger::warn((msg), (tag), __FILE__, __LINE__)
#define ZP_LOG_ERROR(tag, msg)    ::ZoneProtocol::Engine::Logger::error((msg), (tag), __FILE__, __LINE__)
#define ZP_LOG_CRITICAL(tag, msg) ::ZoneProtocol::Engine::Logger::critical((msg), (tag), __FILE__, __LINE__)
#define ZP_LOG_SCRIPT(msg)        ::ZoneProtocol::Engine::Logger::script((msg), ::ZoneProtocol::Engine::LogLevel::INFO)
#define ZP_LOG_SCRIPT_WARN(msg)   ::ZoneProtocol::Engine::Logger::script((msg), ::ZoneProtocol::Engine::LogLevel::WARN, __FILE__, __LINE__)
#define ZP_LOG_SCRIPT_ERR(msg)    ::ZoneProtocol::Engine::Logger::script((msg), ::ZoneProtocol::Engine::LogLevel::ERR, __FILE__, __LINE__)
