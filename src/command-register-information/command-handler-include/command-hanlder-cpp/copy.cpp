#include "../../../../include/token/group-token.hpp"
#include "../../../../include/error/error_hanlder.hpp"
#include "option/option-implementation.hpp"
#include <algorithm>
#include <cstddef>
#include <format>
#include <iostream>
#include <iterator>
#include <queue>
#include <filesystem>
#include <thread>
#include <mutex>
#include <unordered_set>

const int MAX_THREAD = 9;

namespace {
  void copy_dir_process(const std::vector<std::filesystem::directory_entry>& entries_origin, 
      const std::filesystem::directory_entry& destinatary_dirs, std::filesystem::directory_entry& origin_dirs,
      const std::unordered_set<std::string>& collect_option){

    std::filesystem::copy_options copy_option = std::filesystem::copy_options::none;

    for(const auto& option : collect_option){
      if(option == "--force"){
        copy_option = std::filesystem::copy_options::overwrite_existing;
      }
      if(option == "--skip-existing"){
        copy_option = std::filesystem::copy_options::skip_existing;
      }
    }

    bool repeat = false;

    for(const auto& entry : entries_origin){
      std::filesystem::path path = destinatary_dirs.path() /  std::filesystem::relative(entry.path(), origin_dirs.path());
        
        if(collect_option.contains("--no-overwrite") && std::filesystem::exists(path)){
          std::cout << std::format("[STOP] Process stopped; the entry {} already exists in the directory\n",
              path.filename().string());
          return;
        }

        if(std::filesystem::exists(path) && !repeat && 
        (!collect_option.contains("--force") || !collect_option.contains("--skip-existing"))){
          int option = 0;
          while(option == 0){
            std::cout << std::format("The entry {} already exists\n", path.filename().string());
            std::cout << "1. Skip entry \n";
            std::cout << "2. Update entry\n";
            std::cout << "3. Overwrite entry\n";
            std::cout << "If you enter -[election], the process will be performed for the remaining entries where this occurs.\n";
            std::cin >> option;
          
            switch (option) {
              case -1:
                copy_option = std::filesystem::copy_options::skip_existing;
                repeat = true;
                break;
              case -2:
                copy_option = std::filesystem::copy_options::update_existing;
                repeat = true;
                break;
              case -3:
                copy_option = std::filesystem::copy_options::overwrite_existing;
                repeat = true;
                break;

              case 1:
                copy_option = std::filesystem::copy_options::skip_existing;
                break;
              case 2:
                copy_option = std::filesystem::copy_options::update_existing;
                break;
              case 3:
                copy_option = std::filesystem::copy_options::overwrite_existing;
                break;
              default:
                option = 0;
                break;
            }
        }
      }
      if(std::filesystem::is_directory(entry.path())){
        if(copy_option == std::filesystem::copy_options::skip_existing || 
            copy_option == std::filesystem::copy_options::overwrite_existing ||
            copy_option == std::filesystem::copy_options::none){
          std::filesystem::create_directory(path);
          continue;
        }
        if(copy_option == std::filesystem::copy_options::update_existing){
          std::filesystem::remove_all(path);
          std::filesystem::create_directory(path);
          continue;
        }
      }
      if(std::filesystem::is_symlink(entry.path())){
        if(copy_option == std::filesystem::copy_options::skip_existing){
          continue;
        }
        if(copy_option == std::filesystem::copy_options::update_existing || 
            copy_option == std::filesystem::copy_options::overwrite_existing){
          std::filesystem::remove(path);
          std::filesystem::copy_symlink(entry.path(), path);
          continue;
        }

        std::filesystem::copy_symlink(entry.path(),path);
        continue;
      } 
      if(std::filesystem::is_regular_file(entry.path())){
        std::filesystem::copy_file(entry.path() , path, copy_option);
      }
  }}
}

