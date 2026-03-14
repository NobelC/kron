# kron

CLI multifuncional de manipulación e inspección de archivos para desarrolladores y usuarios avanzados.  
Construido en **C++20 moderno** usando exclusivamente STL. Sin dependencias externas.

---

## ¿Qué es kron?

kron es una amalgama de comandos clásicos de shell (`ls`, `cp`, `mv`, `rm`, `find`) con operaciones avanzadas de análisis, indexado, snapshots y automatización del filesystem. Su núcleo es la **visibilidad profunda** del sistema de archivos.

---

## Compilar

```bash
cmake -B build
cmake --build build
```

Para instalar globalmente via symlink:

```bash
sudo ln -sf $(pwd)/build/kron /usr/local/bin/kron
```

Requiere: **CMake 3.15+**, compilador con soporte **C++20**.

---

## Uso

```
kron <comando> [argumentos] [opciones]
```

---

## Comandos

### Básicos

| Comando | Descripción |
|---------|-------------|
| `list [ruta]` | Lista el contenido de un directorio |
| `inspect <ruta>...` | Muestra metadatos completos de archivos o directorios |
| `copy <origen> <destino>` | Copia archivos o directorios |
| `move <origen> <destino>` | Mueve o renombra |
| `delete <ruta>...` | Elimina archivos o directorios |
| `find <directorio>` | Busca por criterios combinables |
| `rename <ruta> <nombre>` | Renombra en el mismo lugar |
| `mkdir <ruta>` | Crea directorios |
| `touch <ruta>...` | Crea archivos vacíos o actualiza fecha |

### Medios

| Comando | Descripción |
|---------|-------------|
| `tree [ruta]` | Árbol visual de directorios |
| `size <ruta>...` | Tamaño real en disco |
| `hash <ruta>...` | Checksum criptográfico |
| `permissions <ruta>...` | Ver y modificar permisos |
| `symlink <origen> <enlace>` | Crear enlaces simbólicos |
| `compare <ruta1> <ruta2>` | Comparar archivos o directorios |
| `watch <ruta>` | Monitoreo de cambios en tiempo real |
| `recent [ruta]` | Archivos modificados recientemente |
| `empty <ruta>` | Detectar archivos y directorios vacíos |

### Avanzados

| Comando | Descripción |
|---------|-------------|
| `duplicates <ruta>` | Encuentra duplicados por contenido (hash) |
| `analyze <ruta>` | Análisis estadístico del filesystem |
| `snapshot <ruta>` | Instantánea del estado del directorio |
| `diff-snapshot <snap1> <snap2>` | Compara dos snapshots |
| `index <ruta>` | Construye un índice consultable |
| `query <índice> <expresión>` | Consulta un índice con expresiones de filtro |
| `flatten <ruta> <destino>` | Aplana un árbol de directorios |
| `organize <ruta>` | Organiza archivos por reglas |
| `encrypt <ruta> <destino>` | Cifra archivos o directorios |
| `decrypt <ruta> <destino>` | Descifra archivos cifrados |

---

## Ejemplos

```bash
# Listar con formato largo, ordenado por tamaño
kron list ./src --long --sort=size --reverse

# Listar solo archivos .cpp
kron list ./src --filter "*.cpp"

# Listar con directorios primero, incluyendo ocultos
kron list . --all --dirs-first

# Buscar archivos .log mayores a 1MB modificados este año
kron find ./logs --ext log --size-gt 1MB --modified-after 2025-01-01

# Ver tamaño de cada subdirectorio
kron size . --depth 1 --sort --reverse
```

---

## Flags globales

Aplicables a todos los comandos.

| Flag | Alias | Descripción |
|------|-------|-------------|
| `--help` | `-h` | Ayuda del comando |
| `--version` | `-v` | Versión del binario |
| `--no-color` | — | Desactiva colores ANSI |
| `--quiet` | `-q` | Solo errores, sin output informativo |
| `--dry-run` | `-n` | Simula sin ejecutar |
| `--output` | `-o` | Formato: `plain`, `json`, `csv` |

---

## Arquitectura

```
argv
  └── tokenization      — clasifica cada argumento en tokens
        └── parsing     — valida existencia de comandos y opciones
              └── validator — validación semántica, tipos, conflictos
                    └── executor — despacha al handler del comando
```

Las opciones se ejecutan en orden cronológico fijo:  
`COLLECTION → FILTERING → SORTING → PRESENTATION`

---

## Estado

| Comando | Estado |
|---------|--------|
| `list` | ✅ Implementado |
| El resto | 🔧 En desarrollo |

---

## Licencia

WIP
