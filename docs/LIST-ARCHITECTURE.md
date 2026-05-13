# List Command Architecture

This document describes the internal architecture of the `list` command in `kls`, focusing on its high-performance multi-threaded engine and security-centric data collection pipeline.

## 1. High-Level Overview

The `list` command follows a decoupled architecture composed of four distinct stages:

1.  **Collection**: Multi-threaded traversal and metadata gathering using `statx`.
2.  **Filtering**: Removal of entries based on user criteria (glob patterns, dates, etc.).
3.  **Sorting**: Reordering of the final set (by name, size, type, or security severity).
4.  **Presentation**: Rendering the data to the terminal in various formats.

---

## 2. Multi-threaded Traversal Engine

The core of the `list` command is a high-concurrency engine designed to maximize I/O throughput when exploring large directory trees.

### Components
- **Pending Queue (`pending_dirs`)**: A thread-safe queue of directories waiting to be explored.
- **Worker Threads**: A pool of threads (up to `MAX_THREAD` or CPU core count) that consume from the queue.
- **Atomic Counters**: `working_threads` tracks active workers to determine when the entire tree has been processed.

### Algorithm
- Every worker pops a directory, opens it with `opendir()`, and iterates through its entries.
- Files are added to a local `file_entry_temp` vector.
- Subdirectories (if `--recursive` is active) are added to a local `temp_pending_dir` vector.
- To minimize lock contention, workers perform **batch transfers** to the global queue and results vector after completing a directory.

---

## 3. Metadata Collection (`statx`)

`kls` uses the modern Linux `statx(2)` system call. This provides several advantages:
- **Birth Time**: Access to file creation time (btime), which is unavailable in the older `stat` call.
- **Performance**: Improved efficiency on modern filesystems and network mounts by requesting only necessary metadata masks.

---

## 4. Security-Centric Design

### The `FileEntry` Struct
Every file is represented by a `FileEntry` object, which stores:
- POSIX metadata (UID, GID, Mode, Inode).
- Security flags (SUID/SGID detection).
- Symlink resolution status (broken link detection).
- Custom health alerts (tracked in the `health` vector).

### Recursive Depth Control
Users can control the intensity of the traversal using the `--depth` flag, preventing accidental "infinite" loops or excessive I/O on extremely deep structures.

---

## 5. Usage & Options

| Option | Alias | Description |
| :--- | :--- | :--- |
| `--recursive` | `-r` | Enable multi-threaded recursive traversal. |
| `--all` | `-a` | Show hidden files (those starting with `.`). |
| `--long` | `-l` | Use the technical long format with security details. |
| `--filter` | | Glob-style filtering (e.g., `*.sh`, `*token*`). Uses `fnmatch`. |
| `--depth` | `-d` | Limit recursion to a specific depth. |
| `--sort` | | Sort results by `name`, `size`, `type`, or `severity`. |

## 6. Optimization Details

- **Batch Commit**: Results are pushed to the global vector in chunks to reduce mutex lock/unlock overhead.
- **String Pre-allocation**: `full_path` strings use `reserve(PATH_MAX)` to avoid frequent reallocations.
- **Condition Variables**: Threads sleep when the queue is empty, ensuring zero CPU wastage.
