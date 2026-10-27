#include <gtest/gtest.h>
#include <string>
#include "xadrez/jogo.hpp"

using namespace xadrez;

static Jogo carrega(const char* fen) {
    Jogo j;
    EXPECT_TRUE(j.carregarFen(fen)) << "FEN rejeitado: " << fen;
    return j;
}

// =====================================================================
// ultimoLance(): o que aconteceu no lance (vem das flags do Lance)
// =====================================================================

TEST(UltimoLance, VazioAntesDoPrimeiroLance) {
    Jogo j;
    EXPECT_EQ(j.ultimoLance().uci, "");
    EXPECT_EQ(j.ultimoLance().tipo, "");
    EXPECT_FALSE(j.ultimoLance().captura);
}

TEST(UltimoLance, LanceNormalEAvancoDuplo) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("g1f3"));
    EXPECT_EQ(j.ultimoLance().uci, "g1f3");
    EXPECT_EQ(j.ultimoLance().tipo, "normal");
    EXPECT_FALSE(j.ultimoLance().captura);

    ASSERT_TRUE(j.tentarLance("e7e5"));
    EXPECT_EQ(j.ultimoLance().tipo, "avanco_duplo");
}

TEST(UltimoLance, CapturaComum) {
    Jogo j = carrega("4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1");
    ASSERT_TRUE(j.tentarLance("e4d5"));
    EXPECT_EQ(j.ultimoLance().tipo, "captura");
    EXPECT_TRUE(j.ultimoLance().captura);
    EXPECT_EQ(j.ultimoLance().pecaCapturada, "p");
    EXPECT_EQ(j.ultimoLance().casaCapturada, "d5");   // captura comum: é o próprio destino
}

TEST(UltimoLance, RoqueCurtoInformaAMovimentacaoDaTorre) {
    Jogo j = carrega("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    ASSERT_TRUE(j.tentarLance("e1g1"));
    EXPECT_EQ(j.ultimoLance().tipo, "roque_rei");
    EXPECT_EQ(j.ultimoLance().torreOrigem, "h1");
    EXPECT_EQ(j.ultimoLance().torreDestino, "f1");
    EXPECT_FALSE(j.ultimoLance().captura);
}

TEST(UltimoLance, RoqueLongoDasPretas) {
    Jogo j = carrega("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");
    ASSERT_TRUE(j.tentarLance("e8c8"));
    EXPECT_EQ(j.ultimoLance().tipo, "roque_dama");
    EXPECT_EQ(j.ultimoLance().torreOrigem, "a8");
    EXPECT_EQ(j.ultimoLance().torreDestino, "d8");
}

TEST(UltimoLance, RoqueCurtoDasPretasELongoDasBrancas) {
    Jogo a = carrega("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");
    ASSERT_TRUE(a.tentarLance("e8g8"));
    EXPECT_EQ(a.ultimoLance().torreOrigem, "h8");
    EXPECT_EQ(a.ultimoLance().torreDestino, "f8");

    Jogo b = carrega("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    ASSERT_TRUE(b.tentarLance("e1c1"));
    EXPECT_EQ(b.ultimoLance().torreOrigem, "a1");
    EXPECT_EQ(b.ultimoLance().torreDestino, "d1");
}

TEST(UltimoLance, EnPassantInformaACasaDoPeaoCapturado) {
    // peão branco e5 captura o peão preto d5 "de passagem", pousando em d6
    Jogo j = carrega("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");
    ASSERT_TRUE(j.tentarLance("e5d6"));
    EXPECT_EQ(j.ultimoLance().tipo, "en_passant");
    EXPECT_TRUE(j.ultimoLance().captura);
    EXPECT_EQ(j.ultimoLance().casaCapturada, "d5");    // NÃO é d6
    EXPECT_EQ(j.ultimoLance().pecaCapturada, "p");
}

TEST(UltimoLance, EnPassantDasPretas) {
    Jogo j = carrega("4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1");
    ASSERT_TRUE(j.tentarLance("d4e3"));
    EXPECT_EQ(j.ultimoLance().tipo, "en_passant");
    EXPECT_EQ(j.ultimoLance().casaCapturada, "e4");
    EXPECT_EQ(j.ultimoLance().pecaCapturada, "P");
}

TEST(UltimoLance, PromocaoESuaVersaoComCaptura) {
    Jogo a = carrega("4k3/P7/8/8/8/8/8/4K3 w - - 0 1");
    ASSERT_TRUE(a.tentarLance("a7a8q"));
    EXPECT_EQ(a.ultimoLance().tipo, "promocao");
    EXPECT_EQ(a.ultimoLance().promocao, "q");
    EXPECT_FALSE(a.ultimoLance().captura);

    Jogo b = carrega("1r2k3/P7/8/8/8/8/8/4K3 w - - 0 1");
    ASSERT_TRUE(b.tentarLance("a7b8n"));
    EXPECT_EQ(b.ultimoLance().tipo, "captura_promocao");
    EXPECT_EQ(b.ultimoLance().promocao, "n");
    EXPECT_EQ(b.ultimoLance().pecaCapturada, "r");
    EXPECT_EQ(b.ultimoLance().casaCapturada, "b8");
}

TEST(UltimoLance, LanceRecusadoNaoMudaAInformacao) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("e2e4"));
    EXPECT_FALSE(j.tentarLance("e2e4"));     // não há mais peão em e2
    EXPECT_FALSE(j.tentarLance("a1a5"));     // torre bloqueada
    EXPECT_EQ(j.ultimoLance().uci, "e2e4");
}

