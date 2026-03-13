#pragma once
#include "token-raw-metadata.hpp"
#include <vector>

struct GroupToken{
  Token command;
  std::vector<Token> options;
  std::vector<Token> positional;
};
