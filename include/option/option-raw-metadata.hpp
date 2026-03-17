#pragma once
#include <any>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

// ============================================================================
// TIPOS DE DATO QUE RECIBEN LAS OPCIONES
// ============================================================================

enum class TypeDataReceived : std::uint8_t{
  DATE,       // Fecha ISO: 2024-01-01
  SIZE,       // Tamaño con unidad: 10MB, 500KB
  EXTENSION,  // String genérico: plain/json/csv, glob, campo, etc.
  NONE,       // Flag booleano, no recibe valor
  STRING,
};

// ============================================================================
// CATEGORÍA DE EJECUCIÓN
// Define el orden cronológico en que se ejecutan las opciones
// ============================================================================

enum class OptionCategory : std::uint8_t{
  COLLECTION   = 0,  // Recolecta datos: --all, --recursive
  FILTERING    = 1,  // Reduce el set: --filter, --no-hidden, --depth, --ext, --name, --type
                     //                --size-gt, --size-lt, --modified-after, --modified-before
  SORTING      = 2,  // Ordena y agrupa: --sort, --reverse, --dirs-first
  PRESENTATION = 3,  // Formato de output: --long, --no-header, --output
  MANIPULATION = 4,  // Operaciones sobre paths: --force, --no-overwrite, --skip-existing
                     //                          --preserve, --dry-run, --match
  CREATION     = 5,  // Creación de entradas: --parents, --timestamp
  GLOBAL       = 6,  // Flags del sistema: --help, --version, --quiet, --verbose, --no-color
};

// ============================================================================
// STRUCTS DE CONTEXTO POR CATEGORÍA
// Cada proceso recibe el struct que corresponde a su categoría
// ============================================================================

// COLLECTION + FILTERING + SORTING
// Todas las opciones que operan sobre el listado de entradas del filesystem
struct FilterStruct{
  std::vector<std::filesystem::directory_entry> entries;
  std::any context;  // dato extra variable: glob, criterio de sort, profundidad, fecha, tamaño
};

// PRESENTATION
// Opciones que controlan cómo se presenta el output al usuario
struct PresentationStruct{
  bool show_header = true;
  bool long_format = false;
  bool no_color    = false;
  enum class Format : std::uint8_t { PLAIN, JSON, CSV } format = Format::PLAIN;
};

// MANIPULATION
// Opciones que controlan el comportamiento de operaciones sobre paths
struct PathStruct{
  std::filesystem::path origin;
  std::filesystem::path destination;
  bool force         = false;
  bool dry_run       = false;
  bool preserve      = false;
  bool skip_existing = false;
  bool no_overwrite  = false;
  bool verbose       = false;
};

// CREATION
// Opciones que controlan la creación de archivos y directorios
struct CreationStruct{
  std::filesystem::path target;
  bool parents   = false;
  bool no_create = false;
  std::optional<std::chrono::sys_seconds> timestamp = std::nullopt;
};

// ============================================================================
// TIPOS DE HANDLER POR CATEGORÍA
// ============================================================================

using FilteringProcess    = std::function<void(FilterStruct&)>;
using PresentationProcess = std::function<void(PresentationStruct&)>;
using PathProcess         = std::function<void(PathStruct&)>;
using CreationProcess     = std::function<void(CreationStruct&)>;

using OptionHandler = std::variant<
  std::monostate,        // GLOBAL — el sistema los maneja, no el pipeline
  FilteringProcess,      // COLLECTION | FILTERING | SORTING
  PresentationProcess,   // PRESENTATION
  PathProcess,           // MANIPULATION
  CreationProcess        // CREATION
>;

// ============================================================================
// METADATA DE OPCIÓN
// ============================================================================

struct OptionMetaData{
  std::string normalized_name;
  std::string alias_name;
  std::vector<std::string> conflict_name;
  std::vector<std::string> requieres_name = {};
  TypeDataReceived data_type;
  OptionCategory category;
  OptionHandler hanlder;
};
