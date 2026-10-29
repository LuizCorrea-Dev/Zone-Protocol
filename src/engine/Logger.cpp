#include "Logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <exception>
#include <filesystem>
#include <algorithm>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")
#endif

namespace ZoneProtocol::Engine {

struct LoadedResourceEntry {
    std::string category;
    std::string path;
    int width = 0;
    int height = 0;
    bool success = true;
};

std::ofstream Logger::s_rptFile;
std::ofstream Logger::s_errorFile;
std::ofstream Logger::s_scriptFile;
std::recursive_mutex Logger::s_mutex;

std::string Logger::s_logsDir = "logs";
std::string Logger::s_rptFilePath = "logs/ZoneProtocol_x64.RPT";
std::string Logger::s_errorFilePath = "logs/error.log";
std::string Logger::s_scriptFilePath = "logs/script.log";
std::string Logger::s_sessionTimestamp = "";
int Logger::s_indentLevel = 0;

std::deque<std::string> Logger::s_recentLogs;
std::deque<std::string> Logger::s_recentScriptLogs;
GameStateDiagnosticSnapshot Logger::s_snapshot;
bool Logger::s_initialized = false;

static std::vector<LoadedResourceEntry> s_loadedResources;
static int s_totalWarnings = 0;
static int s_totalErrors = 0;

std::string Logger::getTimeOnly() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    struct tm buf;
#if defined(_WIN32)
    localtime_s(&buf, &in_time_t);
#else
    localtime_r(&in_time_t, &buf);
#endif
    ss << std::put_time(&buf, "%H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    struct tm buf;
#if defined(_WIN32)
    localtime_s(&buf, &in_time_t);
#else
    localtime_r(&in_time_t, &buf);
#endif
    ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::getDateTimeString() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    struct tm buf;
#if defined(_WIN32)
    localtime_s(&buf, &in_time_t);
#else
    localtime_r(&in_time_t, &buf);
#endif
    ss << std::put_time(&buf, "%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

void Logger::pushIndent() {
    std::lock_guard<std::recursive_mutex> lock(s_mutex);
    if (s_indentLevel < 8) s_indentLevel++;
}

void Logger::popIndent() {
    std::lock_guard<std::recursive_mutex> lock(s_mutex);
    if (s_indentLevel > 0) s_indentLevel--;
}

void Logger::pushRecentLog(const std::string& line) {
    s_recentLogs.push_back(line);
    if (s_recentLogs.size() > 65) {
        s_recentLogs.pop_front();
    }
}

void Logger::pushRecentScriptLog(const std::string& line) {
    s_recentScriptLogs.push_back(line);
    if (s_recentScriptLogs.size() > 65) {
        s_recentScriptLogs.pop_front();
    }
}

std::deque<std::string> Logger::getRecentLogs() {
    std::lock_guard<std::recursive_mutex> lock(s_mutex);
    return s_recentLogs;
}

std::deque<std::string> Logger::getRecentScriptLogs() {
    std::lock_guard<std::recursive_mutex> lock(s_mutex);
    return s_recentScriptLogs;
}

void Logger::updateGameSnapshot(const GameStateDiagnosticSnapshot& snapshot) {
    std::lock_guard<std::recursive_mutex> lock(s_mutex);
    s_snapshot = snapshot;
}

GameStateDiagnosticSnapshot Logger::getGameSnapshot() {
    std::lock_guard<std::recursive_mutex> lock(s_mutex);
    return s_snapshot;
}

void Logger::registerResourceLoad(const std::string& category, const std::string& resourcePath, int width, int height, bool success) {
    std::lock_guard<std::recursive_mutex> lock(s_mutex);
    s_loadedResources.push_back({ category, resourcePath, width, height, success });
}

std::string Logger::normalizeSubsystemColumn(const std::string& rawTag) {
    std::string mapped = rawTag;
    if (rawTag == "INIT" || rawTag == "MAIN" || rawTag == "SYSTEM" || rawTag == "PLATFORM" || rawTag == "HAL") {
        mapped = "ENGINE";
    } else if (rawTag == "ATLAS" || rawTag == "VFS" || rawTag == "TEXTURE" || rawTag == "MODS") {
        mapped = "RESOURCES";
    } else if (rawTag == "HUD" || rawTag == "UI" || rawTag == "BUNKER") {
        mapped = "GUI";
    } else if (rawTag == "WORLDVIEW" || rawTag == "LIGHT") {
        mapped = "RENDER";
    } else if (rawTag == "MAP" || rawTag == "ZONE") {
        mapped = "WORLD";
    } else if (rawTag == "MUTANT" || rawTag == "PLAYER") {
        mapped = "ENTITY";
    } else if (rawTag == "INPUT") {
        mapped = "INPUTS";
    } else if (rawTag == "BRIEFCASE") {
        mapped = "INVENTORY";
    } else if (rawTag == "GAME") {
        mapped = "SCRIPT";
    }

    if (mapped.size() < 9) {
        mapped.append(9 - mapped.size(), ' ');
    } else if (mapped.size() > 9) {
        mapped = mapped.substr(0, 9);
    }
    return mapped;
}

std::string Logger::formatSourceRef(const char* srcFile, int srcLine) {
    if (!srcFile || srcLine <= 0) return "";
    std::string p(srcFile);
    std::replace(p.begin(), p.end(), '\\', '/');
    size_t srcPos = p.find("src/");
    if (srcPos != std::string::npos) {
        p = p.substr(srcPos);
    } else {
        size_t slash = p.find_last_of('/');
        if (slash != std::string::npos) p = p.substr(slash + 1);
    }
    return "@\"" + p + "," + std::to_string(srcLine) + "\": ";
}

std::string Logger::formatGameSnapshotReport() {
    GameStateDiagnosticSnapshot snap = getGameSnapshot();
    std::stringstream ss;
    ss << "-------------------------------------------------------------------------------\n"
       << "TELEMETRIA EM TEMPO REAL DO MOTOR (LIVE GAME STATE SNAPSHOT):\n"
       << "  Estado do Jogo (FSM): " << snap.gameLoopState << " | Uptime: " << std::fixed << std::setprecision(2) << snap.uptimeSeconds << "s | FPS: " << snap.fps << "\n"
       << "  Setor Ativo / Seed:   " << snap.sectorTitle << " (Seed: " << snap.sectorSeed << ")\n"
       << "  Resolucao Logica:     " << snap.screenW << "x" << snap.screenH << " | Mouse Logico: (" << snap.mouseLogicalX << ", " << snap.mouseLogicalY << ")\n"
       << "  Operador (Player):    Pos=(" << std::setprecision(2) << snap.playerX << ", " << snap.playerY << ")"
       << " | Dir=(" << snap.playerDirX << ", " << snap.playerDirY << ")"
       << " | HP=" << snap.playerHP << " | Stamina=" << snap.playerStamina << "\n"
       << "  Combate & Mutantes:   Vivos=" << snap.mutantsAlive << " | Abatidos=" << snap.mutantsKilled << " | Projeteis Ativos=" << snap.activeProjectiles << "\n";
    if (snap.nearestMutantId >= 0) {
        ss << "  Mutante Mais Proximo: ID=#" << snap.nearestMutantId
           << " | Tipo=" << snap.nearestMutantType
           << " | Estado IA=" << snap.nearestMutantAI
           << " | Pos=(" << snap.nearestMutantX << ", " << snap.nearestMutantY << ")"
           << " | Distancia=" << snap.nearestMutantDist << "m\n";
    } else {
        ss << "  Mutante Mais Proximo: Nenhum mutante vivo no raio de busca\n";
    }
    ss << "  Maleta Tatica 6x4:    " << (snap.briefcaseOpen ? "ABERTA" : "FECHADA")
       << " | Itens=" << snap.briefcaseItems << " | Links de Sinergia Ativos=" << snap.synergyLinks << "\n";
    return ss.str();
}

#if defined(_WIN32)
static void writeMiniDumpFile(EXCEPTION_POINTERS* exceptionInfo, const std::string& dmpPath) {
    HANDLE hFile = CreateFileA(
        dmpPath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (hFile == INVALID_HANDLE_VALUE) return;

    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = exceptionInfo;
    mei.ClientPointers = FALSE;

    MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MiniDumpNormal,
        exceptionInfo ? &mei : nullptr,
        nullptr,
        nullptr
    );
    CloseHandle(hFile);
}

static std::string resolveSymbolCallStack(CONTEXT* contextRecord) {
    std::stringstream stackSS;
    HANDLE process = GetCurrentProcess();
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    SymInitialize(process, nullptr, TRUE);

    void* stackFrames[32];
    WORD captured = CaptureStackBackTrace(0, 32, stackFrames, nullptr);
    (void)contextRecord;

    for (WORD i = 0; i < captured; ++i) {
        DWORD64 frameAddr = reinterpret_cast<DWORD64>(stackFrames[i]);
        HMODULE fMod = nullptr;
        char frameModName[MAX_PATH] = "???";
        uintptr_t fBase = 0;
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               reinterpret_cast<LPCSTR>(frameAddr), &fMod) && fMod) {
            char fullPath[MAX_PATH];
            GetModuleFileNameA(fMod, fullPath, MAX_PATH);
            std::string p(fullPath);
            size_t slash = p.find_last_of("\\/");
            if (slash != std::string::npos) {
                strncpy_s(frameModName, p.substr(slash + 1).c_str(), sizeof(frameModName) - 1);
            } else {
                strncpy_s(frameModName, fullPath, sizeof(frameModName) - 1);
            }
            fBase = reinterpret_cast<uintptr_t>(fMod);
        }
        uintptr_t fOffset = static_cast<uintptr_t>(frameAddr) - fBase;

        // Resolve nome da função C++ e arquivo/linha via PDB (DbgHelp)
        char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO pSymbol = reinterpret_cast<PSYMBOL_INFO>(symbolBuffer);
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        pSymbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        std::string funcDesc = "";
        if (SymFromAddr(process, frameAddr, &displacement, pSymbol)) {
            funcDesc = std::string(" -> ") + pSymbol->Name + "()";
            IMAGEHLP_LINE64 lineInfo{};
            lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            DWORD lineDisp = 0;
            if (SymGetLineFromAddr64(process, frameAddr, &lineDisp, &lineInfo) && lineInfo.FileName) {
                std::string fileStr(lineInfo.FileName);
                std::replace(fileStr.begin(), fileStr.end(), '\\', '/');
                size_t srcPos = fileStr.find("src/");
                if (srcPos != std::string::npos) fileStr = fileStr.substr(srcPos);
                funcDesc += " [@\"" + fileStr + "," + std::to_string(lineInfo.LineNumber) + "\"]";
            }
        }

        stackSS << "  [" << std::setfill('0') << std::setw(2) << i << "] 0x"
                << std::hex << frameAddr << std::dec
                << " (" << frameModName << " + 0x" << std::hex << fOffset << std::dec << ")"
                << funcDesc << "\n";
    }
    return stackSS.str();
}

static LONG WINAPI windowsCrashHandler(EXCEPTION_POINTERS* exceptionInfo) {
    DWORD code = exceptionInfo->ExceptionRecord->ExceptionCode;
    void* addr = exceptionInfo->ExceptionRecord->ExceptionAddress;

    bool isFatalCrash = (code == EXCEPTION_ACCESS_VIOLATION ||
                         code == 0xC0000374 || // STATUS_HEAP_CORRUPTION
                         code == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ||
                         code == EXCEPTION_INT_DIVIDE_BY_ZERO ||
                         code == EXCEPTION_STACK_OVERFLOW ||
                         code == EXCEPTION_ILLEGAL_INSTRUCTION ||
                         code == EXCEPTION_IN_PAGE_ERROR ||
                         code == EXCEPTION_DATATYPE_MISALIGNMENT);
    if (!isFatalCrash) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    static bool s_alreadyHandlingCrash = false;
    if (s_alreadyHandlingCrash) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    s_alreadyHandlingCrash = true;

    std::string crashReason = "UNKNOWN_FATAL_EXCEPTION";
    std::string accessDetails = "";

    switch (code) {
        case 0xC0000374:
            crashReason = "STATUS_HEAP_CORRUPTION (0xC0000374 - Escrita fora dos limites de buffer na Heap)";
            break;
        case EXCEPTION_ACCESS_VIOLATION: {
            crashReason = "EXCEPTION_ACCESS_VIOLATION (0xC0000005 - Violacao de Acesso a Memoria)";
            ULONG_PTR accessType = exceptionInfo->ExceptionRecord->ExceptionInformation[0];
            ULONG_PTR targetAddr = exceptionInfo->ExceptionRecord->ExceptionInformation[1];
            std::stringstream ss;
            ss << "Operacao: "
               << (accessType == 0 ? "READ (Leitura Invalida)" : (accessType == 1 ? "WRITE (Escrita Invalida)" : "DEP (Execucao Invalida)"))
               << " | Endereco Alvo: 0x" << std::hex << targetAddr;
            if (targetAddr < 0x1000) {
                ss << " (Ponteiro NULL / Desreferencia Nula)";
            }
            accessDetails = ss.str();
            break;
        }
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            crashReason = "EXCEPTION_ARRAY_BOUNDS_EXCEEDED (Indice fora dos limites)";
            break;
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            crashReason = "EXCEPTION_INT_DIVIDE_BY_ZERO (Divisao inteira por zero)";
            break;
        case EXCEPTION_STACK_OVERFLOW:
            crashReason = "EXCEPTION_STACK_OVERFLOW (Estouro de pilha / Recursao infinita)";
            break;
        case EXCEPTION_ILLEGAL_INSTRUCTION:
            crashReason = "EXCEPTION_ILLEGAL_INSTRUCTION (Instrucao de CPU invalida)";
            break;
        default: {
            std::stringstream ss;
            ss << "EXCEPTION_CODE: 0x" << std::hex << code;
            crashReason = ss.str();
            break;
        }
    }

    char faultModule[MAX_PATH] = "Desconhecido";
    uintptr_t faultModuleBase = 0;
    HMODULE hMod = nullptr;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCSTR>(addr), &hMod) && hMod) {
        GetModuleFileNameA(hMod, faultModule, MAX_PATH);
        faultModuleBase = reinterpret_cast<uintptr_t>(hMod);
    }
    uintptr_t offsetInModule = reinterpret_cast<uintptr_t>(addr) - faultModuleBase;

    MEMORYSTATUSEX memStatus{};
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);
    DWORDLONG totalPhysMB = memStatus.ullTotalPhys / (1024 * 1024);
    DWORDLONG availPhysMB = memStatus.ullAvailPhys / (1024 * 1024);

    std::stringstream regsSS;
