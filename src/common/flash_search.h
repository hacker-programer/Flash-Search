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

extern const std::array<unsigned char, 256> map_fast;
extern const std::array<unsigned char, 256> unmap_fast;

template <bool Use32Bits>
struct Letter;

template <bool Use32Bits>
struct RelativePtr {
    std::conditional_t<Use32Bits, uint32_t, uint16_t> offset = 0; 

    template<typename T>
    T* resolve(void* base_addr) const {
        if (offset == 0) return nullptr;
        return reinterpret_cast<T*>(static_cast<char*>(base_addr) + offset);
    }

    template<typename T>
    void set(void* base_addr, T* target_addr) {
        if (!target_addr) {
            offset = 0;
            return;
        }
        offset = static_cast<decltype(offset)>(reinterpret_cast<char*>(target_addr) - static_cast<char*>(base_addr));
    }
};

template <bool Use32Bits>
struct Letter {
    RelativePtr<Use32Bits> data[29] = {}; 
    void* extra_data = nullptr;
    bool final_end = false;

    std::tuple<bool, Letter<Use32Bits>*> search(unsigned char i, char* buffer_base) {
        // Si el índice es mayor a 28, es un error de entrada
        if (i > 28 || i == 0) {
            throw std::invalid_argument("Letter::search debe recibir un texto YA mapeado por map_text(char*)");
        }

        if (!data) {
            return {false, nullptr};
        }
        auto resolved = data[i].template resolve<Letter<Use32Bits>>(buffer_base);
        if (resolved) {
            return {true, resolved};
        }
        return {false, nullptr};
    }
};

template <bool Use32Bits>
class FlashSearch {
public:
    char* buffer; 
    size_t size;
    size_t cursor = 0;
    Letter<Use32Bits> base;
    static constexpr size_t MAX_OFFSET = Use32Bits ? 4294967295ULL : 65535ULL;
    FlashSearch() {
        // Alocamos en el heap para que el stack esté limpio
        buffer = new char[16 * 1024];
        size = 16*1024;
        std::memset(buffer, 0, 16 * 1024);
        cursor = sizeof(Letter<Use32Bits>);
    }

    ~FlashSearch() {
        delete[] buffer;
    }

    template<typename T>
    std::tuple<T*, bool> allocate() {
        size_t alignment = alignof(T);
        size_t padding = (alignment - (cursor % alignment)) % alignment;
        size_t total_bytes = padding + sizeof(T);
        bool changedSize = false;
        if (size - cursor < total_bytes) {
            size = size * 2;
            if (size > MAX_OFFSET) {
                throw std::runtime_error("Error fatal, Memoria no disponible, lo areglaremos pronto");
            }
            char* newBuffer = (char*)malloc(size);
            if (newBuffer == NULL) {
                throw std::runtime_error("Sin memoria");
            }
            memset(newBuffer+(size/2), 0, size/2);
            memcpy(newBuffer, buffer, size/2);
            free(buffer);
            buffer = newBuffer;
            changedSize = true;
        }

        cursor += padding;
        void* mem = &buffer[cursor];
        cursor += sizeof(T);
        
        return {new (mem) T(), changedSize};
    }

