# kron

A multifunctional CLI tool for file manipulation and inspection, built for developers and power users.  
Written in **modern C++20** using exclusively the STL. Zero external dependencies.

---

## What is kron?

kron is a blend of classic shell commands (`ls`, `cp`, `mv`, `rm`, `find`) with advanced operations for analysis, indexing, snapshots, and filesystem automation. Its core is **deep filesystem visibility**.

---

## Build
```bash
cmake -B build
cmake --build build
```

To install globally via symlink:
```bash
sudo ln -sf $(pwd)/build/kron /usr/local/bin/kron
```

Requirements: **CMake 3.15+**, compiler with **C++20** support.

---

## Usage
```
kron <command> [arguments] [options]
```

---

## Commands

### Basic

| Command | Description | Status |
|---------|-------------|--------|
| `list [path]` | List directory contents | ✅ |
| `inspect <path>...` | Show full file/directory metadata | 🔧 |
| `copy <src> <dst>` | Copy files or directories | 🔧 |
| `move <src> <dst>` | Move or rename | 🔧 |
| `delete <path>...` | Delete files or directories | 🔧 |
| `find <directory>` | Search by combinable criteria | 🔧 |
| `rename <path> <name>` | Rename in place | 🔧 |
| `mkdir <path>` | Create directories | 🔧 |
| `touch <path>...` | Create empty files or update timestamps | 🔧 |

### Intermediate

| Command | Description | Status |
|---------|-------------|--------|
| `tree [path]` | Visual directory tree | 🔧 |
| `size <path>...` | Real disk usage | 🔧 |
| `hash <path>...` | Cryptographic checksum | 🔧 |
| `permissions <path>...` | View and modify permissions | 🔧 |
| `symlink <src> <link>` | Create symbolic links | 🔧 |
| `compare <path1> <path2>` | Compare files or directories | 🔧 |
| `watch <path>` | Real-time change monitoring | 🔧 |
| `recent [path]` | Recently modified files | 🔧 |
| `empty <path>` | Detect empty files and directories | 🔧 |

### Advanced

| Command | Description | Status |
|---------|-------------|--------|
| `duplicates <path>` | Find duplicates by content hash | 🔧 |
| `analyze <path>` | Statistical filesystem analysis | 🔧 |
| `snapshot <path>` | Capture directory state | 🔧 |
| `diff-snapshot <s1> <s2>` | Compare two snapshots | 🔧 |
| `index <path>` | Build a queryable filesystem index | 🔧 |
| `query <index> <expr>` | Query an index with filter expressions | 🔧 |
| `flatten <path> <dst>` | Flatten a directory tree | 🔧 |
| `organize <path>` | Organize files by rules | 🔧 |
| `encrypt <path> <dst>` | Encrypt files or directories | 🔧 |
| `decrypt <path> <dst>` | Decrypt encrypted files | 🔧 |

---

## Examples
```bash
# List with long format, sorted by size descending
kron list ./src --long --sort=size --reverse

# List only .cpp files
kron list ./src --filter "*.cpp"

# List with directories first, including hidden files
kron list . --all --dirs-first

# Find .log files larger than 1MB modified this year
kron find ./logs --ext log --size-gt 1MB --modified-after 2025-01-01

# Show size of each subdirectory
kron size . --depth 1 --sort --reverse
```

---

## Global Flags

Applicable to all commands.

| Flag | Alias | Description |
|------|-------|-------------|
| `--help` | `-h` | Show command help |
| `--version` | `-v` | Print binary version |
| `--no-color` | — | Disable ANSI colors |
| `--quiet` | `-q` | Only show errors |
| `--dry-run` | `-n` | Simulate without executing |
| `--output` | `-o` | Output format: `plain`, `json`, `csv` |

---

## Architecture
```
argv
  └── tokenization      — classifies each argument into tokens
        └── parsing     — validates command and option existence
              └── validator — semantic validation, types, conflicts
                    └── executor — dispatches to the command handler
```

Options execute in fixed chronological order:  
`COLLECTION → FILTERING → SORTING → PRESENTATION`

---

## License

MIT