#if defined(_M_X64) || defined(__x86_64__)
    CONTEXT* ctx = exceptionInfo->ContextRecord;
    regsSS << "  RIP: 0x" << std::hex << ctx->Rip << " | RSP: 0x" << ctx->Rsp << " | RBP: 0x" << ctx->Rbp << "\n"
           << "  RAX: 0x" << ctx->Rax << " | RBX: 0x" << ctx->Rbx << " | RCX: 0x" << ctx->Rcx << " | RDX: 0x" << ctx->Rdx << "\n"
           << "  RSI: 0x" << ctx->Rsi << " | RDI: 0x" << ctx->Rdi << " | R8:  0x" << ctx->R8  << " | R9:  0x" << ctx->R9  << "\n"
           << "  EFLAGS: 0x" << ctx->EFlags << "\n";
#endif

    std::string crashTimestamp = Logger::getTimestamp();
    std::string crashFileDT = Logger::getDateTimeString();
    std::string dmpFile = "logs/ZoneProtocol_x64_" + crashFileDT + ".dmp";
    writeMiniDumpFile(exceptionInfo, dmpFile);

    std::string stackTraceStr = resolveSymbolCallStack(exceptionInfo->ContextRecord);

    std::stringstream crashMsg;
    crashMsg << "\n===============================================================================\n"
             << "Exception code: 0x" << std::hex << code << " " << crashReason << " at 0x" << addr << "\n"
             << "===============================================================================\n"
             << "Data/Hora:         " << crashTimestamp << "\n"
             << "Motivo:            " << crashReason << "\n";
    if (!accessDetails.empty()) {
        crashMsg << "Detalhes:          " << accessDetails << "\n";
    }
    crashMsg << "Modulo da Falha:   " << faultModule << " (+0x" << std::hex << offsetInModule << std::dec << ")\n"
             << "MiniDump Binario:  " << dmpFile << "\n"
             << "Memoria Fisica:    " << availPhysMB << " MB livres de " << totalPhysMB << " MB (Uso: " << memStatus.dwMemoryLoad << "%)\n"
             << Logger::formatGameSnapshotReport()
             << "-------------------------------------------------------------------------------\n"
             << "REGISTRADORES DE CPU (x64):\n"
             << regsSS.str()
             << "-------------------------------------------------------------------------------\n"
             << "PILHA DE CHAMADAS SIMBOLICA (CALL STACK COM FUNCAO E LINHA):\n"
             << stackTraceStr
             << "-------------------------------------------------------------------------------\n"
             << "ULTIMOS EVENTOS DE GAMEPLAY (script.log):\n";

    for (const auto& sLine : Logger::getRecentScriptLogs()) {
        crashMsg << "  " << sLine << "\n";
    }
    crashMsg << "-------------------------------------------------------------------------------\n"
             << "ULTIMOS EVENTOS DO MOTOR (.RPT):\n";
    for (const auto& rLine : Logger::getRecentLogs()) {
        crashMsg << "  " << rLine << "\n";
    }
    crashMsg << "===============================================================================\n";

    std::string report = crashMsg.str();

    Logger::critical(report, "ENGINE");

    std::string dialogMsg =
        "Ocorreu um erro critico no ZONE PROTOCOL [v0.1.2].\n\n"
        "Motivo: " + crashReason + "\n" +
        (accessDetails.empty() ? "" : (accessDetails + "\n")) +
        "\nOs relatórios de diagnóstico e o MiniDump foram salvos em:\n"
        "- " + Logger::getRptPath() + "\n"
        "- " + Logger::getErrorLogPath() + "\n"
        "- " + Logger::getScriptLogPath() + "\n"
        "- " + dmpFile;

    MessageBoxA(nullptr, dialogMsg.c_str(), "ZONE PROTOCOL [v0.1.2] - Crash Diagnostic Report", MB_OK | MB_ICONERROR | MB_TASKMODAL);
    TerminateProcess(GetCurrentProcess(), code);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

