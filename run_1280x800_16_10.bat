@echo off
cd /d "%~dp0"
echo Iniciando ZONE PROTOCOL [v0.1.2] em 1280x800 (16:10 Steam Deck & PC)...
if exist "build\Release\ZoneProtocol_v012.exe" (
    start "" "build\Release\ZoneProtocol_v012.exe" --1610
) else if exist "ZoneProtocol_v012.exe" (
    start "" "ZoneProtocol_v012.exe" --1610
) else if exist "build\Release\ZoneProtocol.exe" (
    start "" "build\Release\ZoneProtocol.exe" --1610
) else if exist "ZoneProtocol.exe" (
    start "" "ZoneProtocol.exe" --1610
) else (
    echo Executavel nao encontrado. Por favor, compile o projeto primeiro.
    pause
)
