#include "../../include/error/error_hanlder.hpp"
#include "../../include/option/option-raw-metadata.hpp"
#include <iostream>
#include <string>
#include <format>

void COMMAND_NOT_FOUND(const std::string& trigger){
  std::cerr << std::format("\n  [ERROR] Unknown command: '{}'\n", trigger);
  std::cerr << std::format("  Run 'kron --help' to see available commands.\n\n");
}

void OPTION_NOT_FOUND(const std::string& trigger){
  std::cerr << std::format("\n  [ERROR] Unknown option: '{}'\n", trigger);
  std::cerr << std::format("  Run 'kron <command> --help' to see available options.\n\n");
}

void OPTION_NEED_VALUE(const std::string& trigger, const TypeDataReceived& type_data){
  std::cerr << std::format("\n  [ERROR] Option '{}' requires a value.\n", trigger);
  std::cerr << std::format("  Expected format: ");
  switch (type_data){
    case TypeDataReceived::DATE:
      std::cerr << "YYYY-MM-DD  (e.g. 2024-01-31)\n\n";
      break;
    case TypeDataReceived::EXTENSION:
      std::cerr << ".ext        (e.g. .cpp, .docx)\n\n";
      break;
    case TypeDataReceived::SIZE:
      std::cerr << "<n><unit>   (e.g. 1024MB, 10GB)\n\n";
      break;
    case TypeDataReceived::NONE:
      std::cerr << "(none — this should not happen)\n\n";
      break;
  }
}

void OPTION_NOT_AVAIBLE_FOR_COMMAND(const std::string& trigger, const std::string& command){
  std::cerr << std::format("\n  [ERROR] Option '{}' is not available for command '{}'.\n", trigger, command);
  std::cerr << std::format("  Run 'kron {} --help' to see supported options.\n\n", command);
}

void OPTION_CONFLICT_WITH(const std::string& trigger){
  std::cerr << std::format("\n  [ERROR] Option '{}' conflicts with another provided option.\n\n", trigger);
}

void INCORRECT_NUMBER_OF_POSITIONAL_NUMBER(const std::string& command, const int& number, const int& min, const int& max){
  std::cerr << std::format("\n  [ERROR] Wrong number of arguments for command '{}'.\n\n", command);
  std::cerr << std::format("  Provided : {}\n", number);
  std::cerr << std::format("  Minimum  : {}\n", min);
  std::cerr << std::format("  Maximum  : {}\n\n", max);
}

void OPTION_REQUIERES_OPTION(const std::string& option,const std::string& requieres){
  std::cerr << std::format("[ERROR] The '{}' option requires the '{}' option to work correctly",option, requieres);
}

void DEFAULT_VALUE_OPTION(const std::string& option, const int& default_value){
  std::cerr << std::format("[WARNING] The '{}' option requires a value, since it does not have one in the input, its default value will be used '{}'", option, default_value);
}

void EQUAL_DIRECTION(){
  std::cerr << std::format("[ERROR] The addresses entered are the same; the process will be skipped\n");
}

void ORIGIN_DIRECTORY_NOT_VALITED(const std::string& dir){
  std::cerr << std::format("[ERROR] The source path is invalid\n", dir);

}
