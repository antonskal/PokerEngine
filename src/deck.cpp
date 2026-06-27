#include "poker/deck.h"

#include <algorithm>
#include <array>

namespace poker {

namespace {

std::uint64_t splitmix64(std::uint64_t& state) {
  std::uint64_t z = (state += 0x9e3779b97f4a7c15ULL);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}

}  // namespace

std::array<int, 52> Deck::ordered() {
  std::array<int, 52> deck{};
  for (int i = 0; i < 52; ++i) {
    deck[i] = i;
  }
  return deck;
}

std::array<int, 52> Deck::shuffled(std::uint64_t seed) {
  auto deck = ordered();
  std::uint64_t rng = seed == 0 ? 1 : seed;
  for (int i = 51; i > 0; --i) {
    const int j = static_cast<int>(splitmix64(rng) % static_cast<std::uint64_t>(i + 1));
    std::swap(deck[i], deck[j]);
  }
  return deck;
}

int Deck::draw(std::array<int, 52>& deck, int& position) {
  return deck[position++];
}

void Deck::burn(std::array<int, 52>& deck, int& position) {
  (void)deck;
  ++position;
}

}  // namespace poker
