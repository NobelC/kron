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

## Installation & Compilation

### Requirements
- **CMake 3.15+**
- **C++23** compatible compiler (GCC 13+ or Clang 16+)

### 🛠️ Developer Build (With Sanitizers & Debug symbols)
For testing, debugging, and development, compile with sanitizers enabled by default:
```bash
cmake -B build
cmake --build build
```

### 🚀 Production Installation (For End Users)
If you want to build and install `kls` on your system for regular use, configure it in **Release** mode with sanitizers disabled:

```bash
# 1. Configure in Release mode (sanitizers disabled for performance)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_SANITIZERS=OFF

# 2. Build the project
cmake --build build

# 3. Install globally (defaults to /usr/local)
sudo cmake --install build

# Or specify a custom prefix (e.g., /usr)
sudo cmake --install build --prefix /usr
```

### 📦 Installation via Package (.deb)
If you generated a Debian package using CPack (`cpack` inside the `build` directory), you can install it directly:
```bash
sudo apt install ./kls-0.1.1-Linux.deb
```

### 🧹 Uninstallation
To completely remove `kls` and its manual pages/completions from your system:
```bash
sudo cmake --build build --target uninstall
```


## Architecture

`kls` is built with performance and safety in mind. It utilizes a custom multi-threaded traversal engine that leverages `statx` for modern Linux metadata collection.

The processing pipeline is divided into:
1. **Collection**: Concurrent filesystem scanning.
2. **Filtering**: Efficient glob-based reduction.
3. **Sorting**: Flexible result ordering.
4. **Presentation**: User-friendly terminal rendering.

For more technical details, see the [List Architecture Documentation](docs/LIST-ARCHITECTURE.md).

---

## License
MIT
