import std;
import cppline;

constexpr cppline::Parser create_parser(const std::string& description) {
    cppline::Parser parser(description);
    parser.add_option<int>("-n", "Number", 10);
    return parser;
}

int main(int argc, char* argv[])
{
    cppline::Parser parser = create_parser("This is a test parser");

    std::vector<std::string> arguments;
    for (int i = 1; i < argc; ++i) {
        arguments.emplace_back(argv[i]);
    }

    parser.parse(arguments);
    std::print("argument: {}", parser.get<int>("-n"));
    return 0;
}
