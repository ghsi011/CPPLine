// main.cpp
import std;
import CPPLine;

int main(int argc, char* argv[]) {
    using namespace cppline;

    Parser parser("Demo Application");

    // Register options with aliases
    parser.add_bool({ "--verbose", "-v" }, "Enable verbose output");
    parser.add_int({ "-n", "--number" }, "Set the number", 10);
    parser.add_string({ "--name" }, "Set the name", "default");
    parser.add_option({ "--keyvalue" }, "Set a key-value pair",
                      [](const std::vector<std::string_view>& args) -> std::any {
                          if (args.empty()) {
                              throw std::runtime_error("Expected key:value pair");
                          }
                          std::string arg = std::string(args[0]);
                          auto pos = arg.find(':');
                          if (pos == std::string::npos) {
                              throw std::runtime_error("Invalid format, expected key:value");
                          }
                          std::string key = arg.substr(0, pos);
                          std::string value{ args[1] };
                          return std::make_pair(key, value);
                      },
                      2);

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
    catch (const std::exception& ex) {
        std::cerr << "Parsing error: " << ex.what() << "\n";
        parser.print_help();
        return 1;
    }

    // Retrieve values
    try {
        bool verbose = parser.get<bool>("--verbose");
        int number = parser.get<int>("-n");
        std::string name = parser.get<std::string>("--name");
        auto key_value = parser.get<std::pair<std::string, std::string>>("--keyvalue");

        std::cout << "Verbose: " << std::boolalpha << verbose << "\n";
        std::cout << "Number: " << number << "\n";
        std::cout << "Name: " << name << "\n";
        std::cout << "Key: " << key_value.first << ", Value: " << key_value.second << "\n";
    }
    catch (const std::exception& ex) {
        std::cerr << "Retrieval error: " << ex.what() << "\n";
        return 1;
    }

    // Access positional arguments
    const auto& positional_args = parser.positional_arguments();
    if (!positional_args.empty()) {
        std::cout << "Positional Arguments:\n";
        for (const auto& arg : positional_args) {
            std::cout << "  " << arg << "\n";
        }
    }

    return 0;
}
