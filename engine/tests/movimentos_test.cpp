#include <gtest/gtest.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>
#include "xadrez/jogo.hpp"

using namespace xadrez;

// =====================================================================
// Como este arquivo está organizado
//
//  1. Auxiliares.
//  2. Tabelas de casos (FEN + casa + lista EXATA de lances legais esperados).
//     Cada caso roda duas vezes:
//       - GeraExatamenteOsLancesLegais: compara o conjunto completo, então um
//         lance errado não "compensa" um lance faltando (como acontecia com
//         EXPECT_EQ(v.size(), N)).
//       - EspelharAsCoresDaOsMesmosLances: o mesmo caso com cores trocadas e
//         tabuleiro invertido tem de dar os lances espelhados. Isso cobre peças
//         pretas sem precisar escrever tudo duas vezes.
//     Grupos: Geometria, Peao, Roque, Legalidade.
//  3. Casos de borda e situações especiais (mate, afogamento, posição inicial).
//  4. Roque executado pelo Jogo (gerar + validar + aplicar).
//  5. Perft: contagem de lances em posições de referência.
//
// Os FENs de teste usam sempre um rei de cada cor e NUNCA deixam o lado que não
// tem a vez em xeque (posição ilegal). Cuidado com rei preto em a8/h8 quando há
// torre branca em a1/h1: a coluna aberta já é xeque.
// =====================================================================

// ---------- 1. auxiliares ----------

using Lista = std::vector<std::string>;

static Lista ordenada(Lista v) {
    std::sort(v.begin(), v.end());
    return v;
}

static Lista lancesDe(const std::string& fen, const std::string& casa) {
    Jogo j;
    if (!j.carregarFen(fen)) {
        ADD_FAILURE() << "FEN rejeitado pelo Jogo: " << fen;
        return {};
    }
    return j.lancesDe(casa);
}

static bool contem(const Lista& v, const std::string& item) {
    return std::find(v.begin(), v.end(), item) != v.end();
}

// Existe um lance de `de` para `para` (com a promoção indicada, se houver)?
static bool temLance(const Lista& v, const char* de, const char* para, char promocao = 0) {
    std::string uci = std::string(de) + para;
    if (promocao) uci += promocao;
    return contem(v, uci);
}

// Soma os lances de todas as peças do lado que tem a vez no FEN (-1 se o FEN falhar).
static int totalDoLado(const std::string& fen) {
    Jogo j;
    if (!j.carregarFen(fen)) {
        ADD_FAILURE() << "FEN rejeitado pelo Jogo: " << fen;
        return -1;
    }
    int total = 0;
    for (char c = 'a'; c <= 'h'; ++c)
        for (char r = '1'; r <= '8'; ++r)
            total += static_cast<int>(j.lancesDe(std::string{c, r}).size());
    return total;
}

// --- espelhamento: inverte as fileiras e troca as cores ("e2" <-> "e7") ---

static char trocaCaixa(char c) {
    const unsigned char u = static_cast<unsigned char>(c);
    return static_cast<char>(std::isupper(u) ? std::tolower(u) : std::toupper(u));
}

static std::string espelharCasa(const std::string& casa) {
    if (casa.size() != 2) return casa;
    return std::string{casa[0], static_cast<char>('1' + '8' - casa[1])};
}

static std::string espelharLance(const std::string& uci) {
    if (uci.size() < 4) return uci;
    return espelharCasa(uci.substr(0, 2)) + espelharCasa(uci.substr(2, 2)) + uci.substr(4);
}