void COPY_HANLDER(const GroupToken& token_group){
  std::queue<std::filesystem::directory_entry> dirs_name_queque;
  std::filesystem::directory_entry origin_dirs = std::filesystem::directory_entry(token_group.positional.front().name);
  std::filesystem::directory_entry destinatary_dirs = std::filesystem::directory_entry(token_group.positional.back().name);
  std::vector<std::filesystem::directory_entry> extract_dirs_origin;
  
  auto depth_it = std::ranges::find_if(token_group.options, [](const auto& token){
      return token.name == "--depth"; 
  });
  int depth_value = 0;

  if(depth_it != token_group.options.end() && !depth_it->value.empty()){
    depth_value = std::stoi(depth_it->value);
  }

  std::unordered_set<std::string> collect_option;

  for(const auto& option : token_group.options){
    if(option.name == "--fields"){
      continue;
    }
    collect_option.insert(option.name);
  }

  if((origin_dirs == destinatary_dirs)){
    EQUAL_DIRECTION();
    return;
  }
  
  if(!origin_dirs.is_directory() || !destinatary_dirs.is_directory()){
    return;
  }

  if(!std::filesystem::exists(destinatary_dirs)){
    std::filesystem::create_directory(destinatary_dirs);
  }

  if(!std::filesystem::exists(origin_dirs)){
    //anadir error de direccion inexistente objetivo inexistente
    return;
  }
  dirs_name_queque.emplace(origin_dirs);
  std::vector<std::filesystem::directory_entry> dirs_entry;
  std::mutex security_dirs_name;
  std::mutex security_entry_data;
  std::vector<std::thread> threads;
  threads.reserve(MAX_THREAD);
  if(collect_option.contains("--recursive")){
    for(size_t i = 0 ; i < MAX_THREAD ; i++ ){
      threads.emplace_back([&]{
          bool not_empty_queque = true;

          security_dirs_name.lock();
          not_empty_queque = !dirs_name_queque.empty();
          security_dirs_name.unlock();

          while(not_empty_queque){
            security_dirs_name.lock();
            if(dirs_name_queque.empty()){
              security_dirs_name.unlock();
              break;
            }
            auto entry(dirs_name_queque.front());
            dirs_name_queque.pop();
            security_dirs_name.unlock();
            if(depth_it != token_group.options.end()){
              if(!entry.is_regular_file()){
                for(auto entry_its = std::filesystem::recursive_directory_iterator(entry);
                    entry_its != std::filesystem::recursive_directory_iterator();
                    ++entry_its)
                {
                  if(entry_its.depth() <= depth_value ){
                    security_entry_data.lock();
                    extract_dirs_origin.push_back(*entry_its);
                    security_entry_data.unlock();
                  
                  }
                }
              }
              else{
                security_entry_data.lock();
                extract_dirs_origin.push_back(entry);
                security_entry_data.unlock();
              }
            }
            else{
              if(!entry.is_regular_file()){
                for(const auto& entry_its : std::filesystem::directory_iterator(entry)){
                  if(!entry_its.is_directory()){
                    security_entry_data.lock();
                    extract_dirs_origin.emplace_back(entry_its);
                    security_entry_data.unlock();
                  }
                  else{
                    security_dirs_name.lock();
                    dirs_name_queque.push(entry_its);
                    security_dirs_name.unlock();
                  
                    security_entry_data.lock();
                    extract_dirs_origin.emplace_back(entry_its);
                    security_entry_data.unlock();
                  }
                }
              }
              else{
                security_entry_data.lock();
                extract_dirs_origin.push_back(entry);
                security_entry_data.unlock();
              }
            }
          }
      });
    }
    for(auto& its : threads){
      its.join();
    }
    //ordenar elementos
    std::vector<std::pair<ptrdiff_t, std::filesystem::directory_entry>> temp_sort;
    
    for(const auto& entry : extract_dirs_origin){
      temp_sort.push_back({std::ranges::distance(entry.path().begin(), entry.path().end()), entry});
    }
    //ordenar elementos
    std::ranges::sort(temp_sort, [](const auto& a, const auto& b)
        {
          return std::pair(a.first , a.second.path().filename()) < std::pair(b.first, b.second.path().filename());
        });
    std::ranges::transform(temp_sort, extract_dirs_origin.begin(), [](const auto& entry){
        return entry.second;
        });
  }
  else{
    for(const auto& entry_its : std::filesystem::directory_iterator(origin_dirs)){
      extract_dirs_origin.emplace_back(entry_its);
    }
  }

  for(const auto& option : token_group.options){
    const auto& option_data = GetOptionData(option.name);

    if(option_data->category == OptionCategory::COLLECTION) { continue; }

    if(option_data->category == OptionCategory::FILTERING || option_data->category == OptionCategory::SORTING){
        FilterStruct fs;
        fs.entries = extract_dirs_origin;
        fs.context = option.value;
        std::visit([&](auto& handler){
          using T = std::decay_t<decltype(handler)>;
          if constexpr(std::is_same_v<T, FilteringProcess>){
            handler(fs);
          }
        }, option_data->hanlder);
        extract_dirs_origin = fs.entries;
    }
  }

  copy_dir_process(extract_dirs_origin, destinatary_dirs, origin_dirs,collect_option);
}


