// engine/tests/qualidade/historico_cheio_test.cpp
// Unitario (infra): fazer_lance / desfazer_lance no limite da pilha de historico.
// Complementa CC_Lances (lances_caminhos_test.cpp).
#include <memory>
#include "helpers_motor.hpp"
using namespace xadrez;

// A ultima vaga da pilha (conta_lance == TAMANHO_HISTORICO - 1) ainda pode ser usada.
TEST(CC_Lances_Historico, UltimaVagaAindaFunciona) {
    Posicao p = Posicao::inicial();
    p.b.conta_lance = TAMANHO_HISTORICO - 1;
    const Lance m = th::acharLance(p.b, "e2e4");
    ASSERT_NE(m, SEM_LANCE);
    ASSERT_TRUE(fazer_lance(p.b, m));
    EXPECT_EQ(p.b.conta_lance, TAMANHO_HISTORICO);
    EXPECT_TRUE(verificar_consistencia(p.b));

    desfazer_lance(p.b, m);                       // e a pilha tambem desfaz no limite
    EXPECT_EQ(p.b.conta_lance, TAMANHO_HISTORICO - 1);
    EXPECT_TRUE(verificar_consistencia(p.b));
}

// Com a pilha cheia, fazer_lance deve RECUSAR (false) e deixar o tabuleiro intacto.
// (Hoje escreve fora do array: rode tambem com ASan.)
TEST(CC_Lances_Historico, PilhaCheiaRecusaOLanceENaoAlteraNada) {
    Posicao p = Posicao::inicial();
    p.b.conta_lance = TAMANHO_HISTORICO;
    auto antes = std::make_unique<Tabuleiro>(p.b);

    const Lance m = th::acharLance(p.b, "e2e4");
    ASSERT_NE(m, SEM_LANCE);
    EXPECT_FALSE(fazer_lance(p.b, m));
    EXPECT_TRUE(th::iguais(*antes, p.b));
    EXPECT_EQ(p.b.conta_lance, TAMANHO_HISTORICO);
}