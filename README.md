# Kron (In Development)

> A low-level CLI tool that speaks directly to the kernel. Zero unnecessary abstractions. Maximum proximity to the hardware.

---

## Philosophy

Most CLI tools are built for convenience. Kron is built for performance and control.

Every command in Kron is designed around a single question: *what is the minimum number of instructions between the user's intent and the hardware?* That means direct syscalls where possible, arena-allocated memory for collection-heavy operations, zero external dependencies, and a pipeline architecture where every stage is a fixed, compiled unit — not a plugin, not a runtime hook, not a wrapper around a wrapper.

Kron does not hide what it is doing. When it makes a `getdents64` call instead of iterating a `DIR*`, that is intentional. When it copies a file with `copy_file_range` and the data never touches userspace, that is a feature, not an implementation detail. When it tells you a "move" is actually a cross-device copy-and-delete, it is because you deserve to know that.

The goal is not to replace `ls`, `cp`, `mv`, or `strace`. The goal is to exist at the layer beneath all of them.

---

## Architecture

Kron follows a fixed four-stage pipeline for every command:

```
[Tokenizer] → [Parser] → [Validator] → [Executor]
```

- **Tokenizer** — splits the raw input into tokens with no interpretation.
- **Parser** — constructs a command struct from the token stream. Fails fast on malformed input.
- **Validator** — checks filesystem state, permissions, and logical constraints before any operation begins.
- **Executor** — executes the resolved operation. This is the only stage that touches the kernel.

Every command is a compiled unit with a fixed interface. No dynamic dispatch, no runtime plugin loading, no virtual tables unless the data model demands it.

Memory for collection-heavy operations (directory traversal, file enumeration, entry sorting) is served from a per-command **arena allocator** backed by a single `mmap` call. No per-entry `malloc`/`free`. When the command exits, the arena is released in a single `munmap`. Allocation is O(1), deallocation is O(1), and fragmentation is zero.

---

## Commands

