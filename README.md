# PokerEngine

A C++ heads-up No-Limit Hold'em (NLHE) poker engine built for correctness and solver integration. Hand strength is evaluated with [Henry Lee's PokerHandEvaluator](https://github.com/HenryRLee/PokerHandEvaluator) (PH Evaluator), a fast perfect-hash 7-card evaluator.

The long-term goal is to use this engine as the game tree for a Counterfactual Regret Minimization (CFR) GTO solver. The current focus is a reliable, testable game layer: dealing, betting, all-ins, side pots, and showdown.

## Features

- **Heads-up NLHE** — Button/SB (player 0) vs BB (player 1)
- **Full betting** — fold, check, call, bet, raise, all-in (including short all-ins)
- **Deterministic dealing** — seeded shuffle for reproducible hands
- **Showdown** — 7-card evaluation, side pots, split pots
- **Interactive CLI** — play a hand in the terminal
- **Library API** — functional state transitions suitable for tree search / CFR

## Requirements

- C++17 compiler (Clang or GCC)
- CMake 3.14+
- Git (for submodules)

On macOS with Homebrew LLVM:

```bash
brew install cmake llvm
```

## Clone

```bash
git clone --recurse-submodules https://github.com/antonskal/PokerEngine.git
cd PokerEngine
```

If you already cloned without submodules:

```bash
git submodule update --init --recursive
```

## Build

### CMake (recommended)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
  -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang

cmake --build build -j
```

Targets:

| Target | Description |
|--------|-------------|
| `poker_engine` | Static library |
| `play` | Interactive terminal client |
| `poker_engine_tests` | Unit tests (GoogleTest) |

### Makefile (alternative)

```bash
make CXX=/opt/homebrew/opt/llvm/bin/clang++ CC=/opt/homebrew/opt/llvm/bin/clang -j
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

Or run the test binary directly:

```bash
./build/poker_engine_tests
```

## Play in the terminal

```bash
./build/play
```

You are the Button / Small Blind. The villain is a simple calling station (checks and calls).

| Input | Action |
|-------|--------|
| `call`, `c` | Call |
| `check`, `x` | Check |
| `fold`, `f` | Fold |
| `bet 4` | Bet to 4 |
| `raise 6` | Raise to 6 |
| `all-in` | Shove |
| `3` | Choose action by index |
| `help` | List legal actions |
| `quit` | Exit |

Optional fixed seed for reproducible deals:

```bash
./build/play interactive 4242
```

## API

```cpp
#include "poker/poker.h"

poker::GameConfig config;
config.small_blind = 1;
config.big_blind = 2;
config.starting_stack = 200;

auto state = poker::HeadsUpGame::new_hand(config, /*seed=*/42);

while (!poker::HeadsUpGame::is_terminal(state)) {
  auto actions = poker::HeadsUpGame::legal_actions(state);
  state = poker::HeadsUpGame::apply(state, actions[0]);
}

auto payoffs = poker::HeadsUpGame::payoffs(state);  // net chips per player
```

`payoffs` is zero-sum: `payoffs[0] + payoffs[1] == 0`.

## Project layout

```
include/poker/     Public headers
src/               Engine implementation
tests/             GoogleTest suite
tools/play.cpp     Interactive CLI
third_party/       PokerHandEvaluator submodule
```

## Roadmap

- [ ] Button rotation across hands
- [ ] Serialized action history / hand replayer
- [ ] Bet-size abstraction for CFR
- [ ] Information-set encoding
- [ ] CFR solver

## Third-party

Hand evaluation is provided by [PokerHandEvaluator](https://github.com/HenryRLee/PokerHandEvaluator) (Apache 2.0), vendored as a git submodule at `third_party/PokerHandEvaluator`.

## License

PokerEngine source in this repository is provided as-is. See PokerHandEvaluator for the evaluator license.
