#include "xadrez/jogo.hpp"
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>
#include "xadrez/posicao.hpp"
#include "tabuleiro/tabuleiro.hpp"
#include "xadrez/movimentos.hpp"
#include "../../../infra-estrutura/movimentos.hpp"   // ListaLances, gerar_lances
#include "lances.hpp"

namespace xadrez {

// Casa vazia lida direto do mailbox. Não usamos casa_vazia() da infra-estrutura
// porque ela está com o sentido invertido (devolve true quando a casa está ocupada).
static bool vazia(const Tabuleiro& b, Quadrado sq) {
    return peca_em(b, sq).tpeca == TipoPeca::PECA_VAZIA;
}

// Converte um Lance de 16 bits para UCI ("e2e4", "e7e8q").
static std::string lanceParaUci(Lance m) {
    const Quadrado origem  = origem_do_lance(m);
    const Quadrado destino = destino_do_lance(m);

    std::string uci = casaParaTexto(static_cast<int>(linha_de(origem)),  static_cast<int>(coluna_de(origem))) +
                      casaParaTexto(static_cast<int>(linha_de(destino)), static_cast<int>(coluna_de(destino)));

    if (eh_promocao(m)) {
        switch (tipo_da_promocao(m)) {
            case TipoPeca::DAMA:   uci += 'q'; break;
            case TipoPeca::TORRE:  uci += 'r'; break;
            case TipoPeca::BISPO:  uci += 'b'; break;
            case TipoPeca::CAVALO: uci += 'n'; break;
            default: break;
        }
    }
    return uci;
}

Jogo::Jogo() : pos_(Posicao::inicial()) {}

bool Jogo::carregarFen(const std::string& fen) {
    try {
        pos_ = Posicao::deFen(fen);   // só troca pos_ se a FEN for válida
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    }
}

std::string Jogo::fen() const {
    return pos_.paraFen();
}

bool Jogo::vezDasBrancas() const {
    return pos_.b.estado.lado_a_jogar == Cor::BRANCO;
}

std::vector<std::string> Jogo::tabuleiro() const {
    static const char CARACTERE[NUM_CORES][NUM_TIPOS] = {
        {'P', 'N', 'B', 'R', 'Q', 'K'},
        {'p', 'n', 'b', 'r', 'q', 'k'}
    };

    // O front-end espera o índice 0 = fileira 8 e o índice 7 = fileira 1.
    std::vector<std::string> linhas;
    for (int l = 7; l >= 0; --l) {
        std::string linha_str;
        for (int c = 0; c < 8; ++c) {
            const Quadrado sq = criar_quadrado(static_cast<Coluna>(c), static_cast<Linha>(l));
            if (vazia(pos_.b, sq)) {
                linha_str += '.';
            } else {
                const Peca p = peca_em(pos_.b, sq);
                linha_str += CARACTERE[idx(p.cor)][idx(p.tpeca)];
            }
        }
        linhas.emplace_back(linha_str);
    }
    return linhas;
}

std::vector<std::string> Jogo::lancesDe(const std::string& casa) const {
    std::vector<std::string> lances;
    int linha = 0, coluna = 0;
    if (!textoParaCasa(casa, linha, coluna)) return lances;

    const Quadrado origem = criar_quadrado(static_cast<Coluna>(coluna), static_cast<Linha>(linha));
    if (vazia(pos_.b, origem)) return lances;

    // Só se pode pedir lances de peça de quem tem a vez
    if (peca_em(pos_.b, origem).cor != pos_.b.estado.lado_a_jogar) return lances;

    ListaLances lista;
    gerar_lances(pos_.b, lista);

    for (int i = 0; i < lista.quantidade; ++i) {
        const Lance m = lista.lances[i];
        if (origem_do_lance(m) != origem) continue;

        // Legalidade: o lance é pseudo-legal; fazer_lance rejeita se o rei ficar em xeque
        Posicao copia = pos_;
        if (fazer_lance(copia.b, m)) lances.push_back(lanceParaUci(m));
    }
    return lances;
}

bool Jogo::tentarLance(const std::string& uci_in) {
    if (uci_in.size() != 4 && uci_in.size() != 5) return false;

    // Aceita a letra da promoção em maiúscula ("e7e8Q")
    std::string uci = uci_in;
    if (uci.size() == 5)
        uci[4] = static_cast<char>(std::tolower(static_cast<unsigned char>(uci[4])));

    ListaLances lista;
    gerar_lances(pos_.b, lista);

    for (int i = 0; i < lista.quantidade; ++i) {
        const Lance m = lista.lances[i];
        if (lanceParaUci(m) != uci) continue;

        Posicao copia = pos_;
        const bool eraVezDasPretas = (copia.b.estado.lado_a_jogar == Cor::PRETO);
        if (fazer_lance(copia.b, m)) {
            if (eraVezDasPretas) ++copia.numeroLance;
            pos_ = copia;
            return true;
        }
        return false;   // o UCI existe, mas o lance deixaria o rei em xeque
    }
    return false;
}

} // namespace xadrez