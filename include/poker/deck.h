#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "poker/card.h"

namespace poker {

class Deck {
 public:
  static std::array<int, 52> ordered();
  static std::array<int, 52> shuffled(std::uint64_t seed);

  static int draw(std::array<int, 52>& deck, int& position);
  static void burn(std::array<int, 52>& deck, int& position);
};

}  // namespace poker