static void terminateHandler() {
    std::string reason = "std::terminate acionado (excecao C++ nao capturada)";
    std::exception_ptr ex = std::current_exception();
    if (ex) {
        try {
            std::rethrow_exception(ex);
        } catch (const std::exception& e) {
            reason += std::string(" -> ") + e.what();
        } catch (...) {
            reason += " -> Excecao C++ desconhecida";
        }
    }

    std::stringstream ss;
    ss << "\n===============================================================================\n"
       << "EXCECAO C++ FATAL NAO TRATADA: " << reason << "\n"
       << Logger::formatGameSnapshotReport()
       << "===============================================================================\n";

    Logger::critical(ss.str(), "ENGINE");
    std::abort();
}

void Logger::installCrashHandler() {
    std::set_terminate(terminateHandler);
#if defined(_WIN32)
    SetUnhandledExceptionFilter(windowsCrashHandler);
    AddVectoredExceptionHandler(0, windowsCrashHandler);
#endif
}

void Logger::init(int argc, char* argv[]) {
    std::lock_guard<std::recursive_mutex> lock(s_mutex);
    if (s_initialized) return;

    s_loadedResources.clear();
    s_totalWarnings = 0;
    s_totalErrors = 0;
    s_indentLevel = 0;

    std::string exeFullPath = "ZoneProtocol_v012.exe";
    std::string exeBuildTime = __DATE__ " " __TIME__;
    std::string baseDir = ".";

#if defined(_WIN32)
    char modulePathBuf[MAX_PATH] = {0};
    if (GetModuleFileNameA(nullptr, modulePathBuf, MAX_PATH) > 0) {
        exeFullPath = modulePathBuf;
        std::filesystem::path ep(exeFullPath);
        if (ep.has_parent_path()) {
            baseDir = ep.parent_path().string();
        }
        std::filesystem::path parentCheck = ep.parent_path().parent_path().parent_path();
        if (ep.parent_path().filename() == "Release" && std::filesystem::exists(parentCheck / "CMakeLists.txt")) {
            baseDir = parentCheck.string();
        }

        WIN32_FILE_ATTRIBUTE_DATA fileAttr{};
        if (GetFileAttributesExA(exeFullPath.c_str(), GetFileExInfoStandard, &fileAttr)) {
            SYSTEMTIME stUTC{}, stLocal{};
            FileTimeToSystemTime(&fileAttr.ftLastWriteTime, &stUTC);
            SystemTimeToTzSpecificLocalTime(nullptr, &stUTC, &stLocal);
            char timeBuf[64];
            std::snprintf(timeBuf, sizeof(timeBuf), "%04d/%02d/%02d %02d:%02d:%02d",
                          stLocal.wYear, stLocal.wMonth, stLocal.wDay,
                          stLocal.wHour, stLocal.wMinute, stLocal.wSecond);
            exeBuildTime = timeBuf;
        }
    }
#endif

    std::string cmdLineStr = "\"" + exeFullPath + "\"";
    for (int i = 1; i < argc; ++i) {
        if (argv && argv[i]) {
            cmdLineStr += " ";
            cmdLineStr += argv[i];
        }
    }

    s_logsDir = baseDir + "/logs";
    std::error_code ec;
    std::filesystem::create_directories(s_logsDir, ec);
    std::filesystem::create_directories("logs", ec);

    s_sessionTimestamp = getDateTimeString();

    // 1. ZoneProtocol_x64_YYYY-MM-DD_HH-MM-SS.RPT
    // 2. error_YYYY-MM-DD_HH-MM-SS.log
    // 3. script_YYYY-MM-DD_HH-MM-SS.log
    s_rptFilePath    = s_logsDir + "/ZoneProtocol_x64_" + s_sessionTimestamp + ".RPT";
    s_errorFilePath  = s_logsDir + "/error_" + s_sessionTimestamp + ".log";
    s_scriptFilePath = s_logsDir + "/script_" + s_sessionTimestamp + ".log";

    s_rptFile.open(s_rptFilePath, std::ios::out | std::ios::trunc);
    s_errorFile.open(s_errorFilePath, std::ios::out | std::ios::trunc);
    s_scriptFile.open(s_scriptFilePath, std::ios::out | std::ios::trunc);

    s_initialized = true;
    installCrashHandler();
    std::atexit(Logger::shutdown);

    // Formata data e hora para os cabeçalhos de sessão
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    struct tm buf;
#if defined(_WIN32)
    localtime_s(&buf, &in_time_t);
#else
    localtime_r(&in_time_t, &buf);
#endif
    std::stringstream curSlashSS, curDashSS, curDotSS;
    curSlashSS << std::put_time(&buf, "%Y/%m/%d %H:%M:%S");
    curDashSS  << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
    curDotSS   << std::put_time(&buf, "%d.%m. %Y - %H:%M:%S");

    // 1. Cabeçalho do relatório mestre .RPT
    std::stringstream rptHeader;
    rptHeader << "=====================================================================\n"
              << "== " << exeFullPath << "\n"
              << "== " << cmdLineStr << "\n"
              << "=====================================================================\n"
              << "Exe timestamp: " << exeBuildTime << "\n"
              << "Current time:  " << curSlashSS.str() << "\n"
              << "Version 0.1.2.1337\n"
              << "=====================================================================\n\n";

    if (s_rptFile.is_open()) {
        s_rptFile << rptHeader.str();
        s_rptFile.flush();
    }
    std::cout << rptHeader.str();

    // 2. Cabeçalho do error_*.log
    if (s_errorFile.is_open()) {
        s_errorFile << "Log " << s_errorFilePath << " started at " << curDashSS.str() << "\n\n";
        s_errorFile.flush();
    }

    // 3. Cabeçalho do script_*.log
    if (s_scriptFile.is_open()) {
        s_scriptFile << "---------------------------------------------\n"
                     << "Log " << s_scriptFilePath << " started at " << curDotSS.str() << "\n\n";
        s_scriptFile.flush();
    }

    // Telemetria inicial de Hardware / Memória / OS no .RPT
#if defined(_WIN32)
    SYSTEM_INFO sysInfo{};
    GetSystemInfo(&sysInfo);
    MEMORYSTATUSEX memStatus{};
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);
    DWORDLONG totalPhysMB = memStatus.ullTotalPhys / (1024 * 1024);
    DWORDLONG availPhysMB = memStatus.ullAvailPhys / (1024 * 1024);

    info("OS: Windows x64 | Logical CPU Cores: " + std::to_string(sysInfo.dwNumberOfProcessors) +
         " | Physical RAM: " + std::to_string(availPhysMB) + "/" + std::to_string(totalPhysMB) +
         " MB free (Load: " + std::to_string(memStatus.dwMemoryLoad) + "%)", "ENGINE");
