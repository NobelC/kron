#include "../include/core-hpp/executor.hpp"
#include "../include/token/group-token.hpp"
#include "../include/command/command-implementation.hpp"

void executor(const GroupToken& token_group){
  const auto& data_command = GetCommandData(token_group.command.name);
  data_command->handler(token_group);
}
