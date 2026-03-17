#include "../../../../include/token/group-token.hpp"
#include "option/option-raw-metadata.hpp"
#include "option/option-implementation.hpp"
#include "token/token-raw-metadata.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <ranges>
#include <vector>

const std::vector<std::pair<std::string, int>> HEADERS = {
  {"PERMS",         10},
  {"INODE",         10},
  {"HARDLINKS",     10},
  {"GROUP",         10},
  {"OWNER",         10},
  {"NAME",          30},
  {"TYPE",           8},
  {"SIZE",           8},
  {"LAST MODIFIED", 12}
};

struct StatsDirs {
  std::uintmax_t total_archive = 0;
  std::uintmax_t total_size    = 0;
  std::uintmax_t total_dirs    = 0;
  std::uintmax_t total_file    = 0;
};

struct DirID {
  dev_t device;
  ino_t inode;
  bool operator==(const DirID& other) const noexcept {
    return device == other.device && inode == other.inode;
  }
};

namespace std {
  template <>
  struct hash<DirID> {
    size_t operator()(const DirID& id) const noexcept {
      size_t h1 = std::hash<dev_t>{}(id.device);
      size_t h2 = std::hash<ino_t>{}(id.inode);
      return h1 ^ (h2 << 1);
    }
  };
}

namespace {

  std::vector<std::string> split_field(std::string_view input){
    std::vector<std::string> result;
    for(auto individual_field : input
        | std::views::filter([](char c){ return c != ' '; })
        | std::views::transform([](char c){ return std::toupper(c); })
        | std::views::split(','))
    {
      result.emplace_back(individual_field.begin(), individual_field.end());
    }
    return result;
  }

  std::string string_perms(const std::filesystem::path& path){
    std::filesystem::perms p = std::filesystem::status(path).permissions();
    auto check = [&](std::filesystem::perms bit, char c){
      return (p & bit) != std::filesystem::perms::none ? c : '-';
    };
    std::string str;
    str += check(std::filesystem::perms::owner_read,   'r');
    str += check(std::filesystem::perms::owner_write,  'w');
    str += check(std::filesystem::perms::owner_exec,   'x');
    str += check(std::filesystem::perms::group_read,   'r');
    str += check(std::filesystem::perms::group_write,  'w');
    str += check(std::filesystem::perms::group_exec,   'x');
    str += check(std::filesystem::perms::others_read,  'r');
    str += check(std::filesystem::perms::others_write, 'w');
    str += check(std::filesystem::perms::others_exec,  'x');
    return str;
  }

  std::string size_parse(const off_t& size){
    const std::array<std::string, 5> units = {"B", "KB", "MB", "GB", "TB"};
    size_t iteration = 0;
    auto s = static_cast<double>(size);
    while(s >= 1024 && iteration < 4){
      s = s / 1024;
      iteration++;
    }
    return std::format("{:.2f}{}", s, units[iteration]);
  }

  std::string date_parse(const std::filesystem::file_time_type& raw_date){
    auto sys = std::chrono::clock_cast<std::chrono::system_clock>(raw_date);
    std::time_t t = std::chrono::system_clock::to_time_t(sys);
    std::tm* tm = std::localtime(&t);
    std::array<char, 64> buffer;
    std::strftime(buffer.data(), sizeof(buffer), "%Y-%m-%d", tm);
    return buffer.data();
  }

  std::vector<std::string> build_entry(
      const std::filesystem::directory_entry& path,
      const struct stat& stat_path)
  {
    const auto perms_path = string_perms(path);
    const auto size_path  = path.is_directory() ? std::string("—") : size_parse(stat_path.st_size);
    passwd* pw = getpwuid(stat_path.st_uid);
    group*  gp = getgrgid(stat_path.st_gid);
    std::string gp_name  = gp ? gp->gr_name : "UNKNOWN";
    std::string pw_name  = pw ? pw->pw_name : "UNKNOWN";
    std::string name_cut = path.path().filename().string().size() > 30
      ? path.path().filename().string().substr(0, 30)
      : path.path().filename().string();
    auto date_path = date_parse(path.last_write_time());
    std::string type = path.is_directory() ? "DIR" : "FIL";

    return {
      perms_path,
      std::to_string(stat_path.st_ino),
      std::to_string(stat_path.st_nlink),
      gp_name,
      pw_name,
      name_cut + (path.is_directory() ? "/" : " "),
      type,
      size_path,
      date_path
    };
  }

