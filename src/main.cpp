#include "tokenizer/tokenizer.hpp"
#include <string>
#include <vector>
#include <iostream>

int main(int argc , char*argv[]){
  std::vector<std::string> argument_raw(argv + 1 , argv + argc );
  std::vector<TokenData> token_raw = tokenizer(argument_raw);

  std::cout << "complete\n";

  return 0;
}
