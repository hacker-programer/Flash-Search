#include <load.h>


SearchData get_data(std::string_view file, FlashSearch& search) {
    SearchData data;
    std::ifstream archivo{file.data()};

    if (!archivo.is_open()) {
        std::cerr << "¡Error! No encontré el archivo: " << file << std::endl;
        return {};
    }

    std::string linea;
    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;

        std::string limpia = search.normalize_utf8(linea); 
        
        if (!limpia.empty()) {
            // 1. Guardamos la palabra limpia y LEGIBLE en los contenedores
            data.s.insert(limpia);
            data.us.insert(limpia);
            data.m[limpia] = 1;
            data.um[limpia] = 1;
            
            // 2. Para FlashSearch, necesitamos la versión mapeada.
            // Creamos una copia temporal para no arruinar la original.
            std::string para_mapear = limpia; 
            search.add(map_text(para_mapear.data()), para_mapear.size());

            // 3. Movemos la original (que sigue siendo "casa") al vector
            data.v.push_back(std::move(limpia));
        }
    }
    return data;
}