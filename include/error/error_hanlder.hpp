#pragma once
#include <string>
#include "../option/option-raw-metadata.hpp"

void COMMAND_NOT_FOUND(const std::string& trigger);
void OPTION_NOT_FOUND(const std::string& trigger);
void OPTION_NEED_VALUE(const std::string& trigger,const TypeDataReceived& type_data);
void OPTION_NOT_AVAIBLE_FOR_COMMAND(const std::string& trigger, const std::string& command);
void OPTION_CONFLICT_WITH(const std::string& trigger);
void INCORRECT_NUMBER_OF_POSITIONAL_NUMBER(const std::string& command, const int& number , const int& min , const int& max);
void OPTION_REQUIERES_OPTION(const std::string& option,const std::string& requieres);
void DEFAULT_VALUE_OPTION(const std::string& option, const int& default_value);
void EQUAL_DIRECTION();
void ORIGIN_DIRECTORY_NOT_VALITED(const std::string& dir);

