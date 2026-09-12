#pragma once
#include <string>
#include <vector>
#include "Movie.hpp"
#include "TextIndex.hpp"

// Resultado de una busqueda, ya ordenado por "importancia".
struct SearchResult {
    int movieId;
    int score;          // puntaje de importancia (ver calcularImportancia en el .cpp)
    int matchedTerms;   // cuantas palabras distintas de la consulta matchearon
};

enum class TagField { Director, Cast, Genre };

// ============================================================================
// SearchEngine
// ----------------------------------------------------------------------------
// Construye los indices (titulo+sinopsis, director, cast, genero) sobre toda
// la base de datos, y expone las operaciones de busqueda pedidas en el
// enunciado: por palabra/frase/sub-cadena, y por tag.
// ============================================================================
class SearchEngine {
public:
    explicit SearchEngine(const std::vector<Movie>& movies);

    // Construye todos los indices. Debe llamarse una vez tras cargar el CSV.
    void buildIndexes();

    // Busqueda por palabra, frase o sub-cadena en TITULO + SINOPSIS.
    // Internamente cada palabra de 'query' se busca como sub-cadena (lo que
    // automaticamente cubre el caso de palabra completa y de prefijo), y los
    // resultados de todas las palabras se combinan sumando su importancia,
    // premiando a las peliculas que matchean MAS palabras de la consulta.
    std::vector<SearchResult> searchText(const std::string& query) const;

    // Busqueda por tag: director, actor/actriz (cast) o genero. Tambien
    // admite sub-cadena (ej. "spiel" encuentra "Steven Spielberg").
    std::vector<SearchResult> searchTag(const std::string& query, TagField field) const;

    // Estadisticas simples, utiles para el reporte / demo.
    void printIndexStats(std::ostream& os) const;

private:
    const std::vector<Movie>& movies_;
    TextIndex contentIndex_;   // titulo (peso alto) + sinopsis (peso normal)
    TextIndex directorIndex_;
    TextIndex castIndex_;
    TextIndex genreIndex_;

    std::vector<SearchResult> rankAndCombine(const std::vector<std::string>& queryTokens,
                                              const TextIndex& index) const;
};
