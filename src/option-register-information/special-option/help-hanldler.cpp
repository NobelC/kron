#include "../../../include/special-option/help-option.hpp"
#include <string>
#include <format>
#include <iostream>

// ─── Column widths ────────────────────────────────────────────────────────────
// OPT  : option name  (--long-option-name)
// ALIAS: short alias  (-x)
// TYPE : value type   (NONE | STRING | INT)
// DESC : description  (rest of the line)
// ─────────────────────────────────────────────────────────────────────────────
static constexpr int W_OPT   = 26;
static constexpr int W_ALIAS =  8;
static constexpr int W_TYPE  = 10;
static constexpr int W_DESC  = 48;

namespace {

// ─── Helpers de formato ───────────────────────────────────────────────────────

void print_section(const std::string& title) {
    std::cout << std::format("\n{}\n{}\n", title, std::string(title.size(), '-'));
}

void print_row(const std::string& opt, const std::string& alias,
               const std::string& type, const std::string& desc) {
    std::cout << std::format("  {:<{}} {:<{}} {:<{}} {}\n",
        opt,   W_OPT,
        alias, W_ALIAS,
        type,  W_TYPE,
        desc);
}

void print_table_header() {
    print_row("Option", "Alias", "Type", "Description");
    std::cout << std::format("  {}\n", std::string(W_OPT + W_ALIAS + W_TYPE + W_DESC + 3, '-'));
}

// ─── FLAGS GLOBALES ───────────────────────────────────────────────────────────

void help_global_flags() {
    print_section("GLOBAL FLAGS");
    std::cout << "  Applicable to all commands without exception.\n";
    print_table_header();
    print_row("--help",     "-h", "NONE",   "Show detailed help for the command");
    print_row("--version",  "-v", "NONE",   "Show binary version");
    print_row("--no-color", "—",  "NONE",   "Disable ANSI colors in output");
    print_row("--quiet",    "-q", "NONE",   "Errors only, no informational output");
    print_row("--dry-run",  "-n", "NONE",   "Simulate without executing (destructive commands)");
    print_row("--output",   "-o", "STRING", "Format: plain | json | csv");
}

// ─── NIVEL BÁSICO ─────────────────────────────────────────────────────────────

void help_list() {
    print_section("kron list [path]");
    std::cout << "  Lists directory contents with explicit control over format and fields.\n"
              << "  Advanced equivalent of ls.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "[path]", "Directory to list. Default: current directory");

    print_section("  Options");
    print_table_header();
    print_row("--all",        "-a", "NONE",   "Include hidden entries (.gitignore)");
    print_row("--long",       "-l", "NONE",   "Long format: permissions, size, date");
    print_row("--sort",       "—",  "STRING", "Sort by: name | size | modified | type");
    print_row("--reverse",    "-r", "NONE",   "Reverse sort order");
    print_row("--dirs-first", "—",  "NONE",   "Directories before files");
    print_row("--filter",     "—",  "STRING", "Filter by glob: *.cpp, test_*");
    print_row("--no-header",  "—",  "NONE",   "Omit table header");

    print_section("  Examples");
    std::cout << "  kron list\n"
              << "  kron list ./src\n"
              << "  kron list ./src --long --sort size --reverse\n"
              << "  kron list --all --filter \"*.cpp\" --dirs-first\n"
              << "  kron list --output json\n";
}

void help_inspect() {
    print_section("kron inspect <path> [path2...]");
    std::cout << "  Shows complete metadata of one or more files or directories.\n"
              << "  Core command of the system.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "One or more space-separated paths");

    print_section("  Options");
    print_table_header();
    print_row("--all",            "-a", "NONE",   "Include hidden files");
    print_row("--recursive",      "-r", "NONE",   "Inspect subdirectories");
    print_row("--depth",          "-d", "INT",    "Maximum recursion depth");
    print_row("--follow-symlinks","—",  "NONE",   "Follow symbolic links");
    print_row("--fields",         "—",  "STRING", "Fields to show (see below)");
    print_row("--sort",           "—",  "STRING", "Sort by: name | size | modified");
    print_row("--reverse",        "—",  "NONE",   "Reverse sort order");
    print_row("--stats",          "—",  "NONE",   "Print statistical info of folder(s)");

    print_section("  Available fields (--fields)");
    std::cout << std::format("  {:<16} {}\n", "name",       "File or directory name");
    std::cout << std::format("  {:<16} {}\n", "size",       "Size in bytes (or readable unit with auto)");
    std::cout << std::format("  {:<16} {}\n", "modified",   "Last modification date and time (ISO 8601)");
    std::cout << std::format("  {:<16} {}\n", "created",    "Creation date (if system supports it)");
    std::cout << std::format("  {:<16} {}\n", "permissions","Permissions in octal and symbolic: 755 / rwxr-xr-x");
    std::cout << std::format("  {:<16} {}\n", "type",       "Entry type: file | dir | symlink");
    std::cout << std::format("  {:<16} {}\n", "inode",      "Inode number in the filesystem");
    std::cout << std::format("  {:<16} {}\n", "checksum",   "SHA256 hash of file content");
    std::cout << std::format("  {:<16} {}\n", "owner",      "Owner and group: user:group");
    std::cout << std::format("  {:<16} {}\n", "hardlinks",  "Number of hard links pointing to the inode");

    print_section("  Examples");
    std::cout << "  kron inspect ./main.cpp\n"
              << "  kron inspect ./src --recursive --depth 2\n"
              << "  kron inspect file1.cpp file2.cpp --fields name,size,checksum\n"
              << "  kron inspect . --sort modified --reverse --output json\n";
}

void help_copy() {
    print_section("kron copy <source> <dest>");
    std::cout << "  Copies a file or directory with explicit conflict control.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<source>", "Source file or directory");
    std::cout << std::format("  {:<20} {}\n", "<dest>",   "Destination file or directory");

