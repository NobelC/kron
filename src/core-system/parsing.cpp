#include "../../include/core-hpp/parsing.hpp"
#include "../../include/error/error_hanlder.hpp"
#include "../../include/option/option-implementation.hpp"
#include "../../include/option/option-raw-metadata.hpp"
#include "../../include/token/token-raw-metadata.hpp"
#include <cstddef>
#include <string>
#include <unordered_set>
#include <vector>

GroupToken parsing(const std::vector<Token> &token_raw) {
  GroupToken token_clasificated;
  std::unordered_set<std::string_view> see_token;

  // Tratamiento y verificacion de existencia
  for (size_t pos = 0; pos < token_raw.size(); pos++) {
    const auto &individual_token = token_raw[pos];

    // Comprobar si una opcion existe
    if (individual_token.type == TypeToken::OPTION_NOT_NORMALIZED) {
      // opciones largas
      if (individual_token.name.starts_with("--")) {
        const auto equal_pos = individual_token.name.find('=');
        // Opciones largas sin signo de igual
        if (equal_pos == std::string::npos) {
          const auto &data_token = GetOptionData(individual_token.name);
          if (data_token == nullptr) {
            OPTION_NOT_FOUND(individual_token.name);
            token_clasificated.is_valid = false;
            return token_clasificated;
          }
          // Para opciones largas sin igual pero que requieren valor
          if (data_token->data_type != TypeDataReceived::NONE) {
            if (pos + 1 < token_raw.size() &&
                token_raw[pos + 1].type == TypeToken::LITERAL) {
              const auto &next_token = token_raw[pos + 1];
              token_clasificated.options.emplace_back(Token{
                  .type = TypeToken::OPTION_NORMALIZED,
                  .name = data_token->normalized_name,
                  .value = next_token.name,
              });
              pos++;
              continue;
            } else {
              OPTION_NEED_VALUE(individual_token.name, data_token->data_type);
              token_clasificated.is_valid = false;
              return token_clasificated;
            }
          }
          token_clasificated.options.emplace_back(Token{
              .type = TypeToken::OPTION_NORMALIZED,
              .name = data_token->normalized_name,
              .value = "",
          });
          continue;
        } else {
          // Para opciones largas las cuales tienen un signo de igual
          const auto &data_token =
              GetOptionData(individual_token.name.substr(0, equal_pos));
          if (data_token == nullptr) {
            OPTION_NOT_FOUND(individual_token.name);
            token_clasificated.is_valid = false;
            return token_clasificated;
          }
          token_clasificated.options.emplace_back(Token{
              .type = TypeToken::OPTION_NORMALIZED,
              .name = data_token->normalized_name,
              .value = individual_token.name.substr(equal_pos + 1),
          });
          continue;
        }
      }
      // Para opciones cortas con o sin igual o agrupas con y sin igual
      for (size_t i = 0; i < individual_token.name.size(); i++) {
        if (individual_token.name[i] == '-') {
          continue;
        }
        const std::string flag = {'-', individual_token.name[i]};
        const auto &data_token = GetOptionData(flag);
        if (data_token == nullptr) {
          OPTION_NOT_FOUND(flag);
          token_clasificated.is_valid = false;
          return token_clasificated;
        }
        if (data_token->data_type != TypeDataReceived::NONE) {
          if (i + 1 < individual_token.name.size() &&
              individual_token.name[i + 1] == '=') {
            const auto &equal_pos = individual_token.name.find('=');
            if (equal_pos != std::string::npos) {
              token_clasificated.options.emplace_back(Token{
                  .type = TypeToken::OPTION_NORMALIZED,
                  .name = data_token->normalized_name,
                  .value = individual_token.name.substr(equal_pos + 1),
              });
              break;
            }
          }
          // Para caso especial de -o que requiera valor (para funciones
          // agrupadas mandara error si no hay valor y se necesita)
          else if (i + 1 >= individual_token.name.size() &&
                   pos + 1 < token_raw.size() &&
                   token_raw[pos + 1].type == TypeToken::LITERAL) {
            token_clasificated.options.emplace_back(Token{
                .type = TypeToken::OPTION_NORMALIZED,
                .name = data_token->normalized_name,
                .value = token_raw[pos + 1].name,
            });
            pos++;
            break;
          } else {
            OPTION_NEED_VALUE(flag, data_token->data_type);
            token_clasificated.is_valid = false;
            return token_clasificated;
          }
        }

        token_clasificated.options.emplace_back(Token{
            .type = TypeToken::OPTION_NORMALIZED,
            .name = data_token->normalized_name,
            .value = "",
        });
        continue;
      }
      continue;
    }

    token_clasificated.positional.emplace_back(Token{
        .type = TypeToken::POSITIONAL,
        .name = individual_token.name,
        .value = "",
    });
  }

  return token_clasificated;
}
