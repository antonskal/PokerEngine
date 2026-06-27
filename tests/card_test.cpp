#include <gtest/gtest.h>

#include "poker/card.h"

TEST(CardTest, ParsesNotation) {
  poker::Card ace_spades("As");
  EXPECT_EQ(ace_spades.to_string(), "As");
  EXPECT_EQ(ace_spades.id(), 51);
}

TEST(CardTest, Equality) {
  EXPECT_EQ(poker::Card("Kd"), poker::Card("Kd"));
  EXPECT_NE(poker::Card("Kd"), poker::Card("Kc"));
}