    print_section("  Options");
    print_table_header();
    print_row("--recursive",     "-r", "NONE", "Copy directories recursively");
    print_row("--force",         "-f", "NONE", "Overwrite destination without asking");
    print_row("--no-overwrite",  "—",  "NONE", "Fail if destination already exists");
    print_row("--skip-existing", "—",  "NONE", "Skip files that already exist at destination");
    print_row("--preserve",      "-p", "NONE", "Preserve metadata: dates and permissions");
    print_row("--dry-run",       "-n", "NONE", "Show what would be copied without executing");
    print_row("--verbose",       "-v", "NONE", "Show each copied file");

    print_section("  Conflicts");
    std::cout << "  --force       x  --no-overwrite   → semantic validation error\n"
              << "  --force       x  --skip-existing  → semantic validation error\n";

    print_section("  Examples");
    std::cout << "  kron copy archivo.txt ./backup/\n"
              << "  kron copy ./src ./dst --recursive --preserve\n"
              << "  kron copy ./src ./dst --recursive --skip-existing --dry-run --verbose\n"
              << "  kron copy config.json ./backup/config.json --no-overwrite\n";
}

void help_move() {
    print_section("kron move <source> <dest>");
    std::cout << "  Moves or renames a file or directory.\n"
              << "  Uses atomic rename(2) when source and destination are on the same filesystem.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<source>", "File or directory to move");
    std::cout << std::format("  {:<20} {}\n", "<dest>",   "New location or new name");

    print_section("  Options");
    print_table_header();
    print_row("--force",        "-f", "NONE", "Overwrite destination without asking");
    print_row("--no-overwrite", "—",  "NONE", "Fail if destination exists");
    print_row("--dry-run",      "-n", "NONE", "Simulate operation without executing");
    print_row("--verbose",      "-v", "NONE", "Show operation details");

    print_section("  Conflicts");
    std::cout << "  --force  x  --no-overwrite  → semantic validation error\n";

    print_section("  Examples");
    std::cout << "  kron move archivo.txt ./otro/directorio/\n"
              << "  kron move ./old-name ./new-name\n"
              << "  kron move ./src ./dst --force --verbose\n"
              << "  kron move log.txt ./archive/ --dry-run\n";
}

void help_delete() {
    print_section("kron delete <path> [path2...]");
    std::cout << "  Deletes files or directories.\n"
              << "  Requires explicit confirmation for non-empty directories unless --force is used.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "One or more paths to delete");

    print_section("  Options");
    print_table_header();
    print_row("--recursive", "-r", "NONE",   "Delete directories and all their contents");
    print_row("--force",     "-f", "NONE",   "No interactive confirmation");
    print_row("--dry-run",   "-n", "NONE",   "Show what would be deleted without executing");
    print_row("--verbose",   "-v", "NONE",   "Show each deleted file");
    print_row("--match",     "—",  "STRING", "Only delete entries matching glob");

    print_section("  Examples");
    std::cout << "  kron delete archivo.txt\n"
              << "  kron delete ./build --recursive --force\n"
              << "  kron delete ./logs --match \"*.log\" --dry-run\n"
              << "  kron delete temp1.txt temp2.txt temp3.txt --verbose\n"
              << "  kron delete ./cache --recursive --match \"*.tmp\" --force\n";
}

