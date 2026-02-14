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

int main() {
    FlashSearch<true> search;
    const size_t iteraciones = 100000;
    const size_t tests = 100;


    // Inicialización de estructuras
    std::vector<std::string> v = {};
    std::set<std::string> s = {};
    std::map<std::string, int> m = {};
    std::unordered_map<std::string, int> um = {};
    std::unordered_set<std::string> us = {};

    std::ifstream archivo("listado-general.txt");

    if (!archivo.is_open()) {
        std::cerr << "¡Error! No encontré el listado-general.txt" << std::endl;
        return 1;
    }

    std::string linea;

    while (std::getline(archivo, linea)) {
        if (linea.empty()) continue;

        // Aquí deberías pasar 'linea' por tu función de limpieza
        // para que FlashSearch no explote con caracteres raros.
        std::string limpia = search.normalize_utf8(linea); 
        
        if (!limpia.empty()) {
            v.push_back(limpia);
            s.insert(limpia);
            us.insert(limpia);
            m[limpia] = 1;
            um[limpia] = 1;
            
            // Cargamos tu FlashSearch
            // Recordá que add necesita el texto mapeado
            std::string para_mapear = limpia;
            search.add(map_text(para_mapear.data()));
        }
    }
    std::random_device rd;  
    std::mt19937 gen(rd()); // Mersenne Twister: rápido y confiable

    // 2. Definimos el rango [0, size - 1]
    std::uniform_int_distribution<> dis(0, v.size() - 1);

    // 3. Obtenemos el índice y el elemento



    auto benchmark = [&](const std::string& name, auto lambda) {
        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < iteraciones; ++i) {
            lambda();
        }
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::nano> total = end - start;
        std::cout << std::left << std::setw(15) << name 
                  << " | Total: " << std::setw(12) << total.count() << " ns"
                  << " | Promedio: " << (total.count() / iteraciones) << " ns" << std::endl;
    };
    char opcion;
    bool vFound;
    while (true) {
        std::cout << "¿Deseas incluir al std::vector en la comparativa? (Tardara milenios) [y/n]: ";
        std::cin >> opcion;
        if (opcion == 'y' || opcion == 'Y') {
            vFound = true;
            break;
        } else if (opcion == 'n' || opcion == 'N') {
            vFound = false;
            break;
        }  else {
            std::cout << "Opcion invalida" << std::endl;
        }
    }

    for (size_t i=0; i<tests; ++i) {
        std::cout << "Test " << i+1 << ":" << std::endl;
        int indiceAzar = dis(gen);
        std::string s_nombre = v[indiceAzar];
        std::cout << "Palabra elegida: " << s_nombre << ", tamaño: " << s_nombre.size() << std::endl;
        bool WithPrefetch = s_nombre.size() > 11;

        std::cout << "--- Comparativa de Busqueda (" << iteraciones << " iteraciones) ---" << std::endl;

        // 1. Vector (O(N))
        if (vFound) {
            benchmark("std::vector", [&]() {
                auto it = std::find(v.begin(), v.end(), s_nombre);
            });
        }

        // 2. Set (O(log N))
        benchmark("std::set", [&]() {
            auto it = s.find(s_nombre);
        });

        // 3. Map (O(log N))
        benchmark("std::map", [&]() {
            auto it = m.find(s_nombre);
        });

        // 4. Unordered Map (O(1))
        benchmark("std::un_map", [&]() {
            auto it = um.find(s_nombre);
        });
        benchmark("std::un_set", [&]() {
            auto it = us.find(s_nombre);
        });
        std::string s_mapeada = s_nombre;
        char* data_lista = map_text(s_mapeada.data());
        benchmark("FlashSearch::Search", [&]() {
            search.search(data_lista);
        });
        benchmark("FlashSearch::fast_search", [&]() {
            search.fast_search(data_lista, WithPrefetch);
        });
    }
}