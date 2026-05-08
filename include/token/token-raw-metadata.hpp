#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

enum class TypeToken : std::uint8_t {
  OPTION_NOT_NORMALIZED,
  OPTION_NORMALIZED,

  LITERAL,
  POSITIONAL,
};

struct Token {
  TypeToken type;
  std::string_view name;
  std::string_view value;

  bool operator==(const Token &other) const noexcept {
    return type == other.type && name == other.name;
  }
};

struct transparent_hash {
  using is_transparent = void;
  std::size_t operator()(std::string_view strsv) const {
    return std::hash<std::string_view>{}(strsv);
  }
  std::size_t operator()(const std::string &str) const {
    return std::hash<std::string_view>{}(str);
  }
  std::size_t operator()(const char *chr) const {
    return std::hash<std::string_view>{}(chr);
  }
};
