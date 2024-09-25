# CPPLine

![CI](https://github.com/ghsi011/CPPLine/actions/workflows/ci.yml/badge.svg)

CPPLine is an example project demonstrating development with C++20 modules as well as a modern multi-paradigm Error Handling approach. All through a simple Command Line parsing library.

It showcases using multiple modules and module partitions with a clean interface and implementation separation, much like traditional headers and source files.

It features a custom mix of std::expected and exceptions for error handling - allowing the user to choose which to use and when.
C++23 stacktraces are used along with other practical utilities for error management.

Development leveraged OpenAI O1-preview and mini-modules for refactoring and design insights, as well as practical things like string parsing. 

## Features

- Support for boolean, integer, string, and custom type options.
- Support for positional arguments.
- Aliases for options (e.g., `--path` and `-p`).
- Customizable error handling with detailed context.

Example usage:
```cpp
Parser parser("Demo Application");

// Register positional arguments
parser.add_string("First positional argument");
parser.add_int("Second positional argument");

// Simple optional argument.
parser.add_string("--name", "Set the name", "default");

// Register option with aliases
parser.add_bool(Aliases{ "--verbose", "-v" }, "Enable verbose output");

// Custom parser for space delimited key value pairs, no default value
parser.add_option("--keyvalue", "Set a key-value pair",
                  [](const std::vector<std::string_view>& args) -> std::any {
                      if (args.size() < 2) {
                          throw Exception(Status::MissingArgument, Context{} << Message::ExpectedKeyAndValue); // Note logging of enum value.
                      }
                      std::string key = std::string(args[0]);
                      std::string value = std::string(args[1]);
                      return std::make_pair(key, value);
                  },
                  2); // Two arguments after the name

// Try register option and handle error cases manually
ExpectedVoid add_result = parser.try_add_int(Aliases{ "-n", "--number" }, "Set the number", 10);
if (!add_result.has_value()) {
    if (add_result.error()->get_error() == Status::OptionAlreadyDefined) {
        // We log and ignore here - A somewhat forced example of applying different logic based on the error condition.
        Logger::log("Option already defined", add_result.error()->get_context());
    }
    else
    {
        Logger::log("Error adding option", add_result.error());
        add_result.error()->throw_self(); // don't ignore other errors.
    }
}
```
```cpp
bool verbose = parser.get<bool>("--verbose");
int number = parser.get<int>("-n");
std::string name = parser.get<std::string>("--name");
auto key_value = parser.get<std::pair<std::string, std::string>>("--keyvalue");

std::string first_pos_arg = parser.get_positional<std::string>(0);
int second_pos_arg = parser.get_positional<int>(1);
```
You can look at the Example project or the tests for more complete usage examples.

## Requirements

- C++20 compatible compiler with modules support (e.g., GCC 10+, Clang 10+, MSVC 19.28+).
- [Google Test](https://github.com/google/googletest) (for running tests).

## TODO:
- Finish introducing Expected overloads for Parser - allow a completely exception-free usage.
- Showcase usage of multiple enums in error handling - currently the infrastructure is there but it's not used much.
- Polish - This code wasn't reviewed and still needs some refactoring. 

## Future Development:
- Cross-Platfrom support - use CMAKE and GCC\clang to compile for Linux.
- Release Package - How best to distribute a Moduels-based library?
- Command-line parsing features: support subcommands, explicitly mark required arguments, better help, etc.

Any contributions are welcome :)

[MIT License](LICENSE.txt)
