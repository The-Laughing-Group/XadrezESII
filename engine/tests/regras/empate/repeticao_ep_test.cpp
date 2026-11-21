#include <algorithm>
#include <initializer_list>
#include "tests/helpers_comuns.hpp"

namespace {
void joga(Jogo& j, std::initializer_list<const char*> lances) {
    for (const char* l : lances) ASSERT_TRUE(j.tentarLance(l)) << l;
}
}

// Caso reportado: 1.e4 e5 2.Nf3 Nf6 3.Ng1 Ng8 4.Nf3 Nf6 5.Ng1 Ng8.
// Apos 1...e5 ninguem pode capturar en passant em e6, entao a posicao e a mesma dos
// meios-lances 6 e 10: 3a ocorrencia no meio-lance 10 (nao no 11).
TEST(Repeticao_EnPassant, CasoReportado_AcusaNoMeioLance10) {
    Jogo j;
    joga(j, {"e2e4", "e7e5", "g1f3", "g8f6", "f3g1", "f6g8", "g1f3", "g8f6", "f3g1"});   // 1..9
    EXPECT_EQ(j.situacao().estado, "em_andamento");
    ASSERT_TRUE(j.tentarLance("f6g8"));                                                  // 10
    EXPECT_EQ(j.situacao().estado, "repeticao");
}

// Trava contra "consertar demais": quando o en passant E capturavel, a posicao e diferente
// da mesma disposicao sem ele, e a repeticao so conta as ocorrencias iguais.
// Meios-lances: 1 e2e4 (d4xe3 possivel) ... a disposicao reaparece (sem en passant) em 5 e 9.
// So 2 ocorrencias iguais no 9 -> ainda em andamento.
TEST(Repeticao_EnPassant, EnPassantCapturavelDiferenciaAPosicao) {
    Jogo j = carrega("4k3/8/8/8/3p4/8/4P3/4K3 w - - 0 1");
    ASSERT_TRUE(j.tentarLance("e2e4"));
    const auto lances = j.lancesDe("d4");
    ASSERT_NE(std::find(lances.begin(), lances.end(), "d4e3"), lances.end())
        << "premissa do teste: o en passant d4xe3 precisa estar disponivel";

    joga(j, {"e8d8", "e1d1", "d8e8", "d1e1",      // 2..5 (disposicao igual a do 1, sem en passant)
             "e8d8", "e1d1", "d8e8"});            // 6..8
    EXPECT_EQ(j.situacao().estado, "em_andamento");
    ASSERT_TRUE(j.tentarLance("d1e1"));            // 9
    EXPECT_EQ(j.situacao().estado, "em_andamento");
}