#endif
    info("Diagnostic Logs Initialized -> RPT: '" + s_rptFilePath + "' | ERROR: '" + s_errorFilePath + "' | SCRIPT: '" + s_scriptFilePath + "'", "ENGINE");
    script("Initializing globals 'ZoneProtocol' v0.1.2 (16:10 Widescreen 1280x800 Steam Deck & PC)");
    script("Module: Core+Gameplay+UI; defines: \"ZONE_PROTOCOL_0_1_2,PLATFORM_WINDOWS,RELEASE,WIDESCREEN_16_10,DDA_LIGHTCASTER,SYNERGY_BRIEFCASE_6X4,SAFE_POCKET_2X2,BUNKER_HUB\"");
}

void Logger::shutdown() {
    std::lock_guard<std::recursive_mutex> lock(s_mutex);
    if (!s_initialized) return;

    script("~ZoneProtocolGame()");

    // Sumário de Recursos Carregados e Contagem de Alertas no encerramento do .RPT
    if (s_rptFile.is_open()) {
        s_rptFile << getTimeOnly() << " RESOURCES    : ==== Loaded Resource Audit (" << s_loadedResources.size() << " assets) ====\n";
        for (const auto& res : s_loadedResources) {
            s_rptFile << getTimeOnly() << "  RESOURCES   : [" << (res.success ? "OK" : "MISSING") << "] "
                      << res.category << " @\"" << res.path << "\"";
            if (res.width > 0 && res.height > 0) {
                s_rptFile << " (" << res.width << "x" << res.height << ")";
            }
            s_rptFile << "\n";
        }
        s_rptFile << getTimeOnly() << " ENGINE       : Session Summary -> Warnings: " << s_totalWarnings
                  << " | Errors: " << s_totalErrors << "\n";
        s_rptFile << getTimeOnly() << "  --- Termination successfully completed --- \n";
        s_rptFile.flush();
        s_rptFile.close();
    }

    if (s_errorFile.is_open()) {
        s_errorFile << getTimeOnly() << " ENGINE       : Session ended cleanly (Warnings: " << s_totalWarnings
                    << ", Errors: " << s_totalErrors << ")\n";
        s_errorFile.flush();
        s_errorFile.close();
    }

    if (s_scriptFile.is_open()) {
        s_scriptFile.flush();
        s_scriptFile.close();
    }

    s_initialized = false;
}

