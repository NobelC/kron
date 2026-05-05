#include "../include/command/command-implementation.hpp"
#include "../include/core-hpp/executor.hpp"
#include "../include/core-hpp/parsing.hpp"
#include "../include/core-hpp/tokenization.hpp"
#include "core-hpp/validator.hpp"
#include "option/option-implementation.hpp"
#include <gtest/gtest.h>

class ExecutorSuite : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    CreatedOptionData();
    CreatedCommandData();
  }
};

TEST_F(ExecutorSuite, HelpFlagCallsHelpHandler) {
  // 1. Preparar el input
  std::vector<std::string> input = {"list", "--recursive", "~"};

  // 2. Correr el pipeline completo
  auto tokens = tokenization(input);
  auto parsed = parsing(tokens);
  ValidationGroupToken(parsed);

  // 3. Capturar lo que imprima executor
  testing::internal::CaptureStdout();
  executor(parsed);
}