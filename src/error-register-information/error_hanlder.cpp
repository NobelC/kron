#include "../../include/error/error_hanlder.hpp"
#include "../../include/option/option-raw-metadata.hpp"
#include <iostream>
#include <string>
#include <format>

void COMMAND_NOT_FOUND(const std::string& trigger){
  std::cerr << std::format("ERROR : COMANDO [{}] NO ENCONTRADO", trigger);
}
void OPTION_NOT_FOUND(const std::string& trigger){
  std::cerr << std::format("ERROR : OPCION [{}] NO ENCONTRADA" , trigger);
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