void Logger::script(const std::string& msg, LogLevel level, const char* srcFile, int srcLine) {
    log(level, "SCRIPT", msg, srcFile, srcLine);
}

void Logger::log(LogLevel level, const std::string& tag, const std::string& msg, const char* srcFile, int srcLine) {
    std::lock_guard<std::recursive_mutex> lock(s_mutex);

    std::string timeCol = getTimeOnly();
    std::string catCol  = normalizeSubsystemColumn(tag);
    std::string indent(static_cast<size_t>(s_indentLevel), ' ');
    std::string srcRef  = formatSourceRef(srcFile, srcLine);

    std::string sevCol = "   : ";
    if (level == LogLevel::WARN) {
        sevCol = "(W): ";
        s_totalWarnings++;
    } else if (level == LogLevel::ERR) {
        sevCol = "(E): ";
        s_totalErrors++;
    } else if (level == LogLevel::CRITICAL) {
        sevCol = "(C): ";
        s_totalErrors++;
    }

    // Formato de linha de telemetria:
    // 09:20:26.185 RESOURCES (W): @"src/engine/SpriteAtlas.cpp,98": Arquivo nao encontrado...
    // 09:20:32.393 SCRIPT       : [KeybindsManager] Registered bind: ...
    std::string formattedLine = timeCol + " " + indent + catCol + " " + sevCol + srcRef + msg;

    pushRecentLog(formattedLine);

    // 1. Grava sempre no .RPT mestre
    if (s_rptFile.is_open()) {
        s_rptFile << formattedLine << "\n";
        s_rptFile.flush();
    }

    // 2. Se for categoria SCRIPT (ou gameplay), grava também no script_*.log
    if (catCol.find("SCRIPT") == 0 || catCol.find("COMBAT") == 0 || catCol.find("INVENTORY") == 0 || catCol.find("WORLD") == 0) {
        pushRecentScriptLog(formattedLine);
        if (s_scriptFile.is_open()) {
            s_scriptFile << formattedLine << "\n";
            s_scriptFile.flush();
        }
    }

    // 3. Se for Warning (W), Error (E) ou Critical (C), grava imediatamente no error_*.log
    if (level == LogLevel::WARN || level == LogLevel::ERR || level == LogLevel::CRITICAL) {
        if (s_errorFile.is_open()) {
            s_errorFile << formattedLine << "\n";
            if (level == LogLevel::ERR || level == LogLevel::CRITICAL) {
                s_errorFile << formatGameSnapshotReport();
            }
            s_errorFile.flush();
        }
        std::cerr << formattedLine << std::endl;
    } else {
        std::cout << formattedLine << std::endl;
    }
}

void Logger::info(const std::string& msg, const std::string& tag) {
    log(LogLevel::INFO, tag, msg);
}

void Logger::warn(const std::string& msg, const std::string& tag, const char* srcFile, int srcLine) {
    log(LogLevel::WARN, tag, msg, srcFile, srcLine);
}

void Logger::error(const std::string& msg, const std::string& tag, const char* srcFile, int srcLine) {
    log(LogLevel::ERR, tag, msg, srcFile, srcLine);
}

void Logger::critical(const std::string& msg, const std::string& tag, const char* srcFile, int srcLine) {
    log(LogLevel::CRITICAL, tag, msg, srcFile, srcLine);
}

const std::string& Logger::getRptPath() {
    return s_rptFilePath;
}

const std::string& Logger::getErrorLogPath() {
    return s_errorFilePath;
}

const std::string& Logger::getScriptLogPath() {
    return s_scriptFilePath;
}

} // namespace ZoneProtocol::Engine
