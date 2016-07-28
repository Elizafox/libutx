# libutx

A cheap utmpx implementation for systems like musl.

## Discussion

The author of musl doesn't believe in having utmpx and thus stubbed it out. In
Adelie Linux, we are aiming for XSI compliance, and working utmpx is required.

## Standards conformance

Complies with POSIX.1-2001, POSIX.1-2008, and XSI

## Compilation

Compile with:

`gcc utmpx.c -o utmpx.so -shared -fPIC -Wall -Wextra`
