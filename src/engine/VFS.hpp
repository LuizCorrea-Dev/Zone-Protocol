#pragma once

#include <string>
#include <vector>

namespace DayDoom::Engine {

class VFS {
public:
    static VFS& get();

    // Inicializa com o diretório base da aplicação
    void init(const std::string& baseDir);

    // Registra caminhos de busca para os assets do projeto
    void addSearchPath(const std::string& path, bool highPriority = true);
    void clearSearchPaths();

    // Resolve o caminho físico absoluto de um arquivo relativo (ex: "assets/sprites/sprites_entities.png")
    std::string resolve(const std::string& relativePath) const;

    // Checa se o arquivo existe em algum dos caminhos de busca
    bool exists(const std::string& relativePath) const;

    // Leitura e escrita de arquivos de texto através do VFS
    std::string readText(const std::string& relativePath) const;
    bool writeText(const std::string& relativePath, const std::string& content);

    const std::string& getBaseDir() const { return m_baseDir; }
    const std::vector<std::string>& getSearchPaths() const { return m_searchPaths; }

private:
    VFS() = default;
    ~VFS() = default;
    VFS(const VFS&) = delete;
    VFS& operator=(const VFS&) = delete;

    std::string m_baseDir;
    std::vector<std::string> m_searchPaths;
};

} // namespace DayDoom::Engine
