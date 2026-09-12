#include "CSVReader.hpp"
#include "TextUtils.hpp"
#include <fstream>
#include <iostream>

namespace CSVReader {

// Lee un unico registro logico (fila) del CSV a partir de la posicion
// actual del stream, respetando comillas que pueden contener comas y
// saltos de linea. Devuelve false si no se pudo leer mas (EOF).
static bool readRecord(std::istream& in, std::vector<std::string>& fields) {
    fields.clear();
    std::string field;
    bool insideQuotes = false;
    bool any = false;
    int c;

    while ((c = in.get()) != EOF) {
        any = true;
        char ch = static_cast<char>(c);

        if (insideQuotes) {
            if (ch == '"') {
                if (in.peek() == '"') { // comilla escapada ""
                    field.push_back('"');
                    in.get();
                } else {
                    insideQuotes = false;
                }
            } else {
                field.push_back(ch);
            }
            continue;
        }

        if (ch == '"') {
            insideQuotes = true;
        } else if (ch == ',') {
            fields.push_back(field);
            field.clear();
        } else if (ch == '\n') {
            fields.push_back(field);
            return true;
        } else if (ch == '\r') {
            // ignorar; el \n que sigue termina el registro
        } else {
            field.push_back(ch);
        }
    }

    if (any) {
        fields.push_back(field);
        return true;
    }
    return false; // EOF real, sin datos pendientes
}

std::vector<Movie> load(const std::string& path, size_t maxRows) {
    std::ifstream in(path, std::ios::binary);
    std::vector<Movie> movies;
    if (!in.is_open()) {
        std::cerr << "[CSVReader] No se pudo abrir: " << path << std::endl;
        return movies;
    }

    std::vector<std::string> header;
    if (!readRecord(in, header)) return movies;

    // Mapear nombres de columna -> indice, para no depender del orden
    // exacto en que vengan las columnas en el archivo.
    auto indexOf = [&](const std::string& name) -> int {
        for (size_t i = 0; i < header.size(); ++i) {
            if (TextUtils::trim(header[i]) == name) return static_cast<int>(i);
        }
        return -1;
    };
    int idxYear   = indexOf("Release Year");
    int idxTitle  = indexOf("Title");
    int idxOrigin = indexOf("Origin/Ethnicity");
    int idxDir    = indexOf("Director");
    int idxCast   = indexOf("Cast");
    int idxGenre  = indexOf("Genre");
    int idxWiki   = indexOf("Wiki Page");
    int idxPlot   = indexOf("Plot");

    auto getField = [](const std::vector<std::string>& row, int idx) -> std::string {
        if (idx < 0 || idx >= static_cast<int>(row.size())) return "";
        return row[idx];
    };

    std::vector<std::string> row;
    int nextId = 0;
    size_t discarded = 0;
    while (readRecord(in, row)) {
        if (maxRows != 0 && movies.size() >= maxRows) break;

        std::string title = TextUtils::trim(getField(row, idxTitle));
        if (title.empty()) { ++discarded; continue; } // preprocesamiento: descarta filas invalidas

        Movie m;
        m.id = nextId++;
        try {
            m.releaseYear = std::stoi(TextUtils::trim(getField(row, idxYear)));
        } catch (...) {
            m.releaseYear = 0; // dato faltante/corrupto -> 0 (desconocido)
        }
        m.title    = title;
        m.origin   = TextUtils::trim(getField(row, idxOrigin));
        m.director = TextUtils::trim(getField(row, idxDir));
        m.cast     = TextUtils::trim(getField(row, idxCast));
        m.genre    = TextUtils::trim(getField(row, idxGenre));
        m.wikiPage = TextUtils::trim(getField(row, idxWiki));
        m.plot     = TextUtils::trim(getField(row, idxPlot));

        // Normalizar "unknown"/"" para genero y director de forma consistente
        std::string genreNorm = TextUtils::normalize(m.genre);
        if (genreNorm == "unknown" || genreNorm.empty()) m.genre = "";
        std::string dirNorm = TextUtils::normalize(m.director);
        if (dirNorm == "unknown" || dirNorm.empty()) m.director = "";

        m.castTokens  = TextUtils::splitAndNormalizeList(m.cast, ',');
        m.genreTokens = TextUtils::splitAndNormalizeList(m.genre, ',');

        movies.push_back(std::move(m));
    }

    std::cerr << "[CSVReader] Cargadas " << movies.size() << " peliculas ("
              << discarded << " filas descartadas por falta de titulo)." << std::endl;
    return movies;
}

} // namespace CSVReader