void help_find() {
    print_section("kron find <directory> [criteria]");
    std::cout << "  Searches for files using combinable criteria (all criteria are AND logic).\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<directory>", "Root directory of the search");

    print_section("  Options");
    print_table_header();
    print_row("--name",            "—",  "STRING", "Search by glob name: test_*.cpp");
    print_row("--ext",             "—",  "STRING", "Search by extension: cpp or .cpp");
    print_row("--type",            "—",  "STRING", "Type: file | dir | symlink");
    print_row("--size-gt",         "—",  "STRING", "Files larger than N: 10MB, 500KB, 2GB");
    print_row("--size-lt",         "—",  "STRING", "Files smaller than N");
    print_row("--modified-after",  "—",  "STRING", "Modified after ISO date");
    print_row("--modified-before", "—",  "STRING", "Modified before ISO date");
    print_row("--recursive",       "-r", "NONE",   "Recursive search (default: true)");
    print_row("--depth",           "—",  "INT",    "Maximum recursion depth");
    print_row("--no-hidden",       "—",  "NONE",   "Exclude hidden files and directories");

    print_section("  Examples");
    std::cout << "  kron find ./src --ext cpp --size-gt 10KB\n"
              << "  kron find . --name \"test_*\" --type file --modified-after 2024-01-01\n"
              << "  kron find ./logs --ext log --size-gt 1MB --output json\n"
              << "  kron find . --size-gt 50MB --size-lt 1GB --no-hidden\n"
              << "  kron find ./src --modified-after 2024-06-01 --modified-before 2024-12-31\n";
}

void help_rename() {
    print_section("kron rename <path> <new-name>");
    std::cout << "  Renames a file or directory in place.\n"
              << "  Atomic operation via rename(2) when possible.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>",     "File or directory to rename");
    std::cout << std::format("  {:<20} {}\n", "<new-name>", "New name (name only, not full path)");

    print_section("  Options");
    print_table_header();
    print_row("--force",   "-f", "NONE", "Overwrite if that name already exists");
    print_row("--dry-run", "-n", "NONE", "Simulate the rename");

    print_section("  Examples");
    std::cout << "  kron rename archivo.txt new-name.txt\n"
              << "  kron rename ./folder new-folder --force\n"
              << "  kron rename main.cpp main_old.cpp --dry-run\n";
}

void help_mkdir() {
    print_section("kron mkdir <path>");
    std::cout << "  Creates one or more directories in the filesystem.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "Path of the directory to create");

    print_section("  Options");
    print_table_header();
    print_row("--parents", "-p", "NONE", "Create intermediate directories if they don't exist");
    print_row("--verbose", "-v", "NONE", "Show each created directory");

    print_section("  Examples");
    std::cout << "  kron mkdir ./new-folder\n"
              << "  kron mkdir ./a/b/c/d --parents\n"
              << "  kron mkdir ./build --verbose\n";
}

void help_touch() {
    print_section("kron touch <path> [path2...]");
    std::cout << "  Creates empty files or updates the modification date of existing files.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "One or more paths");

    print_section("  Options");
    print_table_header();
    print_row("--no-create",  "—", "NONE",   "Only update date if exists, do not create");
    print_row("--timestamp",  "—", "STRING", "Custom ISO date: 2024-06-01T12:00:00");

    print_section("  Examples");
    std::cout << "  kron touch new-file.txt\n"
              << "  kron touch a.txt b.txt c.txt\n"
              << "  kron touch file.cpp --timestamp 2024-01-01T00:00:00\n"
              << "  kron touch file.txt --no-create\n";
}

// ─── NIVEL MEDIO ──────────────────────────────────────────────────────────────

void help_tree() {
    print_section("kron tree [path]");
    std::cout << "  Visualizes directory structure as a hierarchical tree with Unicode box characters.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "[path]", "Root directory. Default: current directory");

    print_section("  Options");
    print_table_header();
    print_row("--depth",     "-d", "INT",    "Maximum tree depth");
    print_row("--all",       "-a", "NONE",   "Include hidden files");
    print_row("--dirs-only", "—",  "NONE",   "Show directories only");
    print_row("--size",      "—",  "NONE",   "Show size of each entry");
    print_row("--filter",    "—",  "STRING", "Filter entries by glob");

    print_section("  Examples");
    std::cout << "  kron tree\n"
              << "  kron tree ./src --depth 3\n"
              << "  kron tree . --all --size --output json\n"
              << "  kron tree ./project --dirs-only --depth 2\n";
}

