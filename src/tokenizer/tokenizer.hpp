#pragma once

#include "../../general-utilities/TokenData.hpp"
#include <vector>
#include <string>

std::vector<TokenData> tokenizer(const std::vector<std::string>& argument_raw);
