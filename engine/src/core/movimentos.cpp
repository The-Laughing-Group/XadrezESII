#include "../../infra-estrutura/movimentos.hpp"
#include "../../infra-estrutura/bitboard/bitboard.hpp"
#include "../../infra-estrutura/tabuleiro/tabuleiro.hpp"
#include "xadrez/movimentos.hpp"
#include "tabuleiro/tabuleiro.hpp"
#include "utils/constantes.hpp"
#include "utils/tipos.hpp"
#include "bitboard/bitboard.hpp"
#include "lances.hpp"

// Funções implementadas pelo Henrique
extern Bitboard ataques_torre(Quadrado sq, Bitboard ocupacao);
extern Bitboard ataques_bispo(Quadrado sq, Bitboard ocupacao);
extern Bitboard ataques_dama(Quadrado sq, Bitboard ocupacao);

// ============================================================================
// Pré-cálculos Estáticos (Gerados na Compilação) para Peças de Pulo
// ============================================================================
struct PrecalcAtaques {
    Bitboard cavalo[NUM_CASAS];
    Bitboard rei[NUM_CASAS];

    constexpr PrecalcAtaques() : cavalo{}, rei{} {
        for (int sq = 0; sq < NUM_CASAS; ++sq) {
            Bitboard bb = 1ULL << sq;

            // Ataques do Cavalo
            Bitboard c = 0;
            c |= (bb << 17) & ~MASCARA_COLUNAS[COLUNA_A];
            c |= (bb << 15) & ~MASCARA_COLUNAS[COLUNA_H];
            c |= (bb << 10) & ~(MASCARA_COLUNAS[COLUNA_A] | MASCARA_COLUNAS[COLUNA_B]);
            c |= (bb << 6)  & ~(MASCARA_COLUNAS[COLUNA_G] | MASCARA_COLUNAS[COLUNA_H]);
            c |= (bb >> 17) & ~MASCARA_COLUNAS[COLUNA_H];
            c |= (bb >> 15) & ~MASCARA_COLUNAS[COLUNA_A];
            c |= (bb >> 10) & ~(MASCARA_COLUNAS[COLUNA_G] | MASCARA_COLUNAS[COLUNA_H]);
            c |= (bb >> 6)  & ~(MASCARA_COLUNAS[COLUNA_A] | MASCARA_COLUNAS[COLUNA_B]);
            cavalo[sq] = c;

            // Ataques do Rei
            Bitboard r = 0;
            Bitboard direita = (bb << 1) & ~MASCARA_COLUNAS[COLUNA_A];
            Bitboard esquerda = (bb >> 1) & ~MASCARA_COLUNAS[COLUNA_H];
            r |= direita | esquerda;
            r |= (bb << 8) | (direita << 8) | (esquerda << 8);
            r |= (bb >> 8) | (direita >> 8) | (esquerda >> 8);
            rei[sq] = r;
        }
    }

};

constexpr PrecalcAtaques ATAQUES = PrecalcAtaques();

// A casa é atacada por alguma peça da cor `atacante`?
bool casa_atacada(const Tabuleiro& b, Quadrado casa, Cor atacante) {
    if (casa >= NUM_CASAS) return false;           // quadrado_vazio

    const int a = idx(atacante);
    const Bitboard bb = BitboardUtils::mascara_casa(casa);
    const Bitboard ocupacao = b.ocupacoes_totais;

    // Peões: de onde um peão atacante alcançaria a casa.
    Bitboard origens_peao;
    if (atacante == Cor::BRANCO)
        origens_peao = ((bb >> 7) & ~MASCARA_COLUNAS[COLUNA_A]) | ((bb >> 9) & ~MASCARA_COLUNAS[COLUNA_H]);
    else
        origens_peao = ((bb << 7) & ~MASCARA_COLUNAS[COLUNA_H]) | ((bb << 9) & ~MASCARA_COLUNAS[COLUNA_A]);
    if (origens_peao & b.pecas[a][idx(TipoPeca::PEAO)]) return true;

    // Cavalo e rei (ataques simétricos).
    if (ATAQUES.cavalo[casa] & b.pecas[a][idx(TipoPeca::CAVALO)]) return true;
    if (ATAQUES.rei[casa]    & b.pecas[a][idx(TipoPeca::REI)])    return true;

    // Peças deslizantes.
    const Bitboard diagonais = b.pecas[a][idx(TipoPeca::BISPO)] | b.pecas[a][idx(TipoPeca::DAMA)];
    const Bitboard retas     = b.pecas[a][idx(TipoPeca::TORRE)] | b.pecas[a][idx(TipoPeca::DAMA)];
    if (ataques_bispo(casa, ocupacao) & diagonais) return true;
    if (ataques_torre(casa, ocupacao) & retas)     return true;

    return false;
}

