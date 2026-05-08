#include "../../../../include/option/option-implementation.hpp"
#include "../../../../include/option/option-raw-metadata.hpp"
#include "../../../../include/token/group-token.hpp"
#include "../../../../include/token/token-raw-metadata.hpp"
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <dirent.h>
#include <fcntl.h>
#include <filesystem>
#include <format>
#include <grp.h>
#include <iostream>
#include <iterator>
#include <limits>
#include <linux/limits.h>
#include <linux/stat.h>
#include <mutex>
#include <pwd.h>
#include <queue>
#include <string>
#include <string_view>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <variant>

namespace {
constexpr unsigned int MAX_THREAD = 10;
static std::unordered_map<uid_t, std::string> cache_owner;
static std::unordered_map<gid_t, std::string> cache_group;

struct Option {
  bool recursive : 1;
  bool all : 1;
  bool long_format : 1;
  bool no_header_format : 1;
  bool follow_symlink : 1;
  bool capabilities : 1;
};

struct PendingDir {
  std::string path;
  int depth;
};

void NormalRecolection(FileEntry &fe, std::string_view current_path,
                       dirent *entry) {
  fe.inode = entry->d_ino;
  fe.size = std::numeric_limits<uint64_t>::max();
  fe.nlinks = 0;
  fe.mode = 0;
  fe.uid = 0;
  fe.gid = 0;
  fe.mtime = 0;
  fe.btime = 0;

  fe.is_directory = (entry->d_type == DT_DIR);
  fe.is_symlink = (entry->d_type == DT_LNK);
  fe.symlink_broken = false;
  fe.has_capabilities = false;

  fe.name = entry->d_name;
  fe.path = current_path;

  fe.symlink_target.clear();
  fe.extension.clear();
  if (!fe.health.empty()) {
    fe.health.clear();
  }

  std::string_view name_view(entry->d_name);
  if (size_t dot_pos = name_view.find_last_of('.');
      dot_pos != std::string_view::npos && dot_pos > 0) {
    fe.extension = std::string(name_view.substr(dot_pos));
  }
}

void LongRecolection(FileEntry &fe, const std::string &full_path, dirent *entry,
                     std::string_view current_path) {
  struct statx stx;
  unsigned int mask = STATX_BASIC_STATS | STATX_BTIME;

  // full_path ya viene construido del loop principal para evitar doble format
  if (statx(AT_FDCWD, full_path.c_str(),
            AT_SYMLINK_NOFOLLOW | AT_STATX_DONT_SYNC, mask, &stx) == 0) {
    fe.inode = stx.stx_ino;
    fe.size = stx.stx_size;
    fe.nlinks = stx.stx_nlink;
    fe.mode = stx.stx_mode;
    fe.uid = stx.stx_uid;
    fe.gid = stx.stx_gid;
    fe.mtime = stx.stx_mtime.tv_sec;

    fe.btime = (stx.stx_mask & STATX_BTIME) ? stx.stx_btime.tv_sec : 0;

    fe.is_directory = (entry->d_type == DT_DIR);
    fe.is_symlink = (entry->d_type == DT_LNK);
    fe.symlink_broken = false;
    fe.has_capabilities = false;

    fe.name = entry->d_name;
    fe.path = current_path;

    fe.symlink_target.clear();
    fe.extension.clear();
    if (!fe.health.empty()) {
      fe.health.clear();
    }

    std::string_view name_view(fe.name);
    if (size_t dot_pos = name_view.find_last_of('.');
        dot_pos != std::string_view::npos && dot_pos > 0) {
      fe.extension = std::string(name_view.substr(dot_pos));
    }
  }
}

void NormalPrinter(const std::vector<FileEntry> &entries) {
  if (entries.empty()) {
    return;
  }

  size_t max_name = 0;
  for (const auto &e : entries) {
    max_name = std::max(max_name, e.name.length());
  }

  size_t col_width = max_name + 2;
  size_t cols = std::max(1UL, 80 / col_width);

  for (size_t i = 0; i < entries.size(); ++i) {
    if (entries[i].is_directory) {
      std::cout << std::format("\033[1;34m{:<{}}\033[0m", entries[i].name,
                               col_width);
    } else {
      std::cout << std::format("{:<{}}", entries[i].name, col_width);
    }

    if ((i + 1) % cols == 0) {
      std::cout << "\n";
    }
  }
  std::cout << "\n";
}

void LongPrinter(const std::vector<FileEntry> &entries) {
  if (entries.empty()) {
    return;
  }

  // Encabezado (si no se activó --no-header)
  std::cout << std::format("{:<10} {:<3} {:<8} {:<8} {:<10} {:<12} {}\n",
                           "PERMS", "LNK", "OWNER", "GROUP", "SIZE", "MODIFIED",
                           "NAME");
  std::cout << std::string(80, '-') << "\n";

  for (const auto &e : entries) {
    // 1. Permisos (Modo)
    std::string perms = e.is_directory ? "d" : (e.is_symlink ? "l" : "-");
    perms += (e.mode & S_IRUSR) ? "r" : "-";
    perms += (e.mode & S_IWUSR) ? "w" : "-";
    perms += (e.mode & S_IXUSR) ? "x" : "-";

    // ... podrías expandir para grupo y otros ...
    std::string owner;
    std::string group_str;
    if (cache_owner.contains(e.uid)) {
      owner = cache_owner.at(e.uid);
    } else {
      passwd *pw = getpwuid(e.uid);
      owner = pw ? pw->pw_name : "UNKNOWN";
      cache_owner[e.uid] = owner;
    }

    if (cache_group.contains(e.gid)) {
      group_str = cache_group.at(e.gid);
    } else {
      group *gp = getgrgid(e.gid);
      group_str = gp ? gp->gr_name : "UNKNOWN";
      cache_group[e.gid] = group_str;
    }

    // 3. Tiempo
    std::string time_str = std::format(
        "{:%b %d %H:%M}", std::chrono::system_clock::from_time_t(e.mtime));
    // 4. Tamaño (Manejo del valor centinela MAX que definimos)
    std::string size_str = (e.size == std::numeric_limits<uint64_t>::max())
                               ? "-"
                               : std::to_string(e.size);

    // Renderizado Final
    std::cout << std::format("{:<10} {:<3} {:<8} {:<8} {:<10} {:<12} ", perms,
                             e.nlinks, owner, group_str, size_str, time_str);

    // Color para el nombre si es directorio
    if (e.is_directory) {
      std::cout << "\033[1;34m" << e.name << "\033[0m\n";
    } else {
      std::cout << e.name << "\n";
    }
  }
}
} // namespace

