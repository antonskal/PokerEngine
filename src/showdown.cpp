#include "poker/showdown.h"

#include <phevaluator/phevaluator.h>

#include <algorithm>
#include <vector>

namespace poker {

namespace {

int hand_rank(const GameState& state, int player) {
  const auto& hole = state.hole_cards[player];
  const auto& board = state.board;
  return evaluate_7cards(hole[0], hole[1], board[0], board[1], board[2], board[3],
                         board[4]);
}

}  // namespace

std::vector<Pot> build_pots(const GameState& state) {
  std::array<int, kNumPlayers> contributions = state.total_committed;
  std::vector<Pot> pots;

  while (true) {
    int min_positive = 0;
    for (int amount : contributions) {
      if (amount > 0 && (min_positive == 0 || amount < min_positive)) {
        min_positive = amount;
      }
    }
    if (min_positive == 0) {
      break;
    }

    Pot pot;
    for (int player = 0; player < kNumPlayers; ++player) {
      if (contributions[player] > 0) {
        pot.amount += min_positive;
        contributions[player] -= min_positive;
        if (!state.folded[player]) {
          pot.eligible[player] = true;
        }
      }
    }
    pots.push_back(pot);
  }

  return pots;
}

std::array<int, kNumPlayers> resolve_showdown(const GameState& state) {
  std::array<int, kNumPlayers> winnings{};
  const auto pots = build_pots(state);

  for (const Pot& pot : pots) {
    if (pot.amount == 0) {
      continue;
    }

    int best_rank = 0;
    std::array<bool, kNumPlayers> contenders{};
    int contender_count = 0;

    for (int player = 0; player < kNumPlayers; ++player) {
      if (pot.eligible[player]) {
        contenders[player] = true;
        ++contender_count;
        const int rank = hand_rank(state, player);
        if (best_rank == 0 || rank < best_rank) {
          best_rank = rank;
        }
      }
    }

    if (contender_count == 0) {
      continue;
    }

    int winners = 0;
    for (int player = 0; player < kNumPlayers; ++player) {
      if (contenders[player] && hand_rank(state, player) == best_rank) {
        ++winners;
      }
    }

    int distributed = 0;
    for (int player = 0; player < kNumPlayers; ++player) {
      if (contenders[player] && hand_rank(state, player) == best_rank) {
        const int share = pot.amount / winners;
        winnings[player] += share;
        distributed += share;
      }
    }

    const int remainder = pot.amount - distributed;
    if (remainder > 0) {
      for (int player = 0; player < kNumPlayers; ++player) {
        if (contenders[player] && hand_rank(state, player) == best_rank) {
          winnings[player] += remainder;
          break;
        }
      }
    }
  }

  return winnings;
}

}  // namespace poker
