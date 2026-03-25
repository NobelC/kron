# kron list — Documento de Diseño

---

## Propósito

`kron list` responde una sola pregunta: **¿qué hay aquí?**

Su unidad de trabajo es el **directorio**. Nunca opera sobre archivos individuales.  
Su profundidad es **configurable** — desde una vista superficial hasta recursiva con límite de niveles.  
Su salida es **adaptable** — texto, árbol, JSON, CSV o estadísticas puras.

---

## Límites claros (lo que list NUNCA hará)

- **Nunca abre el contenido de un archivo.** Solo lee metadata del filesystem.
- **Nunca inspecciona magic bytes, encoding ni estructura interna.** Eso es responsabilidad de `inspect`.
- **Nunca calcula hashes criptográficos completos.** Puede usar hashes rápidos (xxHash) para detección de duplicados, pero no SHA256 por archivo.
- **Nunca opera sobre un archivo individual como argumento principal.** El input siempre es un directorio.
- **Nunca modifica, mueve ni copia nada.** Es read-only sobre el filesystem.

---

## Garantías

- Siempre respeta si el output va a un TTY o a un pipe (colores solo en TTY).
- Siempre adapta el layout al ancho real del terminal.
- En modo recursivo, siempre respeta el límite de profundidad especificado.
- El output estructurado (JSON/CSV) es siempre parseable — sin caracteres de color, sin decoración.

---

## Input

```
kron list [directorio] [opciones]
```

- Si no se especifica directorio, usa el directorio actual.
- Acepta un solo directorio como argumento posicional.

---

## Fases internas

### Fase 1 — Resolución del target
- Validar que el path existe y es un directorio.
- Resolver a path absoluto.
- Determinar profundidad máxima (default: 1 nivel).

### Fase 2 — Recolección multi-hilo
- Usar `getdents64` directo para minimizar syscalls.
- Usar `statx` con mask específico — solo pedir los campos que se van a mostrar.
- Thread pool: recolectar paths primero, luego `statx` en paralelo.
- Respetar filtros de colección: ocultos, profundidad, tipo.

### Fase 3 — Filtrado y ordenamiento
- Aplicar filtros sobre los datos recolectados.
- Aplicar ordenamiento.
- Esta fase opera sobre structs en memoria — cero I/O adicional.

### Fase 4 — Presentación
- Detectar ancho del terminal con `ioctl TIOCGWINSZ`.
- Detectar TTY con `isatty`.
- Renderizar según el modo de output activo.

---

## Opciones

### Colección

| Opción | Descripción |
|---|---|
| `--all` | Incluir archivos ocultos (`.` prefix) |
| `--recursive` | Descender en subdirectorios |
| `--depth=N` | Profundidad máxima en modo recursivo (default: ilimitado si --recursive) |
| `--dirs-only` | Solo mostrar directorios |
| `--files-only` | Solo mostrar archivos regulares |

### Filtrado

| Opción | Descripción |
|---|---|
| `--ext=cpp,hpp` | Filtrar por extensiones (lista separada por coma) |
| `--larger-than=N` | Archivos mayores a N (acepta: 1KB, 2MB, 1GB) |
| `--smaller-than=N` | Archivos menores a N |
| `--modified-since=Nd` | Modificados en los últimos N días |
| `--modified-before=Nd` | Modificados hace más de N días |
| `--created-since=Nd` | Creados en los últimos N días |
| `--name=pattern` | Filtrar por nombre (glob: `*.log`, `main*`) |

### Ordenamiento

| Opción | Descripción |
|---|---|
| `--sort=name` | Por nombre (default) |
| `--sort=size` | Por tamaño |
| `--sort=date` | Por fecha de modificación |
| `--sort=ext` | Por extensión |
| `--sort=type` | Directorios primero, archivos después |
| `--reverse` | Invertir el orden actual |

### Presentación

| Opción | Descripción |
|---|---|
| `--long` | Metadata completa por entrada |
| `--no-header` | Suprimir la línea de encabezados en --long |
| `--tree` | Vista jerárquica tipo árbol |
| `--grid` | Múltiples columnas adaptadas al terminal |
| `--compact` | Solo nombres, máxima densidad |
| `--summary` | Solo estadísticas generales, sin listar entradas |
| `--format=json` | Output estructurado JSON |
| `--format=csv` | Output estructurado CSV |
| `--no-color` | Forzar output sin colores |

### Análisis

| Opción | Descripción |
|---|---|
| `--health` | Marcar archivos problemáticos (symlinks rotos, permisos inconsistentes, UID huérfanos, tamaño cero) |
| `--duplicates` | Detectar archivos con contenido idéntico por hash rápido |
| `--categorize` | Agrupar entradas por categoría semántica (SOURCE, CONFIG, MEDIA, BUILD, DOCS) |

---

## Modos de output

### Compact (default sin --long)
```
src/
include/
main.cpp
CMakeLists.txt
README.md
```

