#include "flash_search.h"

// Usamos std::array para poder inicializarlo con una lambda
const std::array<unsigned char, 256> map_fast = []() {
    std::array<unsigned char, 256> temp{};
    temp.fill(255); // Valor de error sigue siendo 255

    temp[' '] = 0;   temp['a'] = 1;   temp['b'] = 2;   
    temp['c'] = 3;   temp['d'] = 4;   temp['e'] = 5;   temp['f'] = 6;   
    temp['g'] = 7;   temp['h'] = 8;   temp['i'] = 9;   temp['j'] = 10;  
    temp['k'] = 11;  temp['l'] = 12;  temp['m'] = 13;  temp['n'] = 14;  
    temp[241] = 15;  temp['o'] = 16;  temp['p'] = 17;  temp['q'] = 18;  
    temp['r'] = 19;  temp['s'] = 20;  temp['t'] = 21;  temp['u'] = 22;  
    temp['v'] = 23;  temp['w'] = 24;  temp['x'] = 25;  temp['y'] = 26;  
    temp['z'] = 27;
    
    return temp;
}();

const std::array<unsigned char, 256> unmap_fast = []() {
    std::array<unsigned char, 256> temp{};
    temp.fill(255); 

    temp[0] = ' ';   temp[1] = 'a';   temp[2] = 'b';   
    temp[3] = 'c';   temp[4] = 'd';   temp[5] = 'e';   temp[6] = 'f';   
    temp[7] = 'g';   temp[8] = 'h';   temp[9] = 'i';   temp[10] = 'j';  
    temp[11] = 'k';  temp[12] = 'l';  temp[13] = 'm';  temp[14] = 'n';  
    temp[15] = 241;  temp[16] = 'o';  temp[17] = 'p';  temp[18] = 'q';  
    temp[19] = 'r';  temp[20] = 's';  temp[21] = 't';  temp[22] = 'u';  
    temp[23] = 'v';  temp[24] = 'w';  temp[25] = 'x';  temp[26] = 'y';  
    temp[27] = 'z';
    
    return temp;
}();




char* map_text(char* text) {
    if (!text) return nullptr;
    
    for (int i = 0; ; i++) {

        unsigned char original_byte = static_cast<unsigned char>(text[i]);
        if (original_byte == '\0') {
            break;
        }
        unsigned char mapped_value = map_fast[original_byte];

        if (mapped_value == 255) {
            throw std::invalid_argument(std::format("El caracter valor {} de su texto no es valido, los caracters soportado son espacios y letras en MINUSCULA y la ñ siendo 241.", static_cast<uint8_t>(original_byte)));
        }   

        text[i] = mapped_value;
    }
    return text;
}

char* unmap_text(char* text) {
    if (!text) return nullptr;
    
    for (int i = 0; ; i++) {

        unsigned char original_byte = static_cast<unsigned char>(text[i]);
        if (original_byte == '\0') {
            break;
        }
        unsigned char mapped_value = unmap_fast[original_byte];

        if (mapped_value == 255) {
            throw std::invalid_argument(std::format("El caracter valor {} de su texto no es valido, los caracters soportado son espacios y letras en MINUSCULA y la ñ siendo 241.", static_cast<uint8_t>(original_byte)));
        }

        text[i] = mapped_value;
    }
    return text;
}



FlashSearch::FlashSearch() : size(1024 * 64) {
    buffer = (char*)_aligned_malloc(size, 64);
    if (!buffer) throw std::bad_alloc();
    std::memset(buffer, 0, size);

    // La base vive en el primer bloque de 64 bytes (offset 64 si querés saltar el nulo)
    cursor = 64; 
    base = new (buffer + cursor) Letter();
    cursor += sizeof(Letter);
}

FlashSearch::~FlashSearch() {
    _aligned_free(buffer);
}