static std::string espelharFen(const std::string& fen) {
    std::vector<std::string> campos;
    std::istringstream in(fen);
    for (std::string c; in >> c;) campos.push_back(c);
    if (campos.size() < 4) return fen;

    std::vector<std::string> fileiras;
    std::istringstream pecas(campos[0]);
    for (std::string f; std::getline(pecas, f, '/');) fileiras.push_back(f);
    std::reverse(fileiras.begin(), fileiras.end());
    std::string tabuleiro;
    for (std::size_t i = 0; i < fileiras.size(); ++i) {
        if (i) tabuleiro += '/';
        for (char c : fileiras[i]) tabuleiro += trocaCaixa(c);
    }

    const std::string vez = (campos[1] == "w") ? "b" : "w";

    std::string roques;   // volta para a ordem canônica KQkq
    if (campos[2] != "-") {
        std::string trocados;
        for (char c : campos[2]) trocados += trocaCaixa(c);
        for (char c : std::string("KQkq"))
            if (trocados.find(c) != std::string::npos) roques += c;
    }
    if (roques.empty()) roques = "-";

    const std::string passant = (campos[3] == "-") ? "-" : espelharCasa(campos[3]);

    std::string r = tabuleiro + " " + vez + " " + roques + " " + passant;
    for (std::size_t i = 4; i < campos.size(); ++i) r += " " + campos[i];
    return r;
}

// ---------- 2. tabelas de casos ----------

struct CasoLances {
    const char* nome;
    const char* fen;
    const char* casa;
    Lista esperado;   // todos os lances legais da peça em `casa`, em UCI
};

static std::string nomeDoCaso(const ::testing::TestParamInfo<CasoLances>& info) {
    return info.param.nome;
}

class LancesExatos : public ::testing::TestWithParam<CasoLances> {};

TEST_P(LancesExatos, GeraExatamenteOsLancesLegais) {
    const CasoLances& c = GetParam();
    EXPECT_EQ(ordenada(lancesDe(c.fen, c.casa)), ordenada(c.esperado))
        << "FEN: " << c.fen << "  casa: " << c.casa;
}

TEST_P(LancesExatos, EspelharAsCoresDaOsMesmosLances) {
    const CasoLances& c = GetParam();
    Lista espelhados;
    for (const auto& m : c.esperado) espelhados.push_back(espelharLance(m));
    const std::string fenEspelhado = espelharFen(c.fen);
    EXPECT_EQ(ordenada(lancesDe(fenEspelhado, espelharCasa(c.casa))), ordenada(espelhados))
        << "FEN espelhado: " << fenEspelhado << "  casa: " << espelharCasa(c.casa);
}