TEST(UltimoLance, CarregarFenLimpa) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("e2e4"));
    ASSERT_TRUE(j.carregarFen(FEN_INICIAL));
    EXPECT_EQ(j.ultimoLance().uci, "");
    EXPECT_FALSE(j.carregarFen("isto nao e um fen"));
}

// =====================================================================
// situacao(): como está a partida
// =====================================================================

TEST(Situacao, PosicaoInicialEmAndamento) {
    Jogo j;
    const SituacaoPartida s = j.situacao();
    EXPECT_EQ(s.estado, "em_andamento");
    EXPECT_EQ(s.vencedor, "");
    EXPECT_FALSE(s.emXeque);
    EXPECT_FALSE(s.terminou);
}

TEST(Situacao, XequeSemFimDePartida) {
    Jogo j = carrega("4k3/8/8/8/8/8/4R3/4K3 b - - 0 1");
    const SituacaoPartida s = j.situacao();
    EXPECT_EQ(s.estado, "em_andamento");
    EXPECT_TRUE(s.emXeque);
    EXPECT_FALSE(s.terminou);
}

TEST(Situacao, XequeMateInformaOVencedor) {
    Jogo pretasMateadas = carrega("R5k1/5ppp/8/8/8/8/8/6K1 b - - 0 1");
    EXPECT_EQ(pretasMateadas.situacao().estado, "xeque_mate");
    EXPECT_EQ(pretasMateadas.situacao().vencedor, "brancas");
    EXPECT_TRUE(pretasMateadas.situacao().emXeque);
    EXPECT_TRUE(pretasMateadas.situacao().terminou);

    Jogo brancasMateadas = carrega("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
    EXPECT_EQ(brancasMateadas.situacao().estado, "xeque_mate");
    EXPECT_EQ(brancasMateadas.situacao().vencedor, "pretas");
}

TEST(Situacao, Afogamento) {
    Jogo j = carrega("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
    const SituacaoPartida s = j.situacao();
    EXPECT_EQ(s.estado, "afogamento");
    EXPECT_EQ(s.vencedor, "");
    EXPECT_FALSE(s.emXeque);
    EXPECT_TRUE(s.terminou);
}

TEST(Situacao, ConcordaComAsConsultasIndividuais) {
    const char* fens[] = {
        FEN_INICIAL,
        "R5k1/5ppp/8/8/8/8/8/6K1 b - - 0 1",
        "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1",
        "4k3/8/8/8/8/8/4R3/4K3 b - - 0 1",
    };
    for (const char* fen : fens) {
        Jogo j = carrega(fen);
        const SituacaoPartida s = j.situacao();
        EXPECT_EQ(s.emXeque, j.emXeque()) << fen;
        EXPECT_EQ(s.estado == "xeque_mate", j.emXequeMate()) << fen;
        EXPECT_EQ(s.estado == "afogamento", j.emAfogamento()) << fen;
    }
}

TEST(Situacao, ConsultarNaoAlteraAPosicao) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("e2e4"));
    const std::string antes = j.fen();
    (void)j.situacao();
    (void)j.situacao();
    EXPECT_EQ(j.fen(), antes);
    EXPECT_EQ(j.ultimoLance().uci, "e2e4");
}

