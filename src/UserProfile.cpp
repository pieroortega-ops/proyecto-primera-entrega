#include "UserProfile.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

UserProfile::UserProfile(std::string path) : path_(std::move(path)) {
    load();
}

void UserProfile::load() {
    std::ifstream in(path_);
    if (!in.is_open()) return; // primera vez que se corre el programa

    std::string line;
    enum class Section { None, Likes, WatchLater } section = Section::None;
    while (std::getline(in, line)) {
        if (line == "LIKES") { section = Section::Likes; continue; }
        if (line == "WATCHLATER") { section = Section::WatchLater; continue; }
        if (line.empty()) continue;
        try {
            int id = std::stoi(line);
            if (section == Section::Likes && liked_.insert(id).second) likedOrder_.push_back(id);
            if (section == Section::WatchLater && watchLater_.insert(id).second) watchLaterOrder_.push_back(id);
        } catch (...) { /* linea corrupta: se ignora */ }
    }
}

void UserProfile::save() const {
    std::ofstream out(path_, std::ios::trunc);
    if (!out.is_open()) return; // no se pudo persistir; no es fatal para la sesion actual
    out << "LIKES\n";
    for (int id : likedOrder_) out << id << "\n";
    out << "WATCHLATER\n";
    for (int id : watchLaterOrder_) out << id << "\n";
}

void UserProfile::like(int movieId) {
    if (liked_.insert(movieId).second) likedOrder_.push_back(movieId);
    save();
}

void UserProfile::unlike(int movieId) {
    if (liked_.erase(movieId)) {
        likedOrder_.erase(std::remove(likedOrder_.begin(), likedOrder_.end(), movieId), likedOrder_.end());
        save();
    }
}

void UserProfile::addToWatchLater(int movieId) {
    if (watchLater_.insert(movieId).second) watchLaterOrder_.push_back(movieId);
    save();
}

void UserProfile::removeFromWatchLater(int movieId) {
    if (watchLater_.erase(movieId)) {
        watchLaterOrder_.erase(std::remove(watchLaterOrder_.begin(), watchLaterOrder_.end(), movieId), watchLaterOrder_.end());
        save();
    }
}

bool UserProfile::isLiked(int movieId) const { return liked_.count(movieId) > 0; }
bool UserProfile::isInWatchLater(int movieId) const { return watchLater_.count(movieId) > 0; }

double UserProfile::similarity(const Movie& a, const Movie& b) {
    // Conjunto de "atributos" de cada pelicula: generos + director + cada
    // actor/actriz del reparto, cada uno con un prefijo para no confundir,
    // por ejemplo, un genero llamado igual que un actor.
    std::unordered_set<std::string> setA, setB;
    for (auto& g : a.genreTokens) setA.insert("g:" + g);
    for (auto& c : a.castTokens)  setA.insert("c:" + c);
    if (!a.director.empty())      setA.insert("d:" + a.director);

    for (auto& g : b.genreTokens) setB.insert("g:" + g);
    for (auto& c : b.castTokens)  setB.insert("c:" + c);
    if (!b.director.empty())      setB.insert("d:" + b.director);

    if (setA.empty() || setB.empty()) return 0.0;

    size_t interseccion = 0;
    for (const auto& item : setA) if (setB.count(item)) ++interseccion;
    size_t unionSize = setA.size() + setB.size() - interseccion;
    if (unionSize == 0) return 0.0;
    return static_cast<double>(interseccion) / static_cast<double>(unionSize);
}

std::vector<int> UserProfile::recommendSimilarToLikes(const std::vector<Movie>& allMovies, size_t topN) const {
    if (likedOrder_.empty()) return {};

    // Para cada candidata (que no sea ya Like ni Ver-mas-tarde), la
    // similitud final es la MAXIMA similitud contra cualquiera de las
    // peliculas que el usuario marco con Like (asi, si le gusto una
    // comedia y un western, recomendamos lo mejor de ambos mundos, en vez
    // de diluir el puntaje promediando gustos opuestos).
    std::unordered_map<int, double> mejorScore;
    for (int likedId : likedOrder_) {
        if (likedId < 0 || likedId >= static_cast<int>(allMovies.size())) continue;
        const Movie& liked = allMovies[likedId];
        for (const Movie& candidate : allMovies) {
            if (candidate.id == likedId) continue;
            if (liked_.count(candidate.id) || watchLater_.count(candidate.id)) continue;
            double s = similarity(liked, candidate);
            if (s <= 0.0) continue;
            auto it = mejorScore.find(candidate.id);
            if (it == mejorScore.end() || s > it->second) mejorScore[candidate.id] = s;
        }
    }

    std::vector<std::pair<int,double>> ranked(mejorScore.begin(), mejorScore.end());
    std::sort(ranked.begin(), ranked.end(), [](auto& a, auto& b) { return a.second > b.second; });

    std::vector<int> result;
    for (size_t i = 0; i < ranked.size() && i < topN; ++i) result.push_back(ranked[i].first);
    return result;
}
