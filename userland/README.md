# GraphOS Userland

User-space programs and C standard library for GraphOS.

## Structure

```
userland/
├── libc/               # Standard C library
│   ├── syscall.c       # System call wrappers
│   ├── string.c        # String functions
│   ├── stdio.c         # I/O functions
│   ├── stdlib.c        # Standard library
│   └── _start.asm      # Program entry point
├── include/
│   └── libc.h          # Public API
├── hello.c             # Hello World
├── cat.c               # Display file contents
├── ls.c                # List directory
├── fork_test.c         # Test fork()
├── echo.c              # Echo arguments
└── Makefile            # Build system
```

## Building

```bash
cd userland
make all          # Build everything
make programs     # Build all programs
make clean        # Clean build files
make info         # Show information
```

## Output

Programs are built in `build/` directory:
- `libc.a` - Static C library
- `hello`, `cat`, `ls`, `fork_test`, `echo` - User programs (ELF32)

## Library Functions

### System Calls (syscall.c)
- **Process:** `exit()`, `fork()`, `getpid()`, `yield()`, `wait()`
- **File I/O:** `open()`, `close()`, `read()`, `write()`
- **Memory:** `brk()`

### String Functions (string.c)
- **Length/Copy:** `strlen()`, `strcpy()`, `strncpy()`
- **Compare:** `strcmp()`, `strncmp()`
- **Concatenate:** `strcat()`, `strncat()`
- **Search:** `strchr()`, `strrchr()`
- **Memory:** `memcpy()`, `memset()`, `memcmp()`, `memmove()`

### I/O Functions (stdio.c)
- `putchar()` - Output character
- `puts()` - Output string
- `printf()` - Formatted output (supports %s, %d, %c, %x, %%)

### Standard Library (stdlib.c)
- **Memory:** `malloc()`, `free()`, `realloc()`, `calloc()`
- **Conversions:** `atoi()`, `atol()`
- **Utility:** `abort()`, `abs()`

## Writing New Programs

1. Create your `.c` file in `userland/`
2. Include the library: `#include "include/libc.h"`
3. Write your `main()` function
4. Add program name to `PROGRAMS` in Makefile
5. Run `make all`

Example:
```c
#include "include/libc.h"

int main(int argc, char** argv) {
    printf("Hello from user mode!\n");
    printf("My PID is: %d\n", getpid());
    return 0;
}
```

## Program Details

### hello.c
Simple Hello World program demonstrating basic user mode execution and printf.

### cat.c
Display file contents. Usage: `cat <filename>`
Tests file I/O syscalls (open, read, write, close).

### ls.c
List directory contents. Currently a stub awaiting readdir syscall.

### fork_test.c
Test process creation with fork(). Demonstrates parent-child processes and wait().

### echo.c
Echo command line arguments. Tests argc/argv parsing.

## Technical Details

- **Format:** ELF32 executables
- **Architecture:** x86 (32-bit)
- **Linking:** Static linking with libc.a
- **Entry Point:** `_start` (calls main, then exit)
- **Syscalls:** via int 0x80 interface
- **Privilege:** Ring 3 (user mode)

## Limitations

- No dynamic linking support
- malloc() doesn't actually free memory yet (simple brk-based allocator)
- printf() has limited format specifiers
- No stdin support yet
- Limited syscall set

## Future Enhancements

- [ ] Dynamic linking support
- [ ] Better malloc/free implementation
- [ ] Full printf with all format specifiers
- [ ] stdin/scanf support
- [ ] More syscalls (pipe, readdir, etc.)
- [ ] POSIX-compliant APIs
- [ ] Thread support (pthread)

## Version

**Version:** 1.0  
**GraphOS Version:** v1.2.0  
**Last Updated:** 2026-10-07
