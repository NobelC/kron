#pragma once 

#include <cstdint>
#include <string>

enum class typetoken : uint8_t{
  command,
  positional,
  option
};

struct TokenData{
  std::string argument;
  std::string value;
  typetoken type;
};
