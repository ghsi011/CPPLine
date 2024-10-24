export module CPPLine;

import std;
export import ErrorHandling;

using namespace cppline::errors;

namespace cppline {

export using ParseFunctionType = std::function<Expected<std::any>(const std::vector<std::string_view>&)>;

struct Option {
    std::vector<std::string> names; // Empty names indicate a positional argument
    std::string help;
    size_t argument_count; // Number of arguments after the option name
    ParseFunctionType parse_function;
    std::any value;
    bool is_set = false; // Indicates if the option was set
};

export using Aliases = std::vector<std::string>;

export class Parser {
public:
    explicit Parser(const std::string& description);

    // General method to add an option
    ExpectedVoid try_add_option(const Aliases& names,
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
    ExpectedVoid try_add_bool(const Aliases& names, const std::string& help);
    ExpectedVoid try_add_bool(const std::string& name, const std::string& help);
    ExpectedVoid try_add_bool(const std::string& help);

    ExpectedVoid try_add_int(const Aliases& names, const std::string& help, int default_value = 0);
    ExpectedVoid try_add_int(const std::string& name, const std::string& help, int default_value = 0);
    ExpectedVoid try_add_int(const std::string& help);

    ExpectedVoid try_add_string(const Aliases& names, const std::string& help, const std::string& default_value = "");
    ExpectedVoid try_add_string(const std::string& name, const std::string& help, const std::string& default_value = "");
    ExpectedVoid try_add_string(const std::string& help);

    ExpectedVoid try_parse(const std::vector<std::string_view>& arguments);

    template <typename T>
    Expected<T> try_get(const std::string& name) const;

    template <typename T>
    Expected<T> try_get_positional(size_t index) const;

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
    Expected<std::vector<std::string_view>> parse_positional(const std::vector<std::string_view>& arguments);
    ExpectedVoid parse_non_positional(const std::vector<std::string_view>& arguments);

    static std::string join_names(const Aliases& names);

    static Expected<std::any> parse_bool(const std::vector<std::string_view>& args);
    static ParseFunctionType parse_int_factory(const Aliases& names);
    static ParseFunctionType parse_string_factory(const Aliases& names);

    std::string m_description;
    std::vector<Option> m_options;
    std::unordered_map<std::string, size_t> m_option_map; // Maps option names to indices in m_options
    std::vector<Option> m_positional_options;
};

template <typename... Args>
void Parser::add_option(Args&&... args)
{
    auto result = try_add_option(std::forward<Args>(args)...);
    throw_on_error(result);
}

template <typename... Args>
void Parser::add_bool(Args&&... args)
{
    auto result = try_add_bool(std::forward<Args>(args)...);
    throw_on_error(result);
}

template <typename... Args>
void Parser::add_int(Args&&... args)
{
    auto result = try_add_int(std::forward<Args>(args)...);
    throw_on_error(result);
}

template <typename... Args>
void Parser::add_string(Args&&... args)
{
    auto result = try_add_string(std::forward<Args>(args)...);
    throw_on_error(result);
}

template <typename T>
Expected<T> Parser::try_get(const std::string& name) const
{
    if (m_option_map.contains(name)) {
        const auto& option = m_options.at(m_option_map.at(name));
        if (!option.value.has_value()) {
            return make_unexpected(Status::OptionNotSet, Context{ Param::OptionName, name });
        }
        try {
            return std::any_cast<T>(option.value);
        }
        catch (const std::bad_any_cast&) {
            return make_unexpected(Status::InvalidValue, Context{ Param::OptionName, name });
        }
    }
    return make_unexpected(Status::OptionNotFound, Context{ Param::OptionName, name });
}

template <typename T>
T Parser::get(const std::string& name) const
{
    auto result = try_get<T>(name);
    throw_on_error(result);
    return result.value();
}

template <typename T>
Expected<T> Parser::try_get_positional(const size_t index) const
{
    if (index >= m_positional_options.size()) {
        return make_unexpected(Status::IndexOutOfRange, Context{ Param::Index, std::to_string(index) });
    }
    const auto& option = m_positional_options[index];
    if (!option.value.has_value()) {
        return make_unexpected(Status::OptionNotSet, Context{ Param::Index, std::to_string(index) });
    }
    try {
        return std::any_cast<T>(option.value);
    }
    catch (const std::bad_any_cast&) {
        return make_unexpected(Status::InvalidValue, Context{ Param::Index, std::to_string(index) });
    }
}

template <typename T>
T Parser::get_positional(const size_t index) const
{
    auto result = try_get_positional<T>(index);
    throw_on_error(result);
    return result.value();
}

} // namespace cppline