    Letter<Use32Bits>* add(const char* element) {
        Letter<Use32Bits>* actual = &base;
        
        for (int i = 0; ; i++) {
            unsigned char index = static_cast<unsigned char>(element[i]);

            if (index > 28) {
                throw std::invalid_argument("FlashSearch::add debe recibir un texto YA mapeado por map_text(char*)");
            }
            if (index == 0) {;
                actual->final_end = true;
                break;
            }

            auto [success, next_letter] = actual->search(index, buffer);

            if (success) {
                actual = next_letter;
            } else {
                // Si no existe el camino, creamos el nodo
                size_t actual_offset = 0;
                bool is_base = (actual == &base);

                if (!is_base) {
                    actual_offset = (char*)actual - buffer;
                }
                auto [new_node, expandido] = allocate<Letter<Use32Bits>>();
                if (expandido && !is_base) {
                    actual = reinterpret_cast<Letter<Use32Bits>*>(buffer + actual_offset);
                }
                actual->data[index].template set<Letter<Use32Bits>>(buffer, new_node);
                actual = new_node;
            }
        }
        return actual;
    }
    std::tuple<bool, Letter<Use32Bits>*> search(const char* name) {
        Letter<Use32Bits>* actual = &base;

        for (int i = 0; ; i++) {
            unsigned char index = static_cast<unsigned char>(name[i]);
            if (index == 0) break; 
            if (index > 28) {
                throw std::invalid_argument("FlashSearch::search debe recibir un texto YA mapeado por map_text(char*)");
            }
            auto [success, next_letter] = actual->search(index, buffer);

            if (success) {
                actual = next_letter;
            } else {
                return {false, nullptr}; 
            }

            
        }
        if (actual->final_end) {
            return {true, actual};
        } else {
            return {false, nullptr};
        }
    }
    void print() {
        size_t spaces = 0;
        _print(spaces, base);
    }
    void _print(size_t spaces, Letter<Use32Bits>& cl) {
        size_t i = 0;
        for (auto l : cl.data) {
            size_t lSpaces = spaces;
            if (l.template resolve<Letter<Use32Bits>>(buffer)) {
                std::cout << std::string(spaces, ' ') << "Letra: \"" << unmap_fast[i] << "\"\n" << std::string(spaces, ' ') << "Hijos:" << std::endl;
                lSpaces += 4;
                _print(lSpaces, *l.template resolve<Letter<Use32Bits>>(buffer));
            }
            i += 1;
        }
    }
    inline Letter<Use32Bits>* fast_search(const char* m_text, bool WithPrefetch) {
        Letter<Use32Bits>* curr = &base;
        if (WithPrefetch) {
            for (int i = 0; m_text[i] != 0; ++i) {
                // Obtenemos el puntero al siguiente nodo
                RelativePtr<Use32Bits>& next_ptr = curr->data[(unsigned char)m_text[i]];
                curr = next_ptr.template resolve<Letter<Use32Bits>>(buffer);
                
                if (!curr) return nullptr;

                if (m_text[i+1] != 0) {
                    __builtin_prefetch(curr, 0, 3);
                }
            }
        } else {
            for (int i = 0; m_text[i] != 0; ++i) {
                // Obtenemos el puntero al siguiente nodo
                RelativePtr<Use32Bits>& next_ptr = curr->data[(unsigned char)m_text[i]];
                curr = next_ptr.template resolve<Letter<Use32Bits>>(buffer);
                
                if (!curr) return nullptr;

            }
        }
        return curr->final_end ? curr : nullptr;
    }
    std::string normalize_utf8(const std::string& input) {
        std::string resultado = "";
        for (size_t i = 0; i < input.length(); ++i) {
            unsigned char c = input[i];

            // Detectar secuencia UTF-8 para caracteres especiales (á, é, í, ó, ú, ñ, etc.)
            if (c == 0xC3 && i + 1 < input.length()) {
                unsigned char next = input[++i]; // Consumimos el segundo byte
                
                switch (next) {
                    // Tildes minúsculas y mayúsculas -> Letra base
                    case 0xA1: case 0x81: resultado += 'a'; break; // á, Á
                    case 0xA9: case 0x89: resultado += 'e'; break; // é, É
                    case 0xAD: case 0x8D: resultado += 'i'; break; // í, Í
                    case 0xB3: case 0x93: resultado += 'o'; break; // ó, Ó
                    case 0xBA: case 0x9A: resultado += 'u'; break; // ú, Ú
                    case 0xBC: case 0x9C: resultado += 'u'; break; // ü, Ü
                    
                    // La Ñ: la convertimos al byte 241 que definiste
                    case 0xB1: case 0x91: resultado += (char)241; break; // ñ, Ñ
                    
                    default: break; // Ignorar otros caracteres extendidos
                }
            }
            // Letras estándar A-Z -> a-z
            else if (c >= 'A' && c <= 'Z') {
                resultado += (char)(c + 32);
            }
            // Letras estándar a-z, espacios y el byte 241 (por si ya venía mapeado)
            else if ((c >= 'a' && c <= 'z') || c == ' ' || c == 241) {
                resultado += (char)c;
            }
            // Todo lo demás (números, símbolos, \r, \n) se descarta
        }
        return resultado;
    }
};
extern const std::array<unsigned char, 256> map_fast;
extern const std::array<unsigned char, 256> unmap_fast;
char* map_text(char* text);
char* unmap_text(char* text);