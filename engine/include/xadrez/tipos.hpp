#pragma once
#include <string>

namespace xadrez {

// O tabuleiro é uma matriz 8x8 de caracteres, na MESMA orientação do FEN e do frontend:
//   linha 0  = 8ª fileira (onde começam as pretas)
//   linha 7  = 1ª fileira (onde começam as brancas)
//   coluna 0 = coluna 'a', coluna 7 = coluna 'h'
//
// Cada casa guarda um caractere:
//   MAIÚSCULA = peça branca   (P N B R Q K)
//   minúscula = peça preta    (p n b r q k)
//   '.'       = casa vazia
constexpr char VAZIA = '.';

inline bool ehBranca(char peca) { return peca >= 'A' && peca <= 'Z'; }
inline bool ehPreta(char peca)  { return peca >= 'a' && peca <= 'z'; }
inline bool ehVazia(char peca)  { return peca == VAZIA; }

// A peça pertence ao lado indicado? (brancas == true -> lado das brancas)
inline bool pertenceAo(char peca, bool brancas) {
    return brancas ? ehBranca(peca) : ehPreta(peca);
}

// Converte para minúscula, para comparar o TIPO da peça sem olhar a cor.
// Exemplo: minuscula('N') == 'n' e minuscula('n') == 'n'
inline char minuscula(char peca) {
    return ehBranca(peca) ? static_cast<char>(peca - 'A' + 'a') : peca;
}

inline bool dentroDoTabuleiro(int linha, int coluna) {
    return linha >= 0 && linha < 8 && coluna >= 0 && coluna < 8;
}

// Conversão entre o texto da casa e a posição na matriz.
//- "a8" -> linha 0, coluna 0
//- "h1" -> linha 7, coluna 7
//- "e4" -> linha 4, coluna 4
std::string casaParaTexto(int linha, int coluna);

// Devolve false se o texto não for uma casa válida (ex.: "i9", "e", "e44").
bool textoParaCasa(const std::string& texto, int& linha, int& coluna);

}