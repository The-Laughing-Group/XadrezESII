#include <gtest/gtest.h>
#include "chess/position.hpp"

using namespace chess;

TEST(Fen, PosicaoInicialIdaEVolta) {
  EXPECT_EQ(Position::fromFen(START_FEN).toFen(), START_FEN);
}

TEST(Fen, PosicaoComplexaIdaEVolta) {
  const std::string kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
  EXPECT_EQ(Position::fromFen(kiwipete).toFen(), kiwipete);
}

TEST(Fen, EnPassantIdaEVolta) {
  const std::string fen = "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 3";
  EXPECT_EQ(Position::fromFen(fen).toFen(), fen);
}

TEST(Fen, LeCamposCorretamente) {
  Position p = Position::fromFen("4k3/8/8/8/8/8/8/R3K2R b Kq - 7 20");
  EXPECT_EQ(p.sideToMove, Color::Black);
  EXPECT_EQ(p.castling, CASTLE_WK | CASTLE_BQ);
  EXPECT_EQ(p.halfmoveClock, 7);
  EXPECT_EQ(p.fullmoveNumber, 20);
  EXPECT_EQ(p.pieceAt(fromAlgebraic("a1")).type, PieceType::Rook);
  EXPECT_EQ(p.pieceAt(fromAlgebraic("e8")).color, Color::Black);
}

TEST(Fen, RejeitaFenInvalida) {
  EXPECT_THROW(Position::fromFen(""), std::invalid_argument);
  EXPECT_THROW(Position::fromFen("8/8/8/8/8/8/8 w - - 0 1"), std::invalid_argument);          // 7 linhas
  EXPECT_THROW(Position::fromFen("4k3/8/8/8/8/8/8/4K3 x - - 0 1"), std::invalid_argument);    // lado inválido
  EXPECT_THROW(Position::fromFen("4k3/8/8/8/8/8/8/8 w - - 0 1"), std::invalid_argument);      // sem rei branco
  EXPECT_THROW(Position::fromFen("4k3/8/8/8/8/8/8/4Z3 w - - 0 1"), std::invalid_argument);    // peça inválida
}
