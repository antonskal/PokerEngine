#include "poker/session.h"

namespace poker {

bool save_session(const GameState& state, const std::string& path) {
  std::ofstream out(path, std::ios::binary);
  if (!out) {
    return false;
  }
  out.write(reinterpret_cast<const char*>(&state), sizeof(GameState));
  return static_cast<bool>(out);
}

bool load_session(GameState& state, const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    return false;
  }
  in.read(reinterpret_cast<char*>(&state), sizeof(GameState));
  return static_cast<bool>(in);
}

}  // namespace poker
