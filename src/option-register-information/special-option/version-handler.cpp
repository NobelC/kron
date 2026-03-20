#include "../../../include/special-option/version-option.hpp"
#include <iostream>
#include <format>

void VERSION_HANDLER(){
  std::cout << std::format("kron {}\n", KRON_VERSION);
  std::cout << std::format("Copyright (C) 2026 The Nobel Void\n");
  std::cout << std::format("License MIT\n");
}
