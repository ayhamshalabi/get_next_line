Engineered and developed by [Ayham Shalabi](https://github.com/ayhamshalabi)  
https://github.com/ayhamshalabi

# get_next_line: Dynamic Stream Line Reader

A resilient, memory-safe C library engineered to parse and extract arbitrary-length newline-terminated strings sequentially from POSIX file descriptors. Built from low-level POSIX primitives, `get_next_line` implements modular buffer accumulation, persistent state tracking via block-scope static pointers, and systematic rollback deallocation under constrained heap environments.

---

## Table of Contents
1. [Overview & Architecture](#overview--architecture)
2. [Quick Navigation Index](#quick-navigation-index)
3. [Detailed Library Description](#detailed-library-description)
   - [Core Design Principles](#core-design-principles)
   - [Stream Processing Model](#stream-processing-model)
   - [Memory Lifecycle & Safety Strategy](#memory-lifecycle--safety-strategy)
4. [Build & Installation Instructions](#build--installation-instructions)
   - [Prerequisites](#prerequisites)
   - [Makefile Targets](#makefile-targets)
   - [Custom Buffer Sizes](#custom-buffer-sizes)
   - [Integrating into External Projects](#integrating-into-external-projects)
   - [Running Verification Tests](#running-verification-tests)
5. [Exhaustive Function Breakdown](#exhaustive-function-breakdown)
   - [Core Stream Engine](#core-stream-engine)
     - [get_next_line](#get_next_line)
     - [read_and_save](#read_and_save)
     - [extract_line](#extract_line)
     - [keep_leftovers](#keep_leftovers)
   - [Utility Subsystem](#utility-subsystem)
     - [ft_strlen](#ft_strlen)
     - [ft_strchr](#ft_strchr)
     - [ft_strjoin](#ft_strjoin)
6. [Resources & Verified References](#resources--verified-references)
7. [License](#license)

---

## Overview & Architecture

Stream-oriented input/output in standard POSIX environments (`read(2)`) operates over raw byte chunks governed by fixed-capacity memory buffers rather than logical line boundaries. Because file lines can be arbitrarily long, a single `read()` invocation might return only a fragment of a line, or conversely, the termination of the current line followed by portions of subsequent lines.

`get_next_line` bridges the gap between fixed-size block reads and logical record processing. By decoupling physical read chunks (`BUFFER_SIZE`) from logical line boundaries (`\n`), the library provides deterministic, single-line extractions across successive calls until End-Of-File (`EOF`).

```
                    +------------------------------------+
                    |        read(fd, buffer, N)         |
                    +------------------------------------+
                                      |
                                      v
+--------------+    +------------------------------------+
| Static State |<---| Append & Accumulate (read_and_save)|<---+
+--------------+    +------------------------------------+    |
       |                              |                       | Loop until
       |                     Does buffer contain '\n'?        | '\n' or EOF
       |                              |                       |
       |                     +--------+--------+              |
       |                     |                 |              |
       |                  NO v               YES              |
       |            (Continue reading)         |              |
       |                                       v              |
       |                    +----------------------+          |
       +------------------->| Extract Current Line |          |
       |                    +----------------------+          |
       |                               |                      |
       |                               v                      |
       |                    +----------------------+          |
       +====================| Retain Leftover Tail |          |
       |                    +----------------------+          |
       |                               |                      |
       v                               v                      |
  Updated Tail                 Return char *line              |
```

---

## Quick Navigation Index

| Function Name | Prototype | Subsystem | Summary |
| :--- | :--- | :--- | :--- |
| [`get_next_line`](#get_next_line) | `char *get_next_line(int fd);` | Core API | Primary entry point returning a single line from a file descriptor. |
| [`read_and_save`](#read_and_save) | `static char *read_and_save(int fd, char *saved);` | Stream Engine | Reads chunks from descriptor and accumulates into buffer until `\n` or `EOF`. |
| [`extract_line`](#extract_line) | `static char *extract_line(char *saved);` | Stream Engine | Allocates and copies a single line up to and including the first `\n`. |
| [`keep_leftovers`](#keep_leftovers) | `static char *keep_leftovers(char *saved);` | Stream Engine | Trims extracted line from accumulator and preserves trailing bytes. |
| [`ft_strlen`](#ft_strlen) | `size_t ft_strlen(const char *s);` | String Utility | Computes length of null-terminated string with NULL-safety protection. |
| [`ft_strchr`](#ft_strchr) | `char *ft_strchr(const char *s, int c);` | String Utility | Locates first occurrence of character in string with NULL check. |
| [`ft_strjoin`](#ft_strjoin) | `char *ft_strjoin(char *s1, const char *s2);` | Memory Utility | Concatenates two strings with overflow checks and frees source buffer. |

---

## Detailed Library Description

### Core Design Principles
1. **Zero Unbounded Static Allocations:** Unlike naive line-readers that reserve multi-megabyte fixed-size static arrays (wasting BSS segment memory), `get_next_line` maintains only a single block-scope pointer (`static char *saved`). Dynamic heap allocations scale strictly with line length.
2. **Defensive I/O Execution:** System call overhead is minimized through lazy accumulation. When leftover bytes from a prior `read()` already contain a newline delimiter, `get_next_line` returns immediately without executing an unnecessary `read(2)` syscall.
3. **Rollback Resilience:** Any runtime failure—such as a memory exhaustion error during buffer growth or a hardware I/O read failure—triggers immediate deallocation of both heap scratch buffers and accumulated state, leaving no dangling allocations or memory leaks.

### Stream Processing Model
The library partitions stream ingestion into three deterministic pipeline phases:
* **Accumulation (`read_and_save`):** Employs a temporary scratch buffer (`malloc(BUFFER_SIZE + 1)`) to read blocks of `BUFFER_SIZE` bytes. Read segments are appended to the static accumulator until `ft_strchr` locates a newline or `read()` reports `0` (`EOF`).
* **Line Slicing (`extract_line`):** Scans the accumulated string to identify the position of `\n` (or `\0` if the file terminates without a trailing newline). It dynamically sizes a new buffer, transfers the characters, adds a null-terminator, and returns the line to the caller.
* **State Preservation (`keep_leftovers`):** Reallocates the trailing slice of the accumulator that follows `\n`. If no trailing characters remain, the accumulator is deallocated and reset to `NULL`.

### Memory Lifecycle & Safety Strategy
- **Heap Buffer Allocation:** Temporary read buffers are explicitly allocated on the heap rather than the execution stack. This prevents stack frame exhaustion when compiled with large buffer configurations (such as `BUFFER_SIZE=10000000`).
- **Integer Overflow Guards:** All length calculations leverage unsigned `size_t` primitives. `ft_strjoin` incorporates arithmetic boundary assertions (`len1 > SIZE_MAX - len2 - 1`) to eliminate integer wrap vulnerabilities.
- **Negative File Descriptor Protection:** The core engine validates `fd >= 0`, `BUFFER_SIZE > 0`, and `BUFFER_SIZE <= 0x7FFFFFFF` prior to performing any system calls.

---

## Build & Installation Instructions

### Prerequisites
- A POSIX-compliant operating system (Linux, macOS, BSD).
- A C compiler compliant with C99 or later (`cc`, `gcc`, or `clang`).
- GNU Make or compatible build utility.

### Makefile Targets
The repository includes a production-ready `Makefile` providing standard targets:

```bash
# Build the static archive (libgnl.a)
make

# Remove intermediate object files (*.o)
make clean

# Remove object files and the static library
make fclean

# Perform a full rebuild
make re

# Build the verification test executable
make test
```

### Custom Buffer Sizes
You can configure `BUFFER_SIZE` at compilation time without modifying code by using the `-D` preprocessor flag:

```bash
# Compile with a 32-byte buffer
cc -Wall -Wextra -Werror -D BUFFER_SIZE=32 get_next_line.c get_next_line_utils.c main.c -o gnl

# Compile with a 1 MB buffer
cc -Wall -Wextra -Werror -D BUFFER_SIZE=1048576 get_next_line.c get_next_line_utils.c main.c -o gnl
```
If omitted, `BUFFER_SIZE` defaults to `4096` bytes (the standard POSIX virtual memory page size).

### Integrating into External Projects
1. Build the static library:
   ```bash
   make
   ```
2. Include the header in your C source files:
   ```c
   #include "get_next_line.h"
   ```
3. Link the archive during compilation:
   ```bash
   cc -Wall -Wextra -Werror main.c -L. -lgnl -o my_program
   ```

### Running Verification Tests
The project includes a comprehensive test suite in `main.c` testing:
- Invalid file descriptor validation (`fd = -1`).
- Empty file handling (`empty.txt`).
- Multi-line files containing short lines, empty lines, large lines, and files lacking a trailing newline (`test.txt`).

```bash
make test
./test_runner
```

To run memory leak verification with Valgrind:
```bash
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./test_runner
```

---

## Exhaustive Function Breakdown

### Core Stream Engine

#### `get_next_line`
- **Prototype:** `char *get_next_line(int fd);`
- **Parameters:**
  - `int fd`: The open file descriptor to read from.
- **Return Value:**
  - `char *`: Dynamically allocated string containing the next line read from `fd` (including the trailing `\n` if present). Returns `NULL` on EOF, read error, or allocation failure.
- **Algorithmic Approach:**
  1. Validates preconditions (`fd < 0 || BUFFER_SIZE <= 0 || BUFFER_SIZE > 0x7FFFFFFF`).
  2. Invokes `read_and_save` to append stream chunks into the `static char *saved` accumulator until a newline or EOF is found.
  3. If accumulation produces `NULL`, resets state and returns `NULL`.
  4. Invokes `extract_line` to duplicate the prefix up to `\n`.
  5. Invokes `keep_leftovers` to truncate the consumed line from `saved`.
  6. Returns the extracted line.
- **Safety Guards:** Guarantees that on extraction error, `saved` is deallocated and explicitly reset to `NULL` to avoid stale references on subsequent invocations.

---

#### `read_and_save`
- **Prototype:** `static char *read_and_save(int fd, char *saved);`
- **Parameters:**
  - `int fd`: File descriptor target.
  - `char *saved`: Current accumulator buffer pointer.
- **Return Value:**
  - `char *`: Pointer to updated accumulator buffer, or `NULL` on failure.
- **Algorithmic Approach:**
  1. Allocates `BUFFER_SIZE + 1` bytes on the heap.
  2. Loops while `ft_strchr(saved, '\n') == NULL`.
  3. Executes `read(fd, buffer, BUFFER_SIZE)`.
  4. On read failure (`bytes_read == -1`), frees both `saved` and `buffer`, returning `NULL`.
  5. On EOF (`bytes_read == 0`), breaks out of loop.
  6. Null-terminates `buffer` at `buffer[bytes_read] = '\0'`.
  7. Joins `buffer` to `saved` using `ft_strjoin`. If join fails, frees `buffer` and aborts.
  8. Deallocates scratch buffer and returns the accumulated string.
- **Safety Guards:** Heap-allocated scratch buffer protects against runtime stack overflows on large buffer constants.

---

#### `extract_line`
- **Prototype:** `static char *extract_line(char *saved);`
- **Parameters:**
  - `char *saved`: Accumulated stream string.
- **Return Value:**
  - `char *`: Newly allocated string containing a single line, or `NULL` if accumulator is empty or allocation fails.
- **Algorithmic Approach:**
  1. Scans `saved` to determine length `i` up to `\n` or `\0`.
  2. Allocates `i + (saved[i] == '\n') + 1` bytes.
  3. Copies characters into `line`.
  4. If `saved[i] == '\n'`, appends `\n`.
  5. Appends null-terminator `\0` and returns pointer.
- **Safety Guards:** Exact allocation eliminates memory over-allocation.

---

#### `keep_leftovers`
- **Prototype:** `static char *keep_leftovers(char *saved);`
- **Parameters:**
  - `char *saved`: Accumulated stream string containing trailing bytes.
- **Return Value:**
  - `char *`: Truncated accumulator containing only unread trailing bytes, or `NULL` if no unread characters remain.
- **Algorithmic Approach:**
  1. Locates index `i` of newline character.
  2. If `saved[i] == '\0'` or `saved[i + 1] == '\0'`, entire accumulator has been consumed; frees `saved` and returns `NULL`.
  3. Allocates `ft_strlen(saved) - i` bytes for unread remainder.
  4. Copies characters from index `i + 1` onwards.
  5. Null-terminates new string, frees old `saved` buffer, and returns new pointer.
- **Safety Guards:** Automatic memory release at EOF prevents persistent unreachable heap allocations across function calls.

---

### Utility Subsystem

#### `ft_strlen`
- **Prototype:** `size_t ft_strlen(const char *s);`
- **Parameters:**
  - `const char *s`: Null-terminated string.
- **Return Value:**
  - `size_t`: Number of characters preceding `\0`.
- **Algorithmic Approach:**
  - Returns `0` if `s == NULL`. Iterates sequentially until `\0`.
- **Safety Guards:** Explicit NULL pointer check prevents segmentation faults.

---

#### `ft_strchr`
- **Prototype:** `char *ft_strchr(const char *s, int c);`
- **Parameters:**
  - `const char *s`: String to search.
  - `int c`: Character to find (cast to `char`).
- **Return Value:**
  - `char *`: Pointer to first matched character, or `NULL` if not found.
- **Algorithmic Approach:**
  - Scans `s` until match or `\0`. Correctly matches null-terminator when `c == '\0'`.
- **Safety Guards:** NULL-safe guard returns `NULL` immediately if `s == NULL`.

---

#### `ft_strjoin`
- **Prototype:** `char *ft_strjoin(char *s1, const char *s2);`
- **Parameters:**
  - `char *s1`: First string (heap-allocated, freed upon completion).
  - `const char *s2`: Second string to append.
- **Return Value:**
  - `char *`: Newly allocated concatenated string, or `NULL` on failure.
- **Algorithmic Approach:**
  1. Calculates lengths using `ft_strlen`.
  2. Checks for `SIZE_MAX` addition overflow.
  3. Allocates `len1 + len2 + 1` bytes.
  4. Copies `s1` followed by `s2`, null-terminates, frees `s1`, and returns result.
- **Safety Guards:** Takes ownership of `s1` and guarantees deallocation on both success and failure paths, preventing memory leaks during accumulation cycles.

---

## Resources & Verified References

All external documentation and technical specifications referenced below have been verified:

- [Linux Programmer's Manual: getline(3)](https://manpages.debian.org/unstable/manpages-dev/getline.3.en.html) — Reference implementation and semantics for dynamic line ingestion.
- [Linux Programmer's Manual: read(2)](https://manpages.debian.org/unstable/manpages-dev/read.2.en.html) — System call specification for descriptor reads.
- [FreeBSD Hypertext Man Pages: read(2)](https://man.freebsd.org/cgi/man.cgi?query=read&sektion=2) — BSD kernel I/O interface and error conditions.
- [Brian W. Kernighan Home Page (Princeton)](https://www.cs.princeton.edu/~bwk/) — Co-author of The C Programming Language (2nd Edition).
- [GNU C Library Reference Manual: Line Input](https://sourceware.org/glibc/manual/) — glibc stream architecture and buffering guidelines.
- [ISO C Reference: Storage Duration & static Specifier](https://en.cppreference.com/c/language/storage_duration) — Technical documentation on block-scope static variable lifetime.
- [ISO C Standard Reference: cppreference.com](https://en.cppreference.com/c) — Modern C language reference and standard library definitions.

---

## License

This project is licensed under the terms of the [MIT License](LICENSE).