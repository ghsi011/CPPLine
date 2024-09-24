// main.cpp
import std;
import CPPLine;
import ErrorHandling;

using namespace error_handling;

int main(int argc, char* argv[]) {
    using namespace cppline;

    Parser parser("Demo Application");

    // Register options with aliases
    parser.add_bool(std::vector<std::string>{ "--verbose", "-v" }, "Enable verbose output");
    parser.add_int(std::vector<std::string>{ "-n", "--number" }, "Set the number", 10);
    parser.add_string("--name", "Set the name", "default");
    
    parser.add_option("--keyvalue", "Set a key-value pair",
                      [](const std::vector<std::string_view>& args) -> std::any {
                          if (args.size() < 2) {
                              throw Exception(Status::MissingArgument, Context{ Param::ErrorMessage, "Expected key and value" });
                          }
                          std::string key = std::string(args[0]);
                          std::string value = std::string(args[1]);
                          return std::make_pair(key, value);
                      },
                      2); // Two arguments after the name

    // Register positional arguments
    parser.add_string("First positional argument");
    parser.add_int("Second positional argument");

    // Collect arguments
    std::vector<std::string_view> arguments;
    for (int i = 1; i < argc; ++i) {
        arguments.push_back(argv[i]);
    }

    // Check for help option
    auto is_help = [](std::string_view arg) { return arg == "--help" || arg == "-h"; };

    if (std::any_of(arguments.begin(), arguments.end(), is_help)) {
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

    return 0;
}
