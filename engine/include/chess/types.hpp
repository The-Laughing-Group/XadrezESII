#pragma once
#include <cstdint>

namespace chess {

enum class Color : uint8_t { White, Black };
constexpr Color opposite(Color c) { return c == Color::White ? Color::Black : Color::White; }

enum class PieceType : uint8_t { None, Pawn, Knight, Bishop, Rook, Queen, King };

struct Piece {
  PieceType type = PieceType::None;
  Color color = Color::White;
  bool empty() const { return type == PieceType::None; }
};

// Casas de 0 a 63: a1 = 0, b1 = 1, ..., h1 = 7, a2 = 8, ..., h8 = 63
using Square = int8_t;
constexpr Square NO_SQUARE = -1;
constexpr int fileOf(int s) { return s & 7; }  // coluna 0..7 (a..h)
constexpr int rankOf(int s) { return s >> 3; } // linha 0..7 (1..8)
constexpr Square makeSquare(int file, int rank) { return static_cast<Square>(rank * 8 + file); }
constexpr bool onBoard(int file, int rank) { return file >= 0 && file < 8 && rank >= 0 && rank < 8; }

enum class MoveFlag : uint8_t {
  Quiet, DoublePush, Capture, EnPassant, CastleKing, CastleQueen, Promotion
};

struct Move {
  Square from = NO_SQUARE;
  Square to = NO_SQUARE;
  PieceType promotion = PieceType::None;  // só preenchido em promoções
  MoveFlag flag = MoveFlag::Quiet;
};

// Direções como {delta coluna, delta linha}
inline constexpr int KNIGHT_D[8][2] = {{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
inline constexpr int KING_D[8][2]   = {{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}};
inline constexpr int DIAG_D[4][2]   = {{1,1},{1,-1},{-1,1},{-1,-1}};
inline constexpr int ORTHO_D[4][2]  = {{1,0},{-1,0},{0,1},{0,-1}};

}  // namespace chess
