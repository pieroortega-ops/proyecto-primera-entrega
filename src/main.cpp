#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include "CSVReader.hpp"
#include "SearchEngine.hpp"
#include "UserProfile.hpp"

namespace {

void printMovieShort(const std::vector<Movie>& movies, const SearchResult& r, int posicion) {
    const Movie& m = movies[r.movieId];
    std::cout << "  " << posicion << ". " << m.title << " (" << m.releaseYear << ")";
    if (!m.director.empty()) std::cout << " - dir. " << m.director;
    std::cout << "\n";
}

void printMovieDetail(const Movie& m, const UserProfile& user) {
    std::cout << "\n=================================================\n";
    std::cout << m.title << " (" << m.releaseYear << ")\n";
    if (!m.director.empty()) std::cout << "Director: " << m.director << "\n";
    if (!m.cast.empty())     std::cout << "Reparto: " << m.cast << "\n";
    if (!m.genre.empty())    std::cout << "Genero: " << m.genre << "\n";
    std::cout << "-------------------------------------------------\n";
    std::cout << "Sinopsis:\n" << (m.plot.empty() ? "(no disponible)" : m.plot) << "\n";
    std::cout << "-------------------------------------------------\n";
    std::cout << "[Like: " << (user.isLiked(m.id) ? "SI" : "no") << "]  "
              << "[Ver mas tarde: " << (user.isInWatchLater(m.id) ? "SI" : "no") << "]\n";
    std::cout << "=================================================\n";
}

// Muestra una lista de resultados paginada de a 5, y permite: numero para
// ver el detalle de esa pelicula, 'm' para ver las siguientes 5, o vacio
// para volver al menu principal.
void mostrarResultadosPaginados(const std::vector<Movie>& movies,
                                 const std::vector<SearchResult>& results,
                                 UserProfile& user) {
    if (results.empty()) {
        std::cout << "No se encontraron resultados.\n";
        return;
    }
    size_t pagina = 0;
    const size_t PAGE_SIZE = 5;

    while (true) {
        size_t start = pagina * PAGE_SIZE;
        if (start >= results.size()) {
            std::cout << "No hay mas resultados.\n";
            pagina = 0;
            start = 0;
        }
        size_t end = std::min(start + PAGE_SIZE, results.size());

        std::cout << "\nResultados " << (start + 1) << "-" << end << " de " << results.size() << ":\n";
        for (size_t i = start; i < end; ++i) {
            printMovieShort(movies, results[i], static_cast<int>(i - start + 1));
        }
        std::cout << "[1-" << (end - start) << "] ver detalle  |  [m] mas resultados  |  [Enter] volver al menu\n> ";

        std::string input;
        std::getline(std::cin, input);
        if (input.empty()) return;
        if (input == "m" || input == "M") { ++pagina; continue; }

        try {
            int opcion = std::stoi(input);
            if (opcion >= 1 && static_cast<size_t>(opcion) <= (end - start)) {
                const Movie& m = movies[results[start + opcion - 1].movieId];
                printMovieDetail(m, user);
                std::cout << "[l] Like  [v] Ver mas tarde  [Enter] volver\n> ";
                std::string accion;
                std::getline(std::cin, accion);
                if (accion == "l" || accion == "L") { user.like(m.id); std::cout << "Agregado a Likes.\n"; }
                else if (accion == "v" || accion == "V") { user.addToWatchLater(m.id); std::cout << "Agregado a Ver mas tarde.\n"; }
            }
        } catch (...) { /* entrada invalida: se ignora y se re-muestra el menu */ }
    }
}

void mostrarInicio(const std::vector<Movie>& movies, UserProfile& user, const SearchEngine& engine) {
    if (!user.watchLaterMovies().empty()) {
        std::cout << "\n== Ver mas tarde ==\n";
        for (int id : user.watchLaterMovies()) {
            if (id >= 0 && id < static_cast<int>(movies.size()))
                std::cout << "  - " << movies[id].title << " (" << movies[id].releaseYear << ")\n";
        }
    }
    if (!user.likedMovies().empty()) {
        auto recomendadas = user.recommendSimilarToLikes(movies, 5);
        if (!recomendadas.empty()) {
            std::cout << "\n== Porque te gustaron tus peliculas con Like ==\n";
            for (int id : recomendadas) {
                std::cout << "  - " << movies[id].title << " (" << movies[id].releaseYear << ")\n";
            }
        }
    }
    (void)engine;
}

} // namespace

int main(int argc, char** argv) {
    std::string csvPath = "data/wiki_movie_plots_deduped.csv";
    size_t maxRows = 0;
    if (argc > 1) csvPath = argv[1];
    if (argc > 2) maxRows = static_cast<size_t>(std::stoul(argv[2]));

    std::cout << "Cargando base de datos de peliculas...\n";
    std::vector<Movie> movies = CSVReader::load(csvPath, maxRows);
    if (movies.empty()) {
        std::cerr << "No se pudieron cargar peliculas desde: " << csvPath << "\n";
        return 1;
    }

    SearchEngine engine(movies);
    engine.buildIndexes();
    engine.printIndexStats(std::cout);

    UserProfile user("user_profile.txt");
    mostrarInicio(movies, user, engine);

    while (true) {
        std::cout << "\n===================== MENU =====================\n";
        std::cout << "1. Buscar por palabra / frase / sub-cadena\n";
        std::cout << "2. Buscar por director\n";
        std::cout << "3. Buscar por actor/actriz (cast)\n";
        std::cout << "4. Buscar por genero\n";
        std::cout << "5. Ver mis Likes y recomendaciones\n";
        std::cout << "6. Salir\n";
        std::cout << "> ";

        std::string opcion;
        if (!std::getline(std::cin, opcion)) break;

        if (opcion == "1") {
            std::cout << "Buscar: ";
            std::string q; std::getline(std::cin, q);
            mostrarResultadosPaginados(movies, engine.searchText(q), user);
        } else if (opcion == "2") {
            std::cout << "Director: ";
            std::string q; std::getline(std::cin, q);
            mostrarResultadosPaginados(movies, engine.searchTag(q, TagField::Director), user);
        } else if (opcion == "3") {
            std::cout << "Actor/actriz: ";
            std::string q; std::getline(std::cin, q);
            mostrarResultadosPaginados(movies, engine.searchTag(q, TagField::Cast), user);
        } else if (opcion == "4") {
            std::cout << "Genero: ";
            std::string q; std::getline(std::cin, q);
            mostrarResultadosPaginados(movies, engine.searchTag(q, TagField::Genre), user);
        } else if (opcion == "5") {
            mostrarInicio(movies, user, engine);
        } else if (opcion == "6") {
            break;
        } else {
            std::cout << "Opcion invalida.\n";
        }
    }

    std::cout << "Hasta luego!\n";
    return 0;
}
