#include <gtest/gtest.h>

#include <set>

#include "poker/deck.h"

TEST(DeckTest, OrderedDeckHasUniqueCards) {
  const auto deck = poker::Deck::ordered();
  std::set<int> seen;
  for (int card : deck) {
    EXPECT_TRUE(seen.insert(card).second);
  }
  EXPECT_EQ(seen.size(), 52);
}

TEST(DeckTest, ShuffledDeckIsDeterministic) {
  const auto first = poker::Deck::shuffled(42);
  const auto second = poker::Deck::shuffled(42);
  EXPECT_EQ(first, second);
}

TEST(DeckTest, DifferentSeedsProduceDifferentOrders) {
  const auto first = poker::Deck::shuffled(1);
  const auto second = poker::Deck::shuffled(2);
  EXPECT_NE(first, second);
}