void help_size() {
    print_section("kron size <path> [path2...]");
    std::cout << "  Calculates the real disk size of files or directories.\n"
              << "  For directories, calculates the accumulated size of all contents.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "One or more paths");

    print_section("  Options");
    print_table_header();
    print_row("--unit",    "—",  "STRING", "Unit: B | KB | MB | GB | auto (default)");
    print_row("--sort",    "—",  "NONE",   "Sort results by size ascending");
    print_row("--reverse", "—",  "NONE",   "Sort from largest to smallest");
    print_row("--depth",   "-d", "INT",    "Breakdown by depth level");

    print_section("  Examples");
    std::cout << "  kron size ./\n"
              << "  kron size ./src ./build --unit MB --sort --reverse\n"
              << "  kron size . --depth 2 --output csv\n"
              << "  kron size ./node_modules --unit MB\n";
}

void help_hash() {
    print_section("kron hash <path> [path2...]");
    std::cout << "  Calculates the cryptographic checksum of files for integrity verification.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "One or more file paths");

    print_section("  Options");
    print_table_header();
    print_row("--algo",      "—",  "STRING", "Algorithm: md5 | sha1 | sha256 | sha512");
    print_row("--recursive", "-r", "NONE",   "Apply to all files in directory");
    print_row("--compare",   "—",  "NONE",   "Compare two files: returns identical/not");

    print_section("  Examples");
    std::cout << "  kron hash file.bin\n"
              << "  kron hash file1.iso file2.iso --compare\n"
              << "  kron hash ./dist --recursive --algo sha512 --output json\n"
              << "  kron hash ./src --recursive --algo md5 --output csv\n";
}

void help_permissions() {
    print_section("kron permissions <path> [path2...]");
    std::cout << "  Shows permissions of files and directories, and optionally modifies them.\n"
              << "  Supports octal and symbolic notation.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "One or more paths");

    print_section("  Options");
    print_table_header();
    print_row("--set",       "—",  "STRING", "Permissions: octal 755 or symbolic rwxr-xr-x");
    print_row("--recursive", "-r", "NONE",   "Apply change recursively");
    print_row("--dry-run",   "-n", "NONE",   "Simulate changes without applying them");
    print_row("--verbose",   "-v", "NONE",   "Show each change made");

    print_section("  Formats for --set");
    std::cout << std::format("  {:<16} {}\n", "Octal",    "755, 644, 600, 777");
    std::cout << std::format("  {:<16} {}\n", "Symbolic", "rwxr-xr-x, rw-r--r--, rw-------");

    print_section("  Examples");
    std::cout << "  kron permissions ./script.sh\n"
              << "  kron permissions ./script.sh --set 755\n"
              << "  kron permissions ./dist --set 644 --recursive --dry-run\n"
              << "  kron permissions ./private --set 600 --recursive --verbose\n";
}

void help_symlink() {
    print_section("kron symlink <source> <link>");
    std::cout << "  Creates symbolic links with control over path type (absolute or relative).\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<source>", "File or directory the link will point to");
    std::cout << std::format("  {:<20} {}\n", "<link>",   "Path where the symbolic link will be created");

    print_section("  Options");
    print_table_header();
    print_row("--force",    "-f", "NONE", "Overwrite if a link already exists");
    print_row("--relative", "—",  "NONE", "Create relative link instead of absolute");
    print_row("--dry-run",  "-n", "NONE", "Simulate link creation");

    print_section("  Examples");
    std::cout << "  kron symlink /usr/bin/python3 ./python\n"
              << "  kron symlink ../config/settings.json ./settings.json --relative\n"
              << "  kron symlink ./build/output /tmp/latest --force\n";
}

void help_compare() {
    print_section("kron compare <path1> <path2>");
    std::cout << "  Compares two files or directories and shows differences in content, metadata, or both.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path1>", "First file or directory");
    std::cout << std::format("  {:<20} {}\n", "<path2>", "Second file or directory");

    print_section("  Options");
    print_table_header();
    print_row("--mode",          "—",  "STRING", "content | metadata | both (default)");
    print_row("--recursive",     "-r", "NONE",   "Compare complete directories");
    print_row("--ignore-hidden", "—",  "NONE",   "Ignore hidden files");

    print_section("  Examples");
    std::cout << "  kron compare file_v1.cpp file_v2.cpp\n"
              << "  kron compare ./build_old ./build_new --recursive --mode content\n"
              << "  kron compare ./config_a ./config_b --output json\n"
              << "  kron compare ./deploy_a ./deploy_b --recursive --ignore-hidden\n";
}

