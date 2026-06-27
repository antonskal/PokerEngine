#pragma once

#include <array>
#include <vector>

#include "poker/types.h"

namespace poker {

struct Pot {
  int amount = 0;
  std::array<bool, kNumPlayers> eligible{};
};

std::vector<Pot> build_pots(const GameState& state);
std::array<int, kNumPlayers> resolve_showdown(const GameState& state);

}  // namespace poker
