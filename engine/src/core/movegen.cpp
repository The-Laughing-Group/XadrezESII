#include "chess/movegen.hpp"

namespace chess {
namespace {

using Moves = std::vector<Move>;

void add(Moves& out, int from, int to, MoveFlag flag, PieceType promo = PieceType::None) {
  out.push_back(Move{static_cast<Square>(from), static_cast<Square>(to), promo, flag});
}

void addPromotions(Moves& out, int from, int to) {
  for (PieceType t : {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight})
    add(out, from, to, MoveFlag::Promotion, t);
}

void genPawn(const Position& pos, int s, Moves& out) {
  const Color us = pos.sideToMove;
  const int f = fileOf(s), r = rankOf(s);
  const int dir = us == Color::White ? 1 : -1;
  const int startRank = us == Color::White ? 1 : 6;
  const int promoRank = us == Color::White ? 7 : 0;
  const int nr = r + dir;
  if (!onBoard(f, nr)) return;

  // avanço
  if (pos.board[makeSquare(f, nr)].empty()) {
    if (nr == promoRank) addPromotions(out, s, makeSquare(f, nr));
    else {
      add(out, s, makeSquare(f, nr), MoveFlag::Quiet);
      if (r == startRank && pos.board[makeSquare(f, r + 2 * dir)].empty())
        add(out, s, makeSquare(f, r + 2 * dir), MoveFlag::DoublePush);
    }
  }
  // capturas na diagonal e en passant
  for (int df : {-1, 1}) {
    const int nf = f + df;
    if (!onBoard(nf, nr)) continue;
    const Square t = makeSquare(nf, nr);
    const Piece tp = pos.board[t];
    if (!tp.empty() && tp.color != us) {
      if (nr == promoRank) addPromotions(out, s, t);
      else add(out, s, t, MoveFlag::Capture);
    } else if (tp.empty() && t == pos.epSquare) {
      const Piece cap = pos.board[makeSquare(nf, r)];
      if (cap.type == PieceType::Pawn && cap.color != us) add(out, s, t, MoveFlag::EnPassant);
    }
  }
}

// Cavalo e rei: um único passo em cada direção
void genLeaper(const Position& pos, int s, const int (*dirs)[2], int n, Moves& out) {
  const Color us = pos.sideToMove;
  for (int i = 0; i < n; ++i) {
    const int nf = fileOf(s) + dirs[i][0], nr = rankOf(s) + dirs[i][1];
    if (!onBoard(nf, nr)) continue;
    const Square t = makeSquare(nf, nr);
    const Piece tp = pos.board[t];
    if (tp.empty()) add(out, s, t, MoveFlag::Quiet);
    else if (tp.color != us) add(out, s, t, MoveFlag::Capture);   // RN-003: nunca sobre peça própria
  }
}

// Bispo, torre e rainha: andam até esbarrar em alguma peça
void genSlider(const Position& pos, int s, const int (*dirs)[2], int n, Moves& out) {
  const Color us = pos.sideToMove;
  for (int i = 0; i < n; ++i) {
    int nf = fileOf(s) + dirs[i][0], nr = rankOf(s) + dirs[i][1];
    while (onBoard(nf, nr)) {
      const Square t = makeSquare(nf, nr);
      const Piece tp = pos.board[t];
      if (tp.empty()) add(out, s, t, MoveFlag::Quiet);
      else {
        if (tp.color != us) add(out, s, t, MoveFlag::Capture);
        break;
      }
      nf += dirs[i][0]; nr += dirs[i][1];
    }
  }
}

// RN-006: não pode estar em xeque, passar por casa atacada nem terminar em casa atacada
void genCastling(const Position& pos, Moves& out) {
  const Color us = pos.sideToMove, them = opposite(us);
  const int base = (us == Color::White) ? 0 : 56;
  const Piece k = pos.board[base + 4];
  if (k.type != PieceType::King || k.color != us) return;
  if (pos.isAttacked(static_cast<Square>(base + 4), them)) return;

  const int kingSide = us == Color::White ? CASTLE_WK : CASTLE_BK;
  const int queenSide = us == Color::White ? CASTLE_WQ : CASTLE_BQ;
  auto rookAt = [&](int sq) { Piece p = pos.board[sq]; return p.type == PieceType::Rook && p.color == us; };
  auto empty = [&](int sq) { return pos.board[sq].empty(); };
  auto safe = [&](int sq) { return !pos.isAttacked(static_cast<Square>(sq), them); };

  if ((pos.castling & kingSide) && rookAt(base + 7) && empty(base + 5) && empty(base + 6) &&
      safe(base + 5) && safe(base + 6))
    add(out, base + 4, base + 6, MoveFlag::CastleKing);
  if ((pos.castling & queenSide) && rookAt(base) && empty(base + 1) && empty(base + 2) &&
      empty(base + 3) && safe(base + 3) && safe(base + 2))
    add(out, base + 4, base + 2, MoveFlag::CastleQueen);
}

}  // namespace

std::vector<Move> generatePseudoLegalMoves(const Position& pos) {
  Moves out;
  out.reserve(48);
  for (int s = 0; s < 64; ++s) {
    const Piece p = pos.board[s];
    if (p.empty() || p.color != pos.sideToMove) continue;
    switch (p.type) {
      case PieceType::Pawn:   genPawn(pos, s, out); break;
      case PieceType::Knight: genLeaper(pos, s, KNIGHT_D, 8, out); break;
      case PieceType::King:   genLeaper(pos, s, KING_D, 8, out); break;
      case PieceType::Bishop: genSlider(pos, s, DIAG_D, 4, out); break;
      case PieceType::Rook:   genSlider(pos, s, ORTHO_D, 4, out); break;
      case PieceType::Queen:  genSlider(pos, s, DIAG_D, 4, out);
                              genSlider(pos, s, ORTHO_D, 4, out); break;
      case PieceType::None:   break;
    }
  }
  genCastling(pos, out);
  return out;
}

std::vector<Move> generateLegalMoves(const Position& pos) {
  const Color us = pos.sideToMove;
  Moves legal;
  for (const Move& m : generatePseudoLegalMoves(pos)) {
    const Position next = pos.applied(m);
    if (!next.isAttacked(next.kingSquare(us), opposite(us))) legal.push_back(m);   // RN-004
  }
  return legal;
}

}  // namespace chess