// ---------- material insuficiente ----------

TEST(Situacao, MaterialInsuficiente) {
    const char* empates[] = {
        "4k3/8/8/8/8/8/8/4K3 w - - 0 1",            // rei x rei
        "4k3/8/8/8/8/8/8/3NK3 w - - 0 1",           // rei+cavalo x rei
        "4k3/8/8/8/8/8/8/2B1K3 w - - 0 1",          // rei+bispo x rei
        "5b1k/8/8/8/8/8/8/2B1K3 w - - 0 1",         // bispos de casas escuras dos dois lados (c1 e f8)
    };
    for (const char* fen : empates) {
        Jogo j = carrega(fen);
        EXPECT_EQ(j.situacao().estado, "material_insuficiente") << fen;
        EXPECT_TRUE(j.situacao().terminou) << fen;
    }
}

TEST(Situacao, MaterialSuficienteContinuaEmAndamento) {
    const char* jogos[] = {
        "5b1k/8/8/8/8/8/8/4KB2 w - - 0 1",          // bispos de cores DIFERENTES (f1 clara, f8 escura)
        "4k3/8/8/8/8/8/8/2NNK3 w - - 0 1",          // dois cavalos: o mate é possível
        "4k3/8/8/8/8/8/8/3NK2n w - - 0 1",          // cavalo x cavalo
        "4k3/8/8/8/8/8/8/2B1K2n w - - 0 1",         // bispo x cavalo
        "4k3/8/8/8/8/8/8/3RK3 w - - 0 1",           // torre
        "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1",          // peão
        "4k3/8/8/8/8/8/8/3QK3 w - - 0 1",           // dama
    };
    for (const char* fen : jogos) {
        Jogo j = carrega(fen);
        EXPECT_EQ(j.situacao().estado, "em_andamento") << fen;
    }
}

TEST(Situacao, MaterialInsuficienteApareceDepoisDeUmaCaptura) {
    // cavalo captura o último peão preto e sobra rei+cavalo x rei
    Jogo j = carrega("4k3/8/8/8/8/2N5/8/4K3 w - - 0 1");   // já é insuficiente: confere a regra básica
    EXPECT_EQ(j.situacao().estado, "material_insuficiente");

    Jogo k = carrega("4k3/8/3p4/8/4N3/8/8/4K3 w - - 0 1");
    EXPECT_EQ(k.situacao().estado, "em_andamento");
    ASSERT_TRUE(k.tentarLance("e4d6"));                    // Cxd6 -> rei+cavalo x rei
    EXPECT_EQ(k.situacao().estado, "material_insuficiente");
}

// ---------- regra dos 50 lances ----------

TEST(Situacao, RegraDos50Lances) {
    Jogo j = carrega("4k3/8/8/8/8/8/R7/4K3 w - - 99 80");
    EXPECT_EQ(j.situacao().estado, "em_andamento");
    ASSERT_TRUE(j.tentarLance("a2a3"));                    // lance de torre, sem captura: 99 -> 100
    EXPECT_EQ(j.situacao().estado, "regra_dos_50");
    EXPECT_TRUE(j.situacao().terminou);
    EXPECT_EQ(j.situacao().vencedor, "");
}

TEST(Situacao, LanceDePeaoOuCapturaZeraARegraDos50) {
    Jogo j = carrega("4k3/8/8/8/8/8/P6R/4K3 w - - 99 80");
    ASSERT_TRUE(j.tentarLance("a2a3"));                    // lance de peão zera o contador
    EXPECT_EQ(j.situacao().estado, "em_andamento");
}

TEST(Situacao, MateTemPrioridadeSobreARegraDos50) {
    Jogo j = carrega("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 100 3");
    EXPECT_EQ(j.situacao().estado, "xeque_mate");
    EXPECT_EQ(j.situacao().vencedor, "pretas");
}

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

TEST(Situacao, LanceDePeaoQuebraAContagemDeRepeticao) {
    Jogo j;
    jogaTodos(j, {"g1f3", "g8f6", "f3g1", "f6g8"});        // inicial 2x
    jogaTodos(j, {"e2e4", "e7e5"});                        // lance de peão: nada antes disso volta
    jogaTodos(j, {"g1f3", "g8f6", "f3g1", "f6g8"});
    EXPECT_EQ(j.situacao().estado, "em_andamento");        // essa posição só apareceu 2 vezes
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