bool em_xeque(const Tabuleiro& b, Cor cor) {
    return casa_atacada(b, casa_do_rei(b, cor), oposta(cor));
}

// ============================================================================
// Helpers para popular a ListaLances da Equipe
// ============================================================================

static void extrair_movimentos(ListaLances& lista, Quadrado origem, Bitboard destinos, Bitboard inimigas) {
    while (destinos) {
        Quadrado dest = BitboardUtils::remover_lsb(destinos);
        TipoLance flag = (BitboardUtils::mascara_casa(dest) & inimigas) ? LANCE_CAPTURA : LANCE_NORMAL;
        lista.adicionar(criar_lance(origem, dest, flag));
    }
}

static void extrair_movimentos_peao(ListaLances& lista, Quadrado origem, Bitboard destinos, Bitboard mascara_promocao, Bitboard inimigas, Quadrado en_passant_sq) {
    while (destinos) {
        Quadrado dest = BitboardUtils::remover_lsb(destinos);
        bool captura = (BitboardUtils::mascara_casa(dest) & inimigas) != 0;
        bool ep = (dest == en_passant_sq && en_passant_sq != quadrado_vazio);

        if ((BitboardUtils::mascara_casa(dest) & mascara_promocao) != 0) {
            // Promoção
            if (captura) {
                lista.adicionar(criar_lance(origem, dest, LANCE_CAPTURA_PROMO_DAMA));
                lista.adicionar(criar_lance(origem, dest, LANCE_CAPTURA_PROMO_TORRE));
                lista.adicionar(criar_lance(origem, dest, LANCE_CAPTURA_PROMO_BISPO));
                lista.adicionar(criar_lance(origem, dest, LANCE_CAPTURA_PROMO_CAVALO));
            } else {
                lista.adicionar(criar_lance(origem, dest, LANCE_PROMO_DAMA));
                lista.adicionar(criar_lance(origem, dest, LANCE_PROMO_TORRE));
                lista.adicionar(criar_lance(origem, dest, LANCE_PROMO_BISPO));
                lista.adicionar(criar_lance(origem, dest, LANCE_PROMO_CAVALO));
            }
        } else {
            // Normal ou Captura comum
            TipoLance flag = LANCE_NORMAL;
            if (ep) flag = LANCE_EN_PASSANT;
            else if (captura) flag = LANCE_CAPTURA;

            lista.adicionar(criar_lance(origem, dest, flag));
        }
    }
}

