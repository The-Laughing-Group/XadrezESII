#include "xadrez/posicao.hpp"
#include "tabuleiro/tabuleiro.hpp"
#include <stdexcept>
#include "otimizacao/zobrist.hpp"


namespace xadrez {


std::string casaParaTexto(int linha, int coluna) {
    std::string texto;
    texto += static_cast<char>('a' + coluna);
    texto += static_cast<char>('1' + linha);
    return texto;
}

bool textoParaCasa(const std::string& texto, int& linha, int& coluna) {
    if (texto.size() != 2) return false;
    if (texto[0] < 'a' || texto[0] > 'h') return false;
    if (texto[1] < '1' || texto[1] > '8') return false;
    coluna = texto[0] - 'a';
    linha = texto[1] - '1';
    return true;
}

Posicao::Posicao() {
    iniciar_zobrist();
    limpar(b);
}

Posicao Posicao::inicial() {
    Posicao p;
    posicao_inicial(p.b);
    return p;
}

Posicao Posicao::deFen(const std::string& fen) {
    Posicao p;
    if (!carregar_fen(p.b, fen)) {
        throw std::invalid_argument("FEN invalida ou incompleta");
    }
    return p;
}

std::string Posicao::paraFen() const {
    return gerar_fen(b);
}

} // namespace xadrez