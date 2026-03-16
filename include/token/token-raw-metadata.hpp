#pragma once
#include "type-token.hpp"
#include <string>

struct Token{
  TypeToken type;
  std::string name;
  std::string value;

  bool operator==(const Token& other) const noexcept{
      return name == other.name;
  }
};
