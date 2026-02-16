#include <flash_search.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <random>
#include <fstream>
#include <load.h>

#include <map>
#include <iomanip>

struct LevelStats {
    size_t total_hijos = 0;
    size_t nodos_padre = 0; 
};

// Mapa para guardar: <Nivel, Estadísticas de ese nivel>
using StatsMap = std::map<int, LevelStats>;

void _AccumulateLevelStats(Letter l, void* base, int level, StatsMap& levels) {
    uint8_t locales = 0;
    
    for (unsigned char i = 0; i < 28; ++i) {
        auto e = l.resolve<Letter>(i, base);
        if (e) {
            locales++;
            // Bajamos al siguiente nivel (Nietos, Bisnietos...)

            _AccumulateLevelStats(*e, base, level + 1, levels);
        }
    }

    if (locales > 0) {
        levels[level].total_hijos += locales;
        levels[level].nodos_padre += 1;
    }
}

void PrintLevelStats(FlashSearch search) {
    StatsMap levels;
    uint8_t hijos_raiz = 0;

    // Nivel 0: La Raíz
    for (unsigned char i = 0; i < 28; ++i) {
        auto e = search.base->resolve<Letter>(i, search.buffer);
        if (e) {
            hijos_raiz++;
            _AccumulateLevelStats(*e, search.buffer, 1, levels);
        }
    }

    if (hijos_raiz > 0) {
        levels[0].total_hijos = hijos_raiz;
        levels[0].nodos_padre = 1;
    }

    std::cout << "\n--- DESGLOSE POR NIVELES ---\n";
    std::cout << std::left << std::setw(10) << "Nivel" << std::setw(15) << "Promedio" << "Nodos Padre\n";
    
    for (auto const& [lvl, stat] : levels) {
        double promedio = static_cast<double>(stat.total_hijos) / stat.nodos_padre;
        std::cout << std::left << std::setw(10) << lvl 
                  << std::setw(15) << promedio 
                  << stat.nodos_padre << "\n";
    }
}

int main() {
    FlashSearch search;

    auto data = get_data("listado-general.txt", search);
    
    PrintLevelStats(search);
}