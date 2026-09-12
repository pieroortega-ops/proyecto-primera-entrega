#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "SuffixTrie.hpp"

// ============================================================================
// TextIndex
// ----------------------------------------------------------------------------
// Une el vocabulario del dataset (palabras distintas) con el SuffixTrie y un
// indice invertido (wordId -> {movieId: peso_acumulado}). Se usa tanto para
// el indice de "titulo + sinopsis" como, con otra instancia, para cada tag
// (director, cast, genero).
// ============================================================================
class TextIndex {
public:
    // Registra 'tokens' (ya normalizados) como pertenecientes a 'movieId'.
    // 'weight' es cuanto vale cada ocurrencia para el ranking de importancia
    // (por ejemplo: coincidencias en el titulo valen mas que en la sinopsis).
    void indexTokens(int movieId, const std::vector<std::string>& tokens, int weight);

    // Debe llamarse UNA vez, despues de indexar todas las peliculas, para
    // construir el SuffixTrie sobre el vocabulario final.
    void build();

    // Resultado de buscar una palabra/sub-cadena: para cada pelicula que
    // matcheo, cuanto "peso" acumulo.
    using ScoreMap = std::unordered_map<int, int>; // movieId -> peso

    // Busca 'token' como SUB-CADENA de cualquier palabra del vocabulario
    // (un match exacto o un prefijo son casos particulares de esto).
    ScoreMap searchSubstring(const std::string& token) const;

    size_t vocabularySize() const { return idToWord_.size(); }
    size_t trieNodeCount() const { return trie_.nodeCount(); }

private:
    std::unordered_map<std::string, int> wordToId_;
    std::vector<std::string> idToWord_;
    // Por cada wordId: movieId -> peso acumulado de ese termino en esa pelicula.
    std::vector<std::unordered_map<int, int>> postings_;
    SuffixTrie trie_;

    int getOrCreateWordId(const std::string& word);
};
