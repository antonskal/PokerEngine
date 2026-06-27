#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace poker {

constexpr int kNumPlayers = 2;
constexpr int kHoleCards = 2;
constexpr int kBoardCards = 5;

enum class Street : std::uint8_t {
  kPreflop,
  kFlop,
  kTurn,
  kRiver,
  kShowdown,
};

enum class ActionType : std::uint8_t {
  kFold,
  kCheck,
  kCall,
  kBet,
  kRaise,
  kAllIn,
};

struct Action {
  ActionType type = ActionType::kCheck;
  int amount = 0;

  bool operator==(const Action& other) const {
    return type == other.type && amount == other.amount;
  }
};

struct GameConfig {
  int small_blind = 1;
  int big_blind = 2;
  int starting_stack = 200;
};

struct GameState {
  GameConfig config{};
  std::uint64_t seed = 0;
  int hand_number = 0;
  int button = 0;

  std::array<int, kNumPlayers> stacks{};
  std::array<int, kNumPlayers> street_bets{};
  std::array<int, kNumPlayers> total_committed{};
  int pot = 0;

  std::array<std::array<int, kHoleCards>, kNumPlayers> hole_cards{};
  std::array<int, kBoardCards> board{};
  int board_count = 0;

  Street street = Street::kPreflop;
  int actor = 0;
  int current_bet = 0;
  int min_raise_to = 0;
  int players_to_act = 0;

  std::array<bool, kNumPlayers> folded{};
  std::array<bool, kNumPlayers> all_in{};

  bool terminal = false;
  int winner = -1;
  std::array<int, kNumPlayers> payoffs{};

  std::array<int, 52> deck{};
  int deck_pos = 0;
};

std::string street_name(Street street);
std::string action_type_name(ActionType type);
std::string action_to_string(const Action& action);

}  // namespace poker
