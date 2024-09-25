module CPPLine;

import std;
import ErrorHandling;

using namespace cppline::errors;

namespace cppline {

Parser::Parser(const std::string& description)
    : m_description(description) {}

// General method to add an option with multiple names
void Parser::add_option(const std::vector<std::string>& names,
                        const std::string& help,
                        ParseFunctionType parse_function,
                        size_t argument_count,
                        std::any default_value) {
    if (std::ranges::any_of(names, [this](const std::string& name) { return m_option_map.contains(name); })) {
        throw Exception(Status::OptionAlreadyDefined, Context{ Param::OptionName, join_names(names) });
    }

    Option option{ names, help, argument_count, parse_function, default_value, false };
    m_options.push_back(option);

    size_t index = m_options.size() - 1;
    for (const auto& name : names) {
        m_option_map[name] = index;
    }
}

// Overload for single name
void Parser::add_option(const std::string& name,
                        const std::string& help,
                        ParseFunctionType parse_function,
                        size_t argument_count,
                        std::any default_value) {
    add_option(std::vector<std::string>{ name }, help, parse_function, argument_count, default_value);
}

// Overload for positional argument (no names)
void Parser::add_option(const std::string& help,
                        ParseFunctionType parse_function,
                        size_t argument_count,
                        std::any default_value) {
    Option option{ {}, help, argument_count, parse_function, default_value, false };
    m_options.push_back(option);

    m_positional_options.push_back(m_options.back());
}

// Implement `add_bool` overloads
void Parser::add_bool(const std::vector<std::string>& names, const std::string& help) {
    add_option(names, help,
               [](const std::vector<std::string_view>&) -> std::any {
                   return true; // Presence implies true
               },
               0,
               false); // No arguments after the option name
}

void Parser::add_bool(const std::string& name, const std::string& help) {
    add_bool(std::vector{ name }, help);
}

void Parser::add_bool(const std::string& help) {
    add_option(help,
               [](const std::vector<std::string_view>&) -> std::any {
                   return true; // Presence implies true
               },
               0, false); // No arguments after the positional argument
}

// Similar for `add_int` and `add_string`

void Parser::add_int(const std::vector<std::string>& names, const std::string& help, int default_value) {
    add_option(names, help,
               [names](const std::vector<std::string_view>& args) -> std::any {
                   if (args.empty()) {
                       throw Exception(Status::MissingArgument, Context{ Param::OptionName, join_names(names) });
                   }
                   return std::stoi(std::string(args[0]));
               },
               1,
               default_value); // One argument after the name
}

void Parser::add_int(const std::string& name, const std::string& help, int default_value) {
    add_int(std::vector{ name }, help, default_value);
}

void Parser::add_int(const std::string& help) {
    add_option(help,
               [](const std::vector<std::string_view>& args) -> std::any {
                   if (args.empty()) {
                       throw Exception(Status::MissingArgument, Context{ Param::ErrorMessage, "Expected a value" });
                   }
                   return std::stoi(std::string(args[0]));
               },
               1); // One argument after the positional argument
}

// Similarly for `add_string`

void Parser::add_string(const std::vector<std::string>& names, const std::string& help, const std::string& default_value) {
    add_option(names, help,
               [names](const std::vector<std::string_view>& args) -> std::any {
                   if (args.empty()) {
                       throw Exception(Status::MissingArgument, Context{ Param::OptionName, join_names(names) });
                   }
                   return std::string(args[0]);
               },
               1, default_value); // One argument after the name
}

void Parser::add_string(const std::string& name, const std::string& help, const std::string& default_value) {
    add_string(std::vector{ name }, help, default_value);
}

void Parser::add_string(const std::string& help) {
    add_option(help,
               [](const std::vector<std::string_view>& args) -> std::any {
                   if (args.empty()) {
                       throw Exception(Status::MissingArgument);
                   }
                   return std::string(args[0]);
               },
               1); // One argument after the positional argument
}

void Parser::parse(const std::vector<std::string_view>& arguments) {
    auto non_positional_arguments = parse_positional(arguments);

    parse_non_positional(non_positional_arguments);
}

void Parser::print_help() const {
    std::cout << m_description << "\n\n";
    std::cout << "Options:\n";

    for (const auto& option : m_options) {
        std::string names_str;
        if (!option.names.empty()) {
            names_str = std::accumulate(
                option.names.begin(), option.names.end(), std::string{},
                [](const std::string& a, const std::string& b) {
                    return a.empty() ? b : a + ", " + b;
                }
            );
        }
        else {
            names_str = "<positional>";
        }
        std::cout << "  " << names_str << "\t" << option.help << "\n";
    }
}

std::vector<std::string_view> Parser::parse_positional(const std::vector<std::string_view>& arguments)
{
    std::vector<std::string_view> remaining_args = arguments;

    for (const auto& [positional_index, option] : std::views::enumerate(m_positional_options))
    {
        const size_t args_to_consume = option.argument_count;

        if (remaining_args.size() < static_cast<std::ptrdiff_t>(args_to_consume)) {
            const auto context = Context{ Param::ExpectedArgumentCount, std::to_string(args_to_consume) } <<
                Context{ Param::ReceivedArgumentCount, std::to_string(remaining_args.size()) };
            throw Exception(Status::NotEnoughArguments, context);
        }

        // Collect arguments for the parse function
        auto args_view = remaining_args | std::views::take(args_to_consume) | std::ranges::to<std::vector<std::string_view>>();
        remaining_args = remaining_args | std::views::drop(args_to_consume) | std::ranges::to<std::vector<std::string_view>>();
        option.value = option.parse_function(args_view);
        option.is_set = true;
    }

    return remaining_args;
}

void Parser::parse_non_positional(const std::vector<std::string_view>& arguments)
{
    std::vector<std::string_view> remaining_args = arguments;

    while (!remaining_args.empty())
    {
        const std::string argument_name = std::string(remaining_args.front());

        if (!m_option_map.contains(argument_name)) {
            throw Exception(Status::OptionNotFound, Context{ Param::OptionName, argument_name });
        }

        auto& option = m_options[m_option_map[argument_name]];
        if (option.is_set) {
            throw Exception(Status::OptionAlreadySet, Context{ Param::OptionName, argument_name });
        }

        const size_t args_to_consume = option.argument_count;

        if (remaining_args.size() < args_to_consume) {
            const auto context = Context{ Param::ExpectedArgumentCount, std::to_string(args_to_consume) } <<
                Context{ Param::ReceivedArgumentCount, std::to_string(remaining_args.size()) };
            throw Exception(Status::NotEnoughArguments, context);
        }

        // Collect arguments for the parse function
        auto args_view = remaining_args | std::views::drop(1) | std::views::take(args_to_consume) | std::ranges::to<std::vector<std::string_view>>();
        remaining_args = remaining_args | std::views::drop(args_to_consume + 1) | std::ranges::to<std::vector<std::string_view>>();
        option.value = option.parse_function(args_view);
        option.is_set = true;
    }
}

std::string Parser::join_names(const std::vector<std::string>& names) {
    if (names.size() == 1) {
        return names[0];
    }

    return std::accumulate(
        names.begin(), names.end(), std::string{},
        [](const std::string& a, const std::string& b) {
            return a.empty() ? b : a + ", " + b;
        }
    );
}

} // namespace cppline
