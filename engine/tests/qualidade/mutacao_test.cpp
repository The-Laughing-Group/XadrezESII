// engine/tests/qualidade/mutacao_test.cpp
// Mutacao "em processo": cada MUTANTE e uma copia errada de uma funcao constexpr do projeto.
// A mesma bateria (suite*) roda no original (tem que PASSAR) e em cada mutante (tem que FALHAR).
// Se um mutante sobreviver, o EXPECT_FALSE fica vermelho = a bateria e fraca.
// Para mutar o codigo-fonte REAL (tt.cpp, search.cpp, lances.cpp...) use tools/mutacao.ps1.
#include <functional>
#include "helpers_motor.hpp"

namespace {

// ===================== conversoes de mate da TT =====================
using F2 = std::function<int(int, int)>;

bool suiteConversaoMate(const F2& para, const F2& da) {
    bool ok = true;
    const int ply = 7;
    ok &= para(0, ply) == 0;
    ok &= para(150, ply) == 150;
    ok &= para(-150, ply) == -150;
    ok &= para(SCORE_MATE - 3, ply) == SCORE_MATE - 3 + ply;
    ok &= para(-(SCORE_MATE - 3), ply) == -(SCORE_MATE - 3) - ply;
    ok &= para(SCORE_MATE_MIN, ply) == SCORE_MATE_MIN + ply;           // fronteira >=
    ok &= para(SCORE_MATE_MIN - 1, ply) == SCORE_MATE_MIN - 1;
    ok &= para(-SCORE_MATE_MIN, ply) == -SCORE_MATE_MIN - ply;         // fronteira <=
    ok &= para(-SCORE_MATE_MIN + 1, ply) == -SCORE_MATE_MIN + 1;
    ok &= da(150, ply) == 150;
    ok &= da(SCORE_MATE - 3, ply) == SCORE_MATE - 3 - ply;
    ok &= da(-(SCORE_MATE - 3), ply) == -(SCORE_MATE - 3) + ply;
    ok &= da(SCORE_MATE_MIN, ply) == SCORE_MATE_MIN - ply;
    ok &= da(SCORE_MATE_MIN - 1, ply) == SCORE_MATE_MIN - 1;
    ok &= da(-SCORE_MATE_MIN, ply) == -SCORE_MATE_MIN + ply;
    ok &= da(-SCORE_MATE_MIN + 1, ply) == -SCORE_MATE_MIN + 1;
    for (int s : {SCORE_MATE - 1, SCORE_MATE_MIN, 123, -123, -SCORE_MATE + 5})
        ok &= da(para(s, 4), 4) == s;                                  // ida e volta
    return ok;
}

const F2 PARA = [](int p, int ply) { return pontuacao_para_tt(p, ply); };
const F2 DA   = [](int p, int ply) { return pontuacao_da_tt(p, ply); };

}  // namespace

TEST(Mutacao_TT, Original_Passa) { EXPECT_TRUE(suiteConversaoMate(PARA, DA)); }

TEST(Mutacao_TT, Para_SinalDoPlyInvertido_Morre) {
    F2 m = [](int p, int ply) { if (p >= SCORE_MATE_MIN) return p - ply; if (p <= -SCORE_MATE_MIN) return p - ply; return p; };
    EXPECT_FALSE(suiteConversaoMate(m, DA));
}
TEST(Mutacao_TT, Para_MaiorOuIgualViraMaior_Morre) {
    F2 m = [](int p, int ply) { if (p > SCORE_MATE_MIN) return p + ply; if (p <= -SCORE_MATE_MIN) return p - ply; return p; };
    EXPECT_FALSE(suiteConversaoMate(m, DA));
}
TEST(Mutacao_TT, Para_MenorOuIgualViraMenor_Morre) {
    F2 m = [](int p, int ply) { if (p >= SCORE_MATE_MIN) return p + ply; if (p < -SCORE_MATE_MIN) return p - ply; return p; };
    EXPECT_FALSE(suiteConversaoMate(m, DA));
}
TEST(Mutacao_TT, Para_RamoNegativoRemovido_Morre) {
    F2 m = [](int p, int ply) { if (p >= SCORE_MATE_MIN) return p + ply; return p; };
    EXPECT_FALSE(suiteConversaoMate(m, DA));
}
TEST(Mutacao_TT, Para_LimiarErrado_Morre) {
    F2 m = [](int p, int ply) { if (p >= SCORE_MATE) return p + ply; if (p <= -SCORE_MATE) return p - ply; return p; };
    EXPECT_FALSE(suiteConversaoMate(m, DA));
}
TEST(Mutacao_TT, Da_SinalInvertido_Morre) {
    F2 m = [](int p, int ply) { if (p >= SCORE_MATE_MIN) return p + ply; if (p <= -SCORE_MATE_MIN) return p + ply; return p; };
    EXPECT_FALSE(suiteConversaoMate(PARA, m));
}
TEST(Mutacao_TT, Da_MaiorOuIgualViraMaior_Morre) {
    F2 m = [](int p, int ply) { if (p > SCORE_MATE_MIN) return p - ply; if (p <= -SCORE_MATE_MIN) return p + ply; return p; };
    EXPECT_FALSE(suiteConversaoMate(PARA, m));
}
TEST(Mutacao_TT, Da_MenorOuIgualViraMenor_Morre) {
    F2 m = [](int p, int ply) { if (p >= SCORE_MATE_MIN) return p - ply; if (p < -SCORE_MATE_MIN) return p + ply; return p; };
    EXPECT_FALSE(suiteConversaoMate(PARA, m));
}

// ===================== decodificadores de Lance (lances.hpp) =====================
namespace {

struct Dec {
    std::function<bool(Lance)> captura, promocao, enPassant, roque, avanco;
    std::function<TipoPeca(Lance)> tipoPromo;
    std::function<Quadrado(Lance)> origem, destino;
    std::function<int(Lance)> flags;
};

Dec original() {
    return {
        [](Lance m) { return eh_captura(m); },
        [](Lance m) { return eh_promocao(m); },
        [](Lance m) { return eh_en_passant(m); },
        [](Lance m) { return eh_roque(m); },
        [](Lance m) { return eh_avanco_duplo(m); },
        [](Lance m) { return tipo_da_promocao(m); },
        [](Lance m) { return origem_do_lance(m); },
        [](Lance m) { return destino_do_lance(m); },
        [](Lance m) { return flags_do_lance(m); },
    };
}

bool suiteLance(const Dec& d) {
    bool ok = true;
    const TipoLance todos[] = {LANCE_NORMAL, LANCE_AVANCO_DUPLO, LANCE_ROQUE_REI, LANCE_ROQUE_DAMA,
        LANCE_CAPTURA, LANCE_EN_PASSANT, LANCE_PROMO_CAVALO, LANCE_PROMO_BISPO, LANCE_PROMO_TORRE,
        LANCE_PROMO_DAMA, LANCE_CAPTURA_PROMO_CAVALO, LANCE_CAPTURA_PROMO_BISPO,
        LANCE_CAPTURA_PROMO_TORRE, LANCE_CAPTURA_PROMO_DAMA};
    for (TipoLance t : todos) {
        const Lance m = criar_lance(e2, e4, t);
        const int f = static_cast<int>(t);
        ok &= d.captura(m)   == ((f & 4) != 0);
        ok &= d.promocao(m)  == ((f & 8) != 0);
        ok &= d.enPassant(m) == (f == 5);
        ok &= d.roque(m)     == (f == 2 || f == 3);
        ok &= d.avanco(m)    == (f == 1);
        ok &= d.flags(m)     == f;
        if (f & 8) ok &= d.tipoPromo(m) == static_cast<TipoPeca>(idx(TipoPeca::CAVALO) + (f & 3));
    }
    const Quadrado amostra[] = {a1, h1, a8, h8, e4, b7, g2};
    for (Quadrado o : amostra)
        for (Quadrado q : amostra) {
            const Lance m = criar_lance(o, q, LANCE_NORMAL);
            ok &= d.origem(m) == o;
            ok &= d.destino(m) == q;
        }
    return ok;
}

}  // namespace

TEST(Mutacao_Lance, Original_Passa) { EXPECT_TRUE(suiteLance(original())); }

TEST(Mutacao_Lance, Captura_BitErrado_Morre) {
    Dec m = original(); m.captura = [](Lance x) { return ((x >> 15) & 1) != 0; };
    EXPECT_FALSE(suiteLance(m));
}
TEST(Mutacao_Lance, Promocao_BitErrado_Morre) {
    Dec m = original(); m.promocao = [](Lance x) { return ((x >> 14) & 1) != 0; };
    EXPECT_FALSE(suiteLance(m));
}
TEST(Mutacao_Lance, EnPassant_ComparaComCaptura_Morre) {
    Dec m = original(); m.enPassant = [](Lance x) { return flags_do_lance(x) == LANCE_CAPTURA; };
    EXPECT_FALSE(suiteLance(m));
}
TEST(Mutacao_Lance, Roque_SoLadoDoRei_Morre) {
    Dec m = original(); m.roque = [](Lance x) { return flags_do_lance(x) == LANCE_ROQUE_REI; };
    EXPECT_FALSE(suiteLance(m));
}
TEST(Mutacao_Lance, AvancoDuplo_ComparaComNormal_Morre) {
    Dec m = original(); m.avanco = [](Lance x) { return flags_do_lance(x) == LANCE_NORMAL; };
    EXPECT_FALSE(suiteLance(m));
}
TEST(Mutacao_Lance, TipoPromocao_MascaraDeUmBit_Morre) {
    Dec m = original();
    m.tipoPromo = [](Lance x) { return static_cast<TipoPeca>(idx(TipoPeca::CAVALO) + (flags_do_lance(x) & 1)); };
    EXPECT_FALSE(suiteLance(m));
}
TEST(Mutacao_Lance, Origem_MascaraDe5Bits_Morre) {
    Dec m = original(); m.origem = [](Lance x) { return static_cast<Quadrado>(x & 0x1F); };
    EXPECT_FALSE(suiteLance(m));
}
TEST(Mutacao_Lance, Destino_DeslocamentoErrado_Morre) {
    Dec m = original(); m.destino = [](Lance x) { return static_cast<Quadrado>((x >> 5) & 0x3F); };
    EXPECT_FALSE(suiteLance(m));
}
TEST(Mutacao_Lance, Destino_MascaraDe5Bits_Morre) {
    Dec m = original(); m.destino = [](Lance x) { return static_cast<Quadrado>((x >> 6) & 0x1F); };
    EXPECT_FALSE(suiteLance(m));
}
