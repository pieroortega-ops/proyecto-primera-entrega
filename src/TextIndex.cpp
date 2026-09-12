#include "TextIndex.hpp"

int TextIndex::getOrCreateWordId(const std::string& word) {
    auto it = wordToId_.find(word);
    if (it != wordToId_.end()) return it->second;
    int id = static_cast<int>(idToWord_.size());
    idToWord_.push_back(word);
    wordToId_[word] = id;
    postings_.emplace_back();
    return id;
}

void TextIndex::indexTokens(int movieId, const std::vector<std::string>& tokens, int weight) {
    for (const auto& token : tokens) {
        if (token.empty()) continue;
        int id = getOrCreateWordId(token);
        postings_[id][movieId] += weight;
    }
}

void TextIndex::build() {
    // Se inserta cada palabra del vocabulario UNA sola vez (no por cada
    // ocurrencia en cada pelicula), que es la optimizacion documentada en
    // SuffixTrie.hpp.
    for (int id = 0; id < static_cast<int>(idToWord_.size()); ++id) {
        trie_.insertAllSuffixes(idToWord_[id], id);
    }
}

TextIndex::ScoreMap TextIndex::searchSubstring(const std::string& token) const {
    ScoreMap result;
    if (token.empty()) return result;
    std::vector<int> wordIds = trie_.findWordsContaining(token);
    for (int wid : wordIds) {
        for (const auto& [movieId, weight] : postings_[wid]) {
            result[movieId] += weight;
        }
    }
    return result;
}
