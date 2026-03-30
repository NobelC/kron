#pragma once
#include "TokenData.hpp"
#include <span>
#include <string_view>
#include <vector>

struct TokenGroupData{
  std::string_view command;
  std::vector<std::string_view> positional;
  std::span<TokenData> option;
};
