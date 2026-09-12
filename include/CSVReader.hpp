#pragma once
#include <string>
#include <vector>
#include "Movie.hpp"

// Lector de CSV "a mano" (sin librerias externas, tal como exige el
// proyecto: "todo el programa ... debe estar en C++"). Soporta el formato
// RFC4180 basico que usa este dataset: campos entre comillas dobles que
// pueden contener comas y SALTOS DE LINEA reales dentro del campo, y
// comillas dobles escapadas como "".
namespace CSVReader {

    // Lee el archivo completo y devuelve un vector<Movie>. maxRows limita
    // la cantidad de peliculas a cargar (0 = sin limite, todo el archivo).
    // Filas mal formadas o con Title vacio se descartan (preprocesamiento
    // basico de calidad de datos, responsabilidad del grupo segun el
    // enunciado del proyecto).
    std::vector<Movie> load(const std::string& path, size_t maxRows = 0);

}
