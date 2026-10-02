#include "chess/position.hpp"
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace chess {

std::string toAlgebraic(Square s) {
  return std::string{static_cast<char>('a' + fileOf(s)), static_cast<char>('1' + rankOf(s))};
}

Square fromAlgebraic(const std::string& n) {
  if (n.size() != 2 || n[0] < 'a' || n[0] > 'h' || n[1] < '1' || n[1] > '8') return NO_SQUARE;
  return makeSquare(n[0] - 'a', n[1] - '1');
}

namespace {

char pieceChar(Piece p) {
  const char* t = ".pnbrqk";
  char c = t[static_cast<int>(p.type)];
  return p.color == Color::White ? static_cast<char>(std::toupper(c)) : c;
}

Piece pieceFromChar(char c) {
  Color col = std::isupper(static_cast<unsigned char>(c)) ? Color::White : Color::Black;
  switch (std::tolower(static_cast<unsigned char>(c))) {
    case 'p': return {PieceType::Pawn, col};
    case 'n': return {PieceType::Knight, col};
    case 'b': return {PieceType::Bishop, col};
    case 'r': return {PieceType::Rook, col};
    case 'q': return {PieceType::Queen, col};
    case 'k': return {PieceType::King, col};
  }
  throw std::invalid_argument(std::string("FEN: peça inválida '") + c + "'");
}

// Máscara que remove direitos de roque quando uma peça sai de (ou chega em) certas casas
int rightsMask(int s) {
  switch (s) {
    case 4:  return ~(CASTLE_WK | CASTLE_WQ);  // e1
    case 60: return ~(CASTLE_BK | CASTLE_BQ);  // e8
    case 0:  return ~CASTLE_WQ;                // a1
    case 7:  return ~CASTLE_WK;                // h1
    case 56: return ~CASTLE_BQ;                // a8
    case 63: return ~CASTLE_BK;                // h8
    default: return ~0;
  }
}

}  // namespace

Position Position::fromFen(const std::string& fen) {
  Position p;
  std::istringstream in(fen);
  std::string placement, side, castle, ep;
  if (!(in >> placement >> side >> castle >> ep))
    throw std::invalid_argument("FEN: esperados 4 campos ou mais");
  int half = 0, full = 1;
  in >> half >> full;

  int rank = 7, file = 0, whiteKings = 0, blackKings = 0;
  for (char c : placement) {
    if (c == '/') {
      if (file != 8 || rank == 0) throw std::invalid_argument("FEN: linha mal formada");
      --rank; file = 0;
    } else if (c >= '1' && c <= '8') {
      file += c - '0';
      if (file > 8) throw std::invalid_argument("FEN: linha com mais de 8 casas");
    } else {
      if (file >= 8) throw std::invalid_argument("FEN: linha com mais de 8 casas");
      Piece pc = pieceFromChar(c);
      if (pc.type == PieceType::King) (pc.color == Color::White ? whiteKings : blackKings)++;
      p.board[makeSquare(file, rank)] = pc;
      ++file;
    }
  }
  if (rank != 0 || file != 8) throw std::invalid_argument("FEN: tabuleiro incompleto");
  if (whiteKings != 1 || blackKings != 1) throw std::invalid_argument("FEN: cada lado precisa de exatamente 1 rei");

  if (side == "w") p.sideToMove = Color::White;
  else if (side == "b") p.sideToMove = Color::Black;
  else throw std::invalid_argument("FEN: lado a jogar deve ser w ou b");

  if (castle != "-") {
    for (char c : castle) {
      switch (c) {
        case 'K': p.castling |= CASTLE_WK; break;
        case 'Q': p.castling |= CASTLE_WQ; break;
        case 'k': p.castling |= CASTLE_BK; break;
        case 'q': p.castling |= CASTLE_BQ; break;
        default: throw std::invalid_argument("FEN: direitos de roque inválidos");
      }
    }
  }
  if (ep != "-") {
    p.epSquare = fromAlgebraic(ep);
    if (p.epSquare == NO_SQUARE) throw std::invalid_argument("FEN: casa de en passant inválida");
  }
  p.halfmoveClock = half;
  p.fullmoveNumber = full;
  return p;
}

std::string Position::toFen() const {
  std::ostringstream o;
  for (int r = 7; r >= 0; --r) {
    int empty = 0;
    for (int f = 0; f < 8; ++f) {
      Piece pc = board[makeSquare(f, r)];
      if (pc.empty()) { ++empty; continue; }
      if (empty) { o << empty; empty = 0; }
      o << pieceChar(pc);
    }
    if (empty) o << empty;
    if (r > 0) o << '/';
  }
  o << ' ' << (sideToMove == Color::White ? 'w' : 'b') << ' ';
  if (castling == 0) o << '-';
  else {
    if (castling & CASTLE_WK) o << 'K';
    if (castling & CASTLE_WQ) o << 'Q';
    if (castling & CASTLE_BK) o << 'k';
    if (castling & CASTLE_BQ) o << 'q';
  }
  o << ' ' << (epSquare == NO_SQUARE ? "-" : toAlgebraic(epSquare));
  o << ' ' << halfmoveClock << ' ' << fullmoveNumber;
  return o.str();
}

Square Position::kingSquare(Color c) const {
  for (int s = 0; s < 64; ++s)
    if (board[s].type == PieceType::King && board[s].color == c) return static_cast<Square>(s);
  return NO_SQUARE;
}

bool Position::isAttacked(Square sq, Color by) const {
  const int f = fileOf(sq), r = rankOf(sq);

  // peões: um peão branco em (f±1, r-1) ataca (f, r); um preto em (f±1, r+1)
  const int pr = (by == Color::White) ? r - 1 : r + 1;
  for (int df : {-1, 1}) {
    if (!onBoard(f + df, pr)) continue;
    Piece p = board[makeSquare(f + df, pr)];
    if (p.type == PieceType::Pawn && p.color == by) return true;
  }
  for (auto& d : KNIGHT_D) {
    if (!onBoard(f + d[0], r + d[1])) continue;
    Piece p = board[makeSquare(f + d[0], r + d[1])];
    if (p.type == PieceType::Knight && p.color == by) return true;
  }
  for (auto& d : KING_D) {
    if (!onBoard(f + d[0], r + d[1])) continue;
    Piece p = board[makeSquare(f + d[0], r + d[1])];
    if (p.type == PieceType::King && p.color == by) return true;
  }
  // peças deslizantes: anda até achar a primeira peça
  auto ray = [&](const int (*dirs)[2], int n, PieceType a, PieceType b) {
    for (int i = 0; i < n; ++i) {
      int cf = f + dirs[i][0], cr = r + dirs[i][1];
      while (onBoard(cf, cr)) {
        Piece p = board[makeSquare(cf, cr)];
        if (!p.empty()) {
          if (p.color == by && (p.type == a || p.type == b)) return true;
          break;
        }
        cf += dirs[i][0]; cr += dirs[i][1];
      }
    }
    return false;
  };
  return ray(DIAG_D, 4, PieceType::Bishop, PieceType::Queen) ||
         ray(ORTHO_D, 4, PieceType::Rook, PieceType::Queen);
}

bool Position::inCheck() const {
  return isAttacked(kingSquare(sideToMove), opposite(sideToMove));
}

Position Position::applied(const Move& m) const {
  Position n = *this;
  const Piece mover = board[m.from];
  const bool capture = !board[m.to].empty() || m.flag == MoveFlag::EnPassant;

  n.board[m.to] = mover;
  n.board[m.from] = Piece{};

  const int r = rankOf(m.from);
  switch (m.flag) {
    case MoveFlag::EnPassant:   // remove o peão capturado (na linha de origem, coluna de destino)
      n.board[makeSquare(fileOf(m.to), r)] = Piece{};
      break;
    case MoveFlag::CastleKing:  // torre h -> f
      n.board[makeSquare(5, r)] = n.board[makeSquare(7, r)];
      n.board[makeSquare(7, r)] = Piece{};
      break;
    case MoveFlag::CastleQueen: // torre a -> d
      n.board[makeSquare(3, r)] = n.board[makeSquare(0, r)];
      n.board[makeSquare(0, r)] = Piece{};
      break;
    case MoveFlag::Promotion:
      n.board[m.to] = Piece{m.promotion, mover.color};
      break;
    default: break;
  }

  n.castling &= rightsMask(m.from) & rightsMask(m.to);
  n.epSquare = (m.flag == MoveFlag::DoublePush) ? static_cast<Square>((m.from + m.to) / 2) : NO_SQUARE;
  n.halfmoveClock = (capture || mover.type == PieceType::Pawn) ? 0 : halfmoveClock + 1;
  if (sideToMove == Color::Black) ++n.fullmoveNumber;
  n.sideToMove = opposite(sideToMove);
  return n;
}

}  // namespace chess
