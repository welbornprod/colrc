# Documentation {#index}

# ColrC
**ColrC** is a C library for terminal colors/escape-codes on linux.

The only two files that are needed to use ColrC are colr.h and colr.c.

You must include colr.h and link colr.c in with your
program.
```c
#include "colr.h"

// ..use ColrC functions/macros/etc.
```

ColrC uses a couple glibc features (`fileno` and `hsearch`), which may not be
compatible with your system. Most linux distros are compatible. The colr.h header
defines `_GNU_SOURCE` if it's not already defined (see `man feature_test_macros`).

*Be sure to include **libm** (the math library) when compiling*:
```bash
gcc -std=c11 -c myprogram.c colr.c -o myexecutable -lm
```

## Files:

Name   | Description
:----- | :---------------------------------------------
colr.h | The interface to ColrC.
colr.c | Where ColrC is implemented. This must be compiled/linked with your program.
dbug.h | Some debugging support, only enable when compiled with `-DDEBUG`.
