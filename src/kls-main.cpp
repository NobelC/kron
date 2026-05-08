#include "../include/command/command-implementation.hpp"
#include "../include/core-hpp/executor.hpp"
#include "../include/core-hpp/parsing.hpp"
#include "../include/core-hpp/tokenization.hpp"
#include "../include/core-hpp/validator.hpp"
#include "../include/option/option-implementation.hpp"
#include "../include/special-option/help-option.hpp"
#include "../include/token/group-token.hpp"
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
  CreatedOptionData();
  CreatedCommandData();

  std::vector<std::string> arguments_raw(argv + 1, argv + argc);
  if (arguments_raw.empty()) {
    HELP_HANDLER("empty");
    return 1;
  }
  std::vector<Token> group_token_raw = tokenization(arguments_raw);
  GroupToken group_token_final = parsing(group_token_raw);
  if (!ValidationGroupToken(group_token_final)) {
    return 1;
  }
  executor(group_token_final);

  return 0;
}
