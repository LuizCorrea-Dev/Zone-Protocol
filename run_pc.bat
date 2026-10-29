@echo off
setlocal
cd /d "%~dp0"

set "EXE=build\Release\ZoneProtocol.exe"
if exist "build\Release\ZoneProtocol_v012.exe" (
    set "EXE=build\Release\ZoneProtocol_v012.exe"
) else if exist "ZoneProtocol_v012.exe" (
    set "EXE=ZoneProtocol_v012.exe"
) else if exist "build\Release\ZoneProtocol.exe" (
    set "EXE=build\Release\ZoneProtocol.exe"
) else if exist "ZoneProtocol.exe" (
    set "EXE=ZoneProtocol.exe"
)

if not "%~1"=="" (
    "%EXE%" %*
    goto :check_error
)

echo ========================================================
echo   ZONE PROTOCOL [v0.1.2] - Roguelike Tatico Top-Down 2D
echo ========================================================
echo  [1] Iniciar Jogo (1280x800 - 16:10 Steam Deck & PC)
echo  [2] Iniciar em Modo Janela 1280x720 (16:9)
echo ========================================================
set "opt=1"
set /p opt="Escolha uma opcao [1-2] (Padrao: 1): "

if "%opt%"=="2" (
    echo Iniciando no modo Janela 1280x720 (16:9)...
    "%EXE%" --169 --hd
) else (
    echo Iniciando no modo 1280x800 (16:10 Steam Deck & PC)...
    "%EXE%" --1610
)

:check_error
set "EXIT_CODE=%errorlevel%"
if not "%EXIT_CODE%"=="0" (
    echo.
    echo ========================================================
    echo  [ZONE PROTOCOL] O processo encerrou com codigo: %EXIT_CODE%
    if exist "crash.log" (
        echo  [CRASH] Relatorio critico em crash.log:
        echo --------------------------------------------------------
        type "crash.log"
        echo --------------------------------------------------------
    ) else if exist "zoneprotocol.log" (
        echo  [LOG] Ultimas linhas de zoneprotocol.log:
        echo --------------------------------------------------------
        powershell -Command "Get-Content zoneprotocol.log -Tail 20"
        echo --------------------------------------------------------
    )
    echo ========================================================
    pause
)