void help_watch() {
    print_section("kron watch <path>");
    std::cout << "  Monitors changes in real time on a file or directory using kernel events (inotify on Linux).\n"
              << "  Runs indefinitely until SIGINT (Ctrl+C).\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "File or directory to monitor");

    print_section("  Options");
    print_table_header();
    print_row("--events",    "—",  "STRING", "create | modify | delete | all (default)");
    print_row("--recursive", "-r", "NONE",   "Monitor subdirectories");
    print_row("--filter",    "—",  "STRING", "Filter events by name glob");

    print_section("  Examples");
    std::cout << "  kron watch ./src\n"
              << "  kron watch ./logs --events create,delete\n"
              << "  kron watch . --recursive --filter \"*.cpp\" --output json\n";
}

void help_recent() {
    print_section("kron recent [path]");
    std::cout << "  Lists the most recently modified files in a directory, ordered by modification date descending.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "[path]", "Root directory. Default: current directory");

    print_section("  Options");
    print_table_header();
    print_row("--limit",     "-n", "INT",    "Number of results. Default: 10");
    print_row("--since",     "—",  "STRING", "Only files since ISO date: 2024-01-01");
    print_row("--recursive", "-r", "NONE",   "Include subdirectories");
    print_row("--type",      "—",  "STRING", "Filter by type: file | dir");

    print_section("  Examples");
    std::cout << "  kron recent\n"
              << "  kron recent ./src --limit 20 --recursive\n"
              << "  kron recent . --since 2024-06-01 --type file --output csv\n"
              << "  kron recent ./project --recursive --limit 50\n";
}

void help_empty() {
    print_section("kron empty <path>");
    std::cout << "  Detects empty files (0 bytes) or empty directories at the specified path.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "Directory to search for empty entries");

    print_section("  Options");
    print_table_header();
    print_row("--recursive", "-r", "NONE",   "Search recursively in subdirectories");
    print_row("--delete",    "—",  "NONE",   "Delete the empty entries found");
    print_row("--force",     "-f", "NONE",   "Delete without confirmation (requires --delete)");
    print_row("--type",      "—",  "STRING", "Filter: file | dir | both (default: both)");

    print_section("  Examples");
    std::cout << "  kron empty ./src --recursive\n"
              << "  kron empty . --recursive --delete --force\n"
              << "  kron empty ./build --type file --dry-run\n"
              << "  kron empty ./logs --recursive --type file --delete\n";
}

// ─── NIVEL AVANZADO ───────────────────────────────────────────────────────────

void help_duplicates() {
    print_section("kron duplicates <path>");
    std::cout << "  Finds duplicate files by actual content (hash comparison), not by name.\n"
              << "  Groups all files with identical content.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "Root directory to search for duplicates");

    print_section("  Options");
    print_table_header();
    print_row("--algo",        "—",  "STRING", "sha256 (default) | md5 | sha1");
    print_row("--min-size",    "—",  "STRING", "Ignore files smaller than N: 1KB, 10MB");
    print_row("--recursive",   "-r", "NONE",   "Search entire hierarchy");
    print_row("--delete",      "—",  "NONE",   "Delete duplicates (keeps the first)");
    print_row("--interactive", "—",  "NONE",   "Ask which to keep in each group");
    print_row("--force",       "-f", "NONE",   "Delete without confirmation (with --delete)");

    print_section("  Conflicts");
    std::cout << "  --interactive  x  --force  → semantic validation error\n";

    print_section("  Examples");
    std::cout << "  kron duplicates ./photos --recursive\n"
              << "  kron duplicates . --min-size 1MB --algo sha256 --output json\n"
              << "  kron duplicates ./downloads --delete --interactive\n"
              << "  kron duplicates ./project --recursive --min-size 100KB --delete --force\n";
}