// Cavalo, rei, torre, bispo e dama: alcance, bloqueio por peça própria, captura.
static const std::vector<CasoLances> kGeometria = {
    {"CavaloNoCentro",
     "k7/8/8/8/3N4/8/8/7K w - - 0 1", "d4",
     {"d4b3", "d4b5", "d4c2", "d4c6", "d4e2", "d4e6", "d4f3", "d4f5"}},
    {"CavaloNoCanto",
     "k7/8/8/8/8/8/8/N6K w - - 0 1", "a1",
     {"a1b3", "a1c2"}},
    {"CavaloNaoPousaEmPecaPropria",
     "k7/8/2P1P3/8/3N4/8/8/7K w - - 0 1", "d4",
     {"d4b3", "d4b5", "d4c2", "d4e2", "d4f3", "d4f5"}},
    {"CavaloCapturaPecaAdversaria",
     "k7/8/4p3/8/3N4/8/8/7K w - - 0 1", "d4",
     {"d4b3", "d4b5", "d4c2", "d4c6", "d4e2", "d4e6", "d4f3", "d4f5"}},
    {"CavaloPulaPecasNoCaminho",
     "k7/8/8/2PPP3/2PNP3/2PPP3/8/7K w - - 0 1", "d4",
     {"d4b3", "d4b5", "d4c2", "d4c6", "d4e2", "d4e6", "d4f3", "d4f5"}},
    {"ReiNoCentro",
     "k7/8/8/8/4K3/8/8/8 w - - 0 1", "e4",
     {"e4d3", "e4d4", "e4d5", "e4e3", "e4e5", "e4f3", "e4f4", "e4f5"}},
    {"ReiNoCanto",
     "k7/8/8/8/8/8/8/K7 w - - 0 1", "a1",
     {"a1a2", "a1b1", "a1b2"}},
    {"ReiNaoPousaEmPecaPropria",
     "k7/8/8/4P3/4KP2/8/8/8 w - - 0 1", "e4",
     {"e4d3", "e4d4", "e4d5", "e4e3", "e4f3", "e4f5"}},
    {"TorreLivreNoCanto",
     "7k/8/8/8/8/8/8/R6K w - - 0 1", "a1",
     {"a1a2", "a1a3", "a1a4", "a1a5", "a1a6", "a1a7", "a1a8", "a1b1", "a1c1", "a1d1", "a1e1", "a1f1", "a1g1"}},
    {"TorreParaNaPecaPropriaECapturaAdversaria",
     "7k/3p4/8/8/3R4/8/3P4/7K w - - 0 1", "d4",
     {"d4a4", "d4b4", "d4c4", "d4d3", "d4d5", "d4d6", "d4d7", "d4e4", "d4f4", "d4g4", "d4h4"}},
    {"BispoNoCentro",
     "k7/8/8/8/3B4/8/8/7K w - - 0 1", "d4",
     {"d4a1", "d4a7", "d4b2", "d4b6", "d4c3", "d4c5", "d4e3", "d4e5", "d4f2", "d4f6", "d4g1", "d4g7", "d4h8"}},
    {"BispoBloqueadoPorPecaPropria",
     "k7/8/8/4P3/3B4/2P5/8/7K w - - 0 1", "d4",
     {"d4a7", "d4b6", "d4c5", "d4e3", "d4f2", "d4g1"}},
    {"BispoParaNaCapturaEPecaPropria",
     "k7/8/8/2p5/3B4/8/1P6/7K w - - 0 1", "d4",
     {"d4c3", "d4c5", "d4e3", "d4e5", "d4f2", "d4f6", "d4g1", "d4g7", "d4h8"}},
    {"DamaNoCentro",
     "k7/8/8/8/3Q4/8/8/7K w - - 0 1", "d4",
     {"d4a1", "d4a4", "d4a7", "d4b2", "d4b4", "d4b6", "d4c3", "d4c4", "d4c5", "d4d1", "d4d2", "d4d3", "d4d5", "d4d6", "d4d7", "d4d8", "d4e3", "d4e4", "d4e5", "d4f2", "d4f4", "d4f6", "d4g1", "d4g4", "d4g7", "d4h4", "d4h8"}},
    {"DamaBloqueadaECapturando",
     "k7/8/1p6/8/3Q1P2/8/8/7K w - - 0 1", "d4",
     {"d4a1", "d4a4", "d4b2", "d4b4", "d4b6", "d4c3", "d4c4", "d4c5", "d4d1", "d4d2", "d4d3", "d4d5", "d4d6", "d4d7", "d4d8", "d4e3", "d4e4", "d4e5", "d4f2", "d4f6", "d4g1", "d4g7", "d4h8"}},
};
INSTANTIATE_TEST_SUITE_P(Geometria, LancesExatos, ::testing::ValuesIn(kGeometria), nomeDoCaso);

