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
                    size_t argument_count,
                    std::any default_value = {});

    // Overloads for single name and positional arguments
    void add_option(const std::string& name,
                    const std::string& help,
                    ParseFunctionType parse_function,
                    size_t argument_count,
                    std::any default_value = {});

    void add_option(const std::string& help,
                    ParseFunctionType parse_function,
                    size_t argument_count,
                    std::any default_value = {});

    // Specific methods for common types
    void add_bool(const std::vector<std::string>& names, const std::string& help);
    void add_bool(const std::string& name, const std::string& help);
    void add_bool(const std::string& help);

    void add_int(const std::vector<std::string>& names, const std::string& help, int default_value = 0);
    void add_int(const std::string& name, const std::string& help, int default_value = 0);
    void add_int(const std::string& help);

    void add_string(const std::vector<std::string>& names, const std::string& help, const std::string& default_value = "");
    void add_string(const std::string& name, const std::string& help, const std::string& default_value = "");
    void add_string(const std::string& help);

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
    std::vector<std::string_view> parse_positional(const std::vector<std::string_view>& arguments);
    void parse_non_positional(const std::vector<std::string_view>& arguments);

    static std::string join_names(const std::vector<std::string>& names);

    std::string m_description;
    std::vector<Option> m_options;
    std::unordered_map<std::string, size_t> m_option_map; // Maps option names to indices in m_options
    std::vector<Option> m_positional_options; // Pointers to positional Option objects
};

template <typename T>
T Parser::get(const std::string& name) const
{
    auto it = m_option_map.find(name);
    if (it != m_option_map.end()) {
        return std::any_cast<T>(m_options[it->second].value);
    }
    throw Exception(Status::OptionNotFound, Context{ Param::OptionName, name }
    );
}

template <typename T>
T Parser::get_positional(const size_t index) const
{
    if (index >= m_positional_options.size()) {
        throw Exception(Status::IndexOutOfRange, Context{ Param::Index, std::to_string(index) });
    }
    return std::any_cast<T>(m_positional_options[index].value);
}
} // namespace cppline
