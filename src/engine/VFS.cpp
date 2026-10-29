#include "VFS.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace DayDoom::Engine {

namespace fs = std::filesystem;

VFS& VFS::get() {
    static VFS instance;
    return instance;
}

static std::string normalizePath(const std::string& path) {
    std::string norm = path;
    std::replace(norm.begin(), norm.end(), '\\', '/');
    while (!norm.empty() && norm.back() == '/') {
        norm.pop_back();
    }
    return norm;
}

void VFS::init(const std::string& baseDir) {
    m_baseDir = normalizePath(baseDir);
    clearSearchPaths();

    // Adiciona o diretório de assets como busca padrão
    std::string defaultAssets = m_baseDir + "/assets";
    addSearchPath(defaultAssets, false);

    ZoneProtocol::Engine::Logger::info("VFS Mounted Base Directory: '" + m_baseDir + "'", "RESOURCES");
}

void VFS::addSearchPath(const std::string& path, bool highPriority) {
    std::string norm = normalizePath(path);
    auto it = std::find(m_searchPaths.begin(), m_searchPaths.end(), norm);
    if (it != m_searchPaths.end()) {
        m_searchPaths.erase(it);
    }

    if (highPriority) {
        m_searchPaths.insert(m_searchPaths.begin(), norm);
    } else {
        m_searchPaths.push_back(norm);
    }

    ZoneProtocol::Engine::Logger::info("VFS Search Path Registered: '" + norm + "'", "RESOURCES");
}

void VFS::clearSearchPaths() {
    m_searchPaths.clear();
}

std::string VFS::resolve(const std::string& relativePath) const {
    std::string rel = relativePath;
    if (!rel.empty() && (rel[0] == '/' || rel[0] == '\\')) {
        rel = rel.substr(1);
    }

    // 1. Procura primeiro nos caminhos registrados (mods ativos em ordem de carga, depois assets)
    for (const auto& searchPath : m_searchPaths) {
        std::string full = searchPath + "/" + rel;
        if (fs::exists(full)) {
            return full;
        }
    }

    // 2. Procura direto a partir de m_baseDir
    std::string baseFull = m_baseDir + "/" + rel;
    if (fs::exists(baseFull)) {
        return baseFull;
    }

    // 3. Procura no caminho relativo ao diretório atual de trabalho
    if (fs::exists(rel)) {
        return fs::absolute(rel).string();
    }

    // Fallback: se não achar, retorna o caminho no primeiro diretório registrado (para criação/escrita)
    if (!m_searchPaths.empty()) {
        return m_searchPaths[0] + "/" + rel;
    }

    return m_baseDir + "/" + rel;
}

bool VFS::exists(const std::string& relativePath) const {
    std::string rel = relativePath;
    if (!rel.empty() && (rel[0] == '/' || rel[0] == '\\')) {
        rel = rel.substr(1);
    }

    for (const auto& searchPath : m_searchPaths) {
        std::string full = searchPath + "/" + rel;
        if (fs::exists(full)) return true;
    }

    if (fs::exists(m_baseDir + "/" + rel)) return true;
    if (fs::exists(rel)) return true;

    return false;
}

std::string VFS::readText(const std::string& relativePath) const {
    std::string resolved = resolve(relativePath);
    std::ifstream in(resolved);
    if (!in.is_open()) {
        std::cerr << "[VFS] Falha ao ler arquivo: " << relativePath << " (Resolvido: " << resolved << ")" << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

bool VFS::writeText(const std::string& relativePath, const std::string& content) {
    std::string resolved = resolve(relativePath);
    
    // Garante que a pasta pai exista
    fs::path p(resolved);
    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path());
    }

    std::ofstream out(resolved);
    if (!out.is_open()) {
        std::cerr << "[VFS] Falha ao escrever arquivo em: " << resolved << std::endl;
        return false;
    }

    out << content;
    std::cout << "[VFS] Arquivo gravado com sucesso: " << resolved << std::endl;
    return true;
}

} // namespace DayDoom::Engine
