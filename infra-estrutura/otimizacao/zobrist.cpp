#include "otimizacao/zobrist.hpp"

uint64_t ZOBRIST[ZOB_TOTAL];

// Gerador determinístico (splitmix64): mesma sequência em toda execução.
static uint64_t proximo(uint64_t& s) {
    uint64_t z = (s += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

void iniciar_zobrist() {
    static bool pronto = false;   // pode ser chamada várias vezes
    if (pronto) return;
    uint64_t semente = 0x1234ABCD5678EF01ULL;
    for (int i = 0; i < ZOB_TOTAL; ++i) ZOBRIST[i] = proximo(semente);
    pronto = true;
}
