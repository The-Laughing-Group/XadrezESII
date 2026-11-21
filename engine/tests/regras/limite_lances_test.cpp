// engine/tests/regras/limite_lances_test.cpp
// Complementa lances_exatos.cpp: o maior numero conhecido de lances numa posicao (218)
// cabe em ListaLances (MAX_LANCES = 256).
#include <string>
#include "qualidade/helpers_motor.hpp"
#include "tests/helpers_comuns.hpp"
using namespace xadrez;

static const char* FEN_218 = "R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1";

TEST(Regras_Limites, PosicaoDe218Lances_CabeNaLista) {
    Posicao p = Posicao::deFen(FEN_218);
    ListaLances lista;
    gerar_lances(p.b, lista);
    EXPECT_LE(lista.quantidade, MAX_LANCES);
    EXPECT_EQ(th::contarLegais(p.b), 218);
}

TEST(Regras_Limites, PosicaoDe218Lances_PelaApiDoJogo) {
    Jogo j = carrega(FEN_218);
    size_t total = 0;
    for (char col = 'a'; col <= 'h'; ++col)
        for (char lin = '1'; lin <= '8'; ++lin)
            total += j.lancesDe(std::string{col, lin}).size();
    EXPECT_EQ(total, 218u);
}