#include "xadrez/jogo.hpp"
#include <stdexcept>
#include "xadrez/movimentos.hpp"

namespace xadrez {

// Converte um lance para texto UCI: origem + destino (+ peça de promoção, se houver).
static std::string movimentoParaUci(const Movimento& m) {
    std::string uci = casaParaTexto(m.deLinha, m.deColuna) + casaParaTexto(m.paraLinha, m.paraColuna);
    if (m.promocao != 0) uci += m.promocao;
    return uci;
}

Jogo::Jogo() : pos_(Posicao::inicial()) {}

bool Jogo::carregarFen(const std::string& fen) {
    try {
        pos_ = Posicao::deFen(fen);
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    }
}

std::string Jogo::fen() const {
    return pos_.paraFen();
}

bool Jogo::vezDasBrancas() const {
    return pos_.vezDasBrancas;
}

std::vector<std::string> Jogo::tabuleiro() const {
    std::vector<std::string> linhas;
    for (int l = 0; l < 8; ++l)
        linhas.emplace_back(pos_.tabuleiro[l], 8);   // 8 caracteres da linha l
    return linhas;
}

std::vector<std::string> Jogo::lancesDe(const std::string& casa) const {
    std::vector<std::string> lances;

    int linha = 0, coluna = 0;
    if (!textoParaCasa(casa, linha, coluna)) return lances;

    // Só a peça de quem tem a vez pode se mover (casa vazia também cai aqui)
    if (!pertenceAo(pos_.tabuleiro[linha][coluna], pos_.vezDasBrancas)) return lances;

    for (const Movimento& m : movBasicos(pos_, linha, coluna))
        lances.push_back(movimentoParaUci(m));
    return lances;
}

}  // namespace xadrez
