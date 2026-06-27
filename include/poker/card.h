#pragma once

#include <phevaluator/card.h>

#include <string>

namespace poker {

class Card {
 public:
  Card() = default;
  explicit Card(int id) : card_(id) {}
  explicit Card(const char* notation) : card_(notation) {}
  explicit Card(const std::string& notation) : card_(notation) {}

  int id() const { return static_cast<int>(card_); }
  std::string to_string() const { return static_cast<std::string>(card_); }

  bool operator==(const Card& other) const { return id() == other.id(); }
  bool operator!=(const Card& other) const { return !(*this == other); }

  phevaluator::Card pheval() const { return card_; }

 private:
  phevaluator::Card card_;
};

}  // namespace poker
