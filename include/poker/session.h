#pragma once

#include <fstream>
#include <string>

#include "poker/types.h"

namespace poker {

bool save_session(const GameState& state, const std::string& path);
bool load_session(GameState& state, const std::string& path);

}  // namespace poker
