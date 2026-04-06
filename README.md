# kron

A multifunctional CLI tool for file manipulation and inspection, built for developers and power users.  
Written in **modern C++20** using exclusively the STL. Zero external dependencies.

---

## What is kron?

kron is a blend of classic shell commands (`list`, `inspect`, `watch`, `delete`, `find`) with advanced operations for analysis, indexing, snapshots, and filesystem automation. Its core is **deep filesystem visibility**.

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

## License

MIT
