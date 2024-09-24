# CPPLine

![CI](https://github.com/ghsi011/CPPLine/actions/workflows/ci.yml/badge.svg)

CPPLine is a project demonstrating modern C++20 modules, focusing on modularity and clean separation of interface and implementation, much like traditional headers and source files.

It features custom exception handling, using stacktraces along with other practical utilities for error management.
TODO: add Expected utilites
TODO: showcase usage of multiple enums in error handling.

Development leveraged OpenAI O1-preview and mini-modules for refactoring and design insights, as well partical things like string parsing. 
The exception handling utilities were adapted from a previous project.

## Features

- Support for boolean, integer, string and custom type options.
- Support for positional arguments.
- Aliases for options (e.g., `--help` and `-h`).
- Customizable error handling with detailed context.

## Requirements

- C++20 compatible compiler with modules support (e.g., GCC 10+, Clang 10+, MSVC 19.28+).
- [Google Test](https://github.com/google/googletest) (for running tests).
