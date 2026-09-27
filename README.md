*This activity has been created as part of the 42 curriculum by ayhshala.*

# Get Next Line

## Description

**Get Next Line** is a foundational C programming project in the 42 curriculum. Its goal is to implement a function (`get_next_line`) that reads and returns a single line from a given file descriptor (`fd`) on each call, allowing a file (or standard input) to be read sequentially one line at a time using repeated calls in a loop.

Key objectives of this project include:
- Understanding and utilizing **static variables** in C to persist unread data across multiple function calls without using forbidden global variables.
- Managing dynamic heap memory safely (`malloc` and `free`) with zero memory leaks across all edge cases (`EOF`, read errors, empty files, and varying buffer sizes).
- Working directly with POSIX system calls (`read`) and file descriptors.

### Function Prototype
```c
char *get_next_line(int fd);
```

- **Parameters:** `fd` — The file descriptor to read from.
- **Return Value:**
  - The line that was read (including the terminating `\n` character, unless the end of the file was reached without a trailing `\n`).
  - `NULL` if there is nothing left to read or if an error occurred.

---

## Instructions

### Files
- `get_next_line.h` — Header file containing function prototypes, required library includes (`<unistd.h>`, `<stdlib.h>`), and the default `BUFFER_SIZE` macro definition.
- `get_next_line.c` — Core logic (`get_next_line`, `read_and_save`, `extract_line`, and `keep_leftovers`).
- `get_next_line_utils.c` — NULL-safe string helper functions (`ft_strlen`, `ft_strchr`, and `ft_strjoin`).

### Compilation
Compile `get_next_line.c` and `get_next_line_utils.c` alongside your test file (e.g., `main.c`) using `cc` and the standard 42 flags. You can specify any custom `BUFFER_SIZE` at compile time using the `-D BUFFER_SIZE=n` flag:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c main.c -o gnl
```

It also compiles cleanly without the `-D BUFFER_SIZE` flag (defaulting to `BUFFER_SIZE = 42` defined in `get_next_line.h`):

```bash
cc -Wall -Wextra -Werror get_next_line.c get_next_line_utils.c main.c -o gnl
```

### Execution
Run the compiled binary:

```bash
./gnl
```

To check for memory leaks using Valgrind:

```bash
valgrind --leak-check=full --show-leak-kinds=all ./gnl
```

---

## Algorithm Explanation and Justification

### Overview of the Algorithm
Because `read(fd, buffer, BUFFER_SIZE)` reads a fixed chunk of bytes rather than stopping at a newline (`\n`), a single `read()` call may return:
1. Only part of a line (when `BUFFER_SIZE` is smaller than the line length).
2. The end of the current line **plus** the beginning of one or more subsequent lines (when `BUFFER_SIZE` is larger than the remaining characters on the current line).

To solve this while reading as little as possible on each call, our algorithm uses a single **`static char *saved`** pointer inside `get_next_line(int fd)` and processes each call in three modular stages:

1. **Read and Accumulate (`read_and_save`):**
   - Allocates a temporary buffer of size `BUFFER_SIZE + 1` on the heap.
   - Checks if `saved` already contains a `\n` using `ft_strchr(saved, '\n')`.
   - As long as `saved` does **not** contain a `\n`, it calls `read(fd, buffer, BUFFER_SIZE)`, null-terminates `buffer` at `buffer[bytes_read] = '\0'`, and appends `buffer` to `saved` using `ft_strjoin(saved, buffer)` (which frees the previous `saved` block automatically).
   - Stops immediately once a `\n` is detected in `saved`, when `read()` returns `0` (`EOF`), or if `read()` returns `-1` (freeing both `buffer` and `saved` and returning `NULL`).

2. **Extract the Current Line (`extract_line`):**
   - Scans `saved` up to the first `\n` or `\0`.
   - Allocates a new string `line` of exact size `i + (saved[i] == '\n') + 1` and copies the characters of the current line (including `\n` if present) followed by `\0`.

3. **Preserve Leftover Characters (`keep_leftovers`):**
   - Locates the end of the extracted line in `saved`.
   - If the end of `saved` was reached (`!saved[i] || !saved[i + 1]`), meaning no unread characters remain after `\n`, it frees `saved` and returns `NULL`.
   - Otherwise, it allocates a new string `new_saved` containing only the characters after `\n`, frees the old `saved`, and returns `new_saved` so the static pointer retains the unread characters for the next call.

### Justification of Design Choices
- **Heap Allocation for `buffer` (`malloc(BUFFER_SIZE + 1)`):** Allocating `buffer` on the heap instead of the stack (`char buffer[BUFFER_SIZE + 1]`) prevents stack overflow crashes when tested with very large buffer sizes (e.g., `BUFFER_SIZE = 10000000`).
- **Lazy Reading (Minimal `read()` Calls):** By checking `!ft_strchr(saved, '\n')` *before* calling `read()`, if `saved` already contains multiple lines from a prior large read, subsequent calls to `get_next_line()` extract lines directly from `saved` without making unnecessary `read()` system calls.
- **Immediate Cleanup at EOF:** In `keep_leftovers()`, checking `!saved[i] || !saved[i + 1]` immediately frees `saved` and resets it to `NULL` when no characters remain after `\n`, avoiding unnecessary empty-string allocations and ensuring zero reachable heap blocks remain at EOF.

---

## Resources

### Classic References
- [Linux Man Page: `read(2)`](https://man7.org/linux/man-pages/man2/read.2.html) — POSIX specification for reading from a file descriptor.
- [Linux Man Page: `open(2)`](https://man7.org/linux/man-pages/man2/open.2.html) — File descriptor creation and flags (`<fcntl.h>`).
- [C Storage-Class Specifiers (`static`) — cppreference](https://en.cppreference.com/w/c/language/storage_duration) — Documentation on static storage duration and block-scope static variables.

### Use of AI
AI was used as an interactive tutor during this project for the following tasks:
- **Conceptual Understanding:** Explaining POSIX file descriptors, `open()`, `read()`, `close()`, and how `static` variables behave in memory compared to stack and global variables.
- **Algorithm Design & Edge-Case Review:** Walking through the 3-step stash/extract/trim logic, verifying 42 Norm constraints, and designing local debug tests (`main.c` and `test.txt`).
- **Documentation:** Structuring and formatting this `README.md` according to the Chapter 5 requirements of the subject.