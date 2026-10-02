#include <gtest/gtest.h>
#include "chess/rule_set.hpp"
#include "test_helpers.hpp"

using namespace chess;
using testutil::countFlag;
using testutil::hasMove;

static std::vector<Move> legal(const std::string& fen) {
  return ClassicRules{}.legalMoves(Position::fromFen(fen));
}
static GameStatus statusOf(const std::string& fen) {
  return ClassicRules{}.status(Position::fromFen(fen));
}

// ---- RN-001 / RN-003: movimento das peças ----
TEST(Movimento, PosicaoInicialTem20Lances) { EXPECT_EQ(legal(START_FEN).size(), 20u); }

TEST(Movimento, PeaoAvancaUmaOuDuasNaPrimeiraJogada) {
  auto m = legal(START_FEN);
  EXPECT_TRUE(hasMove(m, "e2", "e3", MoveFlag::Quiet));
  EXPECT_TRUE(hasMove(m, "e2", "e4", MoveFlag::DoublePush));
}

TEST(Movimento, PeaoNaoAvancaDuasForaDaCasaInicial) {
  auto m = legal("4k3/8/8/8/8/4P3/8/4K3 w - - 0 1");
  EXPECT_TRUE(hasMove(m, "e3", "e4", MoveFlag::Quiet));
  EXPECT_FALSE(hasMove(m, "e3", "e5", MoveFlag::DoublePush));
}

TEST(Movimento, PeaoCapturaNaDiagonalMasNaoDeFrente) {
  auto m = legal("4k3/8/8/3p1p2/4P3/8/8/4K3 w - - 0 1");
  EXPECT_TRUE(hasMove(m, "e4", "d5", MoveFlag::Capture));
  EXPECT_TRUE(hasMove(m, "e4", "f5", MoveFlag::Capture));
  EXPECT_TRUE(hasMove(m, "e4", "e5", MoveFlag::Quiet));
}

TEST(Movimento, NaoCapturaPecaDaMesmaCor) {
  auto m = legal("4k3/8/8/8/8/8/R7/R3K3 w - - 0 1");   // torre a1 bloqueada pela torre a2
  EXPECT_FALSE(hasMove(m, "a1", "a2", MoveFlag::Capture));
  EXPECT_FALSE(hasMove(m, "a1", "a2", MoveFlag::Quiet));
}

// ---- RN-004: xeque ----
TEST(Xeque, PecaPregadaSoAndaNaLinhaDoPin) {
  auto m = legal("4k3/4r3/8/8/8/8/4R3/4K3 w - - 0 1");   // torre branca e2 pregada pela torre e7
  for (const Move& mv : m)
    if (mv.from == fromAlgebraic("e2")) EXPECT_EQ(fileOf(mv.to), 4);
}

TEST(Xeque, EmXequeSoPodeSairDoXeque) {
  // rei e1 atacado pela torre e8; só lances do rei para fora da coluna e (ou bloquear/capturar)
  auto m = legal("4r1k1/8/8/8/8/8/8/4K3 w - - 0 1");
  for (const Move& mv : m) EXPECT_NE(fileOf(mv.to), 4) << "lance deixa o rei em xeque";
  EXPECT_FALSE(m.empty());
}

// ---- RN-005 / RN-008 / RN-009 / RN-011: fim de partida ----
TEST(FimDePartida, MateDoPastor) {
  EXPECT_EQ(statusOf("r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4"), GameStatus::Checkmate);
}
TEST(FimDePartida, ReiAfogado) {
  EXPECT_EQ(statusOf("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1"), GameStatus::Stalemate);
}
TEST(FimDePartida, CinquentaLances) {
  EXPECT_EQ(statusOf("4k3/8/8/8/8/8/8/R3K3 w - - 100 80"), GameStatus::DrawFiftyMove);
  EXPECT_EQ(statusOf("4k3/8/8/8/8/8/8/R3K3 w - - 99 80"), GameStatus::Ongoing);
}
TEST(FimDePartida, MaterialInsuficiente) {
  EXPECT_EQ(statusOf("4k3/8/8/8/8/8/8/4K3 w - - 0 1"), GameStatus::DrawInsufficientMaterial);       // K x K
  EXPECT_EQ(statusOf("4k3/8/8/8/8/8/8/3BK3 w - - 0 1"), GameStatus::DrawInsufficientMaterial);      // K+B x K
  EXPECT_EQ(statusOf("4k3/8/8/8/8/8/8/3NK3 w - - 0 1"), GameStatus::DrawInsufficientMaterial);      // K+N x K
  EXPECT_EQ(statusOf("4k3/8/8/8/8/8/8/2NNK3 w - - 0 1"), GameStatus::Ongoing);                      // K+N+N x K
}
TEST(FimDePartida, XequeSemMate) {
  EXPECT_EQ(statusOf("4r1k1/8/8/8/8/8/8/4K3 w - - 0 1"), GameStatus::Check);
}

