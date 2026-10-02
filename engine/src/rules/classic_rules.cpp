#include "chess/rule_set.hpp"
#include "chess/movegen.hpp"

namespace chess {
namespace {

// RN-011: K x K, K+B x K, K+N x K
bool insufficientMaterial(const Position& pos) {
  int minors = 0;
  for (const Piece& p : pos.board) {
    switch (p.type) {
      case PieceType::Pawn: case PieceType::Rook: case PieceType::Queen: return false;
      case PieceType::Knight: case PieceType::Bishop: ++minors; break;
      default: break;
    }
  }
  return minors <= 1;
}

}  // namespace

std::vector<Move> ClassicRules::legalMoves(const Position& pos) const {
  return generateLegalMoves(pos);
}

GameStatus ClassicRules::status(const Position& pos) const {
  const bool check = pos.inCheck();
  if (generateLegalMoves(pos).empty())
    return check ? GameStatus::Checkmate    // RN-005
                 : GameStatus::Stalemate;   // RN-008
  if (insufficientMaterial(pos)) return GameStatus::DrawInsufficientMaterial;  // RN-011
  if (pos.halfmoveClock >= 100) return GameStatus::DrawFiftyMove;              // RN-009
  return check ? GameStatus::Check : GameStatus::Ongoing;
}

}  // namespace chess
