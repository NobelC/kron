#include "../../include/error/error_hanlder.hpp"
#include "../../include/option/option-raw-metadata.hpp"
#include <iostream>
#include <string>
#include <format>

void COMMAND_NOT_FOUND(const std::string& trigger){
  std::cerr << std::format("ERROR : COMANDO [{}] NO ENCONTRADO\n", trigger);
}
void OPTION_NOT_FOUND(const std::string& trigger){
  std::cerr << std::format("ERROR : OPCION [{}] NO ENCONTRADA\n" , trigger);
}

void OPTION_NEED_VALUE(const std::string& trigger,const TypeDataReceived& type_data){
  std::cerr << std::format("ERROR : OPCION [{}] NECESITA VALOR DEL TIPO: ", trigger);

  switch (type_data){
    case TypeDataReceived::DATE:
      std::cerr << "YYYY-MM-DD (DATE)\n";
      break;
    case TypeDataReceived::EXTENSION:
      std::cerr << ".CPP | .docx (EXTENSION)\n";
      break;
    case TypeDataReceived::SIZE:
      std::cerr << "1028MB | 10GB (SIZE)\n";
      break;
    case TypeDataReceived::NONE:
      std::cerr << "Aqui no deberia entrar\n";
      break;
  }
}

void OPTION_NOT_AVAIBLE_FOR_COMMAND(const std::string& trigger, const std::string& command){
  std::cerr << std::format("ERROR : OPCION [{}] no disponible para el comando [{}]\n" , trigger, command);
}

void OPTION_CONFLICT_WITH(const std::string& trigger){
  std::cerr << std::format("ERROR : LA OPCION [{}] TIENE CONFLICTOS CON OTRA OPCION LLAMADA\n" , trigger);
}

void INCORRECT_NUMBER_OF_POSITIONAL_NUMBER(const std::string& command,const int& number , const int& min , const int& max){
  std::cerr << std::format("ERROR : CANTIDAD DE POSICIONALES INCORRECTA PARA EL COMANDO [{}]\n", command);
  std::cerr << std::format("POSICIONALES INTRODUCIDOS : {:<10}\n", number);
  std::cerr << std::format("POSICIONALES MINIMOS : {:<10}\n", min);
  std::cerr << std::format("POSICIONALES MAXIMOS : {:<10}\n", max);
}
