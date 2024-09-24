// Parser.ixx
export module CPPLine;

import std;
import ErrorHandling; // Import the error handling module

using namespace error_handling;

namespace cppline {

struct Option {
    std::vector<std::string> names; // Empty names indicate a positional argument
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

    // Overloads for single name and positional arguments
    void add_option(const std::string& name,
                    const std::string& help,
                    std::function<std::any(const std::vector<std::string_view>&)> parse_function,
                    size_t argument_count);

    void add_option(const std::string& help,
                    std::function<std::any(const std::vector<std::string_view>&)> parse_function,
                    size_t argument_count);

    // Specific methods for common types
    void add_bool(const std::vector<std::string>& names, const std::string& help);
    void add_bool(const std::string& name, const std::string& help);
    void add_bool(const std::string& help);

    void add_int(const std::vector<std::string>& names, const std::string& help, int default_value = 0);
    void add_int(const std::string& name, const std::string& help, int default_value = 0);
    void add_int(const std::string& help, int default_value = 0);

    void add_string(const std::vector<std::string>& names, const std::string& help, const std::string& default_value = "");
    void add_string(const std::string& name, const std::string& help, const std::string& default_value = "");
    void add_string(const std::string& help, const std::string& default_value = "");

    // Parse the command-line arguments
    void parse(const std::vector<std::string_view>& arguments);

    // Retrieve the parsed value
    template <typename T>
    T get(const std::string& name) const {
        auto it = values_.find(name);
        if (it != values_.end()) {
            return std::any_cast<T>(it->second);
        }
        throw Exception(Status::InvalidValue, Context{ Param::ErrorMessage, "Option not found" } << Context{ Param::OptionName, name }
        );
    }

    // Retrieve positional argument by index
    template <typename T>
    T get_positional(size_t index) const {
        if (index >= positional_values_.size()) {
            throw Exception(Status::InvalidValue, Context{ Param::ErrorMessage, "Positional argument index out of range" });
        }
        return std::any_cast<T>(positional_values_[index]);
    }

    // Access positional arguments
    const std::vector<std::any>& positional_arguments() const;

    // Print help message
    void print_help() const;

private:
    static std::string join_names(const std::vector<std::string>& names);

    std::string description_;
    std::vector<Option> options_;
    std::unordered_map<std::string, size_t> option_map_; // Maps option names to indices in options_
    std::unordered_map<std::string, std::any> values_;
    std::vector<std::any> positional_values_; // Stores parsed positional argument values
    std::vector<Option*> positional_options_; // Pointers to positional Option objects
    size_t positional_index_ = 0; // Index to keep track of which positional argument we're processing
};

} // namespace cppline
