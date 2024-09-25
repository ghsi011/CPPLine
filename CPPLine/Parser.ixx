// Parser.ixx
export module CPPLine;

import std;
export import ErrorHandling; // Import the error handling module

using namespace cppline::errors;

namespace cppline {

export using ParseFunctionType = std::function<std::any(const std::vector<std::string_view>&)>;

struct Option {
    std::vector<std::string> names; // Empty names indicate a positional argument
    std::string help;
    size_t argument_count; // Number of arguments after the option name
    ParseFunctionType parse_function;
    std::any value;
    bool is_set = false; // Indicates if the option was set
};

export class Parser {
public:
    explicit Parser(const std::string& description);

    // General method to add an option
    void add_option(const std::vector<std::string>& names,
                    const std::string& help,
                    ParseFunctionType parse_function,
                    size_t argument_count);

    // Overloads for single name and positional arguments
    void add_option(const std::string& name,
                    const std::string& help,
                    ParseFunctionType parse_function,
                    size_t argument_count);

    void add_option(const std::string& help,
                    ParseFunctionType parse_function,
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
    T get(const std::string& name) const;

    // Retrieve positional argument by index
    template <typename T>
    T get_positional(size_t index) const;

    // Print help message
    void print_help() const;

private:
    static std::string join_names(const std::vector<std::string>& names);

    std::string m_description;
    std::vector<Option> m_options;
    std::unordered_map<std::string, size_t> m_option_map; // Maps option names to indices in options_
    std::unordered_map<std::string, std::any> m_values;
    std::vector<std::any> m_positional_values; // Stores parsed positional argument values
    std::vector<Option*> m_positional_options; // Pointers to positional Option objects
    size_t m_positional_index = 0; // Index to keep track of which positional argument we're processing
};

template <typename T>
T Parser::get(const std::string& name) const
{
    auto it = m_values.find(name);
    if (it != m_values.end()) {
        return std::any_cast<T>(it->second);
    }
    throw Exception(Status::OptionNotFound, Context{ Param::OptionName, name }
    );
}

template <typename T>
T Parser::get_positional(const size_t index) const
{
    if (index >= m_positional_values.size()) {
        throw Exception(Status::IndexOutOfRange, Context{ Param::Index, std::to_string(index) });
    }
    return std::any_cast<T>(m_positional_values[index]);
}
} // namespace cppline
