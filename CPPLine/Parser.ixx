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
    ExpectedVoid try_add_option(const std::vector<std::string>& names,
                                const std::string& help,
                                ParseFunctionType parse_function,
                                size_t argument_count,
                                std::any default_value = {});

    ExpectedVoid try_add_option(const std::string& name,
                                const std::string& help,
                                ParseFunctionType parse_function,
                                size_t argument_count,
                                std::any default_value = {});

    ExpectedVoid try_add_option(const std::string& help,
                                ParseFunctionType parse_function,
                                size_t argument_count,
                                std::any default_value = {});

    // Specific methods for common types
    ExpectedVoid try_add_bool(const std::vector<std::string>& names, const std::string& help);
    ExpectedVoid try_add_bool(const std::string& name, const std::string& help);
    ExpectedVoid try_add_bool(const std::string& help);

    ExpectedVoid try_add_int(const std::vector<std::string>& names, const std::string& help, int default_value = 0);
    ExpectedVoid try_add_int(const std::string& name, const std::string& help, int default_value = 0);
    ExpectedVoid try_add_int(const std::string& help);

    ExpectedVoid try_add_string(const std::vector<std::string>& names, const std::string& help, const std::string& default_value = "");
    ExpectedVoid try_add_string(const std::string& name, const std::string& help, const std::string& default_value = "");
    ExpectedVoid try_add_string(const std::string& help);

    template <typename... Args>
    void add_option(Args&&... args);

    template <typename... Args>
    void add_bool(Args&&... args);

    template <typename... Args>
    void add_int(Args&&... args);

    template <typename... Args>
    void add_string(Args&&... args);

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

    static std::any parse_bool(const std::vector<std::string_view>& args);
    static ParseFunctionType parse_int_factory(const std::vector<std::string>& names);
    static ParseFunctionType parse_string_factory(const std::vector<std::string>& names);

    std::string m_description;
    std::vector<Option> m_options;
    std::unordered_map<std::string, size_t> m_option_map; // Maps option names to indices in m_options
    std::vector<Option> m_positional_options; // Pointers to positional Option objects
};

template <typename ... Args>
void Parser::add_option(Args&&... args)
{
    // Forward the arguments to try_add_option
    auto result = try_add_option(std::forward<Args>(args)...);
    throw_on_error(result);
}

template <typename ... Args>
void Parser::add_bool(Args&&... args)
{
    auto result = try_add_bool(std::forward<Args>(args)...);
    throw_on_error(result);
}

template <typename ... Args>
void Parser::add_int(Args&&... args)
{
    auto result = try_add_int(std::forward<Args>(args)...);
    throw_on_error(result);
}

template <typename ... Args>
void Parser::add_string(Args&&... args)
{
    auto result = try_add_string(std::forward<Args>(args)...);
    throw_on_error(result);
}

template <typename T>
T Parser::get(const std::string& name) const
{
    if (m_option_map.contains(name)) {
        return std::any_cast<T>(m_options[m_option_map.at(name)].value);
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
