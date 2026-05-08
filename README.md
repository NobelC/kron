# kls (Security-ls)

**"What's in here, and should I be worried?"**

`kls` is a directory listing tool designed under the Unix philosophy: one tool, one responsibility, done to perfection. Unlike `exa` (colors) or `lsd` (icons), `kls` focuses on **security and risk visibility**.

---

## Philosophy
`kls` doesn't try to be an editor or a network monitor. Its sole purpose is to audit your directories and report security anomalies in real-time.

- ✅ **Read-only. Report-only.**
- ❌ No file copying.
- ❌ No file deletion.
- ❌ Non-destructive.

## Key Features
- **Health-check by default**: Every listing includes a security health analysis.
- **Risk Detection**: Identifies SUID/SGID files, world-writable permissions, broken symlinks, and Linux capabilities.
- **Attack Surface Analysis**: A consolidated summary of the total risk for the directory.
- **Timeline Analysis**: Detects timestamp anomalies (ctime < mtime) and files modified in the future.

## Quick Start

```bash
kls                # Listing with security analysis by default
kls -l             # Long format with security metadata
kls --only-alerts  # Show only files with security risks
kls --sort=severity # Most dangerous files first
```

## Installation

### Requirements
- **CMake 3.15+**
- **C++23** compatible compiler

### Build
```bash
mkdir build && cd build
cmake ..
make
```

To install globally:
```bash
sudo ln -sf $(pwd)/build/kls /usr/local/bin/kls
```

---

## License
MIT
