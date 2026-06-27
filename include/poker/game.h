#pragma once

#include <vector>

#include "poker/types.h"

namespace poker {

class HeadsUpGame {
 public:
  static GameState new_hand(const GameConfig& config, std::uint64_t seed,
                            int hand_number = 0, int button = 0);

  static std::vector<Action> legal_actions(const GameState& state);
  static GameState apply(const GameState& state, const Action& action);

  static bool is_terminal(const GameState& state);
  static std::array<int, kNumPlayers> payoffs(const GameState& state);

  static int to_call(const GameState& state, int player);
  static int effective_stack(const GameState& state, int player);
};

}  // namespace poker
