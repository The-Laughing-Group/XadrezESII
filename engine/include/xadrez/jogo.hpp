#pragma once
#include <string>
#include <vector>
#include "xadrez/posicao.hpp"

namespace xadrez {

// Camada de alto nível: é a única coisa que o frontend enxerga.
// Só trabalha com texto simples (FEN, "e2", "e2e4"), para a ponte com o JavaScript ficar fina.
class Jogo {
public:
    Jogo();                                   // começa na posição inicial

    // Carrega uma posição a partir de um FEN. Devolve false (e mantém a posição atual)
    // se o FEN for inválido. Não lança exceção, para não quebrar o WebAssembly.
    bool carregarFen(const std::string& fen);

    std::string fen() const;
    bool vezDasBrancas() const;

    // Oito textos de oito caracteres, da 8ª fileira para a 1ª. '.' = casa vazia.
    // Igual à matriz do tabuleiro: tabuleiro()[linha][coluna].
    std::vector<std::string> tabuleiro() const;

    // Lances da peça em `casa` (ex.: "e2"), em notação UCI ("e2e4", "e7e8q").
    // Devolve lista vazia se a casa for inválida, estiver vazia ou tiver peça de quem não joga.
    //
    // ATENÇÃO: por enquanto são lances pseudo-legais (não descarta lance que deixa o rei em xeque).
    // Quando o filtro de legalidade existir, só esta função muda; o frontend continua igual.
    std::vector<std::string> lancesDe(const std::string& casa) const;

    // Próxima etapa (depende de aplicarLance e do filtro de legalidade):
    // bool tentarLance(const std::string& uci);

private:
    Posicao pos_;
};

}  // namespace xadrez