// Peão: avanço, captura, bordas, promoção e en passant.
static const std::vector<CasoLances> kPeao = {
    {"PeaoBrancoNaLinhaInicialAnda1Ou2",
     "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", "e2",
     {"e2e3", "e2e4"}},
    {"PeaoPretoAndaParaBaixo",
     "4k3/4p3/8/8/8/8/8/4K3 b - - 0 1", "e7",
     {"e7e5", "e7e6"}},
    {"PeaoForaDaLinhaInicialAnda1",
     "4k3/8/8/8/8/4P3/8/4K3 w - - 0 1", "e3",
     {"e3e4"}},
    {"PeaoBloqueadoNaPrimeiraCasaNaoAnda",
     "4k3/8/8/8/8/4n3/4P3/4K3 w - - 0 1", "e2",
     {}},
    {"PeaoBloqueadoNaSegundaCasaAnda1",
     "4k3/8/8/8/4n3/8/4P3/4K3 w - - 0 1", "e2",
     {"e2e3"}},
    {"PeaoCapturaNasDiagonais",
     "4k3/8/8/3p1p2/4P3/8/8/4K3 w - - 0 1", "e4",
     {"e4d5", "e4e5", "e4f5"}},
    {"PeaoNaoCapturaParaFrenteNemPecaPropria",
     "4k3/8/8/3Pp3/4P3/8/8/4K3 w - - 0 1", "e4",
     {}},
    {"PeaoNaColunaANaoSaiDoTabuleiro",
     "4k3/8/8/1p6/P7/8/8/4K3 w - - 0 1", "a4",
     {"a4a5", "a4b5"}},
    {"PeaoNaColunaHNaoSaiDoTabuleiro",
     "4k3/8/8/6p1/7P/8/8/4K3 w - - 0 1", "h4",
     {"h4g5", "h4h5"}},
    {"PeaoPretoBloqueadoNaoAnda",
     "4k3/4p3/4N3/8/8/8/8/4K3 b - - 0 1", "e7",
     {}},
    {"PeaoPretoCapturaParaBaixo",
     "4k3/8/8/4p3/3P1P2/8/8/4K3 b - - 0 1", "e5",
     {"e5d4", "e5e4", "e5f4"}},
    {"PromocaoGeraQuatroLances",
     "k7/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7",
     {"e7e8b", "e7e8n", "e7e8q", "e7e8r"}},
    {"PromocaoComCapturaGeraOitoLances",
     "k2r4/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7",
     {"e7d8b", "e7d8n", "e7d8q", "e7d8r", "e7e8b", "e7e8n", "e7e8q", "e7e8r"}},
    {"PromocaoComFrenteBloqueadaSoCaptura",
     "k2rn3/4P3/8/8/8/8/8/4K3 w - - 0 1", "e7",
     {"e7d8b", "e7d8n", "e7d8q", "e7d8r"}},
    {"PromocaoDoPeaoPreto",
     "4k3/8/8/8/8/8/4p3/K7 b - - 0 1", "e2",
     {"e2e1b", "e2e1n", "e2e1q", "e2e1r"}},
    {"PromocaoDoPeaoPretoComCaptura",
     "4k3/8/8/8/8/8/4p3/K2R4 b - - 0 1", "e2",
     {"e2d1b", "e2d1n", "e2d1q", "e2d1r", "e2e1b", "e2e1n", "e2e1q", "e2e1r"}},
    {"EnPassantEhGerado",
     "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1", "e5",
     {"e5d6", "e5e6"}},
    {"SemCasaDePassantNaoHaCapturaNaCasaVazia",
     "4k3/8/8/3pP3/8/8/8/4K3 w - - 0 1", "e5",
     {"e5e6"}},
    {"EnPassantDasPretas",
     "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1", "d4",
     {"d4d3", "d4e3"}},
    {"EnPassantNaColunaA",
     "4k3/8/8/Pp6/8/8/8/4K3 w - b6 0 1", "a5",
     {"a5a6", "a5b6"}},
    {"EnPassantNaColunaH",
     "4k3/8/8/6pP/8/8/8/4K3 w - g6 0 1", "h5",
     {"h5g6", "h5h6"}},
    {"EnPassantExigePeaoAdjacente",
     "4k3/8/8/3p1P2/8/8/8/4K3 w - d6 0 1", "f5",
     {"f5f6"}},
};
INSTANTIATE_TEST_SUITE_P(Peao, LancesExatos, ::testing::ValuesIn(kPeao), nomeDoCaso);

