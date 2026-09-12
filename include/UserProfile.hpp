#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include "Movie.hpp"

// ============================================================================
// UserProfile
// ----------------------------------------------------------------------------
// Guarda los "Like" y "Ver mas tarde" del usuario, los persiste en un archivo
// de texto simple entre ejecuciones, y calcula recomendaciones de peliculas
// similares a las que le dieron Like (algoritmo de similitud por atributos:
// genero + director + reparto, con la sinopsis compartiendo palabras clave
// relevantes). Ver docs/ARQUITECTURA.md seccion "Recomendacion" para el
// detalle del algoritmo y sus limitaciones.
// ============================================================================
class UserProfile {
public:
    // Carga el perfil desde 'path' si existe; si no, arranca vacio.
    explicit UserProfile(std::string path);

    void like(int movieId);
    void unlike(int movieId);
    void addToWatchLater(int movieId);
    void removeFromWatchLater(int movieId);

    bool isLiked(int movieId) const;
    bool isInWatchLater(int movieId) const;

    const std::vector<int>& likedMovies() const { return likedOrder_; }
    const std::vector<int>& watchLaterMovies() const { return watchLaterOrder_; }

    // Guarda el estado actual en el archivo. Se llama automaticamente tras
    // cada cambio (like/unlike/agregar/quitar), para no perder datos si el
    // programa se cierra inesperadamente.
    void save() const;

    // Recomienda hasta 'topN' peliculas similares a TODO lo que el usuario
    // ha marcado con Like, excluyendo las que ya tiene en Like o Ver mas
    // tarde. 'allMovies' es la base de datos completa.
    std::vector<int> recommendSimilarToLikes(const std::vector<Movie>& allMovies, size_t topN = 5) const;

private:
    std::string path_;
    std::unordered_set<int> liked_;
    std::unordered_set<int> watchLater_;
    std::vector<int> likedOrder_;      // preserva el orden en que se dio Like
    std::vector<int> watchLaterOrder_;

    void load();

    // Similitud de Jaccard entre los "atributos" (genero, director, reparto)
    // de dos peliculas: |interseccion| / |union|. Un valor entre 0 y 1.
    static double similarity(const Movie& a, const Movie& b);
};
