#include "xadrez/movimentos.hpp"

namespace xadrez{

static void adicionar(std::vector<Movimento>& v, int deL, int deC, int paraL, int paraC, char promocao = 0) {
    Movimento m;
    m.deLinha = deL;
    m.deColuna = deC;
    m.paraLinha = paraL;
    m.paraColuna = paraC;
    m.promocao = promocao;
    v.push_back(m);
}

static bool podePousar(const Posicao& pos, int linha, int coluna, bool brancas) {
    char casa_destino = pos.tabuleiro[linha][coluna];
    return ehVazia(casa_destino) || !pertenceAo(casa_destino, brancas);
}


//movimento de peças: cavalo e rei
template <std::size_t N>
static void movimentosDePulo(
        const Posicao& pos, int linha, int coluna, const int (&passos)[N][2],
        std::vector<Movimento>& v
    ){
    for (const auto& passo : passos) {
        int lin = linha + passo[0];
        int col = coluna + passo[1];
        if (dentroDoTabuleiro(lin, col) && podePousar(pos, lin, col, ehBranca(pos.tabuleiro[linha][coluna])))
            adicionar(v, linha, coluna, lin, col);
    }
}

//movimento de peças:
// - Retas - rainha e torre
// - Diagonais - rainha e bispo
//
//anda na direção até encontrar uma peça ou a borda
template <std::size_t N>
static void movimentosContinuos(
        const Posicao& pos, int linha, int coluna, const int (&direcoes)[N][2],
        std::vector<Movimento>& v
    ){
    for (const auto& dir : direcoes) {
        int lin = linha + dir[0];
        int col = coluna + dir[1];
        while (dentroDoTabuleiro(lin, col)) {
            char casa_destino = pos.tabuleiro[lin][col];
            if (ehVazia(casa_destino)) {
                adicionar(v, linha, coluna, lin, col);
            } else {
                if (!pertenceAo(casa_destino, ehBranca(pos.tabuleiro[linha][coluna]))) //peça na casa_destino é de cor diferente da que estou movendo agora?
                    adicionar(v, linha, coluna, lin, col);   // peça inimiga - come e para
                break;                                   // sua peça - para
            }
            lin += dir[0];
            col += dir[1];
        }
    }
}


// Se o peão chega na última fileira, o lance se desdobra em quatro (uma por peça de promoção).
static void adicionarMovPeao(
        std::vector<Movimento>& v, int deL, int deC,
        int paraL, int paraC, int linhaFinal
    ){
    if (paraL == linhaFinal) {
        for (char prom : {'q','r','b','n'})
            adicionar(v, deL, deC, paraL, paraC, prom);
    } else {
        adicionar(v, deL, deC, paraL, paraC); //prom default é '0'
    }
}
 
static void movimentosDePeao(const Posicao& pos, int linha, int coluna, std::vector<Movimento>& v) {
    char peca = pos.tabuleiro[linha][coluna];
    bool branca = ehBranca(peca);
    int direcao = branca? -1 : 1;
    int linhaFinal = branca? 0 : 7;
    int linhaInicial = branca? 6: 1;

    int frente = linha + direcao;
    if (dentroDoTabuleiro(frente, coluna) && ehVazia(pos.tabuleiro[frente][coluna])) {
        adicionarMovPeao(v, linha, coluna, frente, coluna, linhaFinal);

        int duasFrente = linha + 2 * direcao;
        if (linha == linhaInicial && ehVazia(pos.tabuleiro[duasFrente][coluna])) //nem a primeira nem a segunda casa a frente está preenchida
            adicionar(v, linha, coluna, duasFrente, coluna);
    }
 
    // Capturas nas duas diagonais
    for (int diag_c : {-1, 1}) {
        int col = coluna + diag_c;
        if (!dentroDoTabuleiro(frente, col)) continue;
 
        char casa_destino = pos.tabuleiro[frente][col];
        if (!ehVazia(casa_destino)) {
            if (!pertenceAo(casa_destino, branca))
                adicionarMovPeao(v, linha, coluna, frente, col, linhaFinal);
        } else if (pos.vezDasBrancas == branca &&
                   pos.passantLinha == frente && pos.passantColuna == col) { //en passant
            adicionar(v, linha, coluna, frente, col);
        }
    }
}


std::vector<Movimento> movBasicos(const Posicao& pos, int linha, int coluna){
    std::vector<Movimento> v;
    //Movimento mov;

    if(!dentroDoTabuleiro(linha,coluna)) return v;
    char peca = pos.tabuleiro[linha][coluna];
    // int direcao = ehBranca(peca)? -1 : 1;

    if(ehVazia(peca)) return v;

    //vars usadas em verificacoes de cada peça
    static const int cav_passos[8][2] = {{2,1},{2,-1},{1,2},{1,-2},{-2,1},{-2,-1},{-1,2},{-1,-2}};
    static const int rei_passos[8][2] = {{1,0},{1,1},{0,1},{0,-1},{-1,-1},{-1,0},{-1,1},{1,-1}};
    
    //int p_desloc_lin; int p_desloc_col;

    static const int diagonal_dir[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    static const int retas_dir[4][2] = {{1,0},{0,1},{-1,0},{0,-1}}; //vertical e horizontal

    
    switch(minuscula(peca)){
        case 'p': //peao
            movimentosDePeao(pos, linha, coluna, v);
            break;
        case 'n': //cavalo
            movimentosDePulo(pos, linha, coluna, cav_passos, v);
            break;
        case 'k': //rei
            movimentosDePulo(pos, linha, coluna, rei_passos, v);
            break;
        case 'q': //rainha
            movimentosContinuos(pos, linha, coluna, retas_dir, v);
            movimentosContinuos(pos, linha, coluna, diagonal_dir, v);
            break;
        case 'r': //torre
            movimentosContinuos(pos, linha, coluna, retas_dir, v);
            break;
        case 'b': //bispo
            movimentosContinuos(pos, linha, coluna, diagonal_dir, v);
            break;
        default:
            break;
    }

    return v;
}

}