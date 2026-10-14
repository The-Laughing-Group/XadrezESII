#include <cstdlib>
#include "xadrez/movimentos.hpp"

namespace xadrez {

// Quando algo SAI de uma casa ou CHEGA nela, os direitos de roque ligados a essa casa acabam:
//   rei saiu de e1/e8        -> perde os dois roques daquela cor
//   torre saiu de a1/h1/a8/h8 -> perde o roque daquele lado
//   torre foi capturada em a1/h1/a8/h8 -> idem (alguém chegou lá)
// Por isso a função é chamada para a casa de origem E para a de destino do lance.
static void atualizarDireitosDeRoque(Posicao& p, int linha, int coluna) {
    if (linha == 7 && coluna == 4) { p.rocaBrancasRei = false; p.rocaBrancasDama = false; }
    if (linha == 0 && coluna == 4) { p.rocaPretasRei = false;  p.rocaPretasDama = false; }
    if (linha == 7 && coluna == 7) p.rocaBrancasRei = false;
    if (linha == 7 && coluna == 0) p.rocaBrancasDama = false;
    if (linha == 0 && coluna == 7) p.rocaPretasRei = false;
    if (linha == 0 && coluna == 0) p.rocaPretasDama = false;
}

Posicao aplicarLance(const Posicao& pos, const Movimento& m) {
    Posicao nova = pos;   // trabalhamos numa cópia; a posição original fica intacta

    char peca = pos.tabuleiro[m.deLinha][m.deColuna];
    char destino = pos.tabuleiro[m.paraLinha][m.paraColuna];
    bool brancas = ehBranca(peca);
    char tipo = minuscula(peca);

    bool captura = !ehVazia(destino);
    // Peão que anda na diagonal para uma casa VAZIA só pode ser en passant
    bool enPassant = (tipo == 'p' && m.deColuna != m.paraColuna && ehVazia(destino));
    // Rei que anda duas colunas só pode ser roque
    bool roque = (tipo == 'k' && std::abs(m.paraColuna - m.deColuna) == 2);

    // 1) Move a peça
    nova.tabuleiro[m.paraLinha][m.paraColuna] = peca;
    nova.tabuleiro[m.deLinha][m.deColuna] = VAZIA;

    // 2) En passant: o peão capturado está na linha de ORIGEM, na coluna de DESTINO
    if (enPassant) {
        nova.tabuleiro[m.deLinha][m.paraColuna] = VAZIA;
        captura = true;
    }

    // 3) Roque: move também a torre
    if (roque) {
        if (m.paraColuna > m.deColuna) {          // roque curto: torre h -> f
            nova.tabuleiro[m.deLinha][5] = nova.tabuleiro[m.deLinha][7];
            nova.tabuleiro[m.deLinha][7] = VAZIA;
        } else {                                  // roque longo: torre a -> d
            nova.tabuleiro[m.deLinha][3] = nova.tabuleiro[m.deLinha][0];
            nova.tabuleiro[m.deLinha][0] = VAZIA;
        }
    }

    // 4) Promoção: o peão vira a peça escolhida (movBasicos gera 'q','r','b','n' em minúscula)
    if (m.promocao != 0) {
        nova.tabuleiro[m.paraLinha][m.paraColuna] =
            brancas ? static_cast<char>(m.promocao - 'a' + 'A') : m.promocao;
    }

    // 5) Direitos de roque
    atualizarDireitosDeRoque(nova, m.deLinha, m.deColuna);
    atualizarDireitosDeRoque(nova, m.paraLinha, m.paraColuna);

    // 6) Casa de en passant: só existe logo depois de um avanço duplo de peão
    nova.passantLinha = -1;
    nova.passantColuna = -1;
    if (tipo == 'p' && std::abs(m.paraLinha - m.deLinha) == 2) {
        nova.passantLinha = (m.deLinha + m.paraLinha) / 2;   // a casa "pulada"
        nova.passantColuna = m.deColuna;
    }

    // 7) Regra dos 50 lances: zera com lance de peão ou captura
    if (tipo == 'p' || captura) nova.meioLances = 0;
    else nova.meioLances = pos.meioLances + 1;

    // 8) Número do lance sobe depois que as pretas jogam; troca a vez
    if (!brancas) nova.numeroLance = pos.numeroLance + 1;
    nova.vezDasBrancas = !pos.vezDasBrancas;

    return nova;
}

}
