#include "xadrez/posicao.hpp"
#include "tabuleiro/tabuleiro.hpp"
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
#include "otimizacao/zobrist.hpp"
#include "../../../infra-estrutura/movimentos.hpp"   // casa_atacada

void init_raios();   // infra-estrutura/movimentos.cpp: tabelas de raios das peças deslizantes

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

// ============================================================================
// FEN (mantido na engine; o Tabuleiro da infra-estrutura não conhece FEN)
// ============================================================================
namespace {

// Letras de peça por [cor][tipo]: P N B R Q K / p n b r q k
const char LETRA_PECA[NUM_CORES][NUM_TIPOS] = {
    {'P', 'N', 'B', 'R', 'Q', 'K'},
    {'p', 'n', 'b', 'r', 'q', 'k'}
};

// Chave Zobrist calculada do zero, na mesma convenção de fazer_lance:
// peças + direitos de roque (sempre) + coluna do en passant (se houver)
// + ZOB_LADO quando a vez é das pretas.
uint64_t calcularChave(const Tabuleiro& b) {
    uint64_t chave = 0;
    for (int sq = 0; sq < NUM_CASAS; ++sq) {
        const Peca p = b.casas[sq];
        if (p.tpeca != TipoPeca::PECA_VAZIA)
            chave ^= ZOBRIST[zob_peca(p.cor, p.tpeca, static_cast<Quadrado>(sq))];
    }
    chave ^= ZOBRIST[ZOB_ROQUE + b.estado.direitos_roque];
    if (b.estado.en_passant != Quadrado::quadrado_vazio)
        chave ^= ZOBRIST[ZOB_EP + coluna_de(b.estado.en_passant)];
    if (b.estado.lado_a_jogar == Cor::PRETO)
        chave ^= ZOBRIST[ZOB_LADO];
    return chave;
}

bool soDigitos(const std::string& s, int& valor) {
    if (s.empty() || s.size() > 6) return false;
    valor = 0;
    for (char c : s) {
        if (c < '0' || c > '9') return false;
        valor = valor * 10 + (c - '0');
    }
    return true;
}

// Preenche `p` a partir da FEN (4 a 6 campos; meios-lances e número do lance são opcionais).
// Valida: 8 fileiras de 8 casas, peças conhecidas, um rei de cada cor, nenhum peão na
// 1ª/8ª fileira, lado, roque, en passant (fileira coerente com o lado) e números.
// Direitos de roque sem rei/torre na casa de origem são descartados.
// Em caso de erro devolve false e `p` fica em estado indefinido (deFen descarta).
bool lerFen(Posicao& p, const std::string& fen) {
    std::vector<std::string> campos;
    std::istringstream ss(fen);
    std::string tok;
    while (ss >> tok) campos.push_back(tok);
    if (campos.size() < 4 || campos.size() > 6) return false;

    Tabuleiro& t = p.b;
    limpar(t);

    // 1) Posição das peças (fileira 8 primeiro)
    int linha = 7, col = 0;
    for (char ch : campos[0]) {
        if (ch == '/') {
            if (col != 8 || linha == 0) return false;
            --linha;
            col = 0;
        } else if (ch >= '1' && ch <= '8') {
            col += ch - '0';
            if (col > 8) return false;
        } else {
            const bool branca = (ch >= 'A' && ch <= 'Z');
            const char minus = branca ? static_cast<char>(ch - 'A' + 'a') : ch;
            TipoPeca tp;
            switch (minus) {
                case 'p': tp = TipoPeca::PEAO;   break;
                case 'n': tp = TipoPeca::CAVALO; break;
                case 'b': tp = TipoPeca::BISPO;  break;
                case 'r': tp = TipoPeca::TORRE;  break;
                case 'q': tp = TipoPeca::DAMA;   break;
                case 'k': tp = TipoPeca::REI;    break;
                default: return false;
            }
            if (col >= 8) return false;
            if (tp == TipoPeca::PEAO && (linha == 0 || linha == 7)) return false;
            colocar_peca(t, branca ? Cor::BRANCO : Cor::PRETO, tp,
                         criar_quadrado(static_cast<Coluna>(col), static_cast<Linha>(linha)));
            ++col;
        }
    }
    if (linha != 0 || col != 8) return false;
    if (quantidade_de(t, Cor::BRANCO, TipoPeca::REI) != 1) return false;
    if (quantidade_de(t, Cor::PRETO,  TipoPeca::REI) != 1) return false;

    // 2) Lado a jogar
    if (campos[1] == "w") t.estado.lado_a_jogar = Cor::BRANCO;
    else if (campos[1] == "b") t.estado.lado_a_jogar = Cor::PRETO;
    else return false;
    const bool brancas = (t.estado.lado_a_jogar == Cor::BRANCO);

    // 3) Direitos de roque
    uint8_t d = SEM_ROQUE;
    if (campos[2] != "-") {
        for (char ch : campos[2]) {
            switch (ch) {
                case 'K': d |= ROQUE_BRANCO_REI;  break;
                case 'Q': d |= ROQUE_BRANCO_DAMA; break;
                case 'k': d |= ROQUE_PRETO_REI;   break;
                case 'q': d |= ROQUE_PRETO_DAMA;  break;
                default: return false;
            }
        }
    }
    auto tem = [&](Cor c, TipoPeca tp, Quadrado q) {
        const Peca x = peca_em(t, q);
        return x.cor == c && x.tpeca == tp;
    };
    if (!(tem(Cor::BRANCO, TipoPeca::REI, Quadrado::e1) && tem(Cor::BRANCO, TipoPeca::TORRE, Quadrado::h1)))
        d = static_cast<uint8_t>(d & ~ROQUE_BRANCO_REI);
    if (!(tem(Cor::BRANCO, TipoPeca::REI, Quadrado::e1) && tem(Cor::BRANCO, TipoPeca::TORRE, Quadrado::a1)))
        d = static_cast<uint8_t>(d & ~ROQUE_BRANCO_DAMA);
    if (!(tem(Cor::PRETO, TipoPeca::REI, Quadrado::e8) && tem(Cor::PRETO, TipoPeca::TORRE, Quadrado::h8)))
        d = static_cast<uint8_t>(d & ~ROQUE_PRETO_REI);
    if (!(tem(Cor::PRETO, TipoPeca::REI, Quadrado::e8) && tem(Cor::PRETO, TipoPeca::TORRE, Quadrado::a8)))
        d = static_cast<uint8_t>(d & ~ROQUE_PRETO_DAMA);
    t.estado.direitos_roque = d;

    // 4) En passant: fileira 6 se as brancas jogam, fileira 3 se as pretas jogam
    t.estado.en_passant = Quadrado::quadrado_vazio;
    if (campos[3] != "-") {
        const std::string& e = campos[3];
        if (e.size() != 2 || e[0] < 'a' || e[0] > 'h') return false;
        if (e[1] != (brancas ? '6' : '3')) return false;
        t.estado.en_passant = criar_quadrado(static_cast<Coluna>(e[0] - 'a'),
                                             static_cast<Linha>(e[1] - '1'));
    }
    
    // 4b) Posição impossível: o lado que NÃO tem a vez não pode estar em xeque.
    // Isso também rejeita reis adjacentes (um rei ataca o outro).
    {
        const Cor aJogar = t.estado.lado_a_jogar;
        if (casa_atacada(t, casa_do_rei(t, oposta(aJogar)), aJogar)) return false;
    }
    
    // 5) Meios-lances e número do lance (opcionais)
    int meios = 0, numero = 1;
    if (campos.size() >= 5 && !soDigitos(campos[4], meios)) return false;
    if (campos.size() >= 6 && (!soDigitos(campos[5], numero) || numero < 1)) return false;
    t.estado.contador_50 = meios;
    p.numeroLance = numero;

    t.estado.chave_zobrist = calcularChave(t);
    return true;
}

} // namespace

