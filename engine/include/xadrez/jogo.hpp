#pragma once
#include <string>
#include <vector>
#include "xadrez/posicao.hpp"

namespace xadrez {

// ---------------------------------------------------------------------------
// O que ACONTECEU no último lance. Serve para o frontend animar/narrar:
// mover a torre no roque, apagar o peão certo no en passant, tocar o som de captura...
// Todos os campos são texto/bool simples, para a ponte com o JavaScript ficar fina.
// ---------------------------------------------------------------------------
struct InfoLance {
    std::string uci;             // "e2e4"; vazio se ainda não houve lance
    // "normal" | "avanco_duplo" | "roque_rei" | "roque_dama" | "captura" | "en_passant"
    // | "promocao" | "captura_promocao"      (vazio se ainda não houve lance)
    std::string tipo;
    bool        captura = false;
    std::string pecaCapturada;   // mesmo caractere de tabuleiro(): 'p' (peão preto), 'N' (cavalo branco)...
    std::string casaCapturada;   // casa da peça removida. No en passant NÃO é a casa de destino!
    std::string promocao;        // "q" | "r" | "b" | "n", ou vazio
    std::string torreOrigem;     // só no roque: "h1", "a8"...
    std::string torreDestino;    // só no roque: "f1", "d8"...
};

// ---------------------------------------------------------------------------
// Como está a PARTIDA agora (depende da posição, não de um lance).
// ---------------------------------------------------------------------------
struct SituacaoPartida {
    // "em_andamento" | "xeque_mate" | "afogamento" | "material_insuficiente"
    // | "regra_dos_50" | "repeticao"
    std::string estado = "em_andamento";
    std::string vencedor;        // "brancas" | "pretas" no xeque-mate; vazio em empate ou em andamento
    bool        emXeque  = false; // o rei de quem tem a vez está em xeque (inclui o mate)
    bool        terminou = false; // true para qualquer estado diferente de "em_andamento"
};

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
    // Quando o filtro de legalidade existir, só esta função e tentarLance mudam por dentro;
    // o frontend continua igual.
    std::vector<std::string> lancesDe(const std::string& casa) const;

    // Tenta executar o lance (UCI: "e2e4", "e7e8q"). O lance só vale se estiver entre
    // os devolvidos por lancesDe(origem). Se valer, atualiza a posição (troca a vez,
    // remove peça capturada, promove, etc.) e devolve true. Senão, não muda nada e devolve false.
    bool tentarLance(const std::string& uci);

    // ---- Fim de partida (sempre sobre o lado que tem a vez) ----
    // Nenhuma altera a posição do Jogo.

    // O rei de quem tem a vez está em xeque? (útil para destacar o rei na tela)
    bool emXeque() const;

    // Xeque-mate: em xeque e sem nenhum lance legal. Quem VENCEU é o lado que NÃO tem a vez:
    // se emXequeMate() e vezDasBrancas() == true, as pretas venceram (e vice-versa).
    bool emXequeMate() const;

    // Afogamento: sem xeque, mas sem nenhum lance legal. A partida termina empatada.
    bool emAfogamento() const;

    // ---- Estado geral da partida e descrição do último lance ----

    // Visão completa e consistente (calculada de uma vez só). Prioridade quando mais de uma
    // condição vale: mate > afogamento > material insuficiente > 50 lances > repetição.
    // Importante: o Jogo NÃO bloqueia lances depois de uma partida "terminada" por empate;
    // quem trava o tabuleiro é o frontend, olhando situacao().terminou.
    SituacaoPartida situacao() const;

    // O que o último lance aceito por tentarLance fez. Fica vazio (uci == "") logo após
    // construir o Jogo ou carregar um FEN. Um lance recusado não altera este valor.
    InfoLance ultimoLance() const;

    std::string melhorLanceIA(int profundidade);

private:
    Posicao pos_;
    InfoLance ultimo_;   // descrição do último lance aceito
};

}  // namespace xadrez