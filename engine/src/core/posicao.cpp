#include "xadrez/posicao.hpp"
#include <sstream>
#include <stdexcept>

namespace xadrez {

// ---------- casas ----------

std::string casaParaTexto(int linha, int coluna) {
    // coluna 0..7 vira 'a'..'h'; linha 0 é a 8ª fileira, então fileira = 8 - linha
    std::string texto;
    texto += static_cast<char>('a' + coluna);
    texto += static_cast<char>('0' + (8 - linha));
    return texto;
}

bool textoParaCasa(const std::string& texto, int& linha, int& coluna) {
    if (texto.size() != 2) return false;
    if (texto[0] < 'a' || texto[0] > 'h') return false;
    if (texto[1] < '1' || texto[1] > '8') return false;
    coluna = texto[0] - 'a';
    linha = 8 - (texto[1] - '0');
    return true;
}

// ---------- Posicao ----------

Posicao::Posicao() {
    for (int l = 0; l < 8; ++l)
        for (int c = 0; c < 8; ++c)
            tabuleiro[l][c] = VAZIA;
}

Posicao Posicao::inicial() {
    return deFen(FEN_INICIAL);
}

static bool pecaValida(char c) {
    const std::string validas = "pnbrqkPNBRQK";
    return validas.find(c) != std::string::npos;
}

Posicao Posicao::deFen(const std::string& fen) {
    // Um FEN tem 6 campos separados por espaço:
    //   peças  lado  roques  en-passant  meio-lances  número-do-lance
    std::istringstream entrada(fen);
    std::string pecas, lado, roques, passant;
    if (!(entrada >> pecas >> lado >> roques >> passant))
        throw std::invalid_argument("FEN: campos faltando");

    int meios = 0, numero = 1;
    entrada >> meios >> numero;   // opcionais: se faltarem, ficam com o valor padrão

    Posicao p;

    // 1) Peças. O FEN lista da 8ª fileira para a 1ª, da coluna 'a' para 'h',
    //    que é exatamente a ordem da nossa matriz: linha 0 primeiro.
    int linha = 0, coluna = 0;
    int reisBrancos = 0, reisPretos = 0;
    for (char c : pecas) {
        if (c == '/') {
            if (coluna != 8) throw std::invalid_argument("FEN: fileira não tem 8 casas");
            ++linha;
            coluna = 0;
            if (linha > 7) throw std::invalid_argument("FEN: fileiras demais");
        } else if (c >= '1' && c <= '8') {
            coluna += c - '0';                 // dígito = quantidade de casas vazias
            if (coluna > 8) throw std::invalid_argument("FEN: fileira passou de 8 casas");
        } else {
            if (coluna >= 8) throw std::invalid_argument("FEN: fileira passou de 8 casas");
            if (!pecaValida(c)) throw std::invalid_argument(std::string("FEN: peça inválida '") + c + "'");
            p.tabuleiro[linha][coluna] = c;
            if (c == 'K') ++reisBrancos;
            if (c == 'k') ++reisPretos;
            ++coluna;
        }
    }
    if (linha != 7 || coluna != 8) throw std::invalid_argument("FEN: tabuleiro incompleto");
    if (reisBrancos != 1 || reisPretos != 1)
        throw std::invalid_argument("FEN: cada lado precisa de exatamente um rei");

    // 2) De quem é a vez
    if (lado == "w") p.vezDasBrancas = true;
    else if (lado == "b") p.vezDasBrancas = false;
    else throw std::invalid_argument("FEN: o lado deve ser 'w' ou 'b'");

    // 3) Direitos de roque ("-" significa nenhum)
    if (roques != "-") {
        for (char c : roques) {
            if (c == 'K') p.rocaBrancasRei = true;
            else if (c == 'Q') p.rocaBrancasDama = true;
            else if (c == 'k') p.rocaPretasRei = true;
            else if (c == 'q') p.rocaPretasDama = true;
            else throw std::invalid_argument("FEN: direitos de roque inválidos");
        }
    }

    // 4) Casa de en passant ("-" significa nenhuma)
    if (passant != "-") {
        if (!textoParaCasa(passant, p.passantLinha, p.passantColuna))
            throw std::invalid_argument("FEN: casa de en passant inválida");
    }

    p.meioLances = meios;
    p.numeroLance = numero;
    return p;
}

std::string Posicao::paraFen() const {
    std::ostringstream saida;

    // Peças: cada fileira conta as casas vazias seguidas e escreve o número quando a sequência acaba
    for (int l = 0; l < 8; ++l) {
        int vazias = 0;
        for (int c = 0; c < 8; ++c) {
            if (ehVazia(tabuleiro[l][c])) {
                ++vazias;
            } else {
                if (vazias > 0) {
                    saida << vazias;
                    vazias = 0;
                }
                saida << tabuleiro[l][c];
            }
        }
        if (vazias > 0) saida << vazias;
        if (l < 7) saida << '/';
    }

    saida << ' ' << (vezDasBrancas ? 'w' : 'b') << ' ';

    if (!rocaBrancasRei && !rocaBrancasDama && !rocaPretasRei && !rocaPretasDama) {
        saida << '-';
    } else {
        if (rocaBrancasRei)  saida << 'K';
        if (rocaBrancasDama) saida << 'Q';
        if (rocaPretasRei)   saida << 'k';
        if (rocaPretasDama)  saida << 'q';
    }

    saida << ' ';
    if (passantLinha < 0) saida << '-';
    else saida << casaParaTexto(passantLinha, passantColuna);

    saida << ' ' << meioLances << ' ' << numeroLance;
    return saida.str();
}

}
