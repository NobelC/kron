#include "../../include/option/option-raw-metadata.hpp"
#include "../../include/option/option-implementation.hpp"
#include "special-option/version-option.hpp"
#include <algorithm>
#include <any>
#include <cctype>
#include <filesystem>
#include <ranges>
#include <sys/stat.h>

void CreatedOptionData(){

  OptionMetaData help;
  help.normalized_name = "--help";
  help.alias_name      = "-h";
  help.conflict_name   = {};
  help.data_type       = TypeDataReceived::NONE;
  help.category        = OptionCategory::GLOBAL;
  help.hanlder = std::monostate{};
  GeneralOptionLog(help);

  OptionMetaData version;
  version.normalized_name = "--version";
  version.alias_name      = "-v";
  version.conflict_name   = {};
  version.data_type       = TypeDataReceived::NONE;
  version.category        = OptionCategory::GLOBAL;
  version.hanlder = std::monostate{};
  GeneralOptionLog(version);

  OptionMetaData no_color;
  no_color.normalized_name = "--no-color";
  no_color.alias_name      = "";
  no_color.conflict_name   = {};
  no_color.data_type       = TypeDataReceived::NONE;
  no_color.category        = OptionCategory::GLOBAL;
  no_color.hanlder = std::monostate{};
  GeneralOptionLog(no_color);

  OptionMetaData quiet;
  quiet.normalized_name = "--quiet";
  quiet.alias_name      = "-q";
  quiet.conflict_name   = { "--verbose" };
  quiet.data_type       = TypeDataReceived::NONE;
  quiet.category        = OptionCategory::GLOBAL;
  quiet.hanlder = std::monostate{};
  GeneralOptionLog(quiet);

  OptionMetaData verbose;
  verbose.normalized_name = "--verbose";
  verbose.alias_name      = "";
  verbose.conflict_name   = { "--quiet" };
  verbose.data_type       = TypeDataReceived::NONE;
  verbose.category        = OptionCategory::GLOBAL;
  verbose.hanlder = std::monostate{};
  GeneralOptionLog(verbose);

  OptionMetaData dry_run;
  dry_run.normalized_name = "--dry-run";
  dry_run.alias_name      = "-n";
  dry_run.conflict_name   = {};
  dry_run.data_type       = TypeDataReceived::NONE;
  dry_run.category        = OptionCategory::MANIPULATION;
  dry_run.hanlder = std::monostate{};
  GeneralOptionLog(dry_run);

  OptionMetaData output;
  output.normalized_name = "--output";
  output.alias_name      = "-o";
  output.conflict_name   = {};
  output.data_type       = TypeDataReceived::EXTENSION;
  output.category        = OptionCategory::PRESENTATION;
  // output.hanlder = OutputFormatHandler;
  GeneralOptionLog(output);

  OptionMetaData all;
  all.normalized_name = "--all";
  all.alias_name      = "-a";
  all.conflict_name   = { "--no-hidden" };
  all.data_type       = TypeDataReceived::NONE;
  all.category        = OptionCategory::COLLECTION;
  all.hanlder = std::monostate{};
  GeneralOptionLog(all);

  OptionMetaData long_format;
  long_format.normalized_name = "--long";
  long_format.alias_name      = "-l";
  long_format.conflict_name   = {};
  long_format.data_type       = TypeDataReceived::NONE;
  long_format.category        = OptionCategory::PRESENTATION;
  long_format.hanlder = std::monostate{};
  GeneralOptionLog(long_format);

  OptionMetaData recursive;
  recursive.normalized_name = "--recursive";
  recursive.alias_name      = "-r";
  recursive.conflict_name   = {};
  recursive.data_type       = TypeDataReceived::NONE;
  recursive.category        = OptionCategory::COLLECTION;
  recursive.hanlder = std::monostate{};
  GeneralOptionLog(recursive);

  OptionMetaData depth;
  depth.normalized_name = "--depth";
  depth.alias_name      = "-d";
  depth.conflict_name   = {};
  depth.requieres_name  = {"--recursive"};
  depth.data_type       = TypeDataReceived::EXTENSION;
  depth.category        = OptionCategory::FILTERING;
  // depth.hanlder = DepthHandler;
  GeneralOptionLog(depth);

  OptionMetaData stats;
  stats.normalized_name = "--stats";
  stats.alias_name = "";
  stats.conflict_name = {};
  stats.data_type = TypeDataReceived::NONE;
  stats.category = OptionCategory::PRESENTATION;
  stats.hanlder = std::monostate{};
  GeneralOptionLog(stats);

  OptionMetaData stats_only;
  stats_only.normalized_name = "--stats-only";
  stats_only.alias_name = "";
  stats_only.conflict_name = {};
  stats_only.data_type = TypeDataReceived::NONE;
  stats_only.category = OptionCategory::PRESENTATION;
  stats_only.hanlder = std::monostate{};
  GeneralOptionLog(stats_only);

  OptionMetaData sort;
  sort.normalized_name = "--sort";
  sort.alias_name      = "";
  sort.conflict_name   = {};
  sort.data_type       = TypeDataReceived::STRING;
  sort.category        = OptionCategory::SORTING;
  sort.hanlder = FilteringProcess([](FilterStruct& filter_contex){
      const auto* any = std::any_cast<std::string>(&filter_contex.context);
      if(!any) {return ;}
      std::string ordered_filter = *any;
      if(ordered_filter == "name"){
        std::vector<std::string> entry_name;

        for(const auto& entry : filter_contex.entries){
            std::string name_temp = entry.path().filename();
            std::ranges::transform(name_temp, name_temp.begin(), ::tolower);
            entry_name.emplace_back(name_temp);
        }
        std::ranges::sort(entry_name,[](const auto& a,const auto& b)
          {
            return a < b;
          });
        std::ranges::transform(entry_name, filter_contex.entries.begin(), [](const auto& entry){
            std::filesystem::directory_entry entry_convert(entry);
            return entry_convert;
            });
      }
      if(ordered_filter == "size"){
        std::vector<std::pair<off_t, std::filesystem::directory_entry>> temp_data;
        for(const auto& entry : filter_contex.entries){
                 auto size_file = !entry.is_directory() ? entry.file_size() : 0;
                 temp_data.push_back({size_file, entry});
        }
        std::ranges::sort(temp_data, [](const std::pair<off_t, std::filesystem::directory_entry>& a,
            std::pair<off_t,std::filesystem::directory_entry>& b)
            {
              return std::pair(a.first,a.second.path().filename()) > std::pair(b.first,b.second.path().filename()); 
            });
        std::ranges::transform(temp_data, filter_contex.entries.begin(), [](
              std::pair<off_t,std::filesystem::directory_entry>&entry)
            {
              return entry.second;
            });
      }
      if(ordered_filter == "type"){
        std::vector<std::pair<bool , std::filesystem::directory_entry>> temp_data;
        for(const auto& entry : filter_contex.entries){
          bool entry_bool = !entry.is_directory();
          temp_data.push_back({entry_bool, entry});
        }
        std::ranges::sort(temp_data, [](const std::pair<bool, std::filesystem::directory_entry>& a, 
            std::pair<bool, std::filesystem::directory_entry>&b)
            {
              return std::pair(a.first, a.second.path().filename()) > std::pair(b.first,b.second.path().filename());
            });
        std::ranges::transform(temp_data, filter_contex.entries.begin(), [](const auto& entry)
            {
              return entry.second; 
            });
      }
      if(ordered_filter == "modified"){
        std::vector<std::pair<std::filesystem::file_time_type, std::filesystem::directory_entry>> temp_entry;
        for(const auto& entry : filter_contex.entries){
          temp_entry.push_back({entry.last_write_time(), entry});
        }

        std::ranges::sort(temp_entry, [](const auto& a, const auto& b)
        {
          return std::pair(a.first,a.second.path().filename()) < std::pair(b.first,b.second.path().filename()); 
        });

        std::ranges::transform(temp_entry, filter_contex.entries.begin(), [](const auto& entry)
        {
              return entry.second; 
        });


      }
      if(ordered_filter == "ext" || ordered_filter == "extension"){
        std::vector<std::pair<std::string, std::filesystem::directory_entry>> temp_entry;
        for(const auto& entry : filter_contex.entries){
          temp_entry.push_back({entry.path().extension().string(), entry});
        }

        std::ranges::sort(temp_entry, [](const auto& a, const auto& b)
        {
          return std::pair(a.first,a.second.path().filename()) < std::pair(b.first,b.second.path().filename()); 
        });

        std::ranges::transform(temp_entry, filter_contex.entries.begin(), [](const auto& entry)
        {
              return entry.second; 
        });

          std::ranges::sort(filter_contex.entries, [](const std::filesystem::directory_entry& a, 
              const std::filesystem::directory_entry& b){
          return a.path().extension() < b.path().extension(); 
        });
      }
      });
  GeneralOptionLog(sort);

  OptionMetaData reverse;
  reverse.normalized_name = "--reverse";
  reverse.alias_name      = "";
  reverse.conflict_name   = {};
  reverse.data_type       = TypeDataReceived::NONE;
  reverse.category        = OptionCategory::SORTING;
  reverse.hanlder = FilteringProcess([](FilterStruct& filter_contex){
      std::ranges::reverse(filter_contex.entries);
      });
  GeneralOptionLog(reverse);

  OptionMetaData dirs_first;
  dirs_first.normalized_name = "--dirs-first";
  dirs_first.alias_name      = "";
  dirs_first.conflict_name   = {};
  dirs_first.data_type       = TypeDataReceived::NONE;
  dirs_first.category        = OptionCategory::SORTING;
  dirs_first.hanlder = FilteringProcess([](FilterStruct& filter_contex){
      std::ranges::sort(filter_contex.entries, 
          [](const std::filesystem::directory_entry& a , const std::filesystem::directory_entry& b){
            return std::pair(!a.is_directory(), a.path().filename()) < std::pair(!b.is_directory(),b.path().filename());
          });
      });
  GeneralOptionLog(dirs_first);

  OptionMetaData filter;
  filter.normalized_name = "--filter";
  filter.alias_name      = "";
  filter.conflict_name   = {};
  filter.data_type       = TypeDataReceived::STRING;
  filter.category        = OptionCategory::FILTERING;
  filter.hanlder = FilteringProcess([]( FilterStruct& filter_contex){
      const auto* filter_raw = std::any_cast<std::string>(&filter_contex.context);
      if(!filter_raw){return ;}
      
      const std::string& pattern = *filter_raw;
      std::erase_if(filter_contex.entries, [&pattern](const auto& e){
        std::string filename = e.path().filename().string();

        if(pattern.starts_with('*')){
          const std::string end_pattern = pattern.substr(1);
          return !filename.ends_with(end_pattern);
        }
        if(pattern.ends_with('*')){
          const std::string start_pattern = pattern.substr(0, (pattern.size() - 1));
          return !filename.starts_with(start_pattern);
        }
        return filename != pattern;
      });
      
  });
  GeneralOptionLog(filter);

  OptionMetaData no_header;
  no_header.normalized_name = "--no-header";
  no_header.alias_name      = "";
  no_header.conflict_name   = {};
  no_header.data_type       = TypeDataReceived::NONE;
  no_header.category        = OptionCategory::PRESENTATION;
  no_header.hanlder = std::monostate{};
  GeneralOptionLog(no_header);

  OptionMetaData follow_symlinks;
  follow_symlinks.normalized_name = "--follow-symlinks";
  follow_symlinks.alias_name      = "";
  follow_symlinks.conflict_name   = {};
  follow_symlinks.data_type       = TypeDataReceived::NONE;
  follow_symlinks.category        = OptionCategory::COLLECTION;
  // follow_symlinks.hanlder = FollowSymlinksHandler;
  GeneralOptionLog(follow_symlinks);

  OptionMetaData fields;
  fields.normalized_name = "--fields";
  fields.alias_name      = "";
  fields.conflict_name   = {};
  fields.data_type       = TypeDataReceived::STRING;
  fields.category        = OptionCategory::PRESENTATION;
  fields.hanlder = std::monostate{};
  GeneralOptionLog(fields);

  OptionMetaData force;
  force.normalized_name = "--force";
  force.alias_name      = "-f";
  force.conflict_name   = { "--no-overwrite", "--skip-existing" };
  force.data_type       = TypeDataReceived::NONE;
  force.category        = OptionCategory::MANIPULATION;
  // force.hanlder = std::monostate{};
  GeneralOptionLog(force);

  OptionMetaData no_overwrite;
  no_overwrite.normalized_name = "--no-overwrite";
  no_overwrite.alias_name      = "";
  no_overwrite.conflict_name   = { "--force", "--skip-existing" };
  no_overwrite.data_type       = TypeDataReceived::NONE;
  no_overwrite.category        = OptionCategory::MANIPULATION;
  // no_overwrite.hanlder = std::monostate{};
  GeneralOptionLog(no_overwrite);

  OptionMetaData skip_existing;
  skip_existing.normalized_name = "--skip-existing";
  skip_existing.alias_name      = "";
  skip_existing.conflict_name   = { "--force", "--no-overwrite" };
  skip_existing.data_type       = TypeDataReceived::NONE;
  skip_existing.category        = OptionCategory::MANIPULATION;
  // skip_existing.hanlder = std::monostate{};
  GeneralOptionLog(skip_existing);

  OptionMetaData preserve;
  preserve.normalized_name = "--preserve";
  preserve.alias_name      = "-p";
  preserve.conflict_name   = {};
  preserve.data_type       = TypeDataReceived::NONE;
  preserve.category        = OptionCategory::MANIPULATION;
  // preserve.hanlder = std::monostate{};
  GeneralOptionLog(preserve);

  OptionMetaData match;
  match.normalized_name = "--match";
  match.alias_name      = "";
  match.conflict_name   = {};
  match.data_type       = TypeDataReceived::EXTENSION;
  match.category        = OptionCategory::FILTERING;
  // match.hanlder = MatchHandler;
  GeneralOptionLog(match);

  OptionMetaData name;
  name.normalized_name = "--name";
  name.alias_name      = "";
  name.conflict_name   = {};
  name.data_type       = TypeDataReceived::EXTENSION;
  name.category        = OptionCategory::FILTERING;
  // name.hanlder = NameHandler;
  GeneralOptionLog(name);

  OptionMetaData ext;
  ext.normalized_name = "--ext";
  ext.alias_name      = "";
  ext.conflict_name   = {};
  ext.data_type       = TypeDataReceived::EXTENSION;
  ext.category        = OptionCategory::FILTERING;
  // ext.hanlder = ExtHandler;
  GeneralOptionLog(ext);

  OptionMetaData type;
  type.normalized_name = "--type";
  type.alias_name      = "";
  type.conflict_name   = {};
  type.data_type       = TypeDataReceived::EXTENSION;
  type.category        = OptionCategory::FILTERING;
  // type.hanlder = TypeHandler;
  GeneralOptionLog(type);

  OptionMetaData size_gt;
  size_gt.normalized_name = "--size-gt";
  size_gt.alias_name      = "";
  size_gt.conflict_name   = {};
  size_gt.data_type       = TypeDataReceived::SIZE;
  size_gt.category        = OptionCategory::FILTERING;
  // size_gt.hanlder = SizeGtHandler;
  GeneralOptionLog(size_gt);

  OptionMetaData size_lt;
  size_lt.normalized_name = "--size-lt";
  size_lt.alias_name      = "";
  size_lt.conflict_name   = {};
  size_lt.data_type       = TypeDataReceived::SIZE;
  size_lt.category        = OptionCategory::FILTERING;
  // size_lt.hanlder = SizeLtHandler;
  GeneralOptionLog(size_lt);

  OptionMetaData modified_after;
  modified_after.normalized_name = "--modified-after";
  modified_after.alias_name      = "";
  modified_after.conflict_name   = {};
  modified_after.data_type       = TypeDataReceived::DATE;
  modified_after.category        = OptionCategory::FILTERING;
  // modified_after.hanlder = ModifiedAfterHandler;
  GeneralOptionLog(modified_after);

  OptionMetaData modified_before;
  modified_before.normalized_name = "--modified-before";
  modified_before.alias_name      = "";
  modified_before.conflict_name   = {};
  modified_before.data_type       = TypeDataReceived::DATE;
  modified_before.category        = OptionCategory::FILTERING;
  // modified_before.hanlder = ModifiedBeforeHandler;
  GeneralOptionLog(modified_before);

  OptionMetaData no_hidden;
  no_hidden.normalized_name = "--no-hidden";
  no_hidden.alias_name      = "";
  no_hidden.conflict_name   = { "--all" };
  no_hidden.data_type       = TypeDataReceived::NONE;
  no_hidden.category        = OptionCategory::FILTERING;
  // no_hidden.hanlder = NoHiddenHandler;
  GeneralOptionLog(no_hidden);

  OptionMetaData parents;
  parents.normalized_name = "--parents";
  parents.alias_name      = "-p";
  parents.conflict_name   = {};
  parents.data_type       = TypeDataReceived::NONE;
  parents.category        = OptionCategory::CREATION;
  // parents.hanlder = std::monostate{};
  GeneralOptionLog(parents);

  OptionMetaData timestamp;
  timestamp.normalized_name = "--timestamp";
  timestamp.alias_name      = "";
  timestamp.conflict_name   = {};
  timestamp.data_type       = TypeDataReceived::DATE;
  timestamp.category        = OptionCategory::CREATION;
  // timestamp.hanlder = TimestampHandler;
  GeneralOptionLog(timestamp);

  OptionMetaData no_create;
  no_create.normalized_name = "--no-create";
  no_create.alias_name      = "";
  no_create.conflict_name   = {};
  no_create.data_type       = TypeDataReceived::NONE;
  no_create.category        = OptionCategory::CREATION;
  // no_create.hanlder = std::monostate{};
  GeneralOptionLog(no_create);
}