void help_analyze() {
    print_section("kron analyze <path>");
    std::cout << "  Deep statistical analysis of a directory: distribution by type, extension, size and dates.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "Directory to analyze");

    print_section("  Options");
    print_table_header();
    print_row("--recursive", "-r", "NONE",   "Full analysis of directory tree");
    print_row("--by",        "—",  "STRING", "Group by: ext | type | size-range | date");
    print_row("--top",       "—",  "INT",    "Show top N per category. Default: 10");

    print_section("  Examples");
    std::cout << "  kron analyze ./project --recursive\n"
              << "  kron analyze . --by ext --top 20 --output json\n"
              << "  kron analyze ./src --by size-range --recursive\n"
              << "  kron analyze ./downloads --by date --top 5 --output csv\n";
}

void help_snapshot() {
    print_section("kron snapshot <path>");
    std::cout << "  Takes a snapshot of the directory state (names, sizes, hashes, dates, permissions)\n"
              << "  and saves it to a JSON file. Base for audits and future comparisons with diff-snapshot.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "Directory to capture");

    print_section("  Options");
    print_table_header();
    print_row("--output-file", "-f", "STRING", "Destination file. Default: snapshot.json");
    print_row("--algo",        "—",  "STRING", "Hash: sha256 (default) | md5 | sha1");
    print_row("--recursive",   "-r", "NONE",   "Full snapshot of directory tree");
    print_row("--exclude",     "—",  "STRING", "Exclude glob patterns: *.log");
    print_row("--fields",      "—",  "STRING", "name | size | hash | modified | perms");

    print_section("  Examples");
    std::cout << "  kron snapshot ./project --recursive -f initial-state.json\n"
              << "  kron snapshot . --exclude \"*.log\" --exclude \"node_modules\"\n"
              << "  kron snapshot ./src --fields name,size,hash\n"
              << "  kron snapshot ./deploy --recursive --algo sha512 -f pre-deploy.json\n";
}

void help_diff_snapshot() {
    print_section("kron diff-snapshot <snapshot1> <snapshot2>");
    std::cout << "  Compares two snapshots generated by kron snapshot and shows exactly\n"
              << "  what changed between the two filesystem states.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<snapshot1>", "JSON file of previous state (reference)");
    std::cout << std::format("  {:<20} {}\n", "<snapshot2>", "JSON file of current state");

    print_section("  Options");
    print_table_header();
    print_row("--mode", "—", "STRING", "added | deleted | modified | moved | all");

    print_section("  Change categories");
    std::cout << std::format("  {:<16} {}\n", "added",    "In snapshot2 but not snapshot1 (new files)");
    std::cout << std::format("  {:<16} {}\n", "deleted",  "In snapshot1 but not snapshot2 (removed files)");
    std::cout << std::format("  {:<16} {}\n", "modified", "Same path, different hash (content changed)");
    std::cout << std::format("  {:<16} {}\n", "moved",    "Same hash, different path (renamed or moved)");

    print_section("  Examples");
    std::cout << "  kron diff-snapshot initial-state.json current-state.json\n"
              << "  kron diff-snapshot before.json after.json --mode modified,added\n"
              << "  kron diff-snapshot snap1.json snap2.json --output json\n"
              << "  kron diff-snapshot pre-deploy.json post-deploy.json --mode all\n";
}

void help_index() {
    print_section("kron index <path>");
    std::cout << "  Builds a complete queryable index of the filesystem into a file\n"
              << "  for later queries without re-scanning the disk. Base for kron query.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "Root directory to index");

    print_section("  Options");
    print_table_header();
    print_row("--output-file", "-f", "STRING", "Destination file. Default: kron-index");
    print_row("--fields",      "—",  "STRING", "name | size | hash | modified | perms");
    print_row("--exclude",     "—",  "STRING", "Exclude glob patterns");
    print_row("--recursive",   "-r", "NONE",   "Index entire tree");
    print_row("--algo",        "—",  "STRING", "Hash algorithm if hash is in --fields");

    print_section("  Examples");
    std::cout << "  kron index /media/external-disk --recursive -f disk.json\n"
              << "  kron index ./project --fields name,size,hash,modified\n"
              << "  kron index . --exclude \"*.tmp\" --exclude \".git\" --recursive\n"
              << "  kron index ./src --algo sha256 --fields name,size,hash --output-file src-index\n";
}