  void PrintInformation(
      const std::unordered_map<std::string, std::vector<std::filesystem::directory_entry>>& file_entry,
      const std::vector<std::string>& dirs_name, const bool& field, const std::string& field_option)
  {
    for(const auto& dirs : dirs_name){
      if(!file_entry.contains(dirs)) { continue; }

      if(field){
        const std::vector<std::string> header_field = split_field(field_option);

        std::cout << std::format("{}/:\n", dirs);

        for(const auto& header : header_field){
          auto it = std::ranges::find_if(HEADERS, [&](const auto& head){
            return head.first == header;
          });
          if(it != HEADERS.end()){
            std::cout << std::format("{:<{}}", it->first, it->second);
          }
        }
        std::cout << '\n';

        for(const auto& path : file_entry.at(dirs)){
          struct stat stat_path;
          if(stat(path.path().c_str(), &stat_path) == 0){
            const auto entry_temp = build_entry(path, stat_path);

            for(const auto& header : header_field){
              auto it = std::ranges::find_if(HEADERS, [&](const auto& head){
                return head.first == header;
              });
              if(it != HEADERS.end()){
                auto idx = static_cast<size_t>(std::distance(HEADERS.begin(), it));
                std::cout << std::format("{:<{}}", entry_temp[idx], it->second);
              }
            }
            std::cout << '\n';
          }
        }
      }
      else{
        std::cout << std::format("{}/:\n", dirs);

        for(const auto& header : HEADERS){
          std::cout << std::format("{:<{}}", header.first, header.second);
        }
        std::cout << '\n';

        for(const auto& path : file_entry.at(dirs)){
          struct stat stat_path;
          if(stat(path.path().c_str(), &stat_path) == 0){
            const auto entry_temp = build_entry(path, stat_path);

            for(size_t i = 0; i < HEADERS.size(); i++){
              std::cout << std::format("{:<{}}", entry_temp[i], HEADERS[i].second);
            }
            std::cout << '\n';
          }
        }
      }
    }
  }

  void print_stats(const StatsDirs& s){
    std::cout << std::format("Scanning {}  entries...\n", s.total_archive);
    std::cout << std::format("Total size  : {}\n", size_parse(static_cast<off_t>(s.total_size)));
    std::cout << std::format("Directories : {}\n", s.total_dirs);
    std::cout << std::format("Files       : {}\n", s.total_file);
  }
}

void INSPECT_HANDLER(const GroupToken& token_group){
  std::unordered_map<std::string, std::vector<std::filesystem::directory_entry>> multi_entry;
  std::vector<std::string> dirs_name;
  dirs_name.reserve(token_group.positional.size());

  if(token_group.positional.empty()){
    dirs_name.emplace_back(std::filesystem::current_path().string());
  } else {
    for(const auto& dirs : token_group.positional){
      dirs_name.emplace_back(dirs.name);
    }
  }

  bool recursive = std::ranges::any_of(token_group.options, [](const Token& t){
    return t.name == "--recursive";
  });

  std::vector<std::filesystem::directory_entry> temp_entry;

  for(const auto& dirs : dirs_name){
    if(!std::filesystem::exists(dirs)){
      std::cerr << std::format("\n  [ERROR] '{}' does not exist or is not a valid directory.\n\n", dirs);
      continue;
    }

    temp_entry.clear();
    std::unordered_set<DirID> visited;

    if(recursive){
      for(const auto& entry : std::filesystem::recursive_directory_iterator(dirs)){
        struct stat s;
        if(stat(entry.path().c_str(), &s) == 0){
          DirID id{ .device = s.st_dev, .inode = s.st_ino };
          if(!visited.contains(id)){
            visited.insert(id);
            temp_entry.emplace_back(entry);
          }
        }
      }
    } else {
      for(const auto& entry : std::filesystem::directory_iterator(dirs)){
        temp_entry.emplace_back(entry);
      }
    }

    multi_entry[dirs] = temp_entry;
  }

  for(const auto& option : token_group.options){
    const auto& option_data = GetOptionData(option.name);

    if(option_data->category == OptionCategory::COLLECTION) { continue; }

    if(option_data->category == OptionCategory::FILTERING ||
       option_data->category == OptionCategory::SORTING){
      for(const auto& [key, value] : multi_entry){
        FilterStruct fs;
        fs.entries = value;
        fs.context = option.value;
        std::visit([&](auto& handler){
          using T = std::decay_t<decltype(handler)>;
          if constexpr(std::is_same_v<T, FilteringProcess>){
            handler(fs);
          }
        }, option_data->hanlder);
        multi_entry[key] = fs.entries;
      }
    }
  }

  bool field_exist = std::ranges::any_of(token_group.options, [](const Token& t){
    return t.name == "--fields";
  });

  std::string header_field;
  if(field_exist){
    auto field_option = std::ranges::find_if(token_group.options, [](const Token& t){
      return t.name == "--fields";
    });
    header_field = field_option->value;
  }

  bool only_data = std::ranges::any_of(token_group.options, [](const Token& t){
    return t.name == "--stats";
  });

  auto collect_stats = [](StatsDirs acc, const auto& pair){
    for(const auto& entry : pair.second){
      acc.total_archive++;
      if(entry.is_directory()) {acc.total_dirs++;}
      else {
        acc.total_file++;
        acc.total_size += entry.file_size();
      }
    }
    return acc;
  };

  if(token_group.options.size() == 1 && only_data){
    StatsDirs stat_dirs = std::accumulate(
        multi_entry.begin(), multi_entry.end(), StatsDirs{}, collect_stats);
    print_stats(stat_dirs);
  }
  else{
    if(only_data){
      StatsDirs stat_dirs = std::accumulate(
          multi_entry.begin(), multi_entry.end(), StatsDirs{}, collect_stats);
      print_stats(stat_dirs);
    }
    PrintInformation(multi_entry, dirs_name, field_exist, header_field);
  }
}
