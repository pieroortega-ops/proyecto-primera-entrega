#include "SearchEngine.hpp"
#include "TextUtils.hpp"
#include <algorithm>
#include <iostream>

namespace {
    // Pesos de importancia por campo (decision documentada en docs/ARQUITECTURA.md):
    // una coincidencia en el TITULO es mucho mas relevante que una en la
    // sinopsis (una pelicula titulada "El Barco Fantasma" es casi con
    // certeza lo que alguien busca al escribir "barco fantasma", aunque la
    // palabra tambien aparezca de pasada en la sinopsis de otras 50 peliculas).
    constexpr int PESO_TITULO = 5;
    constexpr int PESO_PLOT   = 1;
    constexpr int PESO_TAG    = 4;

    // Bono por cada palabra ADICIONAL de la consulta que matchea en la misma
    // pelicula (para que una coincidencia de frase completa "barco fantasma"
    // supere a una pelicula que solo menciona "barco" muchisimas veces).
    constexpr int BONO_POR_TERMINO_EXTRA = 50;
}

SearchEngine::SearchEngine(const std::vector<Movie>& movies) : movies_(movies) {}

void SearchEngine::buildIndexes() {
    for (const auto& m : movies_) {
        contentIndex_.indexTokens(m.id, TextUtils::tokenize(m.title), PESO_TITULO);
        contentIndex_.indexTokens(m.id, TextUtils::tokenize(m.plot),  PESO_PLOT);

        if (!m.director.empty())
            directorIndex_.indexTokens(m.id, TextUtils::tokenize(m.director), PESO_TAG);
        if (!m.castTokens.empty())
            castIndex_.indexTokens(m.id, m.castTokens, PESO_TAG);
        if (!m.genreTokens.empty())
            genreIndex_.indexTokens(m.id, m.genreTokens, PESO_TAG);
    }
    contentIndex_.build();
    directorIndex_.build();
    castIndex_.build();
    genreIndex_.build();
}

std::vector<SearchResult> SearchEngine::rankAndCombine(const std::vector<std::string>& queryTokens,
                                                        const TextIndex& index) const {
    // movieId -> (score acumulado, cuantos terminos distintos matchearon)
    std::unordered_map<int, std::pair<int,int>> acc;

    for (const auto& term : queryTokens) {
        auto hits = index.searchSubstring(term);
        for (const auto& [movieId, weight] : hits) {
            auto& entry = acc[movieId];
            entry.first  += weight;
            entry.second += 1;
        }
    }

    std::vector<SearchResult> results;
    results.reserve(acc.size());
    for (const auto& [movieId, scoreAndCount] : acc) {
        int importancia = scoreAndCount.first + BONO_POR_TERMINO_EXTRA * (scoreAndCount.second - 1);
        results.push_back({movieId, importancia, scoreAndCount.second});
    }

    // Orden de importancia: primero por cuantos terminos de la consulta
    // matcheo (cobertura de la frase), luego por el score acumulado, y
    // como ultimo criterio de desempate, la pelicula mas reciente primero.
    std::sort(results.begin(), results.end(), [this](const SearchResult& a, const SearchResult& b) {
        if (a.matchedTerms != b.matchedTerms) return a.matchedTerms > b.matchedTerms;
        if (a.score != b.score) return a.score > b.score;
        return movies_[a.movieId].releaseYear > movies_[b.movieId].releaseYear;
    });
    return results;
}

std::vector<SearchResult> SearchEngine::searchText(const std::string& query) const {
    return rankAndCombine(TextUtils::tokenize(query), contentIndex_);
}

std::vector<SearchResult> SearchEngine::searchTag(const std::string& query, TagField field) const {
    const TextIndex* idx = nullptr;
    switch (field) {
        case TagField::Director: idx = &directorIndex_; break;
        case TagField::Cast:     idx = &castIndex_;      break;
        case TagField::Genre:    idx = &genreIndex_;     break;
    }
    return rankAndCombine(TextUtils::tokenize(query), *idx);
}

void SearchEngine::printIndexStats(std::ostream& os) const {
    os << "Peliculas indexadas: " << movies_.size() << "\n";
    os << "Vocabulario titulo+sinopsis: " << contentIndex_.vocabularySize()
       << " palabras, " << contentIndex_.trieNodeCount() << " nodos en el Trie de sufijos\n";
    os << "Vocabulario director: " << directorIndex_.vocabularySize() << " palabras\n";
    os << "Vocabulario cast: " << castIndex_.vocabularySize() << " palabras\n";
    os << "Vocabulario genero: " << genreIndex_.vocabularySize() << " palabras\n";
}
