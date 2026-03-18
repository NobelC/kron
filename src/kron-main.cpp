#include "command/command-implementation.hpp"
#include "../include/option/option-implementation.hpp"
#include "../include/core-hpp/tokenization.hpp"
#include "../include/core-hpp/parsing.hpp"
#include "../include/core-hpp/validator.hpp"
#include "core-hpp/executor.hpp"
#include "token/group-token.hpp"
#include <chrono>
#include <string>
#include <vector>
#include <iostream>


int main(int argc , char* argv[]){
  auto start = std::chrono::high_resolution_clock::now();
  CreatedOptionData();
  CreatedCommandData();

  std::vector<std::string> arguments_raw(argv + 1 , argv + argc);
  std::vector<Token> group_token_raw = tokenization(arguments_raw);
  GroupToken group_token_final = parsing(group_token_raw);
  if(!ValidationGroupToken(group_token_final)){
    return 1;
  }
  executor(group_token_final);
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "\n";
  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";
  
  return 0 ;
}
