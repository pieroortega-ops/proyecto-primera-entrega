#include "TextUtils.hpp"
#include <cctype>
#include <unordered_map>

namespace TextUtils {

std::string trim(const std::string& s) {
    size_t start = 0, end = s.size();
    while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

// Reemplaza las secuencias UTF-8 de vocales acentuadas / ñ mas comunes por
// su version simple. No es una solucion Unicode completa, pero cubre el
// caso practico de titulos/sinopsis en espanol e ingles de este dataset.
static std::string stripAccentsUtf8(const std::string& text) {
    static const std::unordered_map<std::string, char> accentMap = {
        {"\xC3\xA1", 'a'}, {"\xC3\xA9", 'e'}, {"\xC3\xAD", 'i'},
        {"\xC3\xB3", 'o'}, {"\xC3\xBA", 'u'}, {"\xC3\xB1", 'n'},
        {"\xC3\x81", 'a'}, {"\xC3\x89", 'e'}, {"\xC3\x8D", 'i'},
        {"\xC3\x93", 'o'}, {"\xC3\x9A", 'u'}, {"\xC3\x91", 'n'},
        {"\xC3\xBC", 'u'}, {"\xC3\x9C", 'u'},
    };
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size();) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (c >= 0xC0 && i + 1 < text.size()) {
            std::string two = text.substr(i, 2);
            auto it = accentMap.find(two);
            if (it != accentMap.end()) {
                out.push_back(it->second);
                i += 2;
                continue;
            }
        }
        out.push_back(text[i]);
        ++i;
    }
    return out;
}

std::string normalize(const std::string& text) {
    std::string noAccents = stripAccentsUtf8(text);
    std::string out;
    out.reserve(noAccents.size());
    for (char c : noAccents) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

std::vector<std::string> tokenize(const std::string& text) {
    std::string normalized = normalize(text);
    std::vector<std::string> tokens;
    std::string current;
    for (char c : normalized) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            current.push_back(c);
        } else if (!current.empty()) {
            tokens.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

std::vector<std::string> splitAndNormalizeList(const std::string& csvList, char delim) {
    std::vector<std::string> result;
    std::string current;
    for (char c : csvList) {
        if (c == delim) {
            std::string t = trim(normalize(current));
            if (!t.empty()) result.push_back(t);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    std::string t = trim(normalize(current));
    if (!t.empty()) result.push_back(t);
    return result;
}

} // namespace TextUtils