- [`list`](#list) — Raw directory traversal via `getdents64`
- [`inspect`](#inspect) — Full file introspection down to entropy and ELF layout
- [`copy`](#copy) — Kernel-to-kernel copy with reflink and sparse-hole support
- [`mv`](#mv) — Atomic move and exchange via `renameat2`
- [`probe`](#probe) — Hardware-level performance profiling of filesystem operations

---

## `list`

### Identity

`list` is not a pretty-printer. It is a direct reader of directory entries as delivered by the kernel. The difference matters: `ls` shows you the OS's interpretation of a directory. `list` shows you the raw `linux_dirent64` structures that `getdents64` fills into your buffer — inode numbers, raw `d_type` values, and all.

When you run `kron list`, you are reading the filesystem as close to its on-disk representation as userspace allows.

### Syscall Surface

| Syscall | Purpose |
|---|---|
| `getdents64` | Fill buffer with raw directory entries |
| `statx(STATX_ALL)` | Extended metadata including birth time (`stx_btime`) |
| `lstat` | Symlink-safe stat for each entry |
| `lseek(SEEK_HOLE / SEEK_DATA)` | Sparse file detection |
| `openat` | Open directory by fd, not by path string |

The read buffer for `getdents64` is sized to minimize syscall count while staying L1-cache friendly. The arena allocator holds all `FileEntry` structs for the current traversal. No heap allocation per entry.

### Base Behavior

Equivalent to what `ls -la` produces: names, permissions, owner, group, size, modification time, and type indicator for every entry in a target directory.

```
kron list [TARGET]
```

If `TARGET` is omitted, the current working directory is used.

### Extended Options

```
-r, --recursive             Traverse subdirectories recursively
-d, --depth <N>             Limit recursion to N levels
-i, --inode-order           Sort by inode number instead of name
                            Reflects approximate on-disk order; efficient for sequential reads
-b, --blocks                Show allocated blocks (512-byte units) alongside apparent size
                            Exposes the real disk footprint of sparse files
-s, --sparse                Highlight sparse files and show hole/data segment map
-H, --hardlinks             Flag entries sharing an inode with another entry in the same directory
-x, --xattr                 Show extended attributes for each entry
-t, --type <TYPE>           Filter by entry type: file, dir, symlink, pipe, socket, block, char
-I, --inode-range <A:B>     Show only entries with inode numbers in range [A, B]
-B, --born-after <TIME>     Filter by birth time (stx_btime) — not available from stat(2)
    --no-color              Disable ANSI color output
    --json                  Output raw JSON array of FileEntry objects
    --stat-fields <FIELDS>  Comma-separated list of statx fields to include in output
```

### Modes

**Default mode** — single directory, human-readable columns.
```
kron list /var/log
```

**Inode-ordered mode** — sort by inode number. Use before bulk reads to exploit filesystem locality.
```
kron list -i /data/archive
```

**Sparse map mode** — show hole and data segment offsets for every sparse file in the directory.
```
kron list -s /vm/images
```

**JSON mode** — machine-readable output. Each object contains all collected statx fields.
```
kron list --json /etc | jq '.[] | select(.type == "symlink")'
```

**Filtered by birth time** — only entries created after a given timestamp. Uses `stx_btime`, which `stat` does not expose.
```
kron list --born-after 2024-01-01T00:00:00 /home/user/projects
```

### Key Differentiators vs `ls`

- Sorts by **inode number** (on-disk order), not just alphabetically
- Exposes **birth time** via `statx`, which `ls` cannot access through `stat(2)`
- Detects and maps **sparse holes** with byte-level offsets
- Identifies **hardlinks** within the same directory via inode comparison
- Outputs structured **JSON** for pipeline consumption
- Reports **real disk usage** (allocated blocks) vs apparent size side by side
- Filters by **inode range** for surgical directory inspection

---

## `inspect`

### Identity

`inspect` tells you what a file actually is — not what its name implies, not what the OS defaults to. It reads the file's bytes, parses its structure, measures its entropy, and reports everything the kernel knows about it through `statx`.

An `.exe` extension means nothing. A file with random-looking byte distribution and no recognizable header is probably encrypted. An ELF binary has sections, segments, and dependencies that no generic file tool surfaces cleanly. `inspect` surfaces all of it.

### Syscall Surface

| Syscall | Purpose |
|---|---|
| `statx(STATX_ALL)` | Full metadata including generation number, attributes mask |
| `openat` / `read` | Read magic bytes and content blocks |
| `getxattr` / `listxattr` | Extended attributes and capabilities |
| `mmap` | Map file for entropy analysis and ELF parsing |

ELF parsing is done by reading the `Elf64_Ehdr` header and walking sections and program headers directly from the mapped file. No external library. Entropy analysis reads the file in fixed-size blocks and computes Shannon entropy per block using a frequency table over the 256-byte range.

### Base Behavior

Equivalent to combining `stat`, `file`, and `xxd -l 16`: identifies the file type, reports metadata, and prints the first few bytes.

```
kron inspect <TARGET>
```

### Extended Options

```
-e, --elf                   Full ELF dissection: header, sections, segments, dynamic deps, entry point
-E, --entropy               Compute and display entropy map across the file
    --entropy-block <N>     Block size in bytes for entropy analysis (default: 4096)
-x, --xattr                 Show all extended attributes and their values
-c, --capabilities          Show POSIX capabilities if set on the file
-a, --acl                   Show full ACL entries beyond standard permission bits
-m, --mmap-layout           Show page alignment, mmap offset compatibility, and residency estimate
-M, --magic                 Verbose magic byte analysis with offset table
-l, --links                 Show hardlink count and inode details
    --no-hash               Skip content hash computation
    --hash <ALG>            Hash algorithm: xxhash (default), sha256, sha512
    --json                  Output as structured JSON
    --hex <N>               Dump first N bytes in hex+ASCII format
```

### Modes

**Default mode** — type, metadata summary, magic bytes, hash.
```
kron inspect /usr/bin/ssh
```

**ELF dissection mode** — full binary layout including section names, program header types, interpreter path, and dynamic library dependencies.
```
kron inspect --elf /usr/bin/ssh
```
```
ELF64 | Type: ET_DYN (Shared object)
Entry point:    0x0000000000003eb0
Interpreter:    /lib64/ld-linux-x86-64.so.2

Sections (28):
  [0]  .text       PROGBITS    0x003eb0  size: 89432
  [1]  .rodata     PROGBITS    0x018c00  size: 12048
  ...

Dynamic dependencies:
  libcrypto.so.3
  libc.so.6
  ...
```

**Entropy map mode** — visualize entropy distribution across the file. High entropy (>7.5 bits/byte) indicates encrypted or compressed data. Low entropy (<1.0) indicates repeated patterns or sparse content.
```
kron inspect --entropy --entropy-block 8192 /var/backup/data.bin
```
```
Entropy map (8192-byte blocks):
  [0x000000 - 0x001FFF]  7.99 bits/byte  ████████  encrypted/compressed
  [0x002000 - 0x003FFF]  4.32 bits/byte  ████░░░░  structured data
  [0x004000 - 0x005FFF]  0.81 bits/byte  █░░░░░░░  sparse / zero-padded
```

**mmap layout mode** — reports page boundary alignment, whether the file is mmap-able at offset 0, and estimated memory residency if loaded.
```
kron inspect --mmap-layout /data/large_dataset.bin
```

**JSON mode** — full structured output for scripting.
```
kron inspect --json --elf --entropy /usr/lib/libssl.so | jq '.elf.sections'
```

### Key Differentiators vs `stat` + `file`

- **Entropy analysis** per block — detect encryption, compression, and sparse structure visually
- **ELF dissection** without requiring `readelf` or `objdump`
- **Extended attributes and capabilities** in a single pass
- **mmap alignment analysis** — relevant for files you plan to map into memory
- **Birth time** via `statx`, not available through `stat(2)`
- **Per-block content fingerprinting** for large files

---

## `copy`

### Identity

`copy` is the fastest possible data transfer between two paths. The defining principle is: *if the kernel can move the data without it touching userspace, it will*.

The copy hierarchy is: reflink → `copy_file_range` → `sendfile` → manual read/write loop. Kron selects the fastest mechanism the filesystem supports and reports which one it used. It is always explicit about what it is doing and why.

### Syscall Surface

| Syscall | Purpose |
|---|---|
| `ioctl(FICLONE)` | Reflink: O(1) copy-on-write clone (btrfs, xfs, APFS) |
| `copy_file_range` | Kernel-to-kernel copy: zero userspace traffic |
| `sendfile` | Fallback kernel copy path |
| `fallocate` | Pre-reserve destination space before writing |
| `fallocate(FALLOC_FL_PUNCH_HOLE)` | Reproduce sparse holes in destination |
| `lseek(SEEK_HOLE / SEEK_DATA)` | Map sparse regions in source |
| `ioprio_set` | Throttle I/O priority to avoid saturating scheduler |
| `statx` | Detect filesystem type and sparse status |

### Base Behavior

Equivalent to `cp -a`: copy files and directories, preserve metadata.

```
kron copy <SOURCE> <DESTINATION>
```

### Extended Options

```
-r, --recursive             Copy directories recursively
-p, --preserve <ATTRS>      Preserve attributes: timestamps, ownership, permissions, xattr, all
-s, --sparse                Sparse-aware copy: preserve holes, destination has same real footprint
-R, --reflink               Force reflink (FICLONE); fail explicitly if filesystem does not support it
    --reflink=auto          Attempt reflink, fall back to copy_file_range silently (default)
    --reflink=never         Never attempt reflink; always use copy_file_range or sendfile
-n, --no-clobber            Fail if destination exists (atomic, no TOCTOU window)
-c, --checksum              Verify copy integrity via hash comparison after completion
    --hash <ALG>            Algorithm for --checksum: xxhash (default), sha256
-t, --throttle <BPS>        Limit I/O throughput via ioprio_set (bytes per second)
-d, --dry-run               Report what would happen: syscalls, data volume, mechanism selected
    --mechanism             Show which copy mechanism was selected and why
-j, --jobs <N>              Parallel copy threads for directory operations (default: 1)
    --progress              Show real-time throughput and bytes copied
    --json                  Output operation report as JSON
```

### Modes

**Default mode** — single file copy, auto-selects fastest mechanism.
```
kron copy report.pdf /backup/report.pdf
```
```
Mechanism: copy_file_range (kernel-to-kernel)
Bytes copied: 2,418,124
Elapsed: 3.2ms
```

**Reflink mode** — O(1) copy that shares disk blocks until one side is modified. Requires btrfs or xfs.
```
kron copy --reflink /vm/base.qcow2 /vm/clone.qcow2
```
```
Mechanism: FICLONE (reflink, copy-on-write)
Bytes copied: 0 (shared blocks until modified)
Elapsed: 0.4ms
```

**Sparse-aware copy** — reproduces hole/data layout precisely. Destination occupies the same real disk space as source.
```
kron copy --sparse /vm/disk.img /backup/disk.img
```

**Integrity copy** — copy and verify hash. Useful for backups where silent corruption must be detected.
```
kron copy --checksum --hash sha256 /data/archive.tar /backup/archive.tar
```

**Dry-run mode** — simulate the copy and report full operation plan without writing anything.
```
kron copy --dry-run -r /home/user /backup/user
```
```
[DRY RUN] Source:      /home/user (14,832 files, 48.2 GB)
[DRY RUN] Destination: /backup/user
[DRY RUN] Mechanism:   copy_file_range
[DRY RUN] Sparse files: 23 (total hole size: 1.4 GB — will be preserved)
[DRY RUN] Estimated time: ~12s at current I/O rate
[DRY RUN] No data written.
```

**Throttled copy** — limit throughput to avoid saturating I/O during production workloads.
```
kron copy --throttle 50MB /data/large.iso /archive/large.iso
```

### Key Differentiators vs `cp`

- **Reflink support** — O(1) copy-on-write clone; `cp --reflink` exists but does not report mechanism or fallback behavior explicitly
- **Always reports the copy mechanism** used and why
- **Sparse-hole preservation** with accurate hole/data map, not just apparent size
- **Kernel-to-kernel copy** via `copy_file_range` — data never touches userspace buffers
- **Pre-allocation** via `fallocate` to avoid fragmentation during large copies
- **Atomic no-clobber** with no TOCTOU race condition
- **Post-copy checksum verification** in a single command

---

## `mv`

### Identity

`mv` hides a critical distinction: sometimes a "move" is a single atomic syscall, and sometimes it is a copy followed by a delete. Kron makes this distinction explicit and gives you control over both cases.

The centerpiece is `renameat2` — the extended rename syscall that `mv` never exposes. It provides atomic no-replace semantics and atomic path exchange that are impossible to achieve with any combination of standard tools without a race condition.

### Syscall Surface

| Syscall | Purpose |
|---|---|
| `renameat2(RENAME_NOREPLACE)` | Atomic move that fails if destination exists |
| `renameat2(RENAME_EXCHANGE)` | Atomic swap of two paths — single syscall |
| `statx` | Compare `st_dev` to detect cross-device moves |
| `unlinkat` | Remove source after cross-device copy |

`RENAME_EXCHANGE` is the rarest feature: it swaps two paths in the VFS atomically. There is no moment where either path is missing. This is not achievable with any sequence of `cp` and `rm` calls.

### Base Behavior

Equivalent to `mv`: move a file or directory from source to destination.

```
kron mv <SOURCE> <DESTINATION>
```

### Extended Options

```
-n, --no-replace            Fail atomically if destination exists
                            Uses RENAME_NOREPLACE — no TOCTOU window
-x, --exchange              Atomically swap SOURCE and DESTINATION
                            Uses RENAME_EXCHANGE — both paths exist before and after
-v, --verbose               Report which syscall was used and whether the operation was cross-device
-b, --backup                Before overwriting destination, rename it with a timestamp suffix
    --cross-device=error    Fail explicitly if move would cross device boundaries
    --cross-device=copy     Perform cross-device move transparently (copy + unlink) and report it
    --cross-device=ask      Prompt before performing cross-device copy (default)
-r, --recursive             Move directory trees
    --preserve <ATTRS>      Preserve attributes during cross-device moves: timestamps, xattr, ownership
    --dry-run               Show what would happen without executing
    --json                  Output operation report as JSON
```

### Modes

**Default mode** — same-device rename. Single syscall.
```
kron mv draft.md published.md
```
```
renameat2: draft.md → published.md [same device, atomic]
```

**No-replace mode** — fail atomically if destination exists. Safe for concurrent environments.
```
kron mv --no-replace output.log /archive/output.log
```
```
Error: destination exists and --no-replace is set [EEXIST]
No data moved.
```

**Exchange mode** — atomic path swap. Deploys a new version of a file while keeping the old one accessible — zero downtime, zero race window.
```
kron mv --exchange /srv/app/config.json /tmp/new_config.json
```
```
renameat2(RENAME_EXCHANGE): /srv/app/config.json ↔ /tmp/new_config.json [atomic]
```

**Cross-device move** — explicit report that the operation is copy + unlink, not rename.
```
kron mv --cross-device=copy /home/user/data.tar /mnt/external/data.tar
```
```
Warning: cross-device move detected (/dev/sda1 → /dev/sdb1)
Performing: copy_file_range + unlinkat
Bytes transferred: 4,831,838,208
Source unlinked: /home/user/data.tar
```

**Dry-run mode** — see the full operation plan before committing.
```
kron mv --dry-run /home/user /backup/user
```

### Key Differentiators vs `mv`

- **`RENAME_EXCHANGE`** — atomic path swap in a single syscall; impossible with standard tools
- **`RENAME_NOREPLACE`** — atomic no-clobber with zero TOCTOU window
- **Cross-device transparency** — explicit report and user control when a rename becomes copy+delete
- **Always reports the underlying syscall** used
- **Hardlink preservation** during cross-device moves
- **Per-operation atomicity guarantees** stated clearly in output

---

## `probe`

### Identity

`probe` is the command that has no equivalent in standard tooling. It attaches to a Kron operation and measures it at the hardware level — CPU cycles, cache misses, branch mispredictions, real I/O bytes, page faults, and individual syscall latencies. Not approximations. Not sampled averages. Direct readings from hardware performance counters and the kernel's own instrumentation interfaces.

When you run `kron probe copy src/ dst/`, you are not timing a copy. You are reading the CPU's hardware event counters before and after, intercepting every syscall the copy generates via `ptrace`, and reading the kernel's per-process I/O accounting. The result is a complete hardware-level profile of a specific filesystem operation.

### Syscall Surface

| Syscall / Interface | Purpose |
|---|---|
| `perf_event_open(PERF_TYPE_HARDWARE)` | Open hardware counter fds for the target PID |
| `perf_event_open(PERF_TYPE_SOFTWARE)` | Page fault counters (minor and major) |
| `ptrace(PTRACE_SYSCALL)` | Intercept each syscall entry/exit for latency measurement |
| `RDTSC` (x86 inline asm) | Nanosecond-resolution timestamps from the CPU clock |
| `/proc/PID/io` | Kernel-reported bytes read/written vs bytes requested |
| `/proc/PID/stat` | CPU time, context switches, scheduling events |

Hardware counter reads use `read(fd)` on the `perf_event_open` file descriptors. `RDTSC` timestamps each syscall boundary. `ptrace` gives exact entry/exit points for every kernel call the target makes.

### Base Behavior

Attach to a Kron command and profile it.

```
kron probe <COMMAND> [COMMAND_ARGS...]
```

Examples:
```
kron probe list /data/archive
kron probe copy /src /dst
kron probe inspect --elf /usr/bin/gcc
```

### Extended Options

```
-c, --counters <LIST>       Hardware counters to track (comma-separated):
                              cycles, instructions, cache-refs, cache-misses,
                              branch-instructions, branch-misses, bus-cycles
                            Default: cycles,instructions,cache-misses,branch-misses
-s, --syscalls              Record every syscall with entry timestamp and duration
    --syscall-filter <SYS>  Only record specified syscalls (comma-separated names)
-p, --page-faults           Record minor and major page fault counts
-i, --io                    Report real I/O: bytes requested vs bytes transferred (via /proc/PID/io)
-t, --timeline              Output a chronological waterfall of syscall events
    --rdtsc                 Use RDTSC for sub-nanosecond timestamps instead of clock_gettime
-d, --diff <BASELINE>       Compare current profile against a saved JSON baseline
    --save <FILE>           Save profile as JSON for later --diff comparison
    --threshold <N>         In --diff mode, flag regressions larger than N percent
    --json                  Output full profile as JSON
    --no-ptrace             Disable syscall interception (counters only, lower overhead)
    --summary               Print one-line summary per counter after run (default)
    --verbose               Print all raw counter values and per-syscall table
```

### Hardware Counters Reference

| Counter | What it measures |
|---|---|
| `cycles` | Raw CPU cycles consumed |
| `instructions` | Instructions retired |
| `cache-misses` | Last-level cache misses |
| `cache-refs` | Last-level cache references |
| `branch-misses` | Branch prediction failures |
| `branch-instructions` | Total branches evaluated |
| `bus-cycles` | Bus cycles (memory bus activity proxy) |

Derived metrics computed by `probe`:
- **IPC** (Instructions Per Cycle) = `instructions / cycles`
- **Cache miss rate** = `cache-misses / cache-refs`
- **Branch miss rate** = `branch-misses / branch-instructions`
- **Bytes per cache miss** = `io.bytes_read / cache-misses`

### Modes

**Default mode** — profile a command and print a hardware summary.
```
kron probe copy /home/user/data /backup/data
```
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  kron probe — copy /home/user/data /backup/data
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  Hardware Counters
  ─────────────────────────────────────────────
  CPU Cycles:           1,482,304,811
  Instructions:         2,831,004,219
  IPC:                  1.91
  Cache Misses:         14,382,004
  Cache Miss Rate:      2.3%
  Branch Misses:        4,821,003
  Branch Miss Rate:     0.7%

  I/O Accounting (kernel)
  ─────────────────────────────────────────────
  Bytes requested:      48,234,881,024
  Bytes transferred:    48,234,881,024
  Syscall count:        9,841
  Page faults (minor):  12,304
  Page faults (major):  0

  Derived
  ─────────────────────────────────────────────
  Bytes per cache miss: 3,352
  Elapsed (RDTSC):      4,831 ms
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

**Syscall waterfall mode** — chronological log of every syscall with timestamps and duration.
```
kron probe --syscalls --timeline list /data
```
```
  Syscall Waterfall
  ─────────────────────────────────────────────────────────────
  +0.000000ms   openat          → fd=7           [0.021ms]
  +0.021ms      getdents64      → 64 entries     [0.134ms]
  +0.155ms      statx           → ok             [0.018ms]
  +0.173ms      statx           → ok             [0.016ms]
  ...
  +1.832ms      close           → ok             [0.004ms]
  ─────────────────────────────────────────────────────────────
  Total syscalls: 483  |  Total syscall time: 1.832ms
```

**Diff mode** — compare two runs and flag regressions.
```
kron probe --save baseline.json list /data
# (make changes to list implementation)
kron probe --diff baseline.json --threshold 5 list /data
```
```
  Regression Report (threshold: 5%)
  ─────────────────────────────────────────────
  cache-misses:    +23.4%  ⚠ REGRESSION
  cycles:          +8.1%   ⚠ REGRESSION
  instructions:    +0.4%   ✓ ok
  branch-misses:   -2.1%   ✓ ok (improvement)
```

**Counter-only mode** — disable ptrace for minimal probe overhead. Useful when syscall timing would distort results.
```
kron probe --no-ptrace --counters cycles,cache-misses copy /src /dst
```

### Key Differentiators vs `strace` + `perf stat`

- **Attaches to Kron commands natively** — no manual PID tracking or separate terminal
- **Hardware counters + syscall timeline in one output** — `strace` and `perf stat` are separate tools that cannot be trivially combined
- **RDTSC timestamps** — sub-nanosecond precision, not gated by `clock_gettime` resolution
- **Derived efficiency metrics** — IPC, bytes-per-cache-miss, cache miss rate computed automatically
- **Diff mode** — structured regression detection between runs
- **JSON output** — every profile is serializable for storage and comparison

---

## Global Options

These options apply to all commands.

```
    --version               Print version and build information
    --help                  Print command help
-v, --verbose               Increase output verbosity
    --no-color              Disable ANSI escape codes
    --json                  Force JSON output (where supported)
    --dry-run               Simulate operation without writing or moving anything
```

---

## Building

Kron requires a C++20-compliant compiler (GCC 12+ or Clang 15+) and CMake 3.20+. No external dependencies.

```bash
git clone https://github.com/TheNobelVoid/kron
cd kron
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Linux kernel 5.6+ is required for full `renameat2` and `copy_file_range` support. Kernel 5.10+ is recommended for complete `statx` field availability. `perf_event_open` requires either `CAP_PERFMON` or `perf_event_paranoid <= 2` for hardware counter access.

---

## Platform

Kron targets Linux exclusively. The syscall surface it uses — `getdents64`, `statx`, `renameat2`, `copy_file_range`, `perf_event_open`, `FICLONE` — is Linux-specific by design. Portability is not a goal. Proximity to the Linux kernel is.

---

## License

AGPLv3
