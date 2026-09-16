#pragma once
#include <string>
#include <vector>

struct Movie {
    int id = -1;
    int releaseYear = 0;
    std::string title;
    std::string origin;
    std::string director;
    std::string cast;
    std::string genre;
    std::string wikiPage;
    std::string plot;

    // Listas ya separadas (tokenizadas) para acelerar el indexado y el
    // calculo de similitud, evitando re-tokenizar en cada busqueda.
    std::vector<std::string> castTokens;   // nombres de actores individuales
    std::vector<std::string> genreTokens;  // generos individuales (puede venir "comedy, drama")
};
