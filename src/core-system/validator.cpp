#include "../../include/core-hpp/validator.hpp"
// #include "../../include/command/command-implementation.hpp"
#include "../../include/error/error_hanlder.hpp"
#include "../../include/option/option-implementation.hpp"
#include "../../include/option/option-raw-metadata.hpp"
#include "../../include/token/token-raw-metadata.hpp"
#include "special-option/version-option.hpp"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string_view>
#include <unordered_set>

namespace {
bool DATE_VALIDATED(const std::string_view &date_str) {
  if (date_str.empty()) {
    return false;
  }
  std::istringstream str_validated(std::string{date_str});
  std::chrono::sys_days parser_date;

  str_validated >> std::chrono::parse("%F", parser_date);

  if (str_validated.fail()) {
    return false;
  }

  std::chrono::year_month_day year_month_day(parser_date);

  if (!year_month_day.ok()) {
    return false;
  }

  return true;
}

bool SIZE_VALIDATED(std::string_view size_str) {
  // El formato para el tamano de archivos es variado
  // 1028 MB | 1028 gb
  // para valores numericos (1028) sin especificar medida se usaran los MB como
  // estandar
  if (size_str.empty()) {
    return false;
  }

  auto digits = size_str | std::views::take_while(::isdigit);
  const auto num_digits = static_cast<size_t>(std::ranges::distance(digits));

  if (num_digits == 0) {
    return false;
  }
  if (num_digits == size_str.size()) {
    return true;
  }

  const auto unit = size_str.substr(num_digits);
  constexpr std::array valid_units = {
      // ← std::array en vez de C-style array
      std::string_view{"B"},  std::string_view{"KB"}, std::string_view{"MB"},
      std::string_view{"GB"}, std::string_view{"TB"}, std::string_view{"b"},
      std::string_view{"kb"}, std::string_view{"mb"}, std::string_view{"gb"},
      std::string_view{"tb"}};

  return std::ranges::any_of(valid_units,
                             [unit](std::string_view u) { return u == unit; });
}

bool EXTENSION_VALIDATED(std::string_view extension_str) {
  if (extension_str.empty()) {
    return false;
  }
  if (extension_str.starts_with(".")) {
    extension_str.remove_prefix(1);
  }
  if (extension_str.empty()) {
    return false;
  }
  return std::ranges::none_of(extension_str, [](char c) {
    return c == '/' || c == '\\' || c == ' ';
  });
}
} // namespace

bool ValidationGroupToken(GroupToken &group_raw) {
  if (!group_raw.is_valid) {
    return false;
  }

  bool version = std::ranges::any_of(group_raw.options, [](const Token &t) {
    return t.name == "--version" || t.name == "-v";
  });
  bool help_only = std::ranges::any_of(group_raw.options, [](const Token &t) {
    return t.name == "--help" || t.name == "-h";
  });

  if (version) {
    VERSION_HANDLER();
    return false;
  }

  if (group_raw.positional.empty()) {
    group_raw.positional.emplace_back(Token{
        .type = TypeToken::POSITIONAL,
        .name = ".",
        .value = "",
    });
  }

  // Eliminar opciones duplicadas (sera valida solo la primera opcion
  // introducida)
  std::unordered_set<std::string_view> eliminated_duplicated_option;
  eliminated_duplicated_option.reserve(group_raw.options.size());
  std::vector<Token> option_not_duplicated;
  option_not_duplicated.reserve(group_raw.options.size());

  for (const auto &option : group_raw.options) {
    if (eliminated_duplicated_option.contains(option.name)) {
      continue;
    }
    option_not_duplicated.emplace_back(option);
    eliminated_duplicated_option.insert(option.name);
  }

  group_raw.options = option_not_duplicated;
  for (const auto &element : group_raw.options) {
    const auto &option_data = GetOptionData(element.name);
    if (option_data == nullptr) {
      continue; // Should have been caught by parsing, but safety first
    }
    // Comprobamos si alguna opcion tiene conflictos con otra
    for (const auto &conflict_option : option_data->conflict_name) {
      if (eliminated_duplicated_option.contains(conflict_option)) {
        OPTION_CONFLICT_WITH(conflict_option);
        return false;
      }
    }
    for (const auto &requieres_option : option_data->requieres_name) {
      if (!eliminated_duplicated_option.contains(requieres_option)) {
        OPTION_REQUIERES_OPTION(option_data->normalized_name, requieres_option);
        return false;
      }
    }

    // Validar tipo de dato que debe recibir:
    switch (option_data->data_type) {
    case TypeDataReceived::DATE:
      if (!DATE_VALIDATED(element.value)) {

        return false;
      }
      break;
    case TypeDataReceived::EXTENSION:
      if (!EXTENSION_VALIDATED(element.value)) {
        return false;
      };
      break;
    case TypeDataReceived::SIZE:
      if (!SIZE_VALIDATED(element.value)) {
        return false;
      }
      break;
    case TypeDataReceived::STRING:
      if (element.value.empty() || element.value == " ") {
        return false;
      }
      break;
    case TypeDataReceived::NONE:
      break;
    }
  }

  // ordenar las option en base a su cronologia de ejecucion
  std::ranges::sort(group_raw.options, [](const Token &a, const Token &b) {
    return static_cast<uint8_t>(GetOptionData(a.name)->category) <
           static_cast<uint8_t>(GetOptionData(b.name)->category);
  });

  return true;
}