template<typename T>
std::tuple<T*, bool, uint32_t, uint16_t> FlashSearch::allocate() {
    size_t alignment = alignof(T);
    cursor = (cursor + alignment - 1) & ~(alignment - 1);
    
    // Si quedan menos de 1792 bytes (28 hijos posibles * 64 bytes) 
    // en esta página de 64KB, saltamos a la siguiente.
    if ((cursor & 0xFFFF) + 1792 > 0x10000) {
        cursor = (cursor + 0xFFFF) & ~0xFFFF;
        cursor += 64;
    }

    bool changedSize = false;
    if (cursor + sizeof(T) > size) {
        size_t old_size = size;
        size *= 2;
        char* newBuffer = (char*)_aligned_malloc(size, 64);
        if (!newBuffer) throw std::runtime_error("Hambre voraz de RAM.");

        std::memset(newBuffer + old_size, 0, old_size);
        std::memcpy(newBuffer, buffer, old_size);
        _aligned_free(buffer);
        buffer = newBuffer;
        changedSize = true;
    }

    uint32_t num_pagina = static_cast<uint32_t>(cursor >> 16);
    uint16_t offset_dentro = static_cast<uint16_t>((cursor & 0xFFFF) >> 6);

    void* mem = &buffer[cursor];
    cursor += sizeof(T);
    
    return {new (mem) T(), changedSize, num_pagina, offset_dentro};
}

Letter* FlashSearch::add(const char* element, size_t s) {
    Letter* actual = base;
    
    for (size_t i = 0; i < s ; i++) {
        unsigned char index = static_cast<unsigned char>(element[i]);
        if (index > 27) throw std::invalid_argument("Texto no mapeado.");

        size_t actual_offset = ((char*)actual - buffer);
        auto next_letter = actual->resolve<Letter>(index, buffer);

        if (next_letter) {
            actual = next_letter;
        } else {
            auto [new_node, expandido, page, offset] = allocate<Letter>();

            if (expandido) {
                // Si el buffer se movió, TODOS los punteros viejos murieron.
                base = reinterpret_cast<Letter*>(buffer + 64);
                actual = reinterpret_cast<Letter*>(buffer + actual_offset);
                
                // FIX: Recalcular new_node en la nueva dirección del buffer
                uintptr_t displacement = (static_cast<uintptr_t>(page) << 16) + (static_cast<uintptr_t>(offset) << 6);
                new_node = reinterpret_cast<Letter*>(buffer + displacement);
            }

            actual->set<Letter>(index, new_node, page, offset, cursor, buffer);
            actual = new_node;
        }
    }
    actual->final_end = true;
    return actual;
}

std::tuple<bool, Letter*> FlashSearch::search(const char* name, size_t s) {
    Letter* actual = base;

    for (size_t i = 0; i < s ; i++) {
        unsigned char index = static_cast<unsigned char>(name[i]);
        if (index > 27) {
            throw std::invalid_argument("FlashSearch::search debe recibir un texto YA mapeado por map_text(char*)");
        }
        auto next_letter = actual->resolve<Letter>(index, buffer);

        if (next_letter) {
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

void FlashSearch::print() {
    size_t spaces = 0;
    _print(spaces, base);
}

void FlashSearch::_print(size_t spaces, Letter* cl) {
    // Recorremos los 28 posibles hijos (0-27)
    for (unsigned char i = 0; i < 28; ++i) {
        // Resolvemos el hijo usando el índice y el buffer de la clase
        Letter* tmp = cl->resolve<Letter>(i, buffer);
        
        if (tmp) {
            // Usamos i + 1 porque el array es 0-indexed pero tu unmap_fast 
            // espera los valores originales mapeados (1-28)
            std::cout << std::string(spaces, ' ') << "Letra: \"" << unmap_fast[i + 1] << "\"\n" 
                      << std::string(spaces, ' ') << "Hijos:" << std::endl;

            // Recursión con el nuevo nodo, sin pasar 'page' porque ya no existe
            _print(spaces + 4, tmp);
        }
    }
}

std::string FlashSearch::normalize_utf8(const std::string& input) {
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