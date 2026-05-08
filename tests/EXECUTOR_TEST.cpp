#include <gtest/gtest.h>
#include "../include/core-hpp/tokenization.hpp"
#include "../include/core-hpp/parsing.hpp"
#include "../include/core-hpp/validator.hpp"
#include "../include/option/option-implementation.hpp"
#include "../include/core-hpp/executor.hpp"

class ExecutorSuite : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    CreatedOptionData();
  }
};

TEST_F(ExecutorSuite, HelpFlagCallsHelpHandler) {
  // 1. Prepare input
  std::vector<std::string> input = {"--help"};
  
  // 2. Run pipeline
  auto tokens  = tokenization(input);
  auto parsed  = parsing(tokens);
  ValidationGroupToken(parsed);
  
  // 3. Capture output
  testing::internal::CaptureStdout();
  executor(parsed);
  std::string output = testing::internal::GetCapturedStdout();
  
  // 4. Verify
  EXPECT_FALSE(output.empty());
  EXPECT_NE(output.find("kls [path] [options]"), std::string::npos);
}
