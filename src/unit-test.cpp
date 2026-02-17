#include <load.h>
#include <flash_search.h>

int main() {
    // 1. Inicialización
    FlashSearch search;

    // Cargamos los datos. Recordá que get_data ya debería usar normalize_utf8
    // internamente para indexar correctamente en FlashSearch.
    auto data = get_data("listado-general.txt", search);

    if (data.v.empty()) {
        std::cerr << "Che, Fa, el vector está vacío. O el archivo no existe o Pata lo borró por error." << std::endl;
        return 1;
    }

    std::cout << "Iniciando validación de " << data.v.size() << " palabras..." << std::endl;
    size_t i = 0;
    for (const auto& palabra_original : data.v) {
        // --- CRUCIAL: Mantenemos el string vivo en este scope ---
        // Al guardarlo en 'limpia', el búfer de memoria es válido hasta que termine el loop
        std::string limpia = search.normalize_utf8(palabra_original);
        
        if (limpia.empty()) continue;

        // Obtenemos el puntero y el tamaño real de la cadena normalizada
        char* cs = limpia.data();
        size_t longitud = limpia.size();

        // Mapeamos el texto para que FlashSearch lo entienda (ej. a->1, b->2...)
        // map_text modifica el contenido de 'limpia' directamente a través de 'cs'
        map_text(cs); 

        // 2. Prueba con fast_search
        // Usamos 'longitud', NO 'palabra_original.size()'
        bool fast_ok = search.fast_search(cs, longitud);
        
        if (!fast_ok) {
            std::cout << "¡ERROR! Palabra: [" << palabra_original << "] no encontrada en fast_search." << std::endl;
            std::cout << "Info técnica: Size original: " << palabra_original.size() 
                      << " | Size limpio: " << longitud
                      << " | Index palabra (index dentro del vector) " << i << std::endl;
            break; 
        }

        // 3. Prueba con search estándar (la tupla de éxito y el puntero al nodo)
        auto [success, node_ptr] = search.search(cs, longitud);
        
        if (!success) {
            std::cout << "¡ERROR! Palabra: [" << palabra_original << "] falló en búsqueda estándar." << std::endl;
            break;
        }
        ++i;
    }

    std::cout << "Proceso terminado. Si no leíste errores arriba, sos un genio (por ahora)." << std::endl;

    return 0;
}