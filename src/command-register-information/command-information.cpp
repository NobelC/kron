#include "../../include/command/command-raw-metadata.hpp"
#include "../../include/command/command-implementation.hpp"
#include "command-handler-include/command-hanlder-include.hpp"

void CreatedCommandData(){
  // ============================================================================
  // TIER: kron-basic
  // ============================================================================

  // --------------------------------------------------------------------------
  // Comando: list
  // --------------------------------------------------------------------------
  /*
   * Lista el contenido de un directorio. Equivalente a `ls` pero con control 
   * explícito sobre el formato y los campos.
   * 
   * Uso: kron list [ruta]
   * Default: directorio actual
   */
  CommandMetaData list;
  list.default_name       = "list";
  list.minimun_positional = 0;
  list.maximun_positional = 1;
  list.option_avaible.insert(list.option_avaible.end(),{
    "--all",          // -a: Incluye entradas ocultas
    "--long",         // -l: Formato largo con metadatos
    "--sort",         // Ordenar por: name, size, modified, type
    "--reverse",      // Invierte el orden
    "--dirs-first",   // Muestra directorios antes que archivos
    "--filter",       // Filtra por patrón glob: *.cpp, test_*
    "--no-header",
  });
  list.handler = LIST_HANDLER;
  GeneralCommandLog(list);

  // --------------------------------------------------------------------------
  // Comando: inspect
  // --------------------------------------------------------------------------
  /*
   * Muestra metadatos completos de uno o más archivos o directorios. 
   * Es el comando núcleo del sistema. Todo lo demás lo complementa.
   * 
   * Uso: kron inspect <ruta> [ruta2 ...]
   * Acepta múltiples rutas separadas por espacio
   */
  CommandMetaData inspect;
  inspect.default_name       = "inspect";
  inspect.minimun_positional = 0;
  inspect.maximun_positional = 100;
  inspect.option_avaible.insert(inspect.option_avaible.end(),{
    "--all",             // -a: Incluye archivos ocultos
    "--recursive",       // -r: Inspecciona subdirectorios recursivamente
    "--depth",           // -d: Profundidad máxima de recursión
    "--follow-symlinks", // Sigue enlaces simbólicos
    "--fields",          // Selecciona campos: name,size,modified,permissions,type,inode
    "--sort",            // Ordena resultado: name, size, modified
    "--reverse",         // Invierte el orden del sort
    "--filter",
    "--stats",
    "--stats-only",
  });
  inspect.handler = INSPECT_HANDLER;
  GeneralCommandLog(inspect);

  // --------------------------------------------------------------------------
  // Comando: copy
  // --------------------------------------------------------------------------
  /*
   * Copia un archivo o directorio. Con control explícito sobre 
   * comportamiento ante conflictos.
   * 
   * Uso: kron copy <origen> <destino>
   * Origen y destino pueden ser archivos o directorios
   */
  CommandMetaData copy;
  copy.default_name       = "copy";
  copy.minimun_positional = 2;
  copy.maximun_positional = 2;
  copy.option_avaible.insert(copy.option_avaible.end(),{
    "--recursive",      // -r: Copia directorios recursivamente
    "--force",          // -f: Sobreescribe destino sin preguntar
    "--no-overwrite",   // Falla si el destino existe
    "--skip-existing",  // Omite archivos que ya existen en destino
    "--preserve",       // -p: Preserva metadatos: fechas, permisos
    "--dry-run",        // -n: Muestra qué se copiaría sin ejecutar
    "--verbose"         // Muestra cada archivo copiado
  });
  // copy.handler = CopyHandler;
  GeneralCommandLog(copy);

  // --------------------------------------------------------------------------
  // Comando: move
  // --------------------------------------------------------------------------
  /*
   * Mueve o renombra un archivo o directorio.
   * 
   * Uso: kron move <origen> <destino>
   * Destino puede ser nueva ubicación o nuevo nombre
   */
  CommandMetaData move;
  move.default_name       = "move";
  move.minimun_positional = 2;
  move.maximun_positional = 2;
  move.option_avaible.insert(move.option_avaible.end(),{
    "--force",        // -f: Sobreescribe destino sin preguntar
    "--no-overwrite", // Falla si el destino existe
    "--dry-run",      // -n: Simula la operación
    "--verbose"       // Muestra detalles de la operación
  });
  // move.handler = MoveHandler;
  GeneralCommandLog(move);

  // --------------------------------------------------------------------------
  // Comando: delete
  // --------------------------------------------------------------------------
  /*
   * Elimina archivos o directorios. Requiere confirmación explícita para 
   * directorios no vacíos salvo --force.
   * 
   * Uso: kron delete <ruta> [ruta2 ...]
   * Acepta múltiples rutas a eliminar
   */
  CommandMetaData delete_cmd;  // 'delete' es keyword en C++
  delete_cmd.default_name       = "delete";
  delete_cmd.minimun_positional = 1;
  delete_cmd.maximun_positional = 100;
  delete_cmd.option_avaible.insert(delete_cmd.option_avaible.end(),{
    "--recursive",  // -r: Elimina directorios y su contenido
    "--force",      // -f: Sin confirmación interactiva
    "--dry-run",    // -n: Muestra qué se eliminaría
    "--verbose",    // Muestra cada archivo eliminado
    "--match"       // Elimina solo entradas que coincidan con el patrón
  });
  // delete_cmd.handler = DeleteHandler;
  GeneralCommandLog(delete_cmd);

  // --------------------------------------------------------------------------
  // Comando: find
  // --------------------------------------------------------------------------
  /*
   * Busca archivos en un directorio según criterios combinables.
   * Los criterios son acumulativos (AND lógico).
   * 
   * Uso: kron find <directorio> [criterios]
   * Raíz de búsqueda como primer argumento posicional
   */
  CommandMetaData find;
  find.default_name       = "find";
  find.minimun_positional = 1;
  find.maximun_positional = 1;
  find.option_avaible.insert(find.option_avaible.end(),{
    "--name",            // Busca por nombre (glob): test_*.cpp, *config*
    "--ext",             // Busca por extensión: cpp, .cpp (ambos válidos)
    "--type",            // Tipo: file, dir, symlink
    "--size-gt",         // Archivos mayores a n bytes/KB/MB: 10MB
    "--size-lt",         // Archivos menores a n
    "--modified-after",  // Fecha ISO: 2024-01-01
    "--modified-before", // Fecha ISO: 2024-01-01
    "--recursive",       // -r: Búsqueda recursiva (default: true)
    "--depth",           // Profundidad máxima
    "--no-hidden"        // Excluye archivos ocultos
  });
  // find.handler = FindHandler;
  GeneralCommandLog(find);

  // --------------------------------------------------------------------------
  // Comando: rename
  // --------------------------------------------------------------------------
  /*
   * Renombra un archivo o directorio en su misma ubicación.
   * Operación atómica mediante rename(2) cuando es posible.
   * 
   * Uso: kron rename <ruta> <nuevo-nombre>
   * nuevo-nombre es solo el nombre, no una ruta completa
   */
  CommandMetaData rename_cmd;  // 'rename' puede colisionar con rename(2) de <cstdio>
  rename_cmd.default_name       = "rename";
  rename_cmd.minimun_positional = 2;
  rename_cmd.maximun_positional = 2;
  rename_cmd.option_avaible.insert(rename_cmd.option_avaible.end(),{
    "--force",    // -f: Sobreescribe si ya existe ese nombre
    "--dry-run"   // -n: Simula el renombrado
  });
  // rename_cmd.handler = RenameHandler;
  GeneralCommandLog(rename_cmd);

  // --------------------------------------------------------------------------
  // Comando: mkdir
  // --------------------------------------------------------------------------
  /*
   * Crea uno o más directorios en el filesystem.
   * 
   * Uso: kron mkdir <ruta>
   */
  CommandMetaData mkdir_cmd;
  mkdir_cmd.default_name       = "mkdir";
  mkdir_cmd.minimun_positional = 1;
  mkdir_cmd.maximun_positional = 1;
  mkdir_cmd.option_avaible.insert(mkdir_cmd.option_avaible.end(),{
    "--parents",  // -p: Crea directorios intermedios si no existen
    "--verbose"   // Muestra cada directorio creado
  });
  // mkdir_cmd.handler = MkdirHandler;
  GeneralCommandLog(mkdir_cmd);

  // --------------------------------------------------------------------------
  // Comando: touch
  // --------------------------------------------------------------------------
  /*
   * Crea archivos vacíos o actualiza la fecha de modificación
   * de archivos existentes.
   * 
   * Uso: kron touch <ruta> [ruta2 ...]
   * Acepta múltiples rutas
   */
  CommandMetaData touch;
  touch.default_name       = "touch";
  touch.minimun_positional = 1;
  touch.maximun_positional = 100;
  touch.option_avaible.insert(touch.option_avaible.end(),{
    "--no-create",  // Solo actualiza fecha si existe, no crea nuevo
    "--timestamp"   // Fecha ISO personalizada: 2024-06-01T12:00:00
  });
  // touch.handler = TouchHandler;
  GeneralCommandLog(touch);
}
