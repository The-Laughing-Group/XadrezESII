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

static std::string nomeDaCasa(Quadrado sq) {
    return casaParaTexto(static_cast<int>(linha_de(sq)), static_cast<int>(coluna_de(sq)));
}

// 'P' 'N' 'B' 'R' 'Q' 'K' para as brancas, minúsculas para as pretas.
static char caractereDaPeca(Peca p) {
    static const char CARACTERE[NUM_CORES][NUM_TIPOS] = {
        {'P', 'N', 'B', 'R', 'Q', 'K'},
        {'p', 'n', 'b', 'r', 'q', 'k'}
    };
    return CARACTERE[idx(p.cor)][idx(p.tpeca)];
}

// Converte um Lance de 16 bits para UCI ("e2e4", "e7e8q").
static std::string lanceParaUci(Lance m) {
    const Quadrado origem  = origem_do_lance(m);
    const Quadrado destino = destino_do_lance(m);

    std::string uci = nomeDaCasa(origem) + nomeDaCasa(destino);

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

// Descreve o lance olhando o tabuleiro de ANTES dele (depois da jogada a peça capturada já sumiu).
static InfoLance descreverLance(const Tabuleiro& antes, Lance m) {
    InfoLance info;
    info.uci = lanceParaUci(m);

    const int destino = destino_do_lance(m);

    // 1) tipo do lance, a partir das flags
    if (eh_roque(m)) {
        const bool curto = (flags_do_lance(m) == LANCE_ROQUE_REI);
        info.tipo = curto ? "roque_rei" : "roque_dama";
        // O lance é o do REI (e1g1, e1c1...). A torre vai junto:
        //   curto: h -> f   (destino+1 -> destino-1)      longo: a -> d   (destino-2 -> destino+1)
        info.torreOrigem  = nomeDaCasa(static_cast<Quadrado>(curto ? destino + 1 : destino - 2));
        info.torreDestino = nomeDaCasa(static_cast<Quadrado>(curto ? destino - 1 : destino + 1));
    } else if (eh_en_passant(m))   info.tipo = "en_passant";
    else if (eh_promocao(m))       info.tipo = eh_captura(m) ? "captura_promocao" : "promocao";
    else if (eh_avanco_duplo(m))   info.tipo = "avanco_duplo";
    else if (eh_captura(m))        info.tipo = "captura";
    else                           info.tipo = "normal";

    // 2) captura: no en passant a vítima fica ao lado do destino (uma fileira atrás do peão que captura)
    if (eh_captura(m)) {
        info.captura = true;
        int casaVitima = destino;
        if (eh_en_passant(m))
            casaVitima = (antes.estado.lado_a_jogar == Cor::BRANCO) ? destino - 8 : destino + 8;
        info.casaCapturada = nomeDaCasa(static_cast<Quadrado>(casaVitima));
        info.pecaCapturada = std::string(1, caractereDaPeca(peca_em(antes, static_cast<Quadrado>(casaVitima))));
    }

    // 3) promoção: a letra já está no UCI ("e7e8q")
    if (eh_promocao(m)) info.promocao = std::string(1, info.uci.back());

    return info;
}

Jogo::Jogo() : pos_(Posicao::inicial()) {}

bool Jogo::carregarFen(const std::string& fen) {
    try {
        pos_ = Posicao::deFen(fen);   // só troca pos_ se a FEN for válida
        ultimo_ = InfoLance{};        // posição nova: ainda não houve lance
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
    // O front-end espera o índice 0 = fileira 8 e o índice 7 = fileira 1.
    std::vector<std::string> linhas;
    for (int l = 7; l >= 0; --l) {
        std::string linha_str;
        for (int c = 0; c < 8; ++c) {
            const Quadrado sq = criar_quadrado(static_cast<Coluna>(c), static_cast<Linha>(l));
            if (vazia(pos_.b, sq)) {
                linha_str += '.';
            } else {
                linha_str += caractereDaPeca(peca_em(pos_.b, sq));
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
            ultimo_ = descreverLance(pos_.b, m);   // pos_ ainda é a posição de ANTES do lance
            pos_ = copia;
            return true;
        }
        return false;   // o UCI existe, mas o lance deixaria o rei em xeque
    }
    return false;
}

// ---------- fim de partida ----------
// As funções da infra-estrutura recebem Tabuleiro& (sem const) porque testam lances de
// verdade. Como estes métodos são const, trabalhamos numa cópia de pos_.

bool Jogo::emXeque() const {
    return em_xeque(pos_.b, pos_.b.estado.lado_a_jogar);   // só leitura, não precisa de cópia
}

bool Jogo::emXequeMate() const {
    Posicao copia = pos_;
    return em_xeque_mate(copia.b);
}

bool Jogo::emAfogamento() const {
    Posicao copia = pos_;
    return em_afogamento(copia.b);
}

// ---------- estado da partida e último lance ----------

SituacaoPartida Jogo::situacao() const {
    SituacaoPartida s;
    Posicao copia = pos_;                 // tem_lance_legal precisa de Tabuleiro& (faz e desfaz lances)
    Tabuleiro& b = copia.b;
    const Cor vez = b.estado.lado_a_jogar;

    s.emXeque = em_xeque(b, vez);
    const bool semLances = !tem_lance_legal(b);

    if (semLances && s.emXeque) {
        s.estado   = "xeque_mate";
        s.vencedor = (vez == Cor::BRANCO) ? "pretas" : "brancas";   // venceu quem NÃO tem a vez
    } else if (semLances) {
        s.estado = "afogamento";
    } else if (material_insuficiente(b)) {
        s.estado = "material_insuficiente";
    } else if (empate_50(b)) {
        s.estado = "regra_dos_50";
    } else if (contar_repeticoes(b) >= 2) {   // já apareceu 2 vezes antes = esta é a 3ª
        s.estado = "repeticao";
    }

    s.terminou = (s.estado != "em_andamento");
    return s;
}

InfoLance Jogo::ultimoLance() const {
    return ultimo_;
}

} // namespace xadrez