### Long (--long)
```
PERMISOS   INODE      PROPIETARIO  GRUPO   NOMBRE            TIPO  TAMAÑO   MODIFICADO
rwxr-xr-x  1234567    jeremy       jeremy  src/              DIR   —        2026-03-20
rw-r--r--  1234568    jeremy       jeremy  main.cpp          FIL   2.14KB   2026-03-24
```

### Tree (--tree)
```
proyecto/
├── src/
│   ├── main.cpp
│   └── parser.cpp
├── include/
│   └── parser.hpp
└── CMakeLists.txt
```

### Grid (--grid)
```
src/          include/      build/        main.cpp
CMakeLists.txt  README.md   LICENSE
```

### Summary (--summary)
```
Directorio   : /home/jeremy/proyecto
Entradas     : 47 archivos, 12 directorios
Tamaño total : 2.34MB
Más reciente : main.cpp (2026-03-24)
Más grande   : compile_commands.json (1.1MB)
Tipo común   : .cpp (18 archivos)
```

### JSON (--format=json)
```json
{
  "path": "/home/jeremy/proyecto",
  "entries": [
    {
      "name": "main.cpp",
      "type": "file",
      "size": 2190,
      "permissions": "rw-r--r--",
      "modified": "2026-03-24",
      "inode": 1234568
    }
  ],
  "summary": {
    "files": 47,
    "directories": 12,
    "total_size": 2455000
  }
}
```

---

## Categorías semánticas (--categorize)

| Categoría | Extensiones / Patrones |
|---|---|
| SOURCE | `.cpp`, `.c`, `.hpp`, `.h`, `.rs`, `.go`, `.py` |
| CONFIG | `.toml`, `.yaml`, `.yml`, `.ini`, `.json`, `.env` |
| BUILD | `CMakeLists.txt`, `Makefile`, `.cmake`, `build/` |
| DOCS | `.md`, `.txt`, `.pdf`, `.rst` |
| MEDIA | `.png`, `.jpg`, `.mp4`, `.svg` |
| BINARY | `.o`, `.so`, `.a`, archivos ELF sin extensión |

---

## Detección de archivos problemáticos (--health)

| Problema | Condición |
|---|---|
| Symlink roto | `symlink` cuyo target no existe |
| UID huérfano | `st_uid` sin entrada en `/etc/passwd` |
| GID huérfano | `st_gid` sin entrada en `/etc/group` |
| Tamaño cero | Archivo regular con `st_size == 0` |
| Permisos inconsistentes | Ejecutable sin read (`--x------`), writable por otros sin intención explícita |
| Fecha futura | `mtime` mayor al timestamp actual |

---

## Colores ANSI (solo en TTY)

| Tipo | Color |
|---|---|
| Directorio | Azul |
| Ejecutable | Verde |
| Symlink | Cyan |
| Archivo problemático | Rojo |
| Archivo oculto | Gris |
| Archivo regular | Default |

---

## Struct de datos (FileEntry)

Campos que `list` necesita — ni más ni menos:

```cpp
struct FileEntry {
  std::string        name;
  std::filesystem::path path;
  bool               is_directory;
  bool               is_symlink;
  off_t              size;         // -1 si es directorio
  ino_t              inode;
  std::string        permissions;
  std::string        owner;
  std::string        group;
  std::string        modified;     // mtime formateado
  std::string        created;      // ctime formateado — para filtros por fecha
  std::string        extension;    // para filtros y categorización
};
```

Nada de magic bytes. Nada de hashes SHA. Nada de encoding. Esos campos no existen aquí.

---

## Lo que diferencia a kron list de ls

| Capacidad | `ls` | `kron list` |
|---|---|---|
| Output JSON / CSV | ✗ | ✓ |
| Vista árbol integrada | ✗ (requiere `tree`) | ✓ |
| Filtros avanzados inline | Básico | ✓ |
| Resumen estadístico | ✗ | ✓ |
| Categorización semántica | ✗ | ✓ |
| Detección de archivos problemáticos | ✗ | ✓ |
| Detección de duplicados | ✗ | ✓ |
| Adaptación al ancho del terminal | Parcial | ✓ |
| Colores desactivables | Parcial | ✓ |

---

## Roadmap de implementación

```
FASE 1 — Base sólida
  └─ Recolección con getdents64 + statx
  └─ FileEntry struct definido
  └─ Modos: compact, long, summary
  └─ Colores ANSI + detección TTY
  └─ Adaptación al ancho del terminal

FASE 2 — Filtros y ordenamiento
  └─ Todos los filtros definidos arriba
  └─ Ordenamiento completo

FASE 3 — Modos avanzados
  └─ --tree
  └─ --grid
  └─ --format=json / csv

FASE 4 — Análisis
  └─ --health
  └─ --categorize
  └─ --duplicates

FASE 5 — Performance
  └─ Thread pool real para statx paralelo
  └─ io_uring (evaluación)
```
