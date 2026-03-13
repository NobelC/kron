#pragma once
#include <string>
#include "../option/option-raw-metadata.hpp"

void COMMAND_NOT_FOUND(const std::string& trigger);
void OPTION_NOT_FOUND(const std::string& trigger);
void OPTION_NEED_VALUE(const std::string& trigger,const TypeDataReceived& type_data);
