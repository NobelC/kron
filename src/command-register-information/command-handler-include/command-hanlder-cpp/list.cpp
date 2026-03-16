#include "../../../../include/token/group-token.hpp"
#include "option/option-raw-metadata.hpp"
#include "option/option-implementation.hpp"
#include "token/token-raw-metadata.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <ctime>
#include <filesystem>
#include <format>
#include <iostream>
#include <ranges>
#include <string>
#include <sys/types.h>
#include <variant>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

namespace {
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
    std::array<char,64> buffer;
    std::strftime(buffer.data(), sizeof(buffer.data()), "%Y-%m-%d", tm);
    return buffer.data();
  }

  size_t width_dynamic(const std::vector<std::filesystem::directory_entry>& entry_path){
    if(entry_path.empty()) {return 20;}
    auto size_entry = entry_path | std::views::transform([](const auto& e){
        return e.path().filename().string().size();
    });
    return *std::ranges::max_element(size_entry);
  }

  void PrintInformation(const bool& long_format, const bool& no_header_format,
      const std::vector<std::filesystem::directory_entry>& file_entry)
  {
    if(!long_format){
      for(const auto& entry : file_entry){
        std::cout << std::format("{}\n",
            entry.path().filename().string() + (entry.is_directory() ? "/" : ""));
      }
      return;
    }

    size_t size_width = width_dynamic(file_entry) + 5;
    if(!no_header_format){
      std::cout << std::format("{:<10} {:<10} {:<10} {:<12} {:<{}} {:<8} {:<8} {:<20}\n",
          "PERMISOS", "INODE", "GRUPO", "PROPIETARIO",
          "NOMBRE", size_width,
          "TIPO", "TAMANO", "FECHA DE MODIFICACION");
    }

    for(const auto& path : file_entry){
      std::string type      = path.is_directory() ? "DIR" : "FIL";
      const auto perms_path = string_perms(path);
      struct stat stat_path;
      if(stat(path.path().c_str(), &stat_path) == 0){
        const auto size_path = path.is_directory()
            ? std::string("—")
            : size_parse(stat_path.st_size);
        passwd* pw = getpwuid(stat_path.st_uid);
        group*  gp = getgrgid(stat_path.st_gid);
        std::string gp_name = gp ? gp->gr_name : "UNKNOWN";
        std::string pw_name = pw ? pw->pw_name : "UNKNOWN";
        auto date_path = date_parse(path.last_write_time());
        std::cout << std::format("{:<10} {:<10} {:<10} {:<12} {:<{}} {:<8} {:<8} {:<8}\n",
            perms_path,
            stat_path.st_ino,
            gp_name,
            pw_name,
            path.path().filename().string() + (path.is_directory() ? "/" : ""), size_width,
            type,
            size_path,
            date_path);
      }
    }
  }
}

void LIST_HANDLER(const GroupToken& token_group){
  std::vector<std::filesystem::directory_entry> file_entry;
  std::filesystem::path path_entry = std::filesystem::current_path();

  if(!token_group.positional.empty()){
    path_entry = token_group.positional.front().name;
  }

  if(!std::filesystem::exists(path_entry)){
    std::cerr << std::format("ERROR : {} NO EXISTE O NO ES UN DIRECTORIO VALIDO\n",
        path_entry.string());
    return;
  }

  bool all_format = std::ranges::any_of(token_group.options, [](const Token& t){
      return t.name == "--all";
  });

  for(const auto& entry : std::filesystem::directory_iterator(path_entry)){
    if(!all_format && entry.path().filename().string().starts_with('.')){
      continue;
    }
    file_entry.emplace_back(entry);
  }

  bool long_format      = false;
  bool no_header_format = false;

  for(const auto& option : token_group.options){
    const auto& option_data = GetOptionData(option.name);

    if(option_data->category == OptionCategory::COLLECTION) {continue;}

    if(option_data->category == OptionCategory::FILTERING ||
       option_data->category == OptionCategory::SORTING){
      FilterStruct fs;
      fs.entries = file_entry;
      fs.context = option.value;
      std::visit([&](auto& handler){
          using T = std::decay_t<decltype(handler)>;
          if constexpr(std::is_same_v<T, FilteringProcess>){
            handler(fs);
          }
      }, option_data->hanlder);
      file_entry = fs.entries;
      continue;
    }

    if(option_data->category == OptionCategory::PRESENTATION){
      if(option_data->normalized_name == "--long")      {long_format      = true;}
      if(option_data->normalized_name == "--no-header") {no_header_format = true;}
    }
  }

  PrintInformation(long_format, no_header_format, file_entry);
}
