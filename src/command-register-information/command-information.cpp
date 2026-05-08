#include "../../include/command/command-raw-metadata.hpp"
#include "../../include/command/command-implementation.hpp"
#include "command-handler-include/command-hanlder-include.hpp"

void CreatedCommandData(){
  CommandMetaData list;
  list.default_name       = "list";
  list.minimun_positional = 0;
  list.maximun_positional = 1;
  list.option_avaible.insert(list.option_avaible.end(),{
    "--all",          // -a: Incluye entradas ocultas
    "--long",         // -l: Formato largo con metadatos
    "--sort",         // Ordenar por: name, size, modified, type
    "--reverse",       // Invierte el orden
    "--recursive",
    "--dirs-first",   // Muestra directorios antes que archivos
    "--filter",       // Filtra por patrón glob: *.cpp, test_*
    "--no-header",
    "--size-gt",
    "--modified-before",
    "--modified-after"
  });
  list.handler = LIST_HANDLER;
  GeneralCommandLog(list);

 }
