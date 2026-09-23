# Programación III: Proyecto Final (2026-2)

## Integrantes (3 ó 4)
* Dylan van Oordt Arbulú
* Nombre y Apellidos
* Nombre y Apellidos

El siguiente texto debe ser eliminado en su repositorio.

---

## Plataforma de Streaming — Buscador y visualizador de sinopsis de películas

Programa en C++17 que carga una base de datos de películas desde un `.csv`, la indexa en un **Trie de sufijos** para búsqueda rápida por palabra, frase o sub-cadena (y por tag: director/actor/género), calcula una **importancia** para ordenar los resultados, y permite marcar películas como **Like** o **Ver más tarde**, con **recomendaciones** basadas en los Likes del usuario. Todo el programa (lectura del CSV incluida) está escrito en C++, sin librerías externas.

Este `README.md` documenta la entrega. El detalle de diseño y justificación de la estructura de árbol está en [`docs/ARQUITECTURA.md`](docs/ARQUITECTURA.md), y el mapeo punto por punto contra el enunciado del proyecto está en [`docs/RUBRICA.md`](docs/RUBRICA.md).

### Dataset

Se usó el dataset de Wikipedia Movie Plots (`wiki_movie_plots_deduped.csv`, ~35,000 películas, columnas: `Release Year, Title, Origin/Ethnicity, Director, Cast, Genre, Wiki Page, Plot`). Descárguenlo y colóquenlo en `data/wiki_movie_plots_deduped.csv` (no se incluye en el repositorio por su tamaño, ~78 MB — agréguenlo a `.gitignore` o usen Git LFS si su curso lo permite).

**Preprocesamiento aplicado** (responsabilidad del grupo, según el enunciado):
- Filas sin título se descartan.
- `Genre`/`Director` con valor `"unknown"` o vacío se normalizan a cadena vacía (se tratan como "sin dato" en vez de como una categoría real, ya que "unknown" no debería competir en una búsqueda por género).
- Texto normalizado a minúsculas, sin tildes/diacríticos comunes, separado en palabras alfanuméricas — igual para el texto indexado y para las consultas del usuario, para que ambos lados calcen.
- `Cast` y `Genre` (que vienen como listas separadas por comas, ej. `"Comedy, Drama"`) se separan en tokens individuales.

### Cómo compilar y correr

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
./plataforma_peliculas ../data/wiki_movie_plots_deduped.csv
```

El segundo argumento (opcional) limita cuántas filas cargar, útil para probar rápido durante el desarrollo:
```bash
./plataforma_peliculas ../data/wiki_movie_plots_deduped.csv 3000
```

**Rendimiento medido** (dataset completo, ~34,886 películas, laptop de gama media): construir todos los índices toma ~15-20 segundos y usa ~1.3 GB de RAM; una vez construidos, cada búsqueda es prácticamente instantánea (milisegundos).

### Uso

Al iniciar, si el usuario ya tiene películas en **Ver más tarde** o **Likes** (persistidos en `user_profile.txt` entre ejecuciones), se muestran automáticamente junto con recomendaciones. Luego, un menú permite:

1. Buscar por palabra, frase o sub-cadena (busca en título + sinopsis).
2. Buscar por director.
3. Buscar por actor/actriz.
4. Buscar por género.
5. Ver Likes y recomendaciones.
6. Salir.

Los resultados se muestran de 5 en 5 (ordenados por importancia), con la opción de ver las siguientes 5, o seleccionar una película para ver su sinopsis completa y marcarla como **Like** o **Ver más tarde**.

### Estructura del repositorio

```
include/        Headers (interfaces) de cada componente
src/            Implementación (.cpp) de cada componente + main.cpp
data/           (vacío en el repo) coloca aquí el CSV del dataset
docs/           Documentación de arquitectura y mapeo a la rúbrica
CMakeLists.txt  Build del proyecto
```

### Estado del proyecto / próximos pasos

Esta entrega cubre funcionalmente los 6 puntos del enunciado (ver `docs/RUBRICA.md`) con un algoritmo de importancia y uno de recomendación propios, ya probados contra el dataset real completo. Pendiente para el avance final: pruebas unitarias formales, un mejor manejo de sinónimos/errores de tipeo en la búsqueda, y — si el equipo lo considera necesario — una interfaz gráfica en vez de la consola.
