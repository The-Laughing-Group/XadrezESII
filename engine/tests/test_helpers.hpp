#pragma once
#include <algorithm>
#include <vector>
#include "chess/position.hpp"
#include "chess/rule_set.hpp"

namespace testutil {

inline bool hasMove(const std::vector<chess::Move>& moves, const std::string& from, const std::string& to,
                    chess::MoveFlag flag) {
  return std::any_of(moves.begin(), moves.end(), [&](const chess::Move& m) {
    return m.from == chess::fromAlgebraic(from) && m.to == chess::fromAlgebraic(to) && m.flag == flag;
  });
}

inline size_t countFlag(const std::vector<chess::Move>& moves, chess::MoveFlag flag) {
  return std::count_if(moves.begin(), moves.end(), [&](const chess::Move& m) { return m.flag == flag; });
}

}  // namespace testutil
