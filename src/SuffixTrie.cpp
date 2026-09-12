#include "SuffixTrie.hpp"

SuffixTrie::SuffixTrie() {
    nodes_.emplace_back(); // nodo raiz (indice 0)
}

void SuffixTrie::insertSuffix(const std::string& suffix, int wordId) {
    int current = 0; // raiz
    for (char c : suffix) {
        auto& node = nodes_[current];
        auto it = node.children.find(c);
        int next;
        if (it == node.children.end()) {
            next = static_cast<int>(nodes_.size());
            nodes_.emplace_back();
            nodes_[current].children[c] = next; // OJO: nodes_ pudo re-alocar, por eso
                                                 // indexamos de nuevo via nodes_[current]
                                                 // en vez de reusar la referencia 'node'.
        } else {
            next = it->second;
        }
        current = next;

        // Deduplicacion O(1) amortizada: como todos los sufijos de UNA
        // misma palabra se insertan de forma consecutiva (ver
        // insertAllSuffixes), si el ultimo wordId agregado a este nodo es
        // el mismo que el actual, no lo volvemos a agregar. Esto evita que
        // una palabra con letras repetidas (ej. "banana") duplique su
        // propio wordId muchas veces en los nodos compartidos.
        Node& n = nodes_[current];
        if (n.lastWordId != wordId) {
            n.wordIds.push_back(wordId);
            n.lastWordId = wordId;
        }
    }
}

void SuffixTrie::insertAllSuffixes(const std::string& word, int wordId) {
    for (size_t start = 0; start < word.size(); ++start) {
        insertSuffix(word.substr(start), wordId);
    }
}

std::vector<int> SuffixTrie::findWordsContaining(const std::string& query) const {
    if (query.empty()) return {};
    int current = 0;
    for (char c : query) {
        auto it = nodes_[current].children.find(c);
        if (it == nodes_[current].children.end()) return {}; // no existe esa sub-cadena
        current = it->second;
    }
    return nodes_[current].wordIds;
}