// ============================================================================
// Função Principal Exportada
// ============================================================================
// No escopo global, o compilador reconhece o Tabuleiro e a ListaLances
void gerar_lances(const Tabuleiro& b, ListaLances& lista) {
    Cor cor = b.estado.lado_a_jogar;
    bool branca = (cor == Cor::BRANCO);
    Bitboard amigaveis = b.ocupacoes[idx(cor)];
    Bitboard inimigas = b.ocupacoes[idx(oposta(cor))];
    Bitboard ocupacoes_totais = b.ocupacoes_totais;

    Bitboard peças_ativas = amigaveis;
    while (peças_ativas) {
        Quadrado sq = BitboardUtils::remover_lsb(peças_ativas);
        Peca p = peca_em(b, sq);

        switch (p.tpeca) {
            case TipoPeca::PEAO: {
                Bitboard bb = BitboardUtils::mascara_casa(sq);
                Bitboard destinos_avanco = 0;
                Bitboard destinos_captura = 0;
                Bitboard prom_rank = branca ? MASCARA_LINHAS[LINHA_8] : MASCARA_LINHAS[LINHA_1];

                if (branca) {
                    Bitboard frente = (bb << 8) & ~ocupacoes_totais;
                    destinos_avanco |= frente;
                    if (frente && (bb & MASCARA_LINHAS[LINHA_2])) {
                        Bitboard avanço_duplo = (frente << 8) & ~ocupacoes_totais;
                        if (avanço_duplo) {
                            lista.adicionar(criar_lance(sq, BitboardUtils::indice_lsb(avanço_duplo), LANCE_AVANCO_DUPLO));
                        }
                    }
                    destinos_captura |= (((bb << 7) & ~MASCARA_COLUNAS[COLUNA_H]) | ((bb << 9) & ~MASCARA_COLUNAS[COLUNA_A])) & inimigas;
                    if (b.estado.en_passant != quadrado_vazio) {
                        destinos_captura |= (((bb << 7) & ~MASCARA_COLUNAS[COLUNA_H]) | ((bb << 9) & ~MASCARA_COLUNAS[COLUNA_A])) & BitboardUtils::mascara_casa(b.estado.en_passant);
                    }
                } else {
                    Bitboard frente = (bb >> 8) & ~ocupacoes_totais;
                    destinos_avanco |= frente;
                    if (frente && (bb & MASCARA_LINHAS[LINHA_7])) {
                        Bitboard avanço_duplo = (frente >> 8) & ~ocupacoes_totais;
                        if (avanço_duplo) {
                            lista.adicionar(criar_lance(sq, BitboardUtils::indice_lsb(avanço_duplo), LANCE_AVANCO_DUPLO));
                        }
                    }
                    destinos_captura |= (((bb >> 9) & ~MASCARA_COLUNAS[COLUNA_H]) | ((bb >> 7) & ~MASCARA_COLUNAS[COLUNA_A])) & inimigas;
                    if (b.estado.en_passant != quadrado_vazio) {
                        destinos_captura |= (((bb >> 9) & ~MASCARA_COLUNAS[COLUNA_H]) | ((bb >> 7) & ~MASCARA_COLUNAS[COLUNA_A])) & BitboardUtils::mascara_casa(b.estado.en_passant);
                    }
                }
                extrair_movimentos_peao(lista, sq, destinos_avanco | destinos_captura, prom_rank, inimigas, b.estado.en_passant);
                break;
            }

            case TipoPeca::CAVALO: {
                extrair_movimentos(lista, sq, ATAQUES.cavalo[sq] & ~amigaveis, inimigas);
                break;
            }

            case TipoPeca::REI: {
                extrair_movimentos(lista, sq, ATAQUES.rei[sq] & ~amigaveis, inimigas);

                const Cor atacante = oposta(cor);
                auto atacada = [&](Quadrado q) { return casa_atacada(b, q, atacante); };
                auto livre   = [&](Quadrado q) { return !BitboardUtils::testar_bit(ocupacoes_totais, q); };
                auto torre_em = [&](Quadrado q) {
                    Peca t = peca_em(b, q);
                    return t.tpeca == TipoPeca::TORRE && t.cor == cor;
                };

                if (branca && sq == Quadrado::e1) {
                    if ((b.estado.direitos_roque & ROQUE_BRANCO_REI) && torre_em(Quadrado::h1) &&
                        livre(Quadrado::f1) && livre(Quadrado::g1) &&
                        !atacada(Quadrado::e1) && !atacada(Quadrado::f1) && !atacada(Quadrado::g1))
                        lista.adicionar(criar_lance(sq, Quadrado::g1, LANCE_ROQUE_REI));

                    if ((b.estado.direitos_roque & ROQUE_BRANCO_DAMA) && torre_em(Quadrado::a1) &&
                        livre(Quadrado::d1) && livre(Quadrado::c1) && livre(Quadrado::b1) &&
                        !atacada(Quadrado::e1) && !atacada(Quadrado::d1) && !atacada(Quadrado::c1))
                        lista.adicionar(criar_lance(sq, Quadrado::c1, LANCE_ROQUE_DAMA));
                } else if (!branca && sq == Quadrado::e8) {
                    if ((b.estado.direitos_roque & ROQUE_PRETO_REI) && torre_em(Quadrado::h8) &&
                        livre(Quadrado::f8) && livre(Quadrado::g8) &&
                        !atacada(Quadrado::e8) && !atacada(Quadrado::f8) && !atacada(Quadrado::g8))
                        lista.adicionar(criar_lance(sq, Quadrado::g8, LANCE_ROQUE_REI));

                    if ((b.estado.direitos_roque & ROQUE_PRETO_DAMA) && torre_em(Quadrado::a8) &&
                        livre(Quadrado::d8) && livre(Quadrado::c8) && livre(Quadrado::b8) &&
                        !atacada(Quadrado::e8) && !atacada(Quadrado::d8) && !atacada(Quadrado::c8))
                        lista.adicionar(criar_lance(sq, Quadrado::c8, LANCE_ROQUE_DAMA));
                }
                break;
            }

            case TipoPeca::BISPO: {
                extrair_movimentos(lista, sq, ataques_bispo(sq, ocupacoes_totais) & ~amigaveis, inimigas);
                break;
            }

            case TipoPeca::TORRE: {
                extrair_movimentos(lista, sq, ataques_torre(sq, ocupacoes_totais) & ~amigaveis, inimigas);
                break;
            }

            case TipoPeca::DAMA: {
                extrair_movimentos(lista, sq, ataques_dama(sq, ocupacoes_totais) & ~amigaveis, inimigas);
                break;
            }

            default: break;
        }
    }
}