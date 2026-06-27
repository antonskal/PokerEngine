#include <gtest/gtest.h>

#include <phevaluator/phevaluator.h>

#include "poker/game.h"
#include "poker/showdown.h"

TEST(ShowdownTest, ResolvesKnownBoardFromPhevaluatorExample) {
  poker::GameState state{};
  state.config.starting_stack = 100;
  state.folded = {false, false};
  state.total_committed = {10, 10};
  state.hole_cards[0] = {phevaluator::Card("Qc"), phevaluator::Card("6c")};
  state.hole_cards[1] = {phevaluator::Card("2c"), phevaluator::Card("9h")};
  state.board = {phevaluator::Card("9c"), phevaluator::Card("4c"), phevaluator::Card("4s"),
                 phevaluator::Card("9d"), phevaluator::Card("4h")};
  state.board_count = 5;

  const auto winnings = poker::resolve_showdown(state);
  EXPECT_EQ(winnings[0], 0);
  EXPECT_EQ(winnings[1], 20);
}

TEST(ShowdownTest, SplitsPotOnTie) {
  poker::GameState state{};
  state.folded = {false, false};
  state.total_committed = {50, 50};
  state.hole_cards[0] = {phevaluator::Card("Ah"), phevaluator::Card("Kd")};
  state.hole_cards[1] = {phevaluator::Card("Ac"), phevaluator::Card("Kh")};
  state.board = {phevaluator::Card("2s"), phevaluator::Card("3s"), phevaluator::Card("4s"),
                 phevaluator::Card("8c"), phevaluator::Card("9d")};
  state.board_count = 5;

  const auto winnings = poker::resolve_showdown(state);
  EXPECT_EQ(winnings[0], 50);
  EXPECT_EQ(winnings[1], 50);
}
