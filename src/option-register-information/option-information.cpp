#include "../../include/option/option-implementation.hpp"
#include "../../include/option/option-raw-metadata.hpp"
#include <algorithm>
#include <any>
#include <string_view>
#include <tuple>
#include <vector>

#include "../../include/option/option-implementation.hpp"
#include "../../include/option/option-raw-metadata.hpp"
#include <algorithm>
#include <any>
#include <string_view>
#include <tuple>
#include <vector>

void CreatedOptionData() {
  // --- OPCIONES GLOBALES ---

  OptionMetaData help;
  help.normalized_name = "--help";
  help.alias_name = "-h";
  help.data_type = TypeDataReceived::NONE;
  help.category = OptionCategory::GLOBAL;
  help.hanlder = std::monostate{};
  GeneralOptionLog(help);

  OptionMetaData version;
  version.normalized_name = "--version";
  version.alias_name = "-v";
  version.data_type = TypeDataReceived::NONE;
  version.category = OptionCategory::GLOBAL;
  version.hanlder = std::monostate{};
  GeneralOptionLog(version);

  OptionMetaData no_color;
  no_color.normalized_name = "--no-color";
  no_color.data_type = TypeDataReceived::NONE;
  no_color.category = OptionCategory::GLOBAL;
  no_color.hanlder = std::monostate{};
  GeneralOptionLog(no_color);

  OptionMetaData quiet;
  quiet.normalized_name = "--quiet";
  quiet.alias_name = "-q";
  quiet.conflict_name = {"--verbose"};
  quiet.data_type = TypeDataReceived::NONE;
  quiet.category = OptionCategory::GLOBAL;
  quiet.hanlder = std::monostate{};
  GeneralOptionLog(quiet);

  OptionMetaData verbose;
  verbose.normalized_name = "--verbose";
  verbose.conflict_name = {"--quiet"};
  verbose.data_type = TypeDataReceived::NONE;
  verbose.category = OptionCategory::GLOBAL;
  verbose.hanlder = std::monostate{};
  GeneralOptionLog(verbose);

  // --- RECOLECCIÓN Y FILTRADO ---

  OptionMetaData all;
  all.normalized_name = "--all";
  all.alias_name = "-a";
  all.conflict_name = {"--no-hidden"};
  all.data_type = TypeDataReceived::NONE;
  all.category = OptionCategory::COLLECTION;
  all.hanlder = std::monostate{};
  GeneralOptionLog(all);

  OptionMetaData recursive;
  recursive.normalized_name = "--recursive";
  recursive.alias_name = "-r";
  recursive.data_type = TypeDataReceived::NONE;
  recursive.category = OptionCategory::COLLECTION;
  recursive.hanlder = std::monostate{};
  GeneralOptionLog(recursive);

  OptionMetaData depth;
  depth.normalized_name = "--depth";
  depth.alias_name = "-d";
  depth.requieres_name = {"--recursive"};
  depth.data_type = TypeDataReceived::STRING;
  depth.category = OptionCategory::FILTERING;
  GeneralOptionLog(depth);

  OptionMetaData filter;
  filter.normalized_name = "--filter";
  filter.data_type = TypeDataReceived::EXTENSION;
  filter.category = OptionCategory::FILTERING;
  filter.hanlder = FilteringProcess([](FilterStruct &filter_contex) {
    const auto *pattern_ptr =
        std::any_cast<std::string>(&filter_contex.context);
    if (!pattern_ptr) {
      return;
    }

    const std::string &pattern = *pattern_ptr;
    std::erase_if(filter_contex.entries, [&pattern](const FileEntry &e) {
      std::string_view filename = e.name;
      if (pattern.starts_with('*')) {
        return !filename.ends_with(std::string_view(pattern).substr(1));
      }
      if (pattern.ends_with('*')) {
        return !filename.starts_with(
            std::string_view(pattern).substr(0, pattern.size() - 1));
      }
      return filename != pattern;
    });
  });
  GeneralOptionLog(filter);

  OptionMetaData modified_before;
  modified_before.normalized_name = "--modified-before";
  modified_before.data_type = TypeDataReceived::DATE;
  modified_before.category = OptionCategory::FILTERING;
  /*
  filter.hanlder = FilteringProcess([](FilterStruct& filter_contex) {
      const auto* pattern_ptr =
  std::any_cast<std::string>(&filter_contex.context); if (!pattern_ptr)
  {return;}

      const std::string& pattern = *pattern_ptr;
      std::erase_if(filter_contex.entries, [&pattern](const FileEntry& e) {
          std::string_view filename = e.name;
          if (pattern.starts_with('*')) {
              return !filename.ends_with(std::string_view(pattern).substr(1));
          }
          if (pattern.ends_with('*')) {
              return !filename.starts_with(std::string_view(pattern).substr(0,
  pattern.size() - 1));
          }
          return filename != pattern;
      });
  });
  */
  GeneralOptionLog(modified_before);

  OptionMetaData modified_after;
  modified_after.normalized_name = "--modified-after";
  modified_after.data_type = TypeDataReceived::DATE;
  modified_after.category = OptionCategory::FILTERING;
  /*
  filter.hanlder = FilteringProcess([](FilterStruct& filter_contex) {
      const auto* pattern_ptr =
  std::any_cast<std::string>(&filter_contex.context); if (!pattern_ptr)
  {return;}

      const std::string& pattern = *pattern_ptr;
      std::erase_if(filter_contex.entries, [&pattern](const FileEntry& e) {
          std::string_view filename = e.name;
          if (pattern.starts_with('*')) {
              return !filename.ends_with(std::string_view(pattern).substr(1));
          }
          if (pattern.ends_with('*')) {
              return !filename.starts_with(std::string_view(pattern).substr(0,
  pattern.size() - 1));
          }
          return filename != pattern;
      });
  });
  */
  GeneralOptionLog(modified_after);

  // --- ORDENAMIENTO (SORTING) ---

  OptionMetaData sort;
  sort.normalized_name = "--sort";
  sort.data_type = TypeDataReceived::STRING;
  sort.category = OptionCategory::SORTING;
  sort.hanlder = FilteringProcess([](FilterStruct &filter_contex) {
    const auto *criteria_ptr =
        std::any_cast<std::string>(&filter_contex.context);
    if (!criteria_ptr) {
      return;
    }

    std::string criteria = *criteria_ptr;
    if (criteria == "name") {
      std::ranges::sort(filter_contex.entries,
                        [](const FileEntry &a, const FileEntry &b) {
                          return a.name < b.name;
                        });
    } else if (criteria == "size") {
      std::ranges::sort(
          filter_contex.entries, [](const FileEntry &a, const FileEntry &b) {
            // Tamaño descendente, desempate por nombre
            return std::tie(b.size, a.name) < std::tie(a.size, b.name);
          });
    } else if (criteria == "type") {
      std::ranges::sort(filter_contex.entries,
                        [](const FileEntry &a, const FileEntry &b) {
                          // Directorios primero (true > false)
                          return std::tie(b.is_directory, a.name) <
                                 std::tie(a.is_directory, b.name);
                        });
    } else if (criteria == "modified") {
      std::ranges::sort(
          filter_contex.entries, [](const FileEntry &a, const FileEntry &b) {
            return std::tie(a.mtime, a.name) < std::tie(b.mtime, b.name);
          });
    } else if (criteria == "ext" || criteria == "extension") {
      std::ranges::sort(filter_contex.entries, [](const FileEntry &a,
                                                  const FileEntry &b) {
        return std::tie(a.extension, a.name) < std::tie(b.extension, b.name);
      });
    }
  });
  GeneralOptionLog(sort);

  OptionMetaData reverse;
  reverse.normalized_name = "--reverse";
  reverse.category = OptionCategory::SORTING;
  reverse.hanlder = FilteringProcess([](FilterStruct &filter_contex) {
    std::ranges::reverse(filter_contex.entries);
  });
  GeneralOptionLog(reverse);

  OptionMetaData dirs_first;
  dirs_first.normalized_name = "--dirs-first";
  dirs_first.category = OptionCategory::SORTING;
  dirs_first.hanlder = FilteringProcess([](FilterStruct &filter_contex) {
    std::ranges::sort(filter_contex.entries,
                      [](const FileEntry &a, const FileEntry &b) {
                        return std::tie(b.is_directory, a.name) <
                               std::tie(a.is_directory, b.name);
                      });
  });
  GeneralOptionLog(dirs_first);

  // --- PRESENTACIÓN ---

  OptionMetaData long_format;
  long_format.normalized_name = "--long";
  long_format.alias_name = "-l";
  long_format.category = OptionCategory::PRESENTATION;
  long_format.hanlder = std::monostate{};
  GeneralOptionLog(long_format);

  OptionMetaData no_header;
  no_header.normalized_name = "--no-header";
  no_header.category = OptionCategory::PRESENTATION;
  no_header.hanlder = std::monostate{};
  GeneralOptionLog(no_header);

  OptionMetaData stats;
  stats.normalized_name = "--stats";
  stats.category = OptionCategory::PRESENTATION;
  stats.hanlder = std::monostate{};
  GeneralOptionLog(stats);

  // --- MANIPULACIÓN Y CREACIÓN (STUBS) ---

  OptionMetaData dry_run;
  dry_run.normalized_name = "--dry-run";
  dry_run.alias_name = "-n";
  dry_run.category = OptionCategory::MANIPULATION;
  dry_run.hanlder = std::monostate{};
  GeneralOptionLog(dry_run);

  OptionMetaData force;
  force.normalized_name = "--force";
  force.alias_name = "-f";
  force.conflict_name = {"--no-overwrite", "--skip-existing"};
  force.category = OptionCategory::MANIPULATION;
  GeneralOptionLog(force);

  OptionMetaData parents;
  parents.normalized_name = "--parents";
  parents.alias_name = "-p";
  parents.category = OptionCategory::CREATION;
  GeneralOptionLog(parents);

  OptionMetaData output;
  output.normalized_name = "--output";
  output.alias_name = "-o";
  output.conflict_name = {};
  output.category = OptionCategory::CREATION;
  output.data_type = TypeDataReceived::STRING;
  output.hanlder = std::monostate{};
  GeneralOptionLog(output);

  OptionMetaData size_gt;
  size_gt.normalized_name = "--size-gt";
  size_gt.alias_name = "";
  size_gt.conflict_name = {};
  size_gt.category = OptionCategory::FILTERING;
  size_gt.data_type = TypeDataReceived::SIZE;
  size_gt.hanlder = std::monostate{};
  GeneralOptionLog(size_gt);
}
