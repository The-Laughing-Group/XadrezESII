#include <gtest/gtest.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>
#include "include/xadrez/jogo.hpp"

#pragma once

using Jogo = xadrez::Jogo;

using Lista = std::vector<std::string>;

inline Lista ordenada(Lista v) {
    std::sort(v.begin(), v.end());
    return v;
}

inline Lista lancesDe(const std::string& fen, const std::string& casa) {
    Jogo j;
    if (!j.carregarFen(fen)) {
        ADD_FAILURE() << "FEN rejeitado pelo Jogo: " << fen;
        return {};
    }
    return j.lancesDe(casa);
}

inline bool contem(const Lista& v, const std::string& item) {
    return std::find(v.begin(), v.end(), item) != v.end();
}

// Existe um lance de `de` para `para` (com a promoção indicada, se houver)?
inline bool temLance(const Lista& v, const char* de, const char* para, char promocao = 0) {
    std::string uci = std::string(de) + para;
    if (promocao) uci += promocao;
    return contem(v, uci);
}

// Soma os lances de todas as peças do lado que tem a vez no FEN (-1 se o FEN falhar).
inline int totalDoLado(const std::string& fen) {
    Jogo j;
    if (!j.carregarFen(fen)) {
        ADD_FAILURE() << "FEN rejeitado pelo Jogo: " << fen;
        return -1;
    }
    int total = 0;
    for (char c = 'a'; c <= 'h'; ++c)
        for (char r = '1'; r <= '8'; ++r)
            total += static_cast<int>(j.lancesDe(std::string{c, r}).size());
    return total;
}

// --- espelhamento: inverte as fileiras e troca as cores ("e2" <-> "e7") ---

inline char trocaCaixa(char c) {
    const unsigned char u = static_cast<unsigned char>(c);
    return static_cast<char>(std::isupper(u) ? std::tolower(u) : std::toupper(u));
}

inline std::string espelharCasa(const std::string& casa) {
    if (casa.size() != 2) return casa;
    return std::string{casa[0], static_cast<char>('1' + '8' - casa[1])};
}

inline std::string espelharLance(const std::string& uci) {
    if (uci.size() < 4) return uci;
    return espelharCasa(uci.substr(0, 2)) + espelharCasa(uci.substr(2, 2)) + uci.substr(4);
}

inline std::string espelharFen(const std::string& fen) {
    std::vector<std::string> campos;
    std::istringstream in(fen);
    for (std::string c; in >> c;) campos.push_back(c);
    if (campos.size() < 4) return fen;

    std::vector<std::string> fileiras;
    std::istringstream pecas(campos[0]);
    for (std::string f; std::getline(pecas, f, '/');) fileiras.push_back(f);
    std::reverse(fileiras.begin(), fileiras.end());
    std::string tabuleiro;
    for (std::size_t i = 0; i < fileiras.size(); ++i) {
        if (i) tabuleiro += '/';
        for (char c : fileiras[i]) tabuleiro += trocaCaixa(c);
    }

    const std::string vez = (campos[1] == "w") ? "b" : "w";

    std::string roques;   // volta para a ordem canônica KQkq
    if (campos[2] != "-") {
        std::string trocados;
        for (char c : campos[2]) trocados += trocaCaixa(c);
        for (char c : std::string("KQkq"))
            if (trocados.find(c) != std::string::npos) roques += c;
    }
    if (roques.empty()) roques = "-";

    const std::string passant = (campos[3] == "-") ? "-" : espelharCasa(campos[3]);

    std::string r = tabuleiro + " " + vez + " " + roques + " " + passant;
    for (std::size_t i = 4; i < campos.size(); ++i) r += " " + campos[i];
    return r;
}

// ---------- 2. tabelas de casos ----------

struct CasoLances {
    const char* nome;
    const char* fen;
    const char* casa;
    Lista esperado;   // todos os lances legais da peça em `casa`, em UCI
};

inline std::string nomeDoCaso(const ::testing::TestParamInfo<CasoLances>& info) {
    return info.param.nome;
}

class LancesExatos : public ::testing::TestWithParam<CasoLances> {};

//fim de partida

// Total de lances legais do lado que tem a vez.
inline int totalDeLances(const Jogo& j) {
    int total = 0;
    for (char c = 'a'; c <= 'h'; ++c)
        for (char r = '1'; r <= '8'; ++r)
            total += static_cast<int>(j.lancesDe(std::string{c, r}).size());
    return total;
}