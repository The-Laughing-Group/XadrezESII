#pragma once
#include "tipos.hpp"

// ============================================================================
// Lance: 16 bits
//
//   bits 15..12   11..6      5..0
//   [ flags (4) | destino (6) | origem (6) ]
//
// Campo de flags:
//   bit 2 (valor 4) ligado = captura
//   bit 3 (valor 8) ligado = promoção
//   Em promoções, os 2 bits baixos dizem a peça: 0 cavalo, 1 bispo, 2 torre, 3 dama.
//
// CONVENÇÕES (importantes para quem escreve a geração de lances):
//   - Roque: origem e destino são os do REI (e1->g1, e1->c1, e8->g8, e8->c8).
//     A torre é movida por causa da flag LANCE_ROQUE_REI / LANCE_ROQUE_DAMA.
//   - En passant: eh_captura(m) é true, mas a peça capturada NÃO está no destino:
//     está em destino - 8 (brancas capturando) ou destino + 8 (pretas capturando).
//   - Promoção: o tipo da peça vem de tipo_da_promocao(m).
//   - Para promoção com captura, use os valores LANCE_CAPTURA_PROMO_*.
// ============================================================================

using Lance = uint16_t;

// "Nenhum lance": origem == destino == a1 e flag normal. Nunca é um lance real.
// (Não confundir com "lance nulo" de poda, que é passar a vez.)
constexpr Lance SEM_LANCE = 0;

enum TipoLance : uint16_t {
    LANCE_NORMAL               = 0,
    LANCE_AVANCO_DUPLO         = 1,
    LANCE_ROQUE_REI            = 2,
    LANCE_ROQUE_DAMA           = 3,
    LANCE_CAPTURA              = 4,
    LANCE_EN_PASSANT           = 5,   // também é captura (bit 2 ligado)
    LANCE_PROMO_CAVALO         = 8,
    LANCE_PROMO_BISPO          = 9,
    LANCE_PROMO_TORRE          = 10,
    LANCE_PROMO_DAMA           = 11,
    LANCE_CAPTURA_PROMO_CAVALO = 12,  // LANCE_PROMO_CAVALO | LANCE_CAPTURA
    LANCE_CAPTURA_PROMO_BISPO  = 13,
    LANCE_CAPTURA_PROMO_TORRE  = 14,
    LANCE_CAPTURA_PROMO_DAMA   = 15
};

static_assert((LANCE_PROMO_CAVALO | LANCE_CAPTURA) == LANCE_CAPTURA_PROMO_CAVALO);
static_assert((LANCE_PROMO_BISPO  | LANCE_CAPTURA) == LANCE_CAPTURA_PROMO_BISPO);
static_assert((LANCE_PROMO_TORRE  | LANCE_CAPTURA) == LANCE_CAPTURA_PROMO_TORRE);
static_assert((LANCE_PROMO_DAMA   | LANCE_CAPTURA) == LANCE_CAPTURA_PROMO_DAMA);

// Monta o lance encaixando cada campo na sua posição.
constexpr Lance criar_lance(Quadrado origem, Quadrado destino, TipoLance flags) {
    return static_cast<Lance>(origem | (destino << 6) | (flags << 12));
}

constexpr Quadrado origem_do_lance(Lance m) {
    return static_cast<Quadrado>(m & 0x3F);
}

constexpr Quadrado destino_do_lance(Lance m) {
    return static_cast<Quadrado>((m >> 6) & 0x3F);
}

constexpr int flags_do_lance(Lance m) {
    return (m >> 12) & 0x0F;
}

// Inclui o en passant (a peça capturada fica em destino -/+ 8, não no destino).
constexpr bool eh_captura(Lance m) {
    return ((m >> 14) & 1) != 0;
}

constexpr bool eh_promocao(Lance m) {
    return ((m >> 15) & 1) != 0;
}

constexpr bool eh_en_passant(Lance m) {
    return flags_do_lance(m) == LANCE_EN_PASSANT;
}

constexpr bool eh_roque(Lance m) {
    int f = flags_do_lance(m);
    return f == LANCE_ROQUE_REI || f == LANCE_ROQUE_DAMA;
}

constexpr bool eh_avanco_duplo(Lance m) {
    return flags_do_lance(m) == LANCE_AVANCO_DUPLO;
}

// Só faz sentido se eh_promocao(m). Devolve CAVALO, BISPO, TORRE ou DAMA.
constexpr TipoPeca tipo_da_promocao(Lance m) {
    return static_cast<TipoPeca>(idx(TipoPeca::CAVALO) + (flags_do_lance(m) & 3));
}

// ---------------------------------------------------------------------------
// Fazer e desfazer lances
// ---------------------------------------------------------------------------

// Aplica o lance: empilha o Estado, move as peças (tratando captura, en passant,
// roque e promoção) e atualiza o Estado (direitos, en_passant, contador_50,
// lado_a_jogar, capturada) e a chave Zobrist de forma incremental.
//
// Retorna true se o lance foi aplicado e é legal.
// Retorna false se deixaria o próprio rei em xeque; nesse caso o tabuleiro já volta
// ao estado anterior (NÃO chamar desfazer_lance).
// TODO: validar com assert(conta_lance < TAMANHO_HISTORICO).
bool fazer_lance(Tabuleiro& b, Lance m);

// Inverso exato de fazer_lance: devolve as peças e restaura o Estado da pilha.
// Só chamar após um fazer_lance que retornou true, com o mesmo lance.
void desfazer_lance(Tabuleiro& b, Lance m);










//Candidatos pra maquina de regras



// A posição atual já apareceu antes na pilha de estados?
bool repetida(const Tabuleiro& b);

// contador_50 >= LIMITE_REGRA_50 (100 meios-lances)
bool empate_50(const Tabuleiro& b); 