// Roque: o lance é o do rei andando duas colunas (e1g1, e1c1, e8g8, e8c8).
// Casos de FEN inconsistente (direito de roque sem torre, rei fora de casa) NÃO podem
// gerar roque falso.
static const std::vector<CasoLances> kRoque = {
    {"RoqueCurtoDasBrancas",
     "4k3/8/8/8/8/8/8/4K2R w K - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f1", "e1f2", "e1g1"}},
    {"RoqueLongoDasBrancas",
     "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1", "e1",
     {"e1c1", "e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"DoisRoquesDasBrancas",
     "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1c1", "e1d1", "e1d2", "e1e2", "e1f1", "e1f2", "e1g1"}},
    {"DoisRoquesDasPretas",
     "r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", "e8",
     {"e8c8", "e8d7", "e8d8", "e8e7", "e8f7", "e8f8", "e8g8"}},
    {"SoUmDireitoSoUmRoque",
     "r3k2r/8/8/8/8/8/8/4K3 b k - 0 1", "e8",
     {"e8d7", "e8d8", "e8e7", "e8f7", "e8f8", "e8g8"}},
    {"SemDireitoNaoGeraRoque",
     "4k3/8/8/8/8/8/8/R3K2R w - - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"CurtoBloqueadoPorPecaEntreReiETorre",
     "4k3/8/8/8/8/8/8/4KN1R w K - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f2"}},
    {"CurtoBloqueadoPorPecaAdversariaEmG1",
     "4k3/8/8/8/8/8/8/4K1nR w K - 0 1", "e1",
     {"e1d1", "e1d2", "e1f1", "e1f2"}},
    {"LongoBloqueadoPorPecaEmD1",
     "4k3/8/8/8/8/8/8/R2BK3 w Q - 0 1", "e1",
     {"e1d2", "e1e2", "e1f1", "e1f2"}},
    {"LongoBloqueadoPorPecaEmC1",
     "4k3/8/8/8/8/8/8/R1B1K3 w Q - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"LongoPrecisaDaCasaBLivre",
     "4k3/8/8/8/8/8/8/RB2K3 w Q - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"SemATorreNoCantoNaoGera",
     "4k3/8/8/8/8/8/8/4K3 w K - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"BispoAdversarioNoCantoNaoServe",
     "4k3/8/8/8/8/8/8/4K2b w K - 0 1", "e1",
     {"e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"ReiForaDaCasaInicialNaoFazRoque",
     "4k3/8/8/8/8/8/8/3K3R w K - 0 1", "d1",
     {"d1c1", "d1c2", "d1d2", "d1e1", "d1e2"}},
    {"ReiNaColunaHNaoGeraLanceExtra",
     "4k3/8/8/8/8/8/8/7K w KQ - 0 1", "h1",
     {"h1g1", "h1g2", "h1h2"}},
    {"ReiNaColunaGNaoGeraLanceExtra",
     "4k3/8/8/8/8/8/8/6K1 w KQ - 0 1", "g1",
     {"g1f1", "g1f2", "g1g2", "g1h1", "g1h2"}},
    {"ReiBrancoNaFileiraOitoNaoFazRoque",
     "R3K2R/8/8/8/8/8/8/4k3 w KQ - 0 1", "e8",
     {"e8d7", "e8d8", "e8e7", "e8f7", "e8f8"}},
    {"CasaBAtacadaNaoImpedeORoqueLongo",
     "1r6/6k1/8/8/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1c1", "e1d1", "e1d2", "e1e2", "e1f1", "e1f2", "e1g1"}},
    {"TorreAtacadaNaoImpedeORoque",
     "6k1/8/2b5/8/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1c1", "e1d1", "e1d2", "e1e2", "e1f1", "e1f2", "e1g1"}},
    {"ReiEmXequeNaoFazRoque",
     "1k6/8/8/4r3/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1d1", "e1d2", "e1f1", "e1f2"}},
    {"ReiNaoPassaPorCasaAtacadaNoRoqueCurto",
     "1k3r2/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1c1", "e1d1", "e1d2", "e1e2"}},
    {"ReiNaoPassaPorCasaAtacadaNoRoqueLongo",
     "1k1r4/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1e2", "e1f1", "e1f2", "e1g1"}},
    {"ReiNaoTerminaEmCasaAtacada",
     "1k4r1/8/8/8/8/8/8/R3K2R w KQ - 0 1", "e1",
     {"e1c1", "e1d1", "e1d2", "e1e2", "e1f1", "e1f2"}},
    {"PretasTambemRespeitamCasasAtacadas",
     "r3k2r/8/8/8/8/8/8/4KR2 b kq - 0 1", "e8",
     {"e8c8", "e8d7", "e8d8", "e8e7"}},
};
INSTANTIATE_TEST_SUITE_P(Roque, LancesExatos, ::testing::ValuesIn(kRoque), nomeDoCaso);

