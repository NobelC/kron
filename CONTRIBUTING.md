# Contributing to kls

kls is a security-focused directory listing tool.

## Philosophy
- One tool, one responsibility.
- Security-first visibility.

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
kls [options] [args]
```

**Output received**
Paste the actual output or error message.

**Expected behavior**
What you expected kls to do instead.

**System**
OS and compiler version.

---

## Pull requests

- One PR per fix or improvement
- Reference the issue it closes: `Closes #N`
- Keep changes focused — don't mix unrelated fixes in the same PR
