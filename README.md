# CPPLine

![CI](https://github.com/ghsi011/CPPLine/actions/workflows/ci.yml/badge.svg)

CPPLine is a project demonstrating modern C++20 modules and ErrorHandling through a simple Command Line parsing library

It showcases using multiple modules and module partitions with a clean separation of interface and implementation, much like traditional headers and source files.

It features custom exception handling, using C++23 stacktraces along with other practical utilities for error management.
TODO: add Expected utilities
TODO: showcase usage of multiple enums in error handling.

Development leveraged OpenAI O1-preview and mini-modules for refactoring and design insights, as well as practical things like string parsing. 

## Features

- Support for boolean, integer, string, and custom type options.
- Support for positional arguments.
- Aliases for options (e.g., `--help` and `-h`).
- Customizable error handling with detailed context.

## Requirements

- C++20 compatible compiler with modules support (e.g., GCC 10+, Clang 10+, MSVC 19.28+).
- [Google Test](https://github.com/google/googletest) (for running tests).
