#include <benchmark/benchmark.h>
#include <flash_search.h>
#include <load.h>
#include <algorithm>
#include <random>
#include <iostream>

class DataC : public benchmark::Fixture {
public:
    inline static SearchData data;
    inline static FlashSearch search;
    inline static std::string elegido;
    inline static std::string copi;
    inline static char* palab = nullptr;
    inline static size_t s = 0;
    inline static bool initialized = false; 

    void SetUp(const benchmark::State&) override {
        if (!initialized) {
            data = get_data("listado-general.txt", search);
            
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, (int)data.v.size() - 1);
            
            elegido = search.normalize_utf8(data.v[dis(gen)]);
            copi = elegido;
            map_text(copi.data()); // Asumo que esto prepara el puntero
            
            s = copi.size();
            palab = copi.data();
            
            std::cout << "--- Palabra elegida UNA SOLA VEZ: " << elegido 
                      << ", Tamaño: " << s 
                      << ", Prefetch: " << " ---" << std::endl;
            initialized = true; 
        }
    }
};

// --- Registros condicionales basados en el CMakeLists.txt ---

#if BENCHMARK_INCLUDE_FAST_SEARCH
BENCHMARK_F(DataC, FastSearch)(benchmark::State& state) {
    for (auto _ : state) {
        auto it = search.fast_search(palab, s);
        benchmark::DoNotOptimize(it);
    }
}
#endif

#if BENCHMARK_INCLUDE_SEARCH
BENCHMARK_F(DataC, NormalSearch)(benchmark::State& state) {
    for (auto _ : state) {
        auto it = search.search(palab, s);
        benchmark::DoNotOptimize(it);
    }
}
#endif

#if BENCHMARK_INCLUDE_MAP
BENCHMARK_F(DataC, Map)(benchmark::State& state) {
    for (auto _ : state) {
        auto it = data.m.contains(elegido);
        benchmark::DoNotOptimize(it);
    }
}
#endif

#if BENCHMARK_INCLUDE_SET
BENCHMARK_F(DataC, Set)(benchmark::State& state) {
    for (auto _ : state) {
        auto it = data.s.contains(elegido);
        benchmark::DoNotOptimize(it);
    }
}
#endif

#if BENCHMARK_INCLUDE_UNMAP
BENCHMARK_F(DataC, UnMap)(benchmark::State& state) {
    for (auto _ : state) {
        auto it = data.um.contains(elegido);
        benchmark::DoNotOptimize(it);
    }
}
#endif

#if BENCHMARK_INCLUDE_UNSET
BENCHMARK_F(DataC, UnSet)(benchmark::State& state) {
    for (auto _ : state) {
        auto it = data.us.contains(elegido);
        benchmark::DoNotOptimize(it);
    }
}
#endif

BENCHMARK_MAIN();