// main.cpp
import std;
import CPPLine;
import ErrorHandling;

using namespace cppline;
using namespace cppline::errors;

int main(int argc, char* argv[])
{
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
        if (add_result.error().get_error() == Status::OptionAlreadyDefined) {
            // We log and ignore here - A somewhat forced example of applying different logic based on the error condition.
            Logger::log("Option already defined", add_result.error().get_context());
        }
        else
        {
            Logger::log("Error adding option", add_result.error());
            add_result.error().throw_self(); // don't ignore other errors.
        }
    }


    // Collect arguments
    std::vector<std::string_view> arguments;
    for (int i = 1; i < argc; ++i) {
        arguments.push_back(argv[i]);
    }

    // Check for help option
    auto is_help = [](std::string_view arg) { return arg == "--help" || arg == "-h"; };

    if (std::ranges::any_of(arguments, is_help)) {
        parser.print_help();
        return 0;
    }

    // Parse arguments
    try {
        parser.parse(arguments);
    }
    catch (const Exception& ex) {
        Logger::log("Parsing error", ex);
        parser.print_help();
        return 1;
    }

    // Retrieve values
    try {
        bool verbose = parser.get<bool>("--verbose");
        int number = parser.get<int>("-n");
        std::string name = parser.get<std::string>("--name");
        auto key_value = parser.get<std::pair<std::string, std::string>>("--keyvalue");

        std::string first_pos_arg = parser.get_positional<std::string>(0);
        int second_pos_arg = parser.get_positional<int>(1);

        std::cout << "Verbose: " << std::boolalpha << verbose << "\n";
        std::cout << "Number: " << number << "\n";
        std::cout << "Name: " << name << "\n";
        std::cout << "Key: " << key_value.first << ", Value: " << key_value.second << "\n";
        std::cout << "First Positional Argument: " << first_pos_arg << "\n";
        std::cout << "Second Positional Argument: " << second_pos_arg << "\n";
    }
    catch (const Exception& ex) {
        Logger::log("Retrieval error", ex);
        return 1;
    }
    catch (const std::exception& ex) {
        Logger::log(ex.what());
        return 1;
    }
    catch (...) {
        Logger::log("Unknown error occurred");
        return 1;
    }

    return 0;
}
