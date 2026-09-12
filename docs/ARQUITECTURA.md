# Arquitectura y justificación de diseño

## 1. La estructura de Árbol: Trie de sufijos sobre el vocabulario

El enunciado pide cargar el contenido en un **Árbol** que permita búsqueda rápida, usando como referencia **Tries** o **Suffix Trees**, donde los nodos almacenan caracteres (letras/números), y que la búsqueda funcione con:
- una palabra completa (`"barco"`),
- una frase (`"barco fantasma"`),
- o una sub-cadena dentro de una palabra (`"bar"` debe encontrar `"barco"`, `"embarcar"`, etc.)

### Por qué un Suffix Trie y no un Trie de prefijos "normal"

Un Trie de prefijos clásico (el que se usa típicamente para autocompletado) solo permite encontrar palabras que **empiecen** con la consulta. Eso resuelve el caso `"bar"` → `"barco"` (porque "bar" es prefijo de "barco"), pero **no** resolvería encontrar `"barco"` al buscar `"arco"` (que no es un prefijo, está en medio de la palabra). Por eso el enunciado sugiere explícitamente un **Suffix Tree**: si en vez de insertar cada palabra una vez, se insertan **todos sus sufijos**, entonces cualquier sub-cadena de la palabra termina siendo el prefijo de alguno de esos sufijos, y por lo tanto sí se puede encontrar recorriendo el árbol desde la raíz.

Ejemplo: para la palabra `"barco"`, se insertan los sufijos `"barco"`, `"arco"`, `"rco"`, `"co"`, `"o"`. Buscar `"arc"` como sub-cadena equivale a preguntar "¿existe algún sufijo insertado que empiece con `arc`?" — y sí, el sufijo `"arco"` empieza con `"arc"`.

### La optimización clave: indexar el vocabulario, no cada ocurrencia

La forma más directa de implementar esto sería insertar los sufijos de **cada palabra de cada sinopsis de cada película**. Con ~35,000 películas y sinopsis de cientos de palabras cada una, eso significa insertar sufijos de más de **5 millones** de ocurrencias de palabras — inviable en memoria (se probó y generaba decenas de millones de nodos).

La solución implementada separa el problema en dos capas, un patrón estándar de motores de búsqueda (índice invertido):

1. **`SuffixTrie`** (`include/SuffixTrie.hpp`): se construye **una sola vez por palabra distinta** del vocabulario completo (no por cada ocurrencia). Cada nodo guarda qué identificadores de palabra (`wordId`) tienen un sufijo que pasa por ese nodo.
2. **`TextIndex`** (`include/TextIndex.hpp`): mantiene el vocabulario (`palabra <-> wordId`) y, para cada `wordId`, una lista de películas donde esa palabra aparece junto con un peso acumulado (índice invertido / *postings list*).

Una búsqueda de sub-cadena entonces funciona en dos pasos: `SuffixTrie` responde "¿qué palabras del vocabulario contienen esta sub-cadena?" (rápido, proporcional al largo de la consulta) y `TextIndex` traduce esas palabras a películas usando el índice invertido.

**Resultado medido** sobre el dataset completo (34,886 películas, ~138,600 palabras distintas en título+sinopsis): el Trie de sufijos tiene ~858,000 nodos y toma ~15-20 segundos construirse una sola vez al iniciar el programa, usando ~1.3 GB de RAM. Cada búsqueda posterior es instantánea.

### Deduplicación O(1) amortizada

Una palabra con letras repetidas (o dos sufijos distintos de la misma palabra que comparten un prefijo) podría hacer que el mismo `wordId` se agregue muchas veces al mismo nodo. Como todos los sufijos de una palabra se insertan de forma consecutiva, cada nodo recuerda el último `wordId` insertado (`lastWordId`) y solo agrega uno nuevo si cambió — evitando duplicados sin necesitar un `set`.

## 2. Algoritmo de importancia (ranking)

Implementado en `SearchEngine::rankAndCombine`. Cada palabra de la consulta que matchea (como sub-cadena) aporta un peso a la película:

- Coincidencia en el **título**: peso 5.
- Coincidencia en la **sinopsis**: peso 1.
- Coincidencia en un **tag** (director/cast/género): peso 4.

Estos pesos reflejan que un match en el título es una señal mucho más fuerte de que "eso es lo que el usuario busca" que una mención de pasada en la sinopsis.

Para una **frase** (`"barco fantasma"`), se busca cada palabra por separado y se combinan los resultados: además de sumar los pesos, cada palabra *adicional* de la consulta que matchea en la misma película suma un bono fijo (50 puntos). Esto asegura que una película que menciona **ambas** palabras de la frase quede por encima de una que solo menciona una de ellas muchísimas veces. El desempate final es por año de estreno (las más recientes primero), como heurística simple ante la falta de datos de popularidad/rating en este dataset.

El orden final de comparación es: **(1)** cuántas palabras distintas de la consulta matchearon, **(2)** el puntaje acumulado, **(3)** año de estreno.

## 3. Recomendación ("similares a tus Likes")

Implementado en `UserProfile::recommendSimilarToLikes`. Se define la similitud entre dos películas como el **índice de Jaccard** sobre el conjunto de sus atributos: géneros + director + cada integrante del reparto (cada uno con un prefijo `g:`/`d:`/`c:` para no mezclar categorías distintas que compartan texto):

```
similitud(A, B) = |atributos(A) ∩ atributos(B)| / |atributos(A) ∪ atributos(B)|
```

Para cada película candidata (que no esté ya en Like o en Ver-más-tarde), se calcula su similitud contra **cada** película que el usuario marcó con Like, y se toma la **máxima** (no el promedio) — así, si al usuario le gustó tanto una comedia como un western, se le siguen recomendando buenas coincidencias de cualquiera de los dos gustos, en vez de diluir el puntaje promediando preferencias distintas. Se muestran las 5 mejores.

**Limitación conocida:** esta similitud no usa el texto de la sinopsis (solo metadatos estructurados), por una razón práctica: comparar sinopsis de forma ingenua sería `O(n²)` en el vocabulario y demasiado lento para 35,000 películas sin una técnica adicional (ej. TF-IDF + vectores dispersos, o LSH). Queda documentado como mejora futura para el proyecto final.

## 4. Persistencia

`UserProfile` guarda los IDs de las películas en Like/Ver-más-tarde en un archivo de texto plano (`user_profile.txt`) que se re-escribe después de cada cambio, así el estado sobrevive a que el usuario cierre el programa. El formato es intencionalmente simple (dos secciones `LIKES`/`WATCHLATER`, un ID por línea) para poder inspeccionarlo y depurarlo a mano durante el desarrollo.

## 5. Manejo de datos faltantes/corruptos

- Filas del CSV sin `Title` se descartan (no se puede buscar ni mostrar una película sin nombre).
- `Release Year` no numérico o vacío se guarda como `0` en vez de fallar la carga completa del archivo.
- `Director`/`Genre` iguales a `"unknown"` se tratan como dato ausente, no como una categoría de búsqueda válida (de lo contrario, buscar por género "unknown" devolvería miles de películas sin relación real entre sí).
