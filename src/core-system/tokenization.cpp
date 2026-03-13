#include "../../include/core-hpp/tokenization.hpp"
#include "../../include/token/token-raw-metadata.hpp"
#include <string>
#include <vector>
std::vector<Token> tokenization(const std::vector<std::string>& arguments){
  std::vector<Token> argument_raw;
  argument_raw.reserve(arguments.size());
  bool only_positional = false;

  for(const auto& arg : arguments){
    //Guardar primer (que no tenga guion) elemento como comando 
    if(argument_raw.empty() && !arg.starts_with("-") && !only_positional){
      argument_raw.emplace_back(Token{
          .type = TypeToken::COMMAND,
          .name = arg,
          .value = "",
          });
      continue;
    }
    
    //Detectar cuando se introdujo el "solo posicinales"
    if(arg == "--"){
      only_positional = true;
      continue;
    }
    
    //guardar cuando se active solo posicionales 
    if(only_positional){
      argument_raw.emplace_back(Token{
          .type = TypeToken::POSITIONAL,
          .name = arg,
          .value = "",
          });
      continue;
    }
    
    //guardar opciones cortas | largas | con valor explicito
    if(arg.starts_with("-")){
      argument_raw.emplace_back(Token{
          .type = TypeToken::OPTION_NOT_NORMALIZED,
          .name = arg,
          .value = "",
          });
      continue;
    }
    
    //Guardar los literales
    argument_raw.emplace_back(Token{
        .type = TypeToken::LITERAL,
        .name = arg,
        .value = "",
        });
  }

  return argument_raw;
}
