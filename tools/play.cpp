#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "poker/card.h"
#include "poker/game.h"
#include "poker/session.h"

namespace {

constexpr int kHumanPlayer = 0;
const char* kSessionPath = ".poker_session";

std::string card_str(int id) { return poker::Card(id).to_string(); }

std::string cards_str(const int* ids, int count) {
  std::string out;
  for (int i = 0; i < count; ++i) {
    if (i > 0) {
      out += ' ';
    }
    out += card_str(ids[i]);
  }
  return out;
}

void print_state(const poker::GameState& state, bool reveal_villain) {
  const int villain = 1 - kHumanPlayer;
  std::cout << "=== " << poker::street_name(state.street) << " ===\n";
  if (state.board_count > 0) {
    std::cout << "Board: " << cards_str(state.board.data(), state.board_count) << '\n';
  }
  std::cout << "Pot: " << state.pot << '\n';
  std::cout << "Your stack: " << state.stacks[kHumanPlayer]
            << " (in for " << state.total_committed[kHumanPlayer] << " this hand)\n";
  std::cout << "Villain stack: " << state.stacks[villain]
            << " (in for " << state.total_committed[villain] << " this hand)\n";
  std::cout << "Your cards: "
            << cards_str(state.hole_cards[kHumanPlayer].data(), poker::kHoleCards) << '\n';
  if (reveal_villain) {
    std::cout << "Villain cards: "
              << cards_str(state.hole_cards[villain].data(), poker::kHoleCards) << '\n';
  } else {
    std::cout << "Villain cards: ?? ??\n";
  }
  if (!state.terminal) {
    const int to_call = poker::HeadsUpGame::to_call(state, state.actor);
    std::cout << "To act: " << (state.actor == kHumanPlayer ? "YOU" : "Villain")
              << (state.actor == kHumanPlayer ? " (to call: " + std::to_string(to_call) + ")" : "")
              << '\n';
  }
}

void print_actions(const std::vector<poker::Action>& actions) {
  for (std::size_t i = 0; i < actions.size(); ++i) {
    std::cout << "  [" << i << "] " << poker::action_to_string(actions[i]) << '\n';
  }
}

poker::Action choose_villain_action(const poker::GameState& state) {
  const auto actions = poker::HeadsUpGame::legal_actions(state);
  if (actions.empty()) {
    return {poker::ActionType::kCheck, 0};
  }

  for (const auto& action : actions) {
    if (action.type == poker::ActionType::kCheck) {
      return action;
    }
  }
  for (const auto& action : actions) {
    if (action.type == poker::ActionType::kCall) {
      return action;
    }
  }
  return actions.front();
}

void print_result(const poker::GameState& state) {
  print_state(state, true);
  const auto payoffs = poker::HeadsUpGame::payoffs(state);
  std::cout << "\nHand over.\n";
  std::cout << "Your result: " << (payoffs[kHumanPlayer] >= 0 ? "+" : "")
            << payoffs[kHumanPlayer] << " chips\n";
  if (state.winner == kHumanPlayer) {
    std::cout << "You won the pot.\n";
  } else if (state.winner == 1 - kHumanPlayer) {
    std::cout << "Villain won the pot.\n";
  } else {
    std::cout << "Split pot.\n";
  }
}

int run_until_human_or_done(poker::GameState& state) {
  while (!poker::HeadsUpGame::is_terminal(state) && state.actor != kHumanPlayer) {
    const auto villain_action = choose_villain_action(state);
    std::cout << "Villain: " << poker::action_to_string(villain_action) << '\n';
    state = poker::HeadsUpGame::apply(state, villain_action);
    if (state.street != poker::Street::kPreflop || state.board_count > 0) {
      std::cout << "---\n";
    }
  }
  return 0;
}

bool apply_human_action(poker::GameState& state, const std::string& input) {
  if (state.actor != kHumanPlayer) {
    std::cout << "Not your turn.\n";
    return false;
  }

  const auto actions = poker::HeadsUpGame::legal_actions(state);
  if (actions.empty()) {
    std::cout << "No legal actions available.\n";
    return false;
  }

  std::string trimmed = input;
  while (!trimmed.empty() && trimmed.front() == ' ') {
    trimmed.erase(trimmed.begin());
  }
  while (!trimmed.empty() && trimmed.back() == ' ') {
    trimmed.pop_back();
  }
  if (trimmed.empty()) {
    return false;
  }

  for (char& c : trimmed) {
    if (c >= 'A' && c <= 'Z') {
      c = static_cast<char>(c - 'A' + 'a');
    }
  }

  if (trimmed == "help" || trimmed == "h" || trimmed == "?") {
    std::cout << "Commands: fold | check | call | bet <n> | raise <n> | all-in | <index>\n";
    print_actions(actions);
    return false;
  }

  int index = -1;
  if (trimmed == "fold" || trimmed == "f") {
    for (std::size_t i = 0; i < actions.size(); ++i) {
      if (actions[i].type == poker::ActionType::kFold) {
        index = static_cast<int>(i);
        break;
      }
    }
  } else if (trimmed == "check" || trimmed == "x") {
    for (std::size_t i = 0; i < actions.size(); ++i) {
      if (actions[i].type == poker::ActionType::kCheck) {
        index = static_cast<int>(i);
        break;
      }
    }
  } else if (trimmed == "call" || trimmed == "c") {
    for (std::size_t i = 0; i < actions.size(); ++i) {
      if (actions[i].type == poker::ActionType::kCall) {
        index = static_cast<int>(i);
        break;
      }
    }
  } else if (trimmed == "all-in" || trimmed == "allin" || trimmed == "ai") {
    for (std::size_t i = 0; i < actions.size(); ++i) {
      if (actions[i].type == poker::ActionType::kAllIn) {
        index = static_cast<int>(i);
        break;
      }
    }
  } else if (trimmed.rfind("bet ", 0) == 0 || trimmed.rfind("raise ", 0) == 0) {
    const int amount = std::atoi(trimmed.c_str() + (trimmed[0] == 'b' ? 4 : 6));
    const auto target_type = trimmed[0] == 'b' ? poker::ActionType::kBet : poker::ActionType::kRaise;
    for (std::size_t i = 0; i < actions.size(); ++i) {
      if (actions[i].type == target_type && actions[i].amount == amount) {
        index = static_cast<int>(i);
        break;
      }
    }
    if (index < 0) {
      for (std::size_t i = 0; i < actions.size(); ++i) {
        if (actions[i].type == target_type) {
          std::cout << "Invalid size. Available " << poker::action_type_name(target_type)
                    << ": " << actions[i].amount << '\n';
          return false;
        }
      }
    }
  } else {
    char* end = nullptr;
    const long parsed = std::strtol(trimmed.c_str(), &end, 10);
    if (end != trimmed.c_str() && *end == '\0') {
      index = static_cast<int>(parsed);
    }
  }

  if (index < 0 || static_cast<std::size_t>(index) >= actions.size()) {
    std::cout << "Invalid action. Type 'help' for options.\n";
    print_actions(actions);
    return false;
  }

  const auto human_action = actions[static_cast<std::size_t>(index)];
  std::cout << "You: " << poker::action_to_string(human_action) << '\n';
  state = poker::HeadsUpGame::apply(state, human_action);
  return true;
}

poker::GameState start_hand(std::uint64_t seed) {
  poker::GameConfig config;
  config.small_blind = 1;
  config.big_blind = 2;
  config.starting_stack = 100;

  auto state = poker::HeadsUpGame::new_hand(config, seed);
  std::cout << "\nNew hand (blinds 1/2, 100 chip stacks)\n";
  std::cout << "You are the Button / Small Blind.\n\n";
  run_until_human_or_done(state);
  return state;
}

int cmd_interactive(std::uint64_t seed) {
  std::cout << "PokerEngine — Heads-Up NLHE\n";
  std::cout << "Type 'help' at any prompt for actions. 'quit' to exit.\n";

  auto state = start_hand(seed);

  while (true) {
    if (poker::HeadsUpGame::is_terminal(state)) {
      std::cout << '\n';
      print_result(state);
      std::cout << "\nPlay another hand? [y/n]: ";
      std::string again;
      if (!std::getline(std::cin, again)) {
        break;
      }
      if (again.empty() || (again[0] != 'y' && again[0] != 'Y')) {
        break;
      }
      seed += 1;
      state = start_hand(seed);
      continue;
    }

    print_state(state, false);
    std::cout << "\nYour actions:\n";
    print_actions(poker::HeadsUpGame::legal_actions(state));
    std::cout << "> ";

    std::string input;
    if (!std::getline(std::cin, input)) {
      break;
    }
    if (input == "quit" || input == "q" || input == "exit") {
      break;
    }
    if (!apply_human_action(state, input)) {
      continue;
    }

    if (!poker::HeadsUpGame::is_terminal(state)) {
      std::cout << '\n';
      run_until_human_or_done(state);
    }
  }

  return 0;
}

int cmd_new(std::uint64_t seed) {
  auto state = start_hand(seed);

  if (!poker::save_session(state, kSessionPath)) {
    std::cerr << "Failed to save session\n";
    return 1;
  }

  if (poker::HeadsUpGame::is_terminal(state)) {
    print_result(state);
    return 0;
  }

  print_state(state, false);
  std::cout << "\nYour actions:\n";
  print_actions(poker::HeadsUpGame::legal_actions(state));
  return 0;
}

int cmd_act(int index) {
  poker::GameState state{};
  if (!poker::load_session(state, kSessionPath)) {
    std::cerr << "No active session. Run: play new\n";
    return 1;
  }
  if (poker::HeadsUpGame::is_terminal(state)) {
    std::cerr << "Hand is already over. Run: play new\n";
    return 1;
  }
  if (state.actor != kHumanPlayer) {
    std::cerr << "Not your turn.\n";
    return 1;
  }

  const auto actions = poker::HeadsUpGame::legal_actions(state);
  if (index < 0 || static_cast<std::size_t>(index) >= actions.size()) {
    std::cerr << "Invalid action index.\n";
    print_actions(actions);
    return 1;
  }

  const auto human_action = actions[static_cast<std::size_t>(index)];
  std::cout << "You: " << poker::action_to_string(human_action) << '\n';
  state = poker::HeadsUpGame::apply(state, human_action);

  if (!poker::HeadsUpGame::is_terminal(state)) {
    run_until_human_or_done(state);
  }

  if (!poker::save_session(state, kSessionPath)) {
    std::cerr << "Failed to save session\n";
    return 1;
  }

  if (poker::HeadsUpGame::is_terminal(state)) {
    std::cout << '\n';
    print_result(state);
    return 0;
  }

  std::cout << '\n';
  print_state(state, false);
  std::cout << "\nYour actions:\n";
  print_actions(poker::HeadsUpGame::legal_actions(state));
  return 0;
}

int cmd_show() {
  poker::GameState state{};
  if (!poker::load_session(state, kSessionPath)) {
    std::cerr << "No active session. Run: play new\n";
    return 1;
  }
  print_state(state, poker::HeadsUpGame::is_terminal(state));
  if (!poker::HeadsUpGame::is_terminal(state) && state.actor == kHumanPlayer) {
    std::cout << "\nYour actions:\n";
    print_actions(poker::HeadsUpGame::legal_actions(state));
  }
  return 0;
}

void print_usage() {
  std::cout << "Usage:\n"
            << "  play              Interactive mode (recommended)\n"
            << "  play interactive  Interactive mode\n"
            << "  play new [seed]   Start a hand (one-shot CLI)\n"
            << "  play act <n>      Apply action by index (one-shot CLI)\n"
            << "  play show         Show saved session\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  std::uint64_t seed = static_cast<std::uint64_t>(
      std::chrono::steady_clock::now().time_since_epoch().count());

  if (argc < 2) {
    return cmd_interactive(seed);
  }

  const std::string command = argv[1];
  if (command == "interactive" || command == "i") {
    if (argc >= 3) {
      seed = std::strtoull(argv[2], nullptr, 10);
    }
    return cmd_interactive(seed);
  }
  if (command == "new") {
    if (argc >= 3) {
      seed = std::strtoull(argv[2], nullptr, 10);
    }
    return cmd_new(seed);
  }
  if (command == "act") {
    if (argc < 3) {
      print_usage();
      return 1;
    }
    return cmd_act(std::atoi(argv[2]));
  }
  if (command == "show") {
    return cmd_show();
  }

  print_usage();
  return 1;
}
