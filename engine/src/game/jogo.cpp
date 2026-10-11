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

bool Jogo::tentarLance(const std::string& uci) {
    // Formato UCI: 4 caracteres ("e2e4") ou 5 com a promoção ("e7e8q")
    if (uci.size() != 4 && uci.size() != 5) return false;

    int deLinha = 0, deColuna = 0, paraLinha = 0, paraColuna = 0;
    if (!textoParaCasa(uci.substr(0, 2), deLinha, deColuna)) return false;
    if (!textoParaCasa(uci.substr(2, 2), paraLinha, paraColuna)) return false;

    char promocao = 0;
    if (uci.size() == 5) {
        promocao = uci[4];
        if (promocao >= 'A' && promocao <= 'Z') promocao = static_cast<char>(promocao - 'A' + 'a');
    }

    // Só a peça de quem tem a vez pode se mover
    if (!pertenceAo(pos_.tabuleiro[deLinha][deColuna], pos_.vezDasBrancas)) return false;

    // Validar = o lance precisa estar na lista de lances daquela peça
    for (const Movimento& m : movBasicos(pos_, deLinha, deColuna)) {
        if (m.paraLinha == paraLinha && m.paraColuna == paraColuna && m.promocao == promocao) {
            pos_ = aplicarLance(pos_, m);
            return true;
        }
    }
    return false;
}

}