// ---- RN-006: roque ----
TEST(Roque, AmbosOsLadosQuandoLivre) {
  auto m = legal("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
  EXPECT_TRUE(hasMove(m, "e1", "g1", MoveFlag::CastleKing));
  EXPECT_TRUE(hasMove(m, "e1", "c1", MoveFlag::CastleQueen));
}
TEST(Roque, NaoPassaPorCasaAtacada) {
  auto m = legal("4k3/8/8/8/8/8/5r2/R3K2R w KQ - 0 1");   // torre preta ataca f1
  EXPECT_FALSE(hasMove(m, "e1", "g1", MoveFlag::CastleKing));
  EXPECT_TRUE(hasMove(m, "e1", "c1", MoveFlag::CastleQueen));
}
TEST(Roque, NaoEmXeque) {
  auto m = legal("4r1k1/8/8/8/8/8/8/R3K2R w KQ - 0 1");
  EXPECT_EQ(countFlag(m, MoveFlag::CastleKing) + countFlag(m, MoveFlag::CastleQueen), 0u);
}
TEST(Roque, NaoComCaminhoOcupado) {
  auto m = legal("4k3/8/8/8/8/8/8/RN2K2R w KQ - 0 1");
  EXPECT_TRUE(hasMove(m, "e1", "g1", MoveFlag::CastleKing));
  EXPECT_FALSE(hasMove(m, "e1", "c1", MoveFlag::CastleQueen));
}
TEST(Roque, PerdeDireitoDepoisDeMoverATorre) {
  Position p = Position::fromFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1");
  Position q = p.applied(Move{fromAlgebraic("h1"), fromAlgebraic("h2"), PieceType::None, MoveFlag::Quiet});
  EXPECT_EQ(q.castling, CASTLE_WQ);
}
TEST(Roque, MoveATorreJunto) {
  Position p = Position::fromFen("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1");
  Position q = p.applied(Move{fromAlgebraic("e1"), fromAlgebraic("g1"), PieceType::None, MoveFlag::CastleKing});
  EXPECT_EQ(q.pieceAt(fromAlgebraic("f1")).type, PieceType::Rook);
  EXPECT_EQ(q.pieceAt(fromAlgebraic("g1")).type, PieceType::King);
  EXPECT_TRUE(q.pieceAt(fromAlgebraic("h1")).empty());
}

// ---- RN-007: en passant ----
TEST(EnPassant, DisponivelLogoAposOAvancoDuplo) {
  EXPECT_TRUE(hasMove(legal("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1"), "e5", "d6", MoveFlag::EnPassant));
}
TEST(EnPassant, IndisponivelSemACasaDeEnPassant) {
  EXPECT_FALSE(hasMove(legal("4k3/8/8/3pP3/8/8/8/4K3 w - - 0 1"), "e5", "d6", MoveFlag::EnPassant));
}
TEST(EnPassant, RemoveOPeaoCapturado) {
  Position p = Position::fromFen("4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1");
  Position q = p.applied(Move{fromAlgebraic("e5"), fromAlgebraic("d6"), PieceType::None, MoveFlag::EnPassant});
  EXPECT_TRUE(q.pieceAt(fromAlgebraic("d5")).empty());
  EXPECT_EQ(q.pieceAt(fromAlgebraic("d6")).type, PieceType::Pawn);
}
TEST(EnPassant, AvancoDuploCriaACasa) {
  Position p = Position::fromFen(START_FEN);
  Position q = p.applied(Move{fromAlgebraic("e2"), fromAlgebraic("e4"), PieceType::None, MoveFlag::DoublePush});
  EXPECT_EQ(q.epSquare, fromAlgebraic("e3"));
}

// ---- Promoção ----
TEST(Promocao, GeraQuatroOpcoes) {
  auto m = legal("4k3/P7/8/8/8/8/8/4K3 w - - 0 1");
  EXPECT_EQ(countFlag(m, MoveFlag::Promotion), 4u);
}
TEST(Promocao, SubstituiOPeaoPelaPecaEscolhida) {
  Position p = Position::fromFen("4k3/P7/8/8/8/8/8/4K3 w - - 0 1");
  Position q = p.applied(Move{fromAlgebraic("a7"), fromAlgebraic("a8"), PieceType::Queen, MoveFlag::Promotion});
  EXPECT_EQ(q.pieceAt(fromAlgebraic("a8")).type, PieceType::Queen);
}
