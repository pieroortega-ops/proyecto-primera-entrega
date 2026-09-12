#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

// ============================================================================
// SuffixTrie
// ----------------------------------------------------------------------------
// Arbol de caracteres (cada nodo = una letra o digito) que permite responder,
// en tiempo proporcional al largo de la consulta, "que palabras del
// vocabulario contienen esta sub-cadena". Es la estructura que exige el
// enunciado: "Los caracteres deben ser los valores que se almacenen en los
// nodos del Arbol", usando como referencia un Suffix Tree.
//
// DECISION DE DISENO IMPORTANTE (ver docs/ARQUITECTURA.md para el detalle):
// en vez de insertar los sufijos de cada OCURRENCIA de una palabra en cada
// pelicula (lo cual, con ~34,000 peliculas y cientos de palabras por
// sinopsis, generaria decenas de millones de nodos e inviabilizaria la
// memoria), este Trie indexa los sufijos del VOCABULARIO (palabras
// DISTINTAS que aparecen en todo el dataset, sin repetir). Cada nodo
// almacena el conjunto de identificadores de palabra (wordId) cuyo sufijo
// pasa por ese nodo. Luego, un indice invertido separado (ver
// TextIndex.hpp) traduce cada wordId a las peliculas donde esa palabra
// aparece. Esta separacion es exactamente el patron "indice de vocabulario
// + postings list" usado en motores de busqueda reales.
// ============================================================================
class SuffixTrie {
public:
    SuffixTrie();

    // Inserta TODOS los sufijos de 'word' (ya normalizada: minusculas,
    // solo [a-z0-9]) asociandolos al identificador 'wordId'. Debe llamarse
    // una unica vez por palabra distinta del vocabulario.
    void insertAllSuffixes(const std::string& word, int wordId);

    // Devuelve los wordId de toda palabra del vocabulario que contenga
    // 'query' como sub-cadena en cualquier posicion (si query es un
    // prefijo de la palabra, tambien la encuentra: un prefijo es un caso
    // particular de sub-cadena, ya que es sufijo de si mismo desde la
    // posicion 0).
    std::vector<int> findWordsContaining(const std::string& query) const;

    size_t nodeCount() const { return nodes_.size(); }

private:
    struct Node {
        std::unordered_map<char, int32_t> children; // hijo por letra/digito -> indice en nodes_
        std::vector<int> wordIds;   // wordIds cuyo sufijo pasa por este nodo
        int lastWordId = -1;        // truco de deduplicacion (ver .cpp)
    };

    std::vector<Node> nodes_; // nodes_[0] = raiz (arena/pool de nodos)

    void insertSuffix(const std::string& suffix, int wordId);
};
