// Parser.ixx
export module CPPLine;

import std;

namespace cppline {

struct Option {
    std::vector<std::string> names; // Supports aliases
    std::string help;
    size_t argument_count; // Number of arguments after the option name
    std::function<std::any(const std::vector<std::string_view>&)> parse_function;
    std::any value;
    bool is_set = false; // Indicates if the option was set
};

export class Parser {
public:
    Parser(const std::string& description);

    // General method to add an option
    void add_option(const std::vector<std::string>& names,
                    const std::string& help,
                    std::function<std::any(const std::vector<std::string_view>&)> parse_function,
                    size_t argument_count);

    // Specific methods for common types
    void add_bool(const std::vector<std::string>& names, const std::string& help);
    void add_int(const std::vector<std::string>& names, const std::string& help, int default_value = 0);
    void add_string(const std::vector<std::string>& names, const std::string& help, const std::string& default_value = "");

    // Parse the command-line arguments
    void parse(const std::vector<std::string_view>& arguments);

    // Retrieve the parsed value
    template <typename T>
    T get(const std::string& name) const {
        auto it = values_.find(name);
        if (it != values_.end()) {
            return std::any_cast<T>(it->second);
        }
        throw std::runtime_error("Option not found: " + name);
    }

    // Access positional arguments
    const std::vector<std::string_view>& positional_arguments() const;

    // Print help message
    void print_help() const;

private:
    std::string description_;
    std::vector<Option> options_;
    std::unordered_map<std::string, size_t> option_map_; // Maps option names to indices in options_
    std::unordered_map<std::string, std::any> values_;
    std::vector<std::string_view> positional_args_;
};

}
