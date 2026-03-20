#include "../../include/core-hpp/validator.hpp"
#include "../../include/command/command-implementation.hpp"
#include "error/error_hanlder.hpp"
#include "option/option-implementation.hpp"
#include "../../include/option/option-raw-metadata.hpp"
#include "special-option/help-option.hpp"
#include "special-option/version-option.hpp"
#include "token/token-raw-metadata.hpp"
#include <algorithm>
#include <chrono>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <string_view>
#include <unordered_set>
#include <sstream>

namespace {
  bool DATE_VALIDATED(const std::string& date_str){
    std::istringstream str_validated(date_str);
    std::chrono::sys_days parser_date;

    str_validated >> std::chrono::parse("%F", parser_date);
    
    if(str_validated.fail()) {
      return false;
    }
    
    std::chrono::year_month_day year_month_day(parser_date);


    if(!year_month_day.ok()){
      return false;
    }

    return true;
  }

  bool SIZE_VALIDATED(std::string_view size_str){
    //El formato para el tamano de archivos es variado
    //1028 MB | 1028 gb 
    //para valores numericos (1028) sin especificar medida se usaran los MB como estandar
    if(size_str.empty()) {return false;}

    auto digits = size_str | std::views::take_while(::isdigit);
    const auto num_digits = static_cast<size_t>(std::ranges::distance(digits));

    if(num_digits == 0) {return false;}
    if(num_digits == size_str.size()) {return true;}

    const auto unit = size_str.substr(num_digits);
    constexpr std::array valid_units = {  // ← std::array en vez de C-style array
        std::string_view{"B"},  std::string_view{"KB"},
        std::string_view{"MB"}, std::string_view{"GB"},
        std::string_view{"TB"}, std::string_view{"b"},
        std::string_view{"kb"}, std::string_view{"mb"},
        std::string_view{"gb"}, std::string_view{"tb"}
    };

    return std::ranges::any_of(valid_units , [unit](std::string_view u ){
        return u == unit;
    });
  }

  bool EXTENSION_VALIDATED(std::string_view extension_str){
    if(extension_str.starts_with(".")) {extension_str.remove_prefix(1);}
    if(extension_str.empty()) {return false;}
    return std::ranges::none_of(extension_str,[](char c){
        return c == '/' || c == '\\' || c ==  ' ';

    });
  }
}

bool ValidationGroupToken(GroupToken& group_raw){

  //En el parser ya se comprueba si el comando existe y el tokenizador se encarga de que exista un 
  //comando aunque este luego no sea validator
  const auto data_command = GetCommandData(group_raw.command.name);

  bool version = std::ranges::any_of(group_raw.options, [](const Token& t){
        return t.name == "--version";
      });
  bool help_only = std::ranges::any_of(group_raw.options, [](const Token& t){
        return t.name == "--help";
      });

  if(version){
    VERSION_HANDLER();
    return false;
  }

  if(help_only && data_command == nullptr){
    HELP_HANDLER("empty");
    return false;
  }


  if(data_command == nullptr){
    COMMAND_NOT_FOUND(group_raw.command.name);
    return false;
  }
  if(static_cast<int>(group_raw.positional.size()) < data_command->minimun_positional || 
      static_cast<int>(group_raw.positional.size()) > data_command->maximun_positional){
    INCORRECT_NUMBER_OF_POSITIONAL_NUMBER(data_command->default_name, 
        static_cast<int>(group_raw.positional.size()), 
        data_command->minimun_positional, 
        data_command->maximun_positional);
    return false;
  }


  std::unordered_set<std::string> option_for_command;
  option_for_command.reserve(data_command->option_avaible.size());

  for(const auto& option : data_command->option_avaible){
    option_for_command.insert(option);
  }
 
  //Eliminar opciones duplicadas (sera valida solo la primera opcion introducida)
  std::unordered_set<std::string> eliminated_duplicated_option;
  eliminated_duplicated_option.reserve(group_raw.options.size());
  std::vector<Token> option_not_duplicated;
  option_not_duplicated.reserve(group_raw.options.size());

  for(const auto& option : group_raw.options){
    if(eliminated_duplicated_option.contains(option.name)){
      continue;
    }
    option_not_duplicated.emplace_back(option);
    eliminated_duplicated_option.insert(option.name);
  }

  group_raw.options = option_not_duplicated;
  std::vector<std::string> to_erase;
  for(const auto& element : group_raw.options){
    //Comprobar si un elemento esta dentro de las opciones disponibles para el comando
    if(!option_for_command.contains(element.name)){
      OPTION_NOT_AVAIBLE_FOR_COMMAND(element.name, data_command->default_name);
      to_erase.emplace_back(element.name);
      continue;
    }
    //Borramos ese elemento para no comprobar de nuevo el mismo elemento
    option_for_command.erase(element.name);
    const auto& option_data = GetOptionData(element.name);
    //Comprobamos si alguna opcion tiene conflictos con otra
    for(const auto& conflict_option : option_data->conflict_name){
      if(eliminated_duplicated_option.contains(conflict_option)){
        OPTION_CONFLICT_WITH(conflict_option);
        return false;
      }
    }
    for(const auto& requieres_option : option_data->requieres_name){
      if(!eliminated_duplicated_option.contains(requieres_option)){
        OPTION_REQUIERES_OPTION(option_data->normalized_name,requieres_option);
        return false;
      }
    }

    //Validar tipo de dato que debe recibir:
    switch(option_data->data_type){
      case TypeDataReceived::DATE:
        if(!DATE_VALIDATED(element.value)) {

          return false;
        }
        break;
      case TypeDataReceived::EXTENSION:
        if(!EXTENSION_VALIDATED(element.value)){
          return false;
        };
        break;
      case TypeDataReceived::SIZE:
        if(!SIZE_VALIDATED(element.value)){
          return false;
        };
        break;
      case TypeDataReceived::NONE:
        break;
    }

    //Validar si estan las opciones que requiere otra opciones
    //Implementacion cuando alguna lo requiera porque por ahora ninguna requiere 
  }

  std::erase_if(group_raw.options, [&](const Token& t){
      return std::ranges::any_of(to_erase, [&](const std::string& name){
          return name == t.name;
          });
      });
  //ordenar las option en base a su cronologia de ejecucion
  std::ranges::sort(group_raw.options,[](const Token& a, const Token& b){
      return static_cast<uint8_t>(GetOptionData(a.name)->category) <
             static_cast<uint8_t>(GetOptionData(b.name)->category);
      });
  


  return true;
}
