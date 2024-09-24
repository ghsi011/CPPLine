# CPPLine

![CI](https://github.com/ghsi011/CPPLine/actions/workflows/ci.yml/badge.svg)

CPPLine is a showcase project designed to demonstrate the usage of modern C++20 modules with a focus on modularity, clean separation of interface and implementation (similar to classic headers and source files).

It also includes a cool take on custom Exception classes using stacktrace and a custom Context object.

## Features

- Support for boolean, integer, string and custom type options.
- Support for positional arguments.
- Aliases for options (e.g., `--help` and `-h`).
- Customizable error handling with detailed context.

## Requirements

- C++20 compatible compiler with modules support (e.g., GCC 10+, Clang 10+, MSVC 19.28+).
- [Google Test](https://github.com/google/googletest) (for running tests).
