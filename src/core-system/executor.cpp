#include "../include/core-hpp/executor.hpp"
#include "../include/token/group-token.hpp"
#include "../include/command/command-implementation.hpp"
#include "../../include/special-option/help-option.hpp"
#include "token/token-raw-metadata.hpp"
#include <algorithm>

void executor(const GroupToken& token_group){
  const auto& data_command = GetCommandData(token_group.command.name);
  if(token_group.options.empty()){
    std::string empty;
    HELP_HANDLER(empty);
  }
  else{
    bool helper_call = std::ranges::any_of(token_group.options, [](const Token& t){
          return t.name == "--help";
        });

    if(helper_call){
      HELP_HANDLER(token_group.command.name);
      return;
    }
    data_command->handler(token_group);
  }
}
