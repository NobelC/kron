# Contributing to kron

kron is a personal project. I'm not looking for new commands or features that aren't already planned — those decisions are mine.

**What I do accept:**
- Bug fixes
- Performance improvements on existing code
- Output/formatting improvements on existing commands
- Extra functionality on already-implemented commands

If it's not in one of those categories, it will likely be closed.

---

## Before writing any code

Open an issue first. Describe what you want to change and why.  
I'll let you know if it's something I want before you spend time on it.

---

## Code style

- **C++20 standard**
- **STL only** — no external dependencies, no exceptions
- Code should be readable without comments — but comments are welcome when they add real context
- For performance improvements: include a benchmark comment comparing old vs new execution time
- For behavior changes: include a comment explaining what changed and why

---

## Bug reports

Open an issue with the following format:

**Error name / description**
What is the error or unexpected behavior.

**Command executed**
```
kron <command> [options] [args]
```

**Output received**
Paste the actual output or error message.

**Expected behavior**
What you expected kron to do instead.

**System**
OS and compiler version.

---

## Pull requests

- One PR per fix or improvement
- Reference the issue it closes: `Closes #N`
- Keep changes focused — don't mix unrelated fixes in the same PR
