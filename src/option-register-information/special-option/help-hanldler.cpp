#include "../../../include/special-option/help-option.hpp"
#include <string>
#include <format>
#include <iostream>
#include <string_view>

// ─── Column widths ────────────────────────────────────────────────────────────
static constexpr int W_OPT   = 26;
static constexpr int W_ALIAS =  8;
static constexpr int W_TYPE  = 10;
static constexpr int W_DESC  = 48;

namespace {

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

void help_kls() {
    print_section("kls [path] [options]");
    std::cout << "  \"What's in here, and should I be worried?\"\n"
              << "  Directory listing with a focus on security and auditing.\n";

    print_section("  Positionals");
    std::cout << std::format("  {:<20} {}\n", "[path]", "Directory to list. Default: current directory");

    print_section("  General Options");
    print_table_header();
    print_row("--all",        "-a", "NONE",   "Include hidden entries");
    print_row("--recursive",  "-r", "NONE",   "Recurse into subdirectories");
    print_row("--long",       "-l", "NONE",   "Long format with metadata");
    print_row("--sort",       "—",  "STRING", "Sort by: name|size|date|severity");
    print_row("--reverse",    "—",  "NONE",   "Reverse sort order");

    print_section("  Security Options");
    print_table_header();
    print_row("--health",     "—",  "NONE",   "Enable security analysis (Default)");
    print_row("--no-health",  "—",  "NONE",   "Disable security analysis");
    print_row("--only-alerts","—",  "NONE",   "Show only files with security risks");
    print_row("--attack-surface","—", "NONE", "Show attack surface summary");
    print_row("--security-report","—","NONE", "Consolidated report at the end");
    print_row("--alerts-first","—", "NONE",   "Show risky files at the top");

    print_section("  Examples");
    std::cout << "  kls\n"
              << "  kls ./src -l\n"
              << "  kls --only-alerts\n"
              << "  kls --sort=severity\n"
              << "  kls --attack-surface\n";
}

} // namespace

void HELP_HANDLER(std::string_view option_help) {
    help_kls();
}