void help_query() {
    print_section("kron query <index> <expression>");
    std::cout << "  Queries an index generated by kron index using combinable filter expressions.\n"
              << "  Without re-scanning the disk.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<index>",      "Index file generated by kron index");
    std::cout << std::format("  {:<20} {}\n", "<expression>", "Filter expression in quotes");

    print_section("  Options");
    print_table_header();
    print_row("--limit",   "-n", "INT",    "Maximum number of results");
    print_row("--sort",    "—",  "STRING", "Field: name | size | modified");
    print_row("--reverse", "—",  "NONE",   "Reverse sort order");

    print_section("  Expression syntax");
    std::cout << std::format("  {:<20} {}\n", "==",         "Exact equality:    ext == \"cpp\"");
    std::cout << std::format("  {:<20} {}\n", "!=",         "Inequality:        type != \"dir\"");
    std::cout << std::format("  {:<20} {}\n", "> < >= <=",  "Comparison:        size > 10MB");
    std::cout << std::format("  {:<20} {}\n", "&&",         "AND logic:         ext == \"log\" && size > 1MB");
    std::cout << std::format("  {:<20} {}\n", "||",         "OR logic:          ext == \"cpp\" || ext == \"hpp\"");
    std::cout << std::format("  {:<20} {}\n", "~= (glob)",  "Glob match:        name ~= \"test_*\"");

    print_section("  Examples");
    std::cout << "  kron query disk.json \"size > 100MB\"\n"
              << "  kron query index.json \"ext == 'cpp' && modified > 2024-01-01\" --sort size\n"
              << "  kron query index.json \"name ~= 'test_*'\" --limit 50 --output csv\n"
              << "  kron query src-index \"size > 10KB && ext == 'hpp'\" --sort size --reverse\n"
              << "  kron query disk.json \"type == 'file' && size > 1GB\" --output json\n";
}

void help_flatten() {
    print_section("kron flatten <path> <dest>");
    std::cout << "  Moves all files from a directory tree into a single flat destination directory,\n"
              << "  removing the folder hierarchy.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "Source root directory with its tree");
    std::cout << std::format("  {:<20} {}\n", "<dest>", "Flat destination directory");

    print_section("  Options");
    print_table_header();
    print_row("--rename-on-conflict", "—",  "NONE",   "Auto-rename on conflict: file_1.txt");
    print_row("--skip-existing",      "—",  "NONE",   "Skip files that already exist");
    print_row("--dry-run",            "-n", "NONE",   "Show plan without executing");
    print_row("--verbose",            "-v", "NONE",   "Show each moved file");
    print_row("--filter",             "—",  "STRING", "Only flatten files that match glob");

    print_section("  Conflicts");
    std::cout << "  --rename-on-conflict  x  --skip-existing  → semantic validation error\n";

    print_section("  Examples");
    std::cout << "  kron flatten ./organized-photos ./flat-photos --rename-on-conflict\n"
              << "  kron flatten ./downloads ./all --dry-run --verbose\n"
              << "  kron flatten ./src ./flat --filter \"*.cpp\"\n"
              << "  kron flatten ./project ./export --rename-on-conflict --verbose\n";
}

void help_organize() {
    print_section("kron organize <path>");
    std::cout << "  Automatically organizes files in a directory into subdirectories\n"
              << "  based on predefined or custom rules.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "Directory to organize");

    print_section("  Options");
    print_table_header();
    print_row("--by",          "—",  "STRING", "ext | date | size | type");
    print_row("--date-format", "—",  "STRING", "YYYY | YYYY-MM | YYYY-MM-DD");
    print_row("--dry-run",     "-n", "NONE",   "Show proposed organization");
    print_row("--verbose",     "-v", "NONE",   "Show each move");
    print_row("--rules-file",  "—",  "STRING", "JSON file with custom rules");

    print_section("  Organization criteria (--by)");
    std::cout << std::format("  {:<16} {}\n", "ext",        "Subdirectory per extension: cpp/ hpp/ json/ txt/");
    std::cout << std::format("  {:<16} {}\n", "date",       "Subdirectory per date with --date-format: 2024/");
    std::cout << std::format("  {:<16} {}\n", "size",       "micro/(<1KB) small/(<1MB) medium/(<100MB) large/");
    std::cout << std::format("  {:<16} {}\n", "type",       "files/ dirs/ symlinks/");

    print_section("  Examples");
    std::cout << "  kron organize ./downloads --by ext --dry-run\n"
              << "  kron organize ./photos --by date --date-format YYYY-MM\n"
              << "  kron organize . --rules-file my-rules.json --verbose\n"
              << "  kron organize ./project --by size --dry-run --verbose\n";
}

