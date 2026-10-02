#pragma once
#include <vector>
#include "chess/position.hpp"

namespace chess {

// Lances que respeitam o movimento de cada peça (RN-001, 003, 006, 007),
// mas que ainda podem deixar o próprio rei em xeque.
std::vector<Move> generatePseudoLegalMoves(const Position& pos);

// Pseudo-legais filtrados: remove os que deixam o próprio rei em xeque (RN-004).
std::vector<Move> generateLegalMoves(const Position& pos);

}  // namespace chess
