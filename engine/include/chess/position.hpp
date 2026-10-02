#pragma once
#include <array>
#include <string>
#include "chess/types.hpp"

namespace chess {

inline constexpr int CASTLE_WK = 1, CASTLE_WQ = 2, CASTLE_BK = 4, CASTLE_BQ = 8;
inline constexpr const char* START_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

std::string toAlgebraic(Square s);                 // 0 -> "a1"
Square fromAlgebraic(const std::string& name);     // "e4" -> 28, ou NO_SQUARE se inválida

// Estado completo de uma posição. Quem muda o estado é só applied().
struct Position {
  std::array<Piece, 64> board{};
  Color sideToMove = Color::White;
  int castling = 0;                  // bits CASTLE_*
  Square epSquare = NO_SQUARE;       // casa "atrás" do peão que avançou duas casas
  int halfmoveClock = 0;             // lances sem captura nem peão (RN-009), em meio-lances
  int fullmoveNumber = 1;

  static Position fromFen(const std::string& fen);   // lança std::invalid_argument
  std::string toFen() const;

  Piece pieceAt(Square s) const { return board[s]; }
  Square kingSquare(Color c) const;
  bool isAttacked(Square s, Color by) const;         // a casa s é atacada por `by`?
  bool inCheck() const;                              // o rei de quem joga está em xeque?

  // Devolve a posição depois do lance. NÃO valida o lance.
  Position applied(const Move& m) const;
};

}  // namespace chess
