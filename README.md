# libutx

A cheap utmpx implementation for deficient systems like musl.

Complies with POSIX.1-2001 and POSIX.1-2008.

Compile with:

`gcc utmpx.c -o utmpx.so -shared -Wall -Wextra`
