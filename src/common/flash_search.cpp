#include "flash_search.h"

// Usamos std::array para poder inicializarlo con una lambda
const std::array<unsigned char, 256> map_fast = []() {
    std::array<unsigned char, 256> temp{};
    temp.fill(255); // Valor de error

    temp[' '] = 1;   temp['a'] = 2;   temp['b'] = 3;   
    temp['c'] = 4;   temp['d'] = 5;   temp['e'] = 6;   temp['f'] = 7;   
    temp['g'] = 8;   temp['h'] = 9;   temp['i'] = 10;  temp['j'] = 11;  
    temp['k'] = 12;  temp['l'] = 13;  temp['m'] = 14;  temp['n'] = 15;  
    temp[241] = 16;  temp['o'] = 17;  temp['p'] = 18;  temp['q'] = 19;  
    temp['r'] = 20;  temp['s'] = 21;  temp['t'] = 22;  temp['u'] = 23;  
    temp['v'] = 24;  temp['w'] = 25;  temp['x'] = 26;  temp['y'] = 27;  
    temp['z'] = 28;
    
    return temp;
}();
const std::array<unsigned char, 256> unmap_fast = []() {
    std::array<unsigned char, 256> temp{};
    temp.fill(255); // Valor de error

    temp[1] = ' ';   temp[2] = 'a';   temp[3] = 'b';   
    temp[4] = 'c';   temp[5] = 'd';   temp[6] = 'e';   temp[7] = 'f';   
    temp[8] = 'g';   temp[9] = 'h';   temp[10] = 'i';  temp[11] = 'j';  
    temp[12] = 'k';  temp[13] = 'l';  temp[14] = 'm';  temp[15] = 'n';  
    temp[16] = 241;  temp[17] = 'o';  temp[18] = 'p';  temp[19] = 'q';  
    temp[20] = 'r';  temp[21] = 's';  temp[22] = 't';  temp[23] = 'u';  
    temp[24] = 'v';  temp[25] = 'w';  temp[26] = 'x';  temp[27] = 'y';  
    temp[28] = 'z';
    
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
