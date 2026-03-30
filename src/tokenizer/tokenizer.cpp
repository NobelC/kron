#include "tokenizer.hpp"
#include "../../general-utilities/TokenData.hpp"
#include <vector>
#include <string>

std::vector<TokenData> tokenizer(const std::vector<std::string>& argument_raw){
  
  std::vector<TokenData> token_raw;
  token_raw.reserve(argument_raw.size());
  bool command_found = false;
  bool only_positional = false;
  
  for(const auto& argument : argument_raw)
  {

    if(!command_found && !argument.starts_with("-")){
      token_raw.emplace_back(TokenData{
            .argument = argument,
            .value = "",
            .type = typetoken::command
      });
      command_found = true;
      continue;
    }
    if(argument == "--"){
      only_positional = true;
      continue;
    }
    if((only_positional && command_found) || (!argument.starts_with("-"))){
      token_raw.emplace_back(TokenData{
          .argument = argument,
          .value = "",
          .type = typetoken::positional
      });
      continue;
    }
    if(argument.starts_with("-")){
      token_raw.emplace_back(TokenData{
          .argument = argument,
          .value = "",
          .type = typetoken::option,
      });
    }
  }
  return token_raw;
}

