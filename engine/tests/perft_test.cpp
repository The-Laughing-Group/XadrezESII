#include <gtest/gtest.h>
#include <string>
#include "chess/rule_set.hpp"

using namespace chess;

// Perft: conta quantas posições existem após N lances. Os valores corretos são
// conhecidos (chessprogramming.org/Perft_Results); se baterem, o gerador de lances está certo.
static uint64_t perft(const IRuleSet& rules, const Position& pos, int depth) {
  if (depth == 0) return 1;
  auto moves = rules.legalMoves(pos);
  if (depth == 1) return moves.size();
  uint64_t nodes = 0;
  for (const Move& m : moves) nodes += perft(rules, pos.applied(m), depth - 1);
  return nodes;
}

struct PerftCase { const char* name; const char* fen; int depth; uint64_t nodes; };

class PerftTest : public ::testing::TestWithParam<PerftCase> {};

TEST_P(PerftTest, ContaNosCorretamente) {
  const auto& c = GetParam();
  ClassicRules rules;
  EXPECT_EQ(perft(rules, Position::fromFen(c.fen), c.depth), c.nodes);
}

static const char* KIWI = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
static const char* POS3 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
static const char* POS4 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
static const char* POS5 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
static const char* POS6 = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";

INSTANTIATE_TEST_SUITE_P(
    Posicoes, PerftTest,
    ::testing::Values(
        PerftCase{"inicial_d1", START_FEN, 1, 20},
        PerftCase{"inicial_d2", START_FEN, 2, 400},
        PerftCase{"inicial_d3", START_FEN, 3, 8902},
        PerftCase{"inicial_d4", START_FEN, 4, 197281},
        PerftCase{"kiwipete_d1", KIWI, 1, 48},
        PerftCase{"kiwipete_d2", KIWI, 2, 2039},
        PerftCase{"kiwipete_d3", KIWI, 3, 97862},
        PerftCase{"pos3_d1", POS3, 1, 14},
        PerftCase{"pos3_d2", POS3, 2, 191},
        PerftCase{"pos3_d3", POS3, 3, 2812},
        PerftCase{"pos3_d4", POS3, 4, 43238},
        PerftCase{"pos4_d1", POS4, 1, 6},
        PerftCase{"pos4_d2", POS4, 2, 264},
        PerftCase{"pos4_d3", POS4, 3, 9467},
        PerftCase{"pos5_d1", POS5, 1, 44},
        PerftCase{"pos5_d2", POS5, 2, 1486},
        PerftCase{"pos5_d3", POS5, 3, 62379},
        PerftCase{"pos6_d1", POS6, 1, 46},
        PerftCase{"pos6_d2", POS6, 2, 2079},
        PerftCase{"pos6_d3", POS6, 3, 89890}),
    [](const ::testing::TestParamInfo<PerftCase>& i) { return std::string(i.param.name); });
