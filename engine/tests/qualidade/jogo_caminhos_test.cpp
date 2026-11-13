// engine/tests/qualidade/jogo_caminhos_test.cpp
// descreverLance (V=14), tentarLance (V=9), situacao (V=8), melhorLanceIA (V=4)
#include <gtest/gtest.h>
#include <string>
#include "../../include/xadrez/jogo.hpp"
using namespace xadrez;

static Jogo carrega(const char* fen) {
    Jogo j;
    EXPECT_TRUE(j.carregarFen(fen)) << fen;
    return j;
}

TEST(CC_Jogo, MelhorLanceIA_TodasAsGuardas) {
    Jogo brancas;
    EXPECT_EQ(brancas.melhorLanceIA(3), "");                       // vez das brancas

    Jogo j = carrega("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    EXPECT_EQ(j.melhorLanceIA(0), "");                             // profundidade 0
    EXPECT_EQ(j.melhorLanceIA(-1), "");                            // profundidade negativa

    Jogo afog = carrega("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
    EXPECT_EQ(afog.melhorLanceIA(3), "");                          // SEM_LANCE (afogamento)
    Jogo mate = carrega("R5k1/5ppp/8/8/8/8/8/6K1 b - - 0 1");
    EXPECT_EQ(mate.melhorLanceIA(3), "");                          // SEM_LANCE (mate)

    const std::string m = j.melhorLanceIA(2);                      // caminho feliz
    ASSERT_FALSE(m.empty());
    EXPECT_TRUE(j.tentarLance(m));
}

TEST(CC_Jogo, TentarLance_GuardasDeFormato) {
    Jogo j;
    for (const char* t : {"", "e", "e2e", "e2e4q", "e2e4qq", "E2E4", "e2e4\n"})
        EXPECT_FALSE(j.tentarLance(t)) << "'" << t << "'";
    EXPECT_EQ(j.fen(), FEN_INICIAL);
}

TEST(CC_Jogo, TentarLance_Ilegal_NaoMudaNemUltimoLance) {
    Jogo j = carrega("4r1k1/8/8/8/8/8/4R3/4K3 w - - 0 1");
    const std::string antes = j.fen();
    EXPECT_FALSE(j.tentarLance("e2d2"));     // existe em UCI, mas deixa o rei em xeque
    EXPECT_EQ(j.fen(), antes);
    EXPECT_EQ(j.ultimoLance().uci, "");
}

TEST(CC_Jogo, TentarLance_PretasIncrementamONumeroDoLance) {
    Jogo j;
    ASSERT_TRUE(j.tentarLance("e2e4"));
    EXPECT_NE(j.fen().find(" 0 1"), std::string::npos);
    ASSERT_TRUE(j.tentarLance("e7e5"));
    EXPECT_NE(j.fen().find(" 0 2"), std::string::npos);
}

struct CasoTipo { const char* fen; const char* uci; const char* tipo; bool captura; };
TEST(CC_Jogo, DescreverLance_TodosOsTipos) {
    const CasoTipo casos[] = {
        {FEN_INICIAL, "g1f3", "normal", false},
        {FEN_INICIAL, "e2e4", "avanco_duplo", false},
        {"4k3/8/8/3p4/4P3/8/8/4K3 w - - 0 1", "e4d5", "captura", true},
        {"4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "e5d6", "en_passant", true},
        {"4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1", "d4e3", "en_passant", true},
        {"k7/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7e8q", "promocao", false},
        {"k2r4/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7d8q", "captura_promocao", true},
        {"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "e1g1", "roque_rei", false},
        {"r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1", "e1c1", "roque_dama", false},
        {"r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1", "e8g8", "roque_rei", false},
    };
    for (const CasoTipo& c : casos) {
        Jogo j = carrega(c.fen);
        ASSERT_TRUE(j.tentarLance(c.uci)) << c.uci;
        EXPECT_EQ(j.ultimoLance().tipo, c.tipo) << c.uci;
        EXPECT_EQ(j.ultimoLance().captura, c.captura) << c.uci;
    }
}

TEST(CC_Jogo, DescreverLance_CasaDaVitimaNoEnPassant) {
    Jogo w = carrega("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");
    ASSERT_TRUE(w.tentarLance("e5d6"));
    EXPECT_EQ(w.ultimoLance().casaCapturada, "d5");
    Jogo b = carrega("4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1");
    ASSERT_TRUE(b.tentarLance("d4e3"));
    EXPECT_EQ(b.ultimoLance().casaCapturada, "e4");
}

TEST(CC_Jogo, Situacao_UmCaminhoPorEstadoEPrioridades) {
    struct C { const char* fen; const char* estado; const char* vencedor; };
    const C casos[] = {
        {FEN_INICIAL,                                  "em_andamento", ""},
        {"R5k1/5ppp/8/8/8/8/8/6K1 b - - 0 1",           "xeque_mate", "brancas"},
        {"rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3", "xeque_mate", "pretas"},
        {"7k/5Q2/6K1/8/8/8/8/8 b - - 0 1",              "afogamento", ""},
        {"4k3/8/8/8/8/8/8/3NK3 w - - 0 1",              "material_insuficiente", ""},
        {"4k3/8/8/8/8/8/R7/4K3 w - - 100 80",           "regra_dos_50", ""},
        // prioridades: afogamento > 50 lances; material insuficiente > 50 lances; mate > 50 lances
        {"7k/5Q2/6K1/8/8/8/8/8 b - - 100 80",           "afogamento", ""},
        {"4k3/8/8/8/8/8/8/4K3 w - - 100 80",            "material_insuficiente", ""},
        {"R5k1/5ppp/8/8/8/8/8/6K1 b - - 100 80",        "xeque_mate", "brancas"},
    };
    for (const C& c : casos) {
        Jogo j = carrega(c.fen);
        const SituacaoPartida s = j.situacao();
        EXPECT_EQ(s.estado, c.estado) << c.fen;
        EXPECT_EQ(s.vencedor, c.vencedor) << c.fen;
        EXPECT_EQ(s.terminou, std::string(c.estado) != "em_andamento") << c.fen;
    }
}

TEST(CC_Jogo, LancesDe_CravadaEFiltroDeLegalidade) {
    Jogo j = carrega("4r1k1/8/8/8/8/8/4R3/4K3 w - - 0 1");
    for (const std::string& m : j.lancesDe("e2")) {
        EXPECT_EQ(m[0], 'e') << m;     // so anda na coluna e (cravada)
        EXPECT_EQ(m[2], 'e') << m;
    }
    EXPECT_FALSE(j.lancesDe("e2").empty());
}
