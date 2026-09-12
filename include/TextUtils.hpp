#pragma once
#include <string>
#include <vector>

// Utilidades de normalizacion y tokenizacion de texto. Se centralizan aca
// porque tanto el indexado (SuffixTrie) como las busquedas del usuario
// deben normalizar EXACTAMENTE de la misma forma, o las palabras nunca
// calzarian entre si.
namespace TextUtils {

    // Pasa a minusculas y elimina acentos comunes del espanol/ingles
    // (para que "pelicula" y "Pelicula" o "cancion"/"canción" calcen).
    std::string normalize(const std::string& text);

    // Separa un texto normalizado en palabras. Un "separador" es cualquier
    // caracter que no sea letra ni digito (puntuacion, espacios, etc.).
    // Las palabras resultantes solo contienen [a-z0-9].
    std::vector<std::string> tokenize(const std::string& text);

    // Separa una lista tipo "Actor Uno, Actor Dos, Actor Tres" en tokens
    // individuales ya normalizados, preservando espacios internos de cada
    // nombre (a diferencia de tokenize(), que rompe todo por espacios).
    std::vector<std::string> splitAndNormalizeList(const std::string& csvList, char delim = ',');

    // Recorta espacios en blanco al inicio/fin.
    std::string trim(const std::string& s);
}