void LIST_HANDLER(const GroupToken &token_group) {
  std::vector<FileEntry> file_entry;
  std::queue<PendingDir> pending_dirs;

  Option options_bool = {
      .recursive = std::ranges::any_of(
          token_group.options,
          [](const auto &t) { return t.name == "--recursive"; }),
      .all = std::ranges::any_of(
          token_group.options, [](const auto &t) { return t.name == "--all"; }),
      .long_format =
          std::ranges::any_of(token_group.options,
                              [](const auto &t) { return t.name == "--long"; }),
      .no_header_format = std::ranges::any_of(
          token_group.options,
          [](const auto &t) { return t.name == "--no-header"; }),
      .follow_symlink = std::ranges::any_of(
          token_group.options,
          [](const auto &t) { return t.name == "--follow-symlink"; }),
      .capabilities =
          std::ranges::any_of(token_group.options, [](const auto &t) {
            return t.name == "--capabilities";
          })};

  int depth_limit = 0;
  auto it = std::ranges::find_if(
      token_group.options, [](const auto &t) { return t.name == "--depth"; });
  if (it != token_group.options.end()) {
    if (!it->value.empty()) {
      depth_limit = std::stoi(std::string(it->value));
    }
  }
  if (options_bool.recursive && it == token_group.options.end()) {
    depth_limit = std::numeric_limits<int>::max();
  }

  std::string start_path =
      token_group.positional.empty()
          ? "."
          : std::string(token_group.positional.front().name);

  if (!std::filesystem::exists(start_path)) {
    std::cerr << std::format("ERROR: {} NO EXISTE\n", start_path);
    return;
  }

  pending_dirs.push({.path = start_path, .depth = 0});

  unsigned int used_thread = std::min(std::thread::hardware_concurrency() == 0
                                          ? 1
                                          : std::thread::hardware_concurrency(),
                                      MAX_THREAD);
  std::mutex write_dirs_queue;
  std::mutex write_dirs_vector;
  std::vector<std::jthread> threads;
  std::condition_variable condition;
  std::atomic<int> working_threads = 0;
  std::atomic<bool> root_submitted = false;

  threads.reserve(used_thread);
  for (size_t i = 0; i < used_thread; i++) {
    threads.emplace_back([&, options_bool, depth_limit](std::stop_token stop) {
      std::vector<FileEntry> file_entry_temp;
      std::vector<PendingDir> temp_pending_dir;

      std::string full_path;
      full_path.reserve(PATH_MAX);

      while (!stop.stop_requested()) {
        PendingDir current;
        {
          std::unique_lock<std::mutex> lock(write_dirs_queue);
          condition.wait(lock, [&] {
            return !pending_dirs.empty() || stop.stop_requested() ||
                   (root_submitted && working_threads == 0);
          });

          if (stop.stop_requested() ||
              (working_threads == 0 && pending_dirs.empty())) {
            return;
          }
          if (pending_dirs.empty()) {
            continue;
          }

          current = std::move(pending_dirs.front());
          pending_dirs.pop();
          working_threads++;
        }

        DIR *dir_ptr = opendir(current.path.c_str());
        if (!dir_ptr) {
          working_threads--;
          condition.notify_all();
          continue;
        }

        struct dirent *entry;
        while ((entry = readdir(dir_ptr)) != nullptr) {
          std::string_view name(entry->d_name);
          if (name == "." || name == "..") {
            continue;
          }
          if (!options_bool.all && name.starts_with('.')) {
            continue;
          }

          full_path = current.path;
          if (full_path.back() != '/') {
            full_path += "/";
          }
          full_path += name;

          if (entry->d_type == DT_DIR && options_bool.recursive &&
              current.depth < depth_limit) {
            temp_pending_dir.push_back(
                {.path = full_path, .depth = current.depth + 1});
          }

          FileEntry entry_data;
          if (!options_bool.long_format) {
            NormalRecolection(entry_data, current.path, entry);
          } else {
            LongRecolection(entry_data, full_path, entry, current.path);
          }
          file_entry_temp.push_back(std::move(entry_data));
        }
        closedir(dir_ptr);

        // Transferencia masiva a cola global
        if (!temp_pending_dir.empty()) {
          std::lock_guard<std::mutex> lock(write_dirs_queue);
          for (auto &d : temp_pending_dir) {
            pending_dirs.push(d);
          }
          temp_pending_dir.clear();
        }

        // Transferencia masiva a vector global
        if (!file_entry_temp.empty()) {
          std::lock_guard<std::mutex> lock(write_dirs_vector);
          file_entry.insert(file_entry.end(),
                            std::make_move_iterator(file_entry_temp.begin()),
                            std::make_move_iterator(file_entry_temp.end()));
          file_entry_temp.clear();
        }

        working_threads--;
        condition.notify_all();
      }
    });
  }

  root_submitted = true;
  condition.notify_all();
  threads.clear();
  //-----------------------------------------------------------------------------------------
  auto run_pipeline = [&](OptionCategory target_cat) {
    for (const auto &opt : token_group.options) {
      auto metadata = GetOptionData(opt.name);
      if (metadata && metadata->category == target_cat) {
        FilterStruct fs{.entries = file_entry, .context = opt.value};
        std::visit(
            [&](auto &handler) {
              if constexpr (std::is_same_v<std::decay_t<decltype(handler)>,
                                           FilteringProcess>) {
                handler(fs);
              }
            },
            metadata->hanlder);
      }
    }
  };

  run_pipeline(OptionCategory::FILTERING);
  run_pipeline(OptionCategory::SORTING);

  if (options_bool.long_format == true) {
    LongPrinter(file_entry);
  } else {
    NormalPrinter(file_entry);
  }
}
