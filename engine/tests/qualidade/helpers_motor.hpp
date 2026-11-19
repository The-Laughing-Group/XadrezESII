// engine/tests/qualidade/helpers_motor.hpp
#pragma once
#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>
#include "../../include/xadrez/posicao.hpp"
#include "../../../infra-estrutura/tabuleiro/tabuleiro.hpp"
#include "../../../infra-estrutura/otimizacao/zobrist.hpp"
#include "../../../infra-estrutura/otimizacao/tt.hpp"
#include "../../../infra-estrutura/lances.hpp"
#include "../../../infra-estrutura/movimentos.hpp"

namespace th {

inline TipoPeca tipoDaLetra(char c) {
    switch (c) {
        case 'q': return TipoPeca::DAMA;
        case 'r': return TipoPeca::TORRE;
        case 'b': return TipoPeca::BISPO;
        case 'n': return TipoPeca::CAVALO;
        default:  return TipoPeca::PECA_VAZIA;
    }
}

// Procura entre os lances PSEUDO-legais o que corresponde ao UCI (sem validar xeque).
inline Lance acharLance(const Tabuleiro& b, const std::string& uci) {
    int dl = 0, dc = 0, pl = 0, pc = 0;
    if (uci.size() < 4 || !xadrez::textoParaCasa(uci.substr(0, 2), dl, dc) ||
        !xadrez::textoParaCasa(uci.substr(2, 2), pl, pc)) {
        ADD_FAILURE() << "UCI invalido no teste: " << uci;
        return SEM_LANCE;
    }
    const Quadrado de   = criar_quadrado(static_cast<Coluna>(dc), static_cast<Linha>(dl));
    const Quadrado para = criar_quadrado(static_cast<Coluna>(pc), static_cast<Linha>(pl));
    ListaLances lista;
    gerar_lances(b, lista);
    for (int i = 0; i < lista.quantidade; ++i) {
        const Lance m = lista.lances[i];
        if (origem_do_lance(m) != de || destino_do_lance(m) != para) continue;
        if (uci.size() == 5) {
            if (eh_promocao(m) && tipo_da_promocao(m) == tipoDaLetra(uci[4])) return m;
        } else if (!eh_promocao(m)) {
            return m;
        }
    }
    ADD_FAILURE() << "lance nao gerado: " << uci;
    return SEM_LANCE;
}

inline bool aplicar(Tabuleiro& b, const std::string& uci) {
    const Lance m = acharLance(b, uci);
    return m != SEM_LANCE && fazer_lance(b, m);
}

inline std::string paraUci(Lance m) {
    auto sq = [](int s) { return std::string{char('a' + s % 8), char('1' + s / 8)}; };
    std::string r = sq(origem_do_lance(m)) + sq(destino_do_lance(m));
    if (eh_promocao(m)) {
        switch (tipo_da_promocao(m)) {
            case TipoPeca::DAMA:   r += 'q'; break;
            case TipoPeca::TORRE:  r += 'r'; break;
            case TipoPeca::BISPO:  r += 'b'; break;
            default:               r += 'n'; break;
        }
    }
    return r;
}

// Compara bitboards, mailbox, estado e pilha de historico.
inline ::testing::AssertionResult iguais(const Tabuleiro& a, const Tabuleiro& b) {
    for (int c = 0; c < NUM_CORES; ++c) {
        if (a.ocupacoes[c] != b.ocupacoes[c])
            return ::testing::AssertionFailure() << "ocupacoes[" << c << "] diferem";
        for (int t = 0; t < NUM_TIPOS; ++t)
            if (a.pecas[c][t] != b.pecas[c][t])
                return ::testing::AssertionFailure() << "pecas[" << c << "][" << t << "] diferem";
    }
    if (a.ocupacoes_totais != b.ocupacoes_totais)
        return ::testing::AssertionFailure() << "ocupacoes_totais diferem";
    for (int i = 0; i < NUM_CASAS; ++i)
        if (a.casas[i].cor != b.casas[i].cor || a.casas[i].tpeca != b.casas[i].tpeca)
            return ::testing::AssertionFailure() << "mailbox difere na casa " << i;
    const Estado& x = a.estado;
    const Estado& y = b.estado;
    if (x.chave_zobrist != y.chave_zobrist || x.contador_50 != y.contador_50 ||
        x.direitos_roque != y.direitos_roque || x.en_passant != y.en_passant ||
        x.lado_a_jogar != y.lado_a_jogar)
        return ::testing::AssertionFailure() << "estado difere";
    if (a.conta_lance != b.conta_lance)
        return ::testing::AssertionFailure() << "conta_lance difere";
    return ::testing::AssertionSuccess();
}

// Conta folhas usando so gerar_lances + fazer_lance/desfazer_lance.
inline std::uint64_t contarNos(Tabuleiro& b, int prof) {
    if (prof == 0) return 1;
    ListaLances lista;
    gerar_lances(b, lista);
    std::uint64_t n = 0;
    for (int i = 0; i < lista.quantidade; ++i) {
        const Lance m = lista.lances[i];
        if (!fazer_lance(b, m)) continue;
        n += contarNos(b, prof - 1);
        desfazer_lance(b, m);
    }
    return n;
}

inline int contarLegais(Tabuleiro& b) { return static_cast<int>(contarNos(b, 1)); }

// "placement vez roques en-passant" (sem os contadores)
inline std::string semContadores(const std::string& fen) {
    std::size_t pos = 0;
    for (int i = 0; i < 4; ++i) {
        pos = fen.find(' ', pos);
        if (pos == std::string::npos) return fen;
        ++pos;
    }
    return fen.substr(0, pos - 1);
}

// Garante TT alocada e vazia antes de cada teste.
class LimpaTT : public ::testing::Test {
protected:
    void SetUp() override {
        if (tabela_tt.empty()) iniciar_tt(64);
        limpar_tt();
        idade_atual = 0;
    }
};

}  // namespace th
