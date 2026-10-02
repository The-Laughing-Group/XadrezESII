#pragma once
#include <vector>
#include "chess/position.hpp"

namespace chess {

enum class GameStatus {
  Ongoing, Check, Checkmate, Stalemate,
  DrawFiftyMove, DrawRepetition, DrawInsufficientMaterial
};

// Ponto de extensão: o modo Chaos será outra implementação (ex.: herdando de ClassicRules).
class IRuleSet {
 public:
  virtual ~IRuleSet() = default;
  virtual std::vector<Move> legalMoves(const Position& pos) const = 0;
  // Status que depende só da posição. A repetição tripla (RN-010) precisa do histórico,
  // então quem detecta é a classe Game.
  virtual GameStatus status(const Position& pos) const = 0;
};

class ClassicRules : public IRuleSet {
 public:
  std::vector<Move> legalMoves(const Position& pos) const override;
  GameStatus status(const Position& pos) const override;
};

}  // namespace chess
