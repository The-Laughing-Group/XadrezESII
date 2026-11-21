
#include <gtest/gtest.h>
#include <string>
#include "qualidade/helpers_motor.hpp"   // th::acharLance
#include "xadrez/posicao.hpp"
#include "otimizacao/zobrist.hpp"

using namespace xadrez;

static std::uint64_t chaveDe(const char* fen) { return Posicao::deFen(fen).b.estado.chave_zobrist; }

// A chave calculada pelo parser de FEN (posicao.cpp) e a de zobrist.cpp tem que coincidir.
TEST(Chave_Fen, DeFenBateComCalcularChave) {
    const char* fens[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "r3k2r/8/8/8/8/8/8/R3K2R b Kq - 5 20",
        "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1",
        "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1",
        "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1",
    };
    for (const char* f : fens) {
        Posicao p = Posicao::deFen(f);
        EXPECT_EQ(p.b.estado.chave_zobrist, calcular_chave(p.b)) << f;
    }
}

// En passant que ninguem pode capturar NAO entra na chave (FIDE 9.2.3).
TEST(Chave_EnPassant, IgnoradoQuandoNaoHaPeaoParaCapturar) {
    EXPECT_EQ(chaveDe("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"),
              chaveDe("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1"));
    EXPECT_EQ(chaveDe("4k3/8/8/4p3/8/8/8/4K3 w - e6 0 1"),     // brancas sem peao: nada captura
              chaveDe("4k3/8/8/4p3/8/8/8/4K3 w - - 0 1"));
}

// Quando HA peao que captura, a casa de en passant faz parte da posicao.
TEST(Chave_EnPassant, UsadoQuandoHaPeaoParaCapturar) {
    EXPECT_NE(chaveDe("4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1"),    // d4xe3 possivel
              chaveDe("4k3/8/8/8/3pP3/8/8/4K3 b - - 0 1"));
    EXPECT_NE(chaveDe("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1"),    // e5xd6 possivel
              chaveDe("4k3/8/8/3pP3/8/8/8/4K3 w - - 0 1"));
}

// Chave incremental (fazer_lance) == chave recalculada a partir do FEN, depois de lances
// que criam en passant com e sem peao capturador.
TEST(Chave_EnPassant, IncrementalBateComFenNosDoisCasos) {
    struct Caso { const char* fen; const char* uci; };
    const Caso casos[] = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "e2e4"},   // sem capturador
        {"4k3/8/8/8/3p4/8/4P3/4K3 w - - 0 1",                         "e2e4"},   // com capturador
        {"4k3/4p3/8/3P4/8/8/8/4K3 b - - 0 1",                         "e7e5"},   // com capturador
        {"4k3/4p3/8/8/8/8/8/4K3 b - - 0 1",                           "e7e5"},   // sem capturador
    };
    for (const Caso& c : casos) {
        const Posicao p0 = Posicao::deFen(c.fen);
        const Posicao p1 = aplicarLance(p0, th::acharLance(p0.b, c.uci));
        EXPECT_EQ(p1.b.estado.chave_zobrist, Posicao::deFen(p1.paraFen()).b.estado.chave_zobrist)
            << c.fen << " " << c.uci;
        EXPECT_EQ(p1.b.estado.chave_zobrist, calcular_chave(p1.b)) << c.fen << " " << c.uci;
    }
}