void help_encrypt() {
    print_section("kron encrypt <path> <dest>");
    std::cout << "  Encrypts files or complete directories using symmetric cryptography.\n"
              << "  Encrypted files are only recoverable with kron decrypt.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "File or directory to encrypt");
    std::cout << std::format("  {:<20} {}\n", "<dest>", "Path of the encrypted file or directory");

    print_section("  Options");
    print_table_header();
    print_row("--algo",      "—",  "STRING", "Algorithm: aes-256-gcm (default)");
    print_row("--password",  "—",  "STRING", "Password. If omitted: secure prompt");
    print_row("--recursive", "-r", "NONE",   "Encrypt complete directory");
    print_row("--dry-run",   "-n", "NONE",   "Simulate without encrypting");
    print_row("--verbose",   "-v", "NONE",   "Show each processed file");

    print_section("  Security note");
    std::cout << "  Passing --password via command line exposes it in shell history.\n"
              << "  It is recommended to omit it so Kron prompts securely via stdin with echo disabled.\n"
              << "  The password is never stored on disk.\n";

    print_section("  Examples");
    std::cout << "  kron encrypt ./documents ./documents.enc --recursive\n"
              << "  kron encrypt file.txt file.enc\n"
              << "  kron encrypt ./sensitive ./sensitive.enc --recursive --verbose\n";
}

void help_decrypt() {
    print_section("kron decrypt <path> <dest>");
    std::cout << "  Decrypts files or directories encrypted by kron encrypt.\n"
              << "  Requires the same password used during encryption.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "<path>", "Encrypted file or directory");
    std::cout << std::format("  {:<20} {}\n", "<dest>", "Destination path for decrypted content");

    print_section("  Options");
    print_table_header();
    print_row("--password",  "—",  "STRING", "Password. If omitted: secure prompt");
    print_row("--recursive", "-r", "NONE",   "Decrypt complete directory");
    print_row("--verbose",   "-v", "NONE",   "Show each processed file");

    print_section("  Examples");
    std::cout << "  kron decrypt ./documents.enc ./restored-documents --recursive\n"
              << "  kron decrypt file.enc file.txt\n"
              << "  kron decrypt ./sensitive.enc ./sensitive --recursive --verbose\n";
}

} // namespace

// ─── HANDLER ──────────────────────────────────────────────────────────────────

void HELP_HANDLER(const std::string& option_help) {
    if (option_help == std::string("empty")) {
        help_global_flags();
        help_list();
        help_inspect();
        help_copy();
        help_move();
        help_delete();
        help_find();
        help_rename();
        help_mkdir();
        help_touch();
        help_tree();
        help_size();
        help_hash();
        help_permissions();
        help_symlink();
        help_compare();
        help_watch();
        help_recent();
        help_empty();
        help_duplicates();
        help_analyze();
        help_snapshot();
        help_diff_snapshot();
        help_index();
        help_query();
        help_flatten();
        help_organize();
        help_encrypt();
        help_decrypt();
        return;
    }

    const auto& cmd = option_help;

    if (cmd == "list")          { help_list();          return;}
    if (cmd == "inspect")       { help_inspect();       return;}
    if (cmd == "copy")          { help_copy();          return;}
    if (cmd == "move")          { help_move();          return ; }
    if (cmd == "delete")        { help_delete();        return ; }
    if (cmd == "find")          { help_find();          return ; }
    if (cmd == "rename")        { help_rename();        return ; }
    if (cmd == "mkdir")         { help_mkdir();         return ; }
    if (cmd == "touch")         { help_touch();         return ; }
    if (cmd == "tree")          { help_tree();          return ; }
    if (cmd == "size")          { help_size();          return ; }
    if (cmd == "hash")          { help_hash();          return ; }
    if (cmd == "permissions")   { help_permissions();   return ; }
    if (cmd == "symlink")       { help_symlink();       return ; }
    if (cmd == "compare")       { help_compare();       return ; }
    if (cmd == "watch")         { help_watch();         return ; }
    if (cmd == "recent")        { help_recent();        return ; }
    if (cmd == "empty")         { help_empty();         return ; }
    if (cmd == "duplicates")    { help_duplicates();    return ; }
    if (cmd == "analyze")       { help_analyze();       return ; }
    if (cmd == "snapshot")      { help_snapshot();      return ; }
    if (cmd == "diff-snapshot") { help_diff_snapshot(); return ; }
    if (cmd == "index")         { help_index();         return ; }
    if (cmd == "query")         { help_query();         return ; }
    if (cmd == "flatten")       { help_flatten();       return ; }
    if (cmd == "organize")      { help_organize();      return ; }
    if (cmd == "encrypt")       { help_encrypt();       return ; }
    if (cmd == "decrypt")       { help_decrypt();       return ; }
    return;
}
