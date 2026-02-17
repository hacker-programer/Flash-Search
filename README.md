# Flash Search

**Flash Search** es un motor de búsqueda de alto rendimiento diseñado para lenguaje natural con una complejidad algorítmica de **O(L)**, donde *L* es la longitud del texto. Es la solución ideal para sistemas que requieren búsquedas instantáneas en textos cortos.

## 🚀 Características Principales

* **Complejidad Constante respecto al Corpus:** La velocidad de búsqueda solo depende del largo de la consulta, no del tamaño de la base de datos.
* **Optimización de Memoria:** Utiliza un sistema de *Pool* de memoria para evitar la fragmentación y mejorar la caché.
* **Soporte de Alfabeto:** Optimizado para el alfabeto español (27 letras en minúscula y espacios).
* **Compatibilidad:** Exclusivo para **Windows** (compiladores GCC y Clang).

---

## 🛠️ API Reference

### Procesamiento de Texto

| Función | Descripción |
| :--- | :--- |
| `char* map_text(char* text)` | **ADVERTENCIA:** Modifica el buffer original. Mapea el texto para que sea compatible con los algoritmos internos. Su uso es obligatorio antes de indexar o buscar. |
| `char* unmap_text(char* text, size_t s)` | **ADVERTENCIA:** Modifica el buffer. Revierte el mapeo a un formato ASCII legible para humanos. |
| `std::string normalize_utf8(const std::string& input)` | Normaliza cadenas UTF-8 (maneja tildes, eñes de 2 bytes y mayúsculas) devolviendo una cadena compatible. |

### Gestión de Estructura (FlashSearch)

#### `template<typename T> std::tuple<T*, bool, uint32_t, uint16_t> allocate()`
Reserva espacio dentro del pool de memoria.
* **Retorna:** Un tuple con `{puntero_nuevo, fue_expandido, página, offset}`.
* *Nota:* Si el pool se expande, es necesario recalcular los punteros de los objetos dentro de el usando la página y el offset.

#### `Letter* add(const char* element, size_t s)`
Añade un elemento **ya mapeado** a la estructura. Permite adjuntar información extra en el nodo final (`Letter`).

#### `std::tuple<bool, Letter*> search(const char* name, size_t s)`
Búsqueda segura. Lanza `std::invalid_argument` si el texto no está mapeado.
* **Retorna:** `{encontrado, puntero_a_letter}`.

#### `inline Letter* fast_search(const char* m_text, size_t s)`
Búsqueda de máximo rendimiento. No realiza validaciones.
* **Riesgo:** Provocará un `segfault` si el texto no está mapeado o el tamaño `s` es incorrecto.

### Visualización y Debug
* `void print()`: Imprime una representación visual del árbol en consola.
* `void _print(size_t spaces, Letter* cl)`: Helper recursivo para la visualización de nodos y descendientes.

---

## ⚠️ Consideraciones de Seguridad (Memory Safety)

Este proyecto prioriza la velocidad sobre la protección contra errores del desarrollador:
1.  **Mapeo Obligatorio:** Pasar texto sin procesar a `fast_search` o `add` resultará en un comportamiento indefinido o `segfault`.
2.  **Gestión de Punteros:** Al usar `allocate()`, si el valor de `expandido` es `true`, los punteros anteriores pueden quedar invalidados. Siempre use el `page` y `offset` para re-resolver direcciones.
3.  **Uso de `Letter::set`:** No intente usar Letter::set con memoria en el heap/stack que no haya sido creada con allocate, si lo hace obtendra un segmentation fault o comportamiento indefinido.