// Legalidade: cravada, xeque, xeque duplo, rei em casa atacada, en passant cravado.
static const std::vector<CasoLances> kLegalidade = {
    {"PecaCravadaSoAndaNaLinhaDaCravada",
     "4r1k1/8/8/8/8/8/4R3/4K3 w - - 0 1", "e2",
     {"e2e3", "e2e4", "e2e5", "e2e6", "e2e7", "e2e8"}},
    {"ReiNaoEntraEmCasaAtacada",
     "4k3/8/8/8/8/8/r7/4K3 w - - 0 1", "e1",
     {"e1d1", "e1f1"}},
    {"ReiNaoCapturaPecaProtegida",
     "4k3/8/8/8/1b6/8/3r4/4K3 w - - 0 1", "e1",
     {"e1f1"}},
    {"EmXequeCavaloNaoTemLances",
     "k3r3/8/8/8/8/8/8/1N2K3 w - - 0 1", "b1",
     {}},
    {"EmXequeReiSoFoge",
     "k3r3/8/8/8/8/8/8/1N2K3 w - - 0 1", "e1",
     {"e1d1", "e1d2", "e1f1", "e1f2"}},
    {"EmXequePodeBloquear",
     "1k2r3/8/8/8/R7/8/8/4K3 w - - 0 1", "a4",
     {"a4e4"}},
    {"DuploXequeSoOReiAnda",
     "4r1k1/8/8/8/1b6/8/8/R3K1N1 w - - 0 1", "e1",
     {"e1d1", "e1f1", "e1f2"}},
    {"DuploXequeTorreNaoAnda",
     "4r1k1/8/8/8/1b6/8/8/R3K1N1 w - - 0 1", "a1",
     {}},
    {"DuploXequeCavaloNaoAnda",
     "4r1k1/8/8/8/1b6/8/8/R3K1N1 w - - 0 1", "g1",
     {}},
    {"EnPassantQueExpoeOReiNaFileiraEhIlegal",
     "8/8/8/8/k2Pp2Q/8/8/3K4 b - d3 0 1", "e4",
     {"e4e3"}},
    {"PeaoCravadoNaDiagonalSoCapturaOCravador",
     "4k3/8/8/8/8/2b5/3P4/4K3 w - - 0 1", "d2",
     {"d2c3"}},
};
INSTANTIATE_TEST_SUITE_P(Legalidade, LancesExatos, ::testing::ValuesIn(kLegalidade), nomeDoCaso);

// ---------- 3. casos de borda e situações especiais ----------

TEST(Movimentos, CasaVaziaNaoTemLances) {
    EXPECT_TRUE(lancesDe("4k3/8/8/8/8/8/8/4K3 w - - 0 1", "d4").empty());
}

TEST(Movimentos, ForaDoTabuleiroOuMalFormadoNaoTemLances) {
    Jogo j;
    EXPECT_TRUE(j.lancesDe("a0").empty());
    EXPECT_TRUE(j.lancesDe("a9").empty());
    EXPECT_TRUE(j.lancesDe("i1").empty());
    EXPECT_TRUE(j.lancesDe("").empty());
    EXPECT_TRUE(j.lancesDe("e").empty());
    EXPECT_TRUE(j.lancesDe("e22").empty());
    EXPECT_TRUE(j.lancesDe("e2e4").empty());
}

