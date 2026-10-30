cmake_minimum_required(VERSION 3.16)
project(ZoneProtocol VERSION 0.1.5 LANGUAGES CXX C RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Resolução canônica interna e metadados oficiais do Zone Protocol v0.1.5
add_compile_definitions(
    SCREEN_WIDTH=640
    SCREEN_HEIGHT=400
    WINDOW_TITLE="ZONE PROTOCOL [v0.1.5] - Roguelike Tatico Top-Down 2D"
    ZONE_PROTOCOL_VERSION="v0.1.5"
)

# Inclusão de diretórios de código
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game
    ${CMAKE_CURRENT_SOURCE_DIR}/src/game/UI
    ${CMAKE_CURRENT_SOURCE_DIR}/src/objetos
)

# Detecção, inclusão local ou download automático da SDL2
find_package(SDL2 QUIET)

if(TARGET SDL2::SDL2)
    set(SDL2_LIBRARIES SDL2::SDL2)
elseif(TARGET SDL2::SDL2-static)
    set(SDL2_LIBRARIES SDL2::SDL2-static)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL2/CMakeLists.txt")
    message(STATUS "Usando fontes locais do SDL2 em external/SDL2...")
    set(SDL_SHARED TRUE CACHE BOOL "Build shared SDL library" FORCE)
    set(SDL_STATIC FALSE CACHE BOOL "Build static SDL library" FORCE)
    set(SDL_TEST FALSE CACHE BOOL "Build SDL test programs" FORCE)
    add_subdirectory(external/SDL2 EXCLUDE_FROM_ALL)
    set(SDL2_LIBRARIES SDL2::SDL2)
else()
    message(STATUS "Baixando SDL2 via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.30.2
        GIT_SHALLOW TRUE
    )
    set(SDL_SHARED TRUE CACHE BOOL "Build shared SDL library" FORCE)
    set(SDL_STATIC FALSE CACHE BOOL "Build static SDL library" FORCE)
    set(SDL_TEST FALSE CACHE BOOL "Build SDL test programs" FORCE)
    FetchContent_MakeAvailable(SDL2)
    set(SDL2_LIBRARIES SDL2::SDL2)
endif()

# Arquivos de código-fonte
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.c"
    "src/*.hpp"
    "src/*.h"
)

if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/version.rc")
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/version.rc")
endif()

add_executable(ZoneProtocol ${SOURCES})

if(MSVC)
    target_compile_options(ZoneProtocol PRIVATE /Zi)
    target_link_options(ZoneProtocol PRIVATE /DEBUG /OPT:REF /OPT:ICF)
endif()

if(WIN32)
    target_link_libraries(ZoneProtocol PRIVATE dbghelp psapi)
endif()

if(TARGET SDL2::SDL2main)
    target_link_libraries(ZoneProtocol PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(ZoneProtocol PRIVATE ${SDL2_LIBRARIES})

# Geração automática do executável versionado e cópia para a raiz
add_custom_command(TARGET ZoneProtocol POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:ZoneProtocol>"
    "$<TARGET_FILE_DIR:ZoneProtocol>/ZoneProtocol_v015.exe"
    COMMENT "Gerando executavel versionado ZoneProtocol_v015.exe"
)

add_custom_command(TARGET ZoneProtocol POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:ZoneProtocol>"
    "${CMAKE_CURRENT_SOURCE_DIR}/ZoneProtocol_v015.exe"
    COMMENT "Gerando executavel versionado ZoneProtocol_v015.exe na raiz"
)

add_custom_command(TARGET ZoneProtocol POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "$<TARGET_FILE:ZoneProtocol>"
    "${CMAKE_CURRENT_SOURCE_DIR}/ZoneProtocol.exe"
    COMMENT "Atualizando executavel ZoneProtocol.exe na raiz"
)

# Copiar SDL2.dll se existir na raiz para a pasta de saída do executável
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2.dll")
    add_custom_command(TARGET ZoneProtocol POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL2.dll"
        "$<TARGET_FILE_DIR:ZoneProtocol>/SDL2.dll"
    )
endif()

# Copiar assets para a pasta de saída do executável
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET ZoneProtocol POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:ZoneProtocol>/assets"
    )
endif()
