#include "poker/game.h"

#include <algorithm>
#include <phevaluator/phevaluator.h>
#include <stdexcept>
#include <string>

#include "poker/deck.h"
#include "poker/showdown.h"

namespace poker {

namespace {

constexpr int opponent(int player) { return 1 - player; }

int active_player_count(const GameState& state) {
  int count = 0;
  for (int player = 0; player < kNumPlayers; ++player) {
    if (!state.folded[player]) {
      ++count;
    }
  }
  return count;
}

int players_who_can_still_bet(const GameState& state) {
  int count = 0;
  for (int player = 0; player < kNumPlayers; ++player) {
    if (!state.folded[player] && !state.all_in[player]) {
      ++count;
    }
  }
  return count;
}

void commit_chips(GameState& state, int player, int amount) {
  const int actual = std::min(amount, state.stacks[player]);
  state.stacks[player] -= actual;
  state.street_bets[player] += actual;
  state.total_committed[player] += actual;
  state.pot += actual;
  if (state.stacks[player] == 0) {
    state.all_in[player] = true;
  }
}

void deal_hole_cards(GameState& state) {
  for (int card = 0; card < kHoleCards; ++card) {
    for (int player = 0; player < kNumPlayers; ++player) {
      state.hole_cards[player][card] = Deck::draw(state.deck, state.deck_pos);
    }
  }
}

void deal_board_cards(GameState& state, int count) {
  Deck::burn(state.deck, state.deck_pos);
  for (int i = 0; i < count; ++i) {
    state.board[state.board_count++] = Deck::draw(state.deck, state.deck_pos);
  }
}

void begin_postflop_round(GameState& state) {
  state.street_bets = {0, 0};
  state.current_bet = 0;
  state.actor = 1;
  state.players_to_act = 1;
  state.min_raise_to = state.config.big_blind;
}

void compute_payoffs(GameState& state) {
  for (int player = 0; player < kNumPlayers; ++player) {
    state.payoffs[player] = state.stacks[player] - state.config.starting_stack;
  }
}

void finalize_fold(GameState& state, int winner) {
  state.stacks[winner] += state.pot;
  state.pot = 0;
  state.terminal = true;
  state.winner = winner;
  state.street = Street::kShowdown;
  compute_payoffs(state);
}

void finalize_showdown(GameState& state) {
  const auto winnings = resolve_showdown(state);
  for (int player = 0; player < kNumPlayers; ++player) {
    state.stacks[player] += winnings[player];
  }
  state.pot = 0;
  state.terminal = true;
  state.street = Street::kShowdown;

  int best = -1;
  int best_rank = 0;
  for (int player = 0; player < kNumPlayers; ++player) {
    if (state.folded[player]) {
      continue;
    }
    const int rank = evaluate_7cards(state.hole_cards[player][0], state.hole_cards[player][1],
                                     state.board[0], state.board[1], state.board[2],
                                     state.board[3], state.board[4]);
    if (best == -1 || rank < best_rank) {
      best = player;
      best_rank = rank;
    }
  }
  state.winner = best;
  compute_payoffs(state);
}

void advance_street(GameState& state) {
  if (players_who_can_still_bet(state) <= 1 && active_player_count(state) > 1) {
    while (state.street != Street::kRiver) {
      switch (state.street) {
        case Street::kPreflop:
          state.street = Street::kFlop;
          deal_board_cards(state, 3);
          break;
        case Street::kFlop:
          state.street = Street::kTurn;
          deal_board_cards(state, 1);
          break;
        case Street::kTurn:
          state.street = Street::kRiver;
          deal_board_cards(state, 1);
          break;
        default:
          break;
      }
    }
    if (state.street == Street::kRiver && state.board_count == kBoardCards) {
      finalize_showdown(state);
      return;
    }
  }

  switch (state.street) {
    case Street::kPreflop:
      state.street = Street::kFlop;
      deal_board_cards(state, 3);
      begin_postflop_round(state);
      break;
    case Street::kFlop:
      state.street = Street::kTurn;
      deal_board_cards(state, 1);
      begin_postflop_round(state);
      break;
    case Street::kTurn:
      state.street = Street::kRiver;
      deal_board_cards(state, 1);
      begin_postflop_round(state);
      break;
    case Street::kRiver:
      finalize_showdown(state);
      break;
    case Street::kShowdown:
      break;
  }
}

void close_betting_round_if_needed(GameState& state) {
  if (state.players_to_act > 0) {
    return;
  }

  const bool bets_matched = state.street_bets[0] == state.street_bets[1] ||
                            state.all_in[0] || state.all_in[1];
  if (!bets_matched) {
    return;
  }

  if (state.street == Street::kRiver) {
    finalize_showdown(state);
    return;
  }
  advance_street(state);
}

void apply_raise(GameState& state, int player, int raise_to) {
  const int previous_bet = state.current_bet;
  const int raise_size = raise_to - previous_bet;
  const int to_put = raise_to - state.street_bets[player];

  commit_chips(state, player, to_put);
  state.current_bet = raise_to;
  state.min_raise_to = raise_to + std::max(raise_size, state.config.big_blind);
  state.actor = opponent(player);
  state.players_to_act = 1;
}

}  // namespace

std::string street_name(Street street) {
  switch (street) {
    case Street::kPreflop:
      return "preflop";
    case Street::kFlop:
      return "flop";
    case Street::kTurn:
      return "turn";
    case Street::kRiver:
      return "river";
    case Street::kShowdown:
      return "showdown";
  }
  return "unknown";
}

std::string action_type_name(ActionType type) {
  switch (type) {
    case ActionType::kFold:
      return "fold";
    case ActionType::kCheck:
      return "check";
    case ActionType::kCall:
      return "call";
    case ActionType::kBet:
      return "bet";
    case ActionType::kRaise:
      return "raise";
    case ActionType::kAllIn:
      return "all-in";
  }
  return "unknown";
}

std::string action_to_string(const Action& action) {
  if (action.type == ActionType::kBet || action.type == ActionType::kRaise ||
      action.type == ActionType::kAllIn) {
    return action_type_name(action.type) + " " + std::to_string(action.amount);
  }
  return action_type_name(action.type);
}

GameState HeadsUpGame::new_hand(const GameConfig& config, std::uint64_t seed, int hand_number,
                                int button) {
  GameState state{};
  state.config = config;
  state.seed = seed;
  state.hand_number = hand_number;
  state.button = button % kNumPlayers;
  state.stacks = {config.starting_stack, config.starting_stack};
  state.deck = Deck::shuffled(seed);
  state.deck_pos = 0;

  const int sb_player = state.button;
  const int bb_player = opponent(sb_player);

  const int sb_amount = std::min(config.small_blind, state.stacks[sb_player]);
  const int bb_amount = std::min(config.big_blind, state.stacks[bb_player]);

  commit_chips(state, sb_player, sb_amount);
  commit_chips(state, bb_player, bb_amount);

  deal_hole_cards(state);

  state.street = Street::kPreflop;
  state.actor = sb_player;
  state.current_bet = state.street_bets[bb_player];
  state.min_raise_to = state.current_bet + (state.current_bet - state.street_bets[sb_player]);
  state.players_to_act = 1;

  if (state.all_in[sb_player] && state.all_in[bb_player]) {
    advance_street(state);
  }

  return state;
}

int HeadsUpGame::to_call(const GameState& state, int player) {
  return std::max(0, state.current_bet - state.street_bets[player]);
}

int HeadsUpGame::effective_stack(const GameState& state, int player) {
  return state.stacks[player];
}

std::vector<Action> HeadsUpGame::legal_actions(const GameState& state) {
  if (state.terminal) {
    return {};
  }

  std::vector<Action> actions;
  const int player = state.actor;
  const int call_amount = to_call(state, player);
  const int stack = state.stacks[player];
  const int max_raise_to = state.street_bets[player] + stack;

  if (call_amount > 0) {
    actions.push_back({ActionType::kFold, 0});
  }

  if (call_amount == 0) {
    actions.push_back({ActionType::kCheck, 0});
  }

  if (call_amount > 0 && stack > 0) {
    if (call_amount >= stack) {
      actions.push_back({ActionType::kAllIn, max_raise_to});
    } else {
      actions.push_back({ActionType::kCall, state.current_bet});
      if (max_raise_to > state.current_bet) {
        actions.push_back({ActionType::kAllIn, max_raise_to});
      }
    }
  }

  if (call_amount == 0 && stack > 0) {
    const int min_bet = std::min(state.min_raise_to, max_raise_to);
    if (min_bet <= max_raise_to && min_bet > 0) {
      actions.push_back({ActionType::kBet, min_bet});
    }
    if (max_raise_to > state.current_bet) {
      if (max_raise_to < min_bet || max_raise_to > min_bet) {
        if (max_raise_to >= state.min_raise_to) {
          actions.push_back({ActionType::kRaise, state.min_raise_to});
        }
        if (max_raise_to != state.min_raise_to) {
          actions.push_back({ActionType::kAllIn, max_raise_to});
        }
      }
    }
  } else if (call_amount > 0 && stack > call_amount) {
    if (max_raise_to >= state.min_raise_to) {
      actions.push_back({ActionType::kRaise, state.min_raise_to});
    }
    if (max_raise_to > state.current_bet && max_raise_to < state.min_raise_to) {
      actions.push_back({ActionType::kAllIn, max_raise_to});
    }
  }

  return actions;
}

GameState HeadsUpGame::apply(const GameState& state, const Action& action) {
  if (state.terminal) {
    throw std::logic_error("cannot act on terminal state");
  }

  const auto legal = legal_actions(state);
  if (std::find(legal.begin(), legal.end(), action) == legal.end()) {
    throw std::invalid_argument("illegal action: " + action_to_string(action));
  }

  GameState next = state;
  const int player = next.actor;

  switch (action.type) {
    case ActionType::kFold:
      next.folded[player] = true;
      finalize_fold(next, opponent(player));
      return next;

    case ActionType::kCheck:
      next.players_to_act -= 1;
      next.actor = opponent(player);
      close_betting_round_if_needed(next);
      return next;

    case ActionType::kCall: {
      const int call_amount = to_call(next, player);
      commit_chips(next, player, call_amount);
      next.players_to_act -= 1;
      next.actor = opponent(player);
      close_betting_round_if_needed(next);
      return next;
    }

    case ActionType::kBet:
    case ActionType::kRaise:
      apply_raise(next, player, action.amount);
      close_betting_round_if_needed(next);
      return next;

    case ActionType::kAllIn: {
      const int raise_to = action.amount;
      if (raise_to <= next.current_bet) {
        const int call_amount = to_call(next, player);
        commit_chips(next, player, call_amount);
        next.players_to_act -= 1;
        next.actor = opponent(player);
      } else if (raise_to < next.min_raise_to) {
        const int call_amount = to_call(next, player);
        commit_chips(next, player, call_amount);
        next.current_bet = std::max(next.current_bet, next.street_bets[player]);
        next.players_to_act -= 1;
        next.actor = opponent(player);
      } else {
        apply_raise(next, player, raise_to);
      }
      close_betting_round_if_needed(next);
      return next;
    }
  }

  throw std::logic_error("unhandled action type");
}

bool HeadsUpGame::is_terminal(const GameState& state) { return state.terminal; }

std::array<int, kNumPlayers> HeadsUpGame::payoffs(const GameState& state) {
  if (!state.terminal) {
    throw std::logic_error("hand is not complete");
  }
  return state.payoffs;
}

}  // namespace poker
