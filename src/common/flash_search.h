#pragma once
#include <tuple>
#include <map>
#include <stdexcept>
#include <array>
#include <iostream>
#include <cstddef> // Para max_align_t y size_t
#include <new>     // Para placement new
#include <cstring>
#include <cstdlib>
#include <malloc.h>


extern const std::array<unsigned char, 256> map_fast;
extern const std::array<unsigned char, 256> unmap_fast;

#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

struct Letter;

struct alignas(64) Letter {
    // 28 punteros de 16 bits = 56 bytes
    
    uint16_t offsets[28]; 
    // 1 numero de 32 bits = 4 bytes
    uint32_t page;
    
    // 1 byte. Total = 61 bytes exactos. Una sola línea de caché L1.
    bool final_end;       

    // Constructor vital para limpiar la basura de la RAM
    Letter() {
        std::memset(this, 0, sizeof(Letter));
        page = 0xFFFFFFFF; // Ahora sí evitamos que el 'if' haga desastres en la primera vuelta.
    }

    template<typename T>
    inline T* resolve(unsigned char i, void* base_addr) const {
        // Si el offset es 0, no hay hijo
        if (unlikely(offsets[i] == 0)) return nullptr;

        // 1. Calculamos el desplazamiento total usando uintptr_t para evitar desbordamientos
        // Usamos 65536 (1 << 16) para máxima velocidad
        uintptr_t displacement = (static_cast<uintptr_t>(page) << 16) + (static_cast<uintptr_t>(offsets[i]) << 6);

        // 2. Sumamos el desplazamiento a la base y casteamos a PUNTERO (T*)
        return reinterpret_cast<T*>(static_cast<char*>(base_addr) + displacement);
    }

    template<typename T>
    void set(unsigned char i, T* target_addr, uint32_t Npage, uint16_t N_offset, size_t& cursor, void* buffer_base) {
        if (!target_addr) return;

        // Si la página del padre cambia, hay que mover a todos los hijos 
        // a la nueva página para que el cálculo 'resolve' siga funcionando.
        if (Npage != page && page != 0xFFFFFFFF) {
            for (uint16_t& e : offsets) {
                if (e != 0) {
                    uintptr_t old_addr = reinterpret_cast<uintptr_t>(buffer_base) + (static_cast<uintptr_t>(page) << 16) + (static_cast<uintptr_t>(e) << 6);
                    
                    // Alineamos y movemos
                    cursor = (cursor + alignof(T) - 1) & ~(alignof(T) - 1);
                    uintptr_t new_addr = reinterpret_cast<uintptr_t>(buffer_base) + cursor;

                    std::memcpy(reinterpret_cast<void*>(new_addr), reinterpret_cast<void*>(old_addr), sizeof(T));
                    
                    e = static_cast<uint16_t>((cursor & 0xFFFF) >> 6);
                    cursor += sizeof(T);
                }
            }
        }

        page = Npage;
        offsets[i] = N_offset; 
    }
};

class FlashSearch {
public:
    char* buffer; 
    size_t size;
    size_t cursor = 64;
    Letter* base;
    FlashSearch();

    ~FlashSearch();

    template<typename T>
    std::tuple<T*, bool, uint32_t, uint16_t> allocate();

    Letter* add(const char* element, size_t s);

    std::tuple<bool, Letter*> search(const char* name, size_t s);
    void print();
    void _print(size_t spaces, Letter* cl);

    inline Letter* fast_search(const char* m_text, size_t s) {
        Letter* curr = base;
        for (size_t i = 0; unlikely(i < s); ++i) {
            curr = curr->resolve<Letter>((unsigned char)m_text[i], buffer);
            if (!curr) return nullptr;
        }
        return curr->final_end ? curr : nullptr;
    }
    std::string normalize_utf8(const std::string& input);
};
extern const std::array<unsigned char, 256> map_fast;
extern const std::array<unsigned char, 256> unmap_fast;
char* map_text(char* text);
char* unmap_text(char* text);