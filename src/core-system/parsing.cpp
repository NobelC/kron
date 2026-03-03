#include "../include/core-hpp/parsing.hpp"
#include "command/command-raw-metadata.hpp"
#include "option/option-implementation.hpp"
#include "option/option-raw-metadata.hpp"
#include "token/token-raw-metadata.hpp"
#include <string>
#include <unordered_set>
#include <vector>

namespace {

}

GroupToken parsing(const std::vector<Token>& token_raw){
  GroupToken token_group;
  std::unordered_set<std::string> see_argument;

  for(size_t i = 0 ; i < token_raw.size() ; i++){
    const auto& object = token_raw[i];
    
    if(see_argument.contains(object.name)){
      continue;
    }

    //Tratamiento de opciones
    if(object.type == TypeToken::OPTION_NOT_NORMALIZED){
      const auto& option_data = GetOptionData(object.name);
      //Verificar si un token de tipo opcion necesita valor asignado
      // 1. en caso de tener se omite y se guarda como normalizado
      // 2. en caso de tener un signo igual se separa y se guarda
      // 3. en caso de no tener se da el siguiente token como su valor aninado si es un literal
      // 4. en caso de que ninguna de estas se cumpla se retorna un error
      if(option_data->data_type != TypeDataReceived::NONE){
        auto its = object.name.find('=');
        if(its != std::string::npos){
          token_group.options.emplace_back(Token{
              .type = TypeToken::OPTION_NORMALIZED,
              .name = option_data->normalized_name,
              .value = object.name.substr(its + 1),
              });
          continue;
        }
        if( i + i < token_raw.size() && token_raw[i + 1].type == TypeToken::LITERAL){
          token_group.options.emplace_back(Token{
              .type = TypeToken::OPTION_NORMALIZED,
              .name = option_data->normalized_name,
              .value = token_raw[i + 1].name,
              });
          see_argument.insert(token_raw[i+1].name);
          continue;
        }
      }
      //Verificar si la opcion es del tipo -abc o -abc=VALOR (opcions cortas agrupadas)
      //1. si no tiene iguales simplemente se separa y se normalizada
      // 2. en caso de tener iguales se separa y la ultima flag se robara el valor despues del signo
      if(!object.name.starts_with("--") && object.name.starts_with('-') && object.name.size() > 2){
        auto its = object.name.find("=");
        //Caso 1 :
        if(its == std::string::npos){
          for(const auto& element : object.name){
            if(element == '-'){
              continue;
            }
            const auto normalized_token = GetOptionData(std::string("-") + element);
            token_group.options.emplace_back(Token{
                .type = TypeToken::OPTION_NORMALIZED,
                .name = normalized_token->normalized_name,
                .value = "",
                });
          }
          continue;
        }
        //Caso 2:

      }

    }

    //Verificar la existencia de un comando 
    if(object.type == TypeToken::COMMAND){

    }

  }

  return token_group;
}
