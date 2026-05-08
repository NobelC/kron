#include "../../include/core-hpp/tokenization.hpp"
#include "../../include/token/token-raw-metadata.hpp"
#include <string>
#include <vector>
std::vector<Token> tokenization(const std::vector<std::string> &arguments) {
  std::vector<Token> argument_raw;
  argument_raw.reserve(arguments.size());
  bool only_positional = false;
  int real_count = 0;

  for (size_t i = 0; i < arguments.size(); i++) {

    if (arguments[i].empty() || arguments[i] == " ") {
      continue;
    }
    real_count++;

    // Register the first non-hyphenated element as the command
    if (real_count == 1 && !arguments[i].starts_with("-") && !only_positional) {
      argument_raw.emplace_back(Token{
          .type = TypeToken::COMMAND,
          .name = arguments[i],
          .value = "",
      });
      continue;
    }

    // Detect the delimiter "--" that forces all subsequent arguments to be
    // treated as positional
    if (arguments[i] == "--") {
      only_positional = true;
      continue;
    }

    if (arguments[i] == "-") {
      argument_raw.emplace_back(Token{
          .type = TypeToken::POSITIONAL,
          .name = arguments[i],
          .value = "",
      });
    }

    // Handle cases where only positional arguments are allowed (after "--")
    if (only_positional) {
      argument_raw.emplace_back(Token{
          .type = TypeToken::POSITIONAL,
          .name = arguments[i],
          .value = "",
      });
      continue;
    }

    // Identify and store options (starting with '-') that are not yet
    // normalized
    if (arguments[i].starts_with("-")) {
      argument_raw.emplace_back(Token{
          .type = TypeToken::OPTION_NOT_NORMALIZED,
          .name = arguments[i],
          .value = "",
      });
      continue;
    }

    // Store remaining arguments as literal tokens
    argument_raw.emplace_back(Token{
        .type = TypeToken::LITERAL,
        .name = arguments[i],
        .value = "",
    });
  }

  return argument_raw;
}
