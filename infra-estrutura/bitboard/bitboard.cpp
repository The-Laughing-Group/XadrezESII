#include "bitboard.hpp"
#include <iostream>

namespace BitboardUtils {
// Imprime '1' se tiver peça e '0' caso contrário
    void imprimir_bitboard(Bitboard bb) {
        std::cout << "\n* A B C D E F G H\n";
        for (int i = 56; i >= 0; i = i - 8) {
            std::cout << (i / 8) + 1 << "|";
            for (int j = 0; j < 8; j++) {
                std::cout << ((bb >> (i + j)) & 1ULL) << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

} // namespace BitboardUtils