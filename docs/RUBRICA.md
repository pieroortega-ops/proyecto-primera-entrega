# Mapeo del enunciado a la implementación

| Requisito del enunciado | Dónde está implementado | Notas |
|---|---|---|
| Leer la base de datos en formato `.csv` | `CSVReader::load` (`src/CSVReader.cpp`) | Parser propio (sin librerías externas), soporta campos entre comillas con comas y saltos de línea internos, tal como aparecen en el dataset real. |
| Pre-procesamiento de los datos | `CSVReader::load` + `TextUtils` | Descarta filas sin título, normaliza `unknown`/vacíos, normaliza texto (minúsculas, sin tildes), tokeniza título/sinopsis/cast/género. Ver `docs/ARQUITECTURA.md` §5. |
| Cargar el contenido en un Árbol (Trie/Suffix Tree) para búsqueda rápida | `SuffixTrie` (`include/SuffixTrie.hpp`, `src/SuffixTrie.cpp`) | Trie de sufijos a nivel de caracter sobre el vocabulario del dataset. Justificación completa de la elección en `docs/ARQUITECTURA.md` §1. |
| Buscar por palabra, frase o sub-cadena | `TextIndex::searchSubstring` + `SearchEngine::searchText` | Una sola implementación cubre los tres casos: una sub-cadena es la búsqueda base; una palabra completa y un prefijo son casos particulares; una frase combina varias sub-cadenas. Probado con `"bar"` → encuentra `"barco"`, `"embarcadero"`, etc. |
| Buscar por Tag (director, casting, género) | `SearchEngine::searchTag` + `TagField` | Reutiliza el mismo mecanismo de índice/Trie que la búsqueda de texto, con tres índices independientes (`directorIndex_`, `castIndex_`, `genreIndex_`). |
| Mostrar las 5 más importantes + opción de ver 5 más | `mostrarResultadosPaginados` (`src/main.cpp`) | Paginación de a 5 sobre resultados ya ordenados por importancia. |
| Algoritmo propio de importancia | `SearchEngine::rankAndCombine` | Pesos por campo (título > tag > sinopsis) + bono por cobertura de la frase + desempate por año. Detalle en `docs/ARQUITECTURA.md` §2. |
| Ver sinopsis + opciones Like / Ver más tarde | `printMovieDetail` (`src/main.cpp`) | Se muestra al seleccionar una película desde cualquier lista de resultados. |
| Mostrar "Ver más tarde" al iniciar | `mostrarInicio` (`src/main.cpp`), llamada en `main()` antes del menú | — |
| Mostrar películas similares a las que dieron Like | `UserProfile::recommendSimilarToLikes` | Similitud de Jaccard sobre género + director + reparto. Algoritmo propio, documentado en `docs/ARQUITECTURA.md` §3. |
| Subir a GitHub con documentación | (pendiente del equipo) | Este entregable ya trae `README.md` + `docs/` listos; falta que el equipo cree el repositorio, agregue a los integrantes reales y suba el código. |
| Programa completo en C++ | Todo `include/`, `src/` | Sin dependencias externas más allá de la librería estándar de C++17. |

## Lo que falta o se puede mejorar para la entrega final (semana 16)

- **Pruebas unitarias formales** (actualmente la validación fue manual/exploratoria contra el dataset real; ver sección de pruebas abajo).
- **Interfaz** más amigable que la consola (opcional, no lo exige el enunciado).
- **Recomendación basada también en texto de la sinopsis** (actualmente solo usa género/director/reparto — ver limitación documentada en `docs/ARQUITECTURA.md` §3).
- **Manejo de errores de tipeo** en la búsqueda (ej. distancia de edición) — actualmente la búsqueda es exacta a nivel de sub-cadena, sin tolerancia a errores.

## Cómo se validó esta entrega

Se probó contra el dataset real completo (`wiki_movie_plots_deduped.csv`, 34,886 películas tras el pre-procesamiento):
- Carga + construcción de índices: ~15-20 segundos, ~1.3 GB de RAM.
- Búsqueda por palabra (`"train"`), por frase (`"robbery train"`) y por sub-cadena en medio de palabra (`"obber"` → encuentra "Robbery") devuelven resultados correctos y ordenados por importancia.
- Búsqueda por tag (`director: "porter"`, `genero: "western"`) devuelve resultados correctos.
- Flujo completo probado de punta a punta: buscar → ver detalle → dar Like → salir → volver a abrir el programa → la película recomendada aparece automáticamente basada en el Like anterior (persistencia confirmada en `user_profile.txt`).
