#include "../../../include/special-option/help-option.hpp"
#include <string>
#include <format>
#include <iostream>
#include <string_view>

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

void print_section(std::string_view title) {
    std::cout << std::format("\n{}\n{}\n", title, std::string(title.size(), '-'));
}

void print_row(std::string_view opt, std::string_view alias,
               std::string_view type,std::string_view desc) {
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

}




 // namespace

// ─── HANDLER ──────────────────────────────────────────
void HELP_HANDLER(std::string_view option_help) {
    if (option_help == "empty") { 
        help_list();
        return;
    }

    const auto& cmd = option_help;

    if (cmd == "list")          { help_list();          return;}
    return;
}
