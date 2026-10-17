#include "xadrez/jogo.hpp"
#include <stdexcept>
#include "xadrez/posicao.hpp"
#include "tabuleiro/tabuleiro.hpp"
#include "../../../infra-estrutura/movimentos.hpp"
#include "lances.hpp"
#include "../../engine/include/xadrez/tipos.hpp"
#include <stdexcept>
#include <vector>
#include <string>

namespace xadrez {

// Auxiliar: Converte um Lance de 16 bits para a notação de texto UCI esperada pelo front-end (ex: "e2e4" ou "e7e8q")
static std::string lanceParaUci(Lance m) {
    Quadrado origem = origem_do_lance(m);
    Quadrado destino = destino_do_lance(m);

    std::string uci = casaParaTexto(static_cast<int>(linha_de(origem)), static_cast<int>(coluna_de(origem))) +
                      casaParaTexto(static_cast<int>(linha_de(destino)), static_cast<int>(coluna_de(destino)));

    if (eh_promocao(m)) {
        TipoPeca promo = tipo_da_promocao(m);
        if (promo == TipoPeca::DAMA) uci += 'q';
        else if (promo == TipoPeca::TORRE) uci += 'r';
        else if (promo == TipoPeca::BISPO) uci += 'b';
        else if (promo == TipoPeca::CAVALO) uci += 'n';
    }
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
    // Lê diretamente da estrutura de estado otimizada
    return pos_.b.estado.lado_a_jogar == Cor::BRANCO;
}

std::vector<std::string> Jogo::tabuleiro() const {
    std::vector<std::string> linhas;
    char CARACTERE[NUM_CORES][NUM_TIPOS] = {
        {'P', 'N', 'B', 'R', 'Q', 'K'},   // brancas  (índice 0)
        {'p', 'n', 'b', 'r', 'q', 'k'}    // pretas   (índice 1)
    };

    // O front-end espera que o índice 0 do vetor seja a linha 8 do xadrez, e o índice 7 seja a linha 1.
    // Na nossa arquitetura LERF (0 a 63), a linha 7 corresponde à fileira 8. Logo, varremos de 7 a 0.
    for (int l = 7; l >= 0; --l) {
        std::string linha_str = "";
        for (int c = 0; c < 8; ++c) {
            Quadrado sq = criar_quadrado(static_cast<Coluna>(c), static_cast<Linha>(l));
            if (casa_vazia(pos_.b, sq)) {
                linha_str += '-'; // Mantém o caractere de casa vazia para o front-end ler corretamente
            } else {
                Peca p = peca_em(pos_.b, sq);
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

    Quadrado origem = criar_quadrado(static_cast<Coluna>(coluna), static_cast<Linha>(linha));

    if (casa_vazia(pos_.b, origem)) return lances;

    Peca p = peca_em(pos_.b, origem);
    // O jogador só pode pedir lances da peça se for do lado que tem a vez
    if (p.cor != pos_.b.estado.lado_a_jogar) return lances;

    ListaLances lista;
    gerar_lances(pos_.b, lista);

    for (int i = 0; i < lista.quantidade; ++i) {
        Lance m = lista.lances[i];
        if (origem_do_lance(m) == origem) {
            // Teste de legalidade: simula o lance para ver se o rei fica em xeque
            Posicao copia = pos_;
            if (fazer_lance(copia.b, m)) {
                lances.push_back(lanceParaUci(m));
            }
        }
    }
    return lances;
}

bool Jogo::tentarLance(const std::string& uci_in) {
    if (uci_in.size() != 4 && uci_in.size() != 5) return false;
    std::string uci = uci_in;
    if (uci.size() == 5) uci[4] = static_cast<char>(std::tolower(static_cast<unsigned char>(uci[4])));

    ListaLances lista;
    gerar_lances(pos_.b, lista);

    // Varre os lances válidos gerados pelo motor procurando pelo UCI fornecido
    for (int i = 0; i < lista.quantidade; ++i) {
        Lance m = lista.lances[i];
        if (lanceParaUci(m) == uci) {
            Posicao copia = pos_;
            if (fazer_lance(copia.b, m)) {
                pos_ = copia; // O lance é legal, o jogo avança
                return true;
            }
        }
    }
    return false;
}

} // namespace xadrez
