#include <gtest/gtest.h>

#include "poker/game.h"

namespace {

poker::GameState play_to_showdown(poker::GameState state) {
  while (!poker::HeadsUpGame::is_terminal(state)) {
    const auto actions = poker::HeadsUpGame::legal_actions(state);
    if (actions.empty()) {
      break;
    }
    const auto check = std::find_if(actions.begin(), actions.end(),
                                    [](const poker::Action& action) {
                                      return action.type == poker::ActionType::kCheck;
                                    });
    const auto call = std::find_if(actions.begin(), actions.end(),
                                   [](const poker::Action& action) {
                                     return action.type == poker::ActionType::kCall;
                                   });
    if (check != actions.end()) {
      state = poker::HeadsUpGame::apply(state, *check);
    } else if (call != actions.end()) {
      state = poker::HeadsUpGame::apply(state, *call);
    } else {
      state = poker::HeadsUpGame::apply(state, actions.front());
    }
  }
  return state;
}

poker::GameConfig test_config() {
  poker::GameConfig config;
  config.small_blind = 1;
  config.big_blind = 2;
  config.starting_stack = 100;
  return config;
}

}  // namespace

TEST(GameTest, PostsBlindsAndDealsCards) {
  const auto config = test_config();
  auto state = poker::HeadsUpGame::new_hand(config, 1234);

  EXPECT_EQ(state.street, poker::Street::kPreflop);
  EXPECT_EQ(state.actor, 0);
  EXPECT_EQ(state.stacks[0], 99);
  EXPECT_EQ(state.stacks[1], 98);
  EXPECT_EQ(state.pot, 3);
  EXPECT_EQ(state.hole_cards[0][0], state.hole_cards[0][0]);
  EXPECT_NE(state.hole_cards[0][0], state.hole_cards[1][0]);
}

TEST(GameTest, FoldAwardsPotToOpponent) {
  const auto config = test_config();
  auto state = poker::HeadsUpGame::new_hand(config, 99);
  state = poker::HeadsUpGame::apply(state, {poker::ActionType::kFold, 0});

  ASSERT_TRUE(poker::HeadsUpGame::is_terminal(state));
  EXPECT_EQ(state.winner, 1);
  const auto payoffs = poker::HeadsUpGame::payoffs(state);
  EXPECT_EQ(payoffs[0], -1);
  EXPECT_EQ(payoffs[1], 1);
}

TEST(GameTest, CheckDownPreservesChipTotal) {
  const auto config = test_config();
  auto state = play_to_showdown(poker::HeadsUpGame::new_hand(config, 7));

  const auto payoffs = poker::HeadsUpGame::payoffs(state);
  EXPECT_EQ(payoffs[0] + payoffs[1], 0);
  EXPECT_EQ(state.stacks[0] + state.stacks[1], 200);
}

TEST(GameTest, RejectsIllegalAction) {
  const auto config = test_config();
  auto state = poker::HeadsUpGame::new_hand(config, 5);
  EXPECT_THROW(poker::HeadsUpGame::apply(state, {poker::ActionType::kCheck, 0}),
               std::invalid_argument);
}

TEST(GameTest, RaiseAndCallCompletesHand) {
  const auto config = test_config();
  auto state = poker::HeadsUpGame::new_hand(config, 11);

  const auto actions = poker::HeadsUpGame::legal_actions(state);
  const auto raise = std::find_if(actions.begin(), actions.end(), [](const poker::Action& action) {
    return action.type == poker::ActionType::kRaise;
  });
  ASSERT_NE(raise, actions.end());

  state = poker::HeadsUpGame::apply(state, *raise);
  state = poker::HeadsUpGame::apply(state, {poker::ActionType::kCall, raise->amount});
  state = play_to_showdown(state);

  EXPECT_TRUE(poker::HeadsUpGame::is_terminal(state));
  const auto payoffs = poker::HeadsUpGame::payoffs(state);
  EXPECT_EQ(payoffs[0] + payoffs[1], 0);
}
