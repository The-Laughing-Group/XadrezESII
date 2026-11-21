#include <gtest/gtest.h>
#include <string>
#include "xadrez/jogo.hpp"
#include "tests/helpers_comuns.hpp" 

using namespace xadrez;

// ---------- repetição tripla ----------

static void jogaTodos(Jogo& j, std::initializer_list<const char*> lances) {
    for (const char* l : lances) ASSERT_TRUE(j.tentarLance(l)) << l;
}

TEST(Situacao, RepeticaoTripla) {
    Jogo j;
    // ida e volta dos cavalos: a posição inicial reaparece a cada 4 meios-lances
    jogaTodos(j, {"g1f3", "g8f6", "f3g1", "f6g8"});
    EXPECT_EQ(j.situacao().estado, "em_andamento");        // 2ª vez: ainda não acaba
    jogaTodos(j, {"g1f3", "g8f6", "f3g1"});
    EXPECT_EQ(j.situacao().estado, "em_andamento");
    ASSERT_TRUE(j.tentarLance("f6g8"));                    // 3ª vez
    EXPECT_EQ(j.situacao().estado, "repeticao");
    EXPECT_TRUE(j.situacao().terminou);
    EXPECT_EQ(j.situacao().vencedor, "");
}

TEST(Situacao, RepeticaoEmOrdemDeLancesDiferente) {
    // a posição é a mesma mesmo se os cavalos voltam em outra ordem
    Jogo j;
    jogaTodos(j, {"g1f3", "g8f6", "b1c3", "b8c6", "f3g1", "f6g8", "c3b1", "c6b8"});   // 2ª vez
    EXPECT_EQ(j.situacao().estado, "em_andamento");
    jogaTodos(j, {"g1f3", "g8f6", "b1c3", "b8c6", "f3g1", "f6g8", "c3b1", "c6b8"});   // 3ª vez
    EXPECT_EQ(j.situacao().estado, "repeticao");
}


TEST(Situacao, MesmaDisposicaoComOutraVezNaoEhRepeticao) {
    // O rei branco faz um triângulo (3 lances) e o preto vai e volta (2): a disposição das peças
    // reaparece 3 vezes, mas sempre com um lado diferente a jogar, então nenhuma posição passa de 2.
    Jogo j = carrega("4k3/8/8/8/8/8/8/R3K3 w - - 0 1");
    jogaTodos(j, {"e1d1", "e8d8", "d1d2", "d8e8", "d2e1", "e8d8",
                  "e1d1", "d8e8", "d1d2", "e8d8", "d2e1", "d8e8"});
    EXPECT_EQ(j.situacao().estado, "em_andamento");
}

TEST(Situacao, DireitoDeRoqueFazParteDaPosicao) {
    // As peças voltam à disposição inicial nos meios-lances 0, 4 e 8, mas só o 0 tinha
    // direito de roque. Se a repetição ignorasse o roque, o meio-lance 8 contaria como 3ª vez.
    Jogo j = carrega("r3k3/8/8/8/8/8/8/R3K3 w Qq - 0 1");
    jogaTodos(j, {"a1a2", "a8a7", "a2a1", "a7a8", "a1a2", "a8a7", "a2a1", "a7a8"});
    EXPECT_EQ(j.situacao().estado, "em_andamento");

    // A posição "sem roque, torres em a2/a7" aparece nos meios-lances 2, 6 e 10
    jogaTodos(j, {"a1a2"});
    EXPECT_EQ(j.situacao().estado, "em_andamento");
    jogaTodos(j, {"a8a7"});
    EXPECT_EQ(j.situacao().estado, "repeticao");
}