// ============================================================================
// Posicao
// ============================================================================

Posicao::Posicao() {
    iniciar_zobrist();
    static const bool raiosProntos = (init_raios(), true);   // roda uma única vez
    (void)raiosProntos;
    limpar(b);
}

Posicao Posicao::inicial() {
    Posicao p;
    posicao_inicial(p.b);
    p.b.estado.chave_zobrist = calcularChave(p.b);   // a infra deixa a chave em 0
    p.numeroLance = 1;
    return p;
}

Posicao Posicao::deFen(const std::string& fen) {
    Posicao p;
    if (!lerFen(p, fen)) {
        throw std::invalid_argument("FEN invalida ou incompleta");
    }
    return p;
}

std::string Posicao::paraFen() const {
    std::string s;

    for (int linha = 7; linha >= 0; --linha) {
        int vazias = 0;
        for (int col = 0; col < 8; ++col) {
            const Peca p = b.casas[linha * 8 + col];
            if (p.tpeca == TipoPeca::PECA_VAZIA) {
                ++vazias;
            } else {
                if (vazias) { s += static_cast<char>('0' + vazias); vazias = 0; }
                s += LETRA_PECA[idx(p.cor)][idx(p.tpeca)];
            }
        }
        if (vazias) s += static_cast<char>('0' + vazias);
        if (linha > 0) s += '/';
    }

    s += (b.estado.lado_a_jogar == Cor::BRANCO) ? " w " : " b ";

    const uint8_t d = b.estado.direitos_roque;
    if (d == SEM_ROQUE) {
        s += '-';
    } else {
        if (d & ROQUE_BRANCO_REI)  s += 'K';
        if (d & ROQUE_BRANCO_DAMA) s += 'Q';
        if (d & ROQUE_PRETO_REI)   s += 'k';
        if (d & ROQUE_PRETO_DAMA)  s += 'q';
    }

    s += ' ';
    if (b.estado.en_passant == Quadrado::quadrado_vazio) {
        s += '-';
    } else {
        s += casaParaTexto(static_cast<int>(linha_de(b.estado.en_passant)),
                           static_cast<int>(coluna_de(b.estado.en_passant)));
    }

    s += ' ' + std::to_string(b.estado.contador_50);
    s += ' ' + std::to_string(numeroLance);
    return s;
}

} // namespace xadrez