TEST(Movimentos, PecasPresasNaPosicaoInicialNaoTemLances) {
    Jogo j;
    for (const char* casa : {"a1", "c1", "d1", "e1", "f1", "h1"})
        EXPECT_TRUE(j.lancesDe(casa).empty()) << casa;
}

TEST(Movimentos, PosicaoInicialTemVinteLancesParaCadaLado) {
    EXPECT_EQ(totalDoLado("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"), 20);
    EXPECT_EQ(totalDoLado("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"), 20);
}

TEST(Movimentos, MateNaoTemLances) {
    // mate do louco: 1.f3 e5 2.g4 Dh4#
    EXPECT_EQ(totalDoLado("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3"), 0);
}

TEST(Movimentos, AfogamentoNaoTemLances) {
    // rei preto em h8 sem lances e SEM estar em xeque
    EXPECT_EQ(totalDoLado("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1"), 0);
}

// ---------- 4. roque executado pelo Jogo (gerar + validar + aplicar) ----------

TEST(Movimentos, RoqueNoJogoListaOsDoisRoques) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    auto v = j.lancesDe("e1");
    EXPECT_TRUE(contem(v, "e1g1"));
    EXPECT_TRUE(contem(v, "e1c1"));
}

TEST(Movimentos, RoqueNoJogoCurtoDasBrancasMoveReiETorre) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    EXPECT_TRUE(j.tentarLance("e1g1"));
    EXPECT_EQ(j.fen(), "4k3/8/8/8/8/8/8/R4RK1 b - - 1 1");   // perdeu os dois direitos
}

TEST(Movimentos, RoqueNoJogoCurtoDasPretasMoveReiETorre) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1"));
    EXPECT_TRUE(j.tentarLance("e8g8"));
    EXPECT_EQ(j.fen(), "r4rk1/8/8/8/8/8/8/4K3 w - - 1 2");
}

TEST(Movimentos, RoqueNoJogoLongoDasPretasMoveReiETorre) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1"));
    EXPECT_TRUE(j.tentarLance("e8c8"));
    EXPECT_EQ(j.fen(), "2kr3r/8/8/8/8/8/8/4K3 w - - 1 2");
}

TEST(Movimentos, RoqueNoJogoTorreMovidaTiraSoOSeuRoque) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    ASSERT_TRUE(j.tentarLance("h1h2"));      // brancas
    ASSERT_TRUE(j.tentarLance("e8d8"));      // pretas
    auto v = j.lancesDe("e1");
    EXPECT_FALSE(contem(v, "e1g1"));         // a torre h já saiu
    EXPECT_TRUE(contem(v, "e1c1"));          // a torre a continua valendo
}

TEST(Movimentos, RoqueNoJogoReiMovidoNaoVoltaAFazerRoque) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    ASSERT_TRUE(j.tentarLance("e1e2"));
    ASSERT_TRUE(j.tentarLance("e8d8"));
    ASSERT_TRUE(j.tentarLance("e2e1"));      // o rei volta para a casa inicial
    ASSERT_TRUE(j.tentarLance("d8e8"));
    auto v = j.lancesDe("e1");
    EXPECT_FALSE(contem(v, "e1g1"));
    EXPECT_FALSE(contem(v, "e1c1"));
}

TEST(Movimentos, RoqueNoJogoTorreCapturadaNoCantoTiraORoqueDoDono) {
    Jogo j;
    // a torre preta sai de h8 e captura a torre branca em h1:
    // as brancas perdem o roque curto (K) e as pretas também (k), porque a torre delas saiu de h8
    ASSERT_TRUE(j.carregarFen("4k2r/8/8/8/8/8/8/R3K2R b KQk - 0 1"));
    ASSERT_TRUE(j.tentarLance("h8h1"));
    EXPECT_EQ(j.fen(), "4k3/8/8/8/8/8/8/R3K2r w Q - 0 2");
}

TEST(Movimentos, RoqueNoJogoEhRecusadoQuandoOReiPassaPorCasaAtacada) {
    Jogo j;
    // torre preta em f8 ataca f1: o roque curto é ilegal, o longo continua valendo
    ASSERT_TRUE(j.carregarFen("1k3r2/8/8/8/8/8/8/R3K2R w KQ - 0 1"));
    const std::string antes = j.fen();
    EXPECT_FALSE(j.tentarLance("e1g1"));
    EXPECT_EQ(j.fen(), antes);
    EXPECT_TRUE(j.tentarLance("e1c1"));
}

// ---------- 5. perft ----------
// Perft conta os lances legais folha a folha até uma profundidade. Os valores são os
// publicados para as posições de referência (chessprogramming.org/Perft_Results) e
// batem com o python-chess. Pegam de uma vez erros de roque, en passant, promoção,
// cravada e xeque. Se um falhar, o EXPECT diz em que profundidade a contagem desvia:
// compare lance a lance com um motor de referência a partir da profundidade anterior.
//
// "Rapido" roda sempre. "Profundo" fica DISABLED_ (leva minutos pela API de strings);
// para rodar: --gtest_also_run_disabled_tests --gtest_filter='*PerftProfundo*'

static std::uint64_t perft(Jogo base, int prof) {
    std::uint64_t n = 0;
    for (char c = 'a'; c <= 'h'; ++c)
        for (char r = '1'; r <= '8'; ++r)
            for (const auto& m : base.lancesDe(std::string{c, r})) {
                if (prof == 1) { ++n; continue; }
                Jogo filho = base;
                if (!filho.tentarLance(m)) {
                    ADD_FAILURE() << "tentarLance recusou o lance gerado " << m << " em " << base.fen();
                    continue;
                }
                n += perft(filho, prof - 1);
            }
    return n;
}

struct CasoPerft {
    const char* nome;
    const char* fen;
    std::vector<std::uint64_t> esperado;   // índice 0 = profundidade 1
};

static std::string nomeDoPerft(const ::testing::TestParamInfo<CasoPerft>& info) {
    return info.param.nome;
}

static void verificarPerft(const CasoPerft& c) {
    Jogo j;
    ASSERT_TRUE(j.carregarFen(c.fen)) << c.fen;
    for (std::size_t d = 0; d < c.esperado.size(); ++d)
        EXPECT_EQ(perft(j, static_cast<int>(d) + 1), c.esperado[d]) << c.nome << ", profundidade " << d + 1;
}

class Perft : public ::testing::TestWithParam<CasoPerft> {};
TEST_P(Perft, ContagemPorProfundidade) { verificarPerft(GetParam()); }

static const std::vector<CasoPerft> kPerftRapido = {
    {"PosicaoInicial", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", {20, 400, 8902}},
    {"Kiwipete",       "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", {48, 2039, 97862}},
    {"Posicao3",       "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", {14, 191, 2812, 43238}},
    {"Posicao4",       "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {6, 264, 9467}},
    {"Posicao5",       "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", {44, 1486, 62379}},
    {"Posicao6",       "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", {46, 2079, 89890}},
};
INSTANTIATE_TEST_SUITE_P(Rapido, Perft, ::testing::ValuesIn(kPerftRapido), nomeDoPerft);

class PerftProfundo : public ::testing::TestWithParam<CasoPerft> {};
TEST_P(PerftProfundo, DISABLED_ContagemPorProfundidade) { verificarPerft(GetParam()); }

static const std::vector<CasoPerft> kPerftProfundo = {
    {"PosicaoInicial", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", {20, 400, 8902, 197281}},
    {"Kiwipete",       "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", {48, 2039, 97862, 4085603}},
    {"Posicao3",       "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", {14, 191, 2812, 43238, 674624}},
    {"Posicao4",       "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {6, 264, 9467, 422333}},
    {"Posicao5",       "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", {44, 1486, 62379, 2103487}},
    {"Posicao6",       "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", {46, 2079, 89890, 3894594}},
};
INSTANTIATE_TEST_SUITE_P(Profundo, PerftProfundo, ::testing::ValuesIn(kPerftProfundo), nomeDoPerft);