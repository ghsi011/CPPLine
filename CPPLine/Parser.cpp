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
                        size_t argument_count) {
    Option option{ names, help, argument_count, parse_function, std::any{}, false };
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
                        size_t argument_count) {
    add_option(std::vector<std::string>{ name }, help, parse_function, argument_count);
}

// Overload for positional argument (no names)
void Parser::add_option(const std::string& help,
                        ParseFunctionType parse_function,
                        size_t argument_count) {
    Option option{ {}, help, argument_count, parse_function, std::any{}, false };
    m_options.push_back(option);

    m_positional_options.push_back(&m_options.back());
}

// Implement `add_bool` overloads
void Parser::add_bool(const std::vector<std::string>& names, const std::string& help) {
    add_option(names, help,
               [](const std::vector<std::string_view>&) -> std::any {
                   return true; // Presence implies true
               },
               0); // No arguments after the option name
    // Set default value for all names
    for (const auto& name : names) {
        m_values[name] = false; // Default is false
    }
}

void Parser::add_bool(const std::string& name, const std::string& help) {
    add_bool(std::vector{ name }, help);
}

void Parser::add_bool(const std::string& help) {
    add_option(help,
               [](const std::vector<std::string_view>&) -> std::any {
                   return true; // Presence implies true
               },
               0); // No arguments after the positional argument
    // For positional arguments, default value is false
    m_positional_values.push_back(false);
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
               1); // One argument after the name
    // Set default value for all names
    for (const auto& name : names) {
        m_values[name] = default_value;
    }
}

void Parser::add_int(const std::string& name, const std::string& help, int default_value) {
    add_int(std::vector{ name }, help, default_value);
}

void Parser::add_int(const std::string& help, int default_value) {
    add_option(help,
               [](const std::vector<std::string_view>& args) -> std::any {
                   if (args.empty()) {
                       throw Exception(Status::MissingArgument, Context{ Param::ErrorMessage, "Expected a value" });
                   }
                   return std::stoi(std::string(args[0]));
               },
               1); // One argument after the positional argument
    // For positional arguments, store default value
    m_positional_values.push_back(default_value);
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
               1); // One argument after the name
    // Set default value for all names
    for (const auto& name : names) {
        m_values[name] = default_value;
    }
}

void Parser::add_string(const std::string& name, const std::string& help, const std::string& default_value) {
    add_string(std::vector{ name }, help, default_value);
}

void Parser::add_string(const std::string& help, const std::string& default_value) {
    add_option(help,
               [](const std::vector<std::string_view>& args) -> std::any {
                   if (args.empty()) {
                       throw Exception(Status::MissingArgument);
                   }
                   return std::string(args[0]);
               },
               1); // One argument after the positional argument
    // For positional arguments, store default value
    m_positional_values.push_back(default_value);
}

void Parser::parse(const std::vector<std::string_view>& arguments) {
    using namespace std::ranges;

    auto args = arguments | views::common;
    auto it = args.begin();
    auto end = args.end();

    // First process positional arguments
    m_positional_index = 0;
    while (it != end && m_positional_index < m_positional_options.size()) {
        Option& option = *m_positional_options[m_positional_index];
        const size_t args_to_consume = option.argument_count;

        if (std::distance(it, end) < static_cast<std::ptrdiff_t>(args_to_consume)) {
            throw Exception(Status::NotEnoughArguments,
                            Context{ Param::ErrorMessage, "Not enough arguments for positional argument" });
        }

        // Collect arguments for the parse function
        std::vector<std::string_view> args_view;
        if (args_to_consume > 0) {
            auto args_end = std::next(it, args_to_consume);
            args_view.assign(it, args_end);
            it = args_end;
        }

        // Parse and store the value
        try {
            std::any value = option.parse_function(args_view);
            option.value = value;
            option.is_set = true;

            // Store the value
            if (m_positional_index < m_positional_values.size()) {
                m_positional_values[m_positional_index] = value;
            }
            else {
                m_positional_values.push_back(value);
            }
        }
        catch (const std::exception& e) {
            throw Exception(Status::ParsingError, Context{ Param::ErrorMessage, e.what() });
        }

        ++m_positional_index;
    }

    // Now process named options
    while (it != end) {
        std::string_view arg = *it;

        // Check if the argument matches any option name
        auto opt_map_it = m_option_map.find(std::string(arg));
        if (opt_map_it != m_option_map.end()) {
            // Option found
            Option& option = m_options[opt_map_it->second];

            size_t args_to_consume = option.argument_count;
            if (std::distance(it, end) - 1 < static_cast<std::ptrdiff_t>(args_to_consume)) {
                throw Exception(Status::NotEnoughArguments,
                                Context{ Param::OptionName, join_names(option.names) }
                                << Context{ Param::ExpectedArgumentCount, std::to_string(option.argument_count) }
                << Context{ Param::ReceivedArgumentCount, std::to_string(std::distance(it, end) - 1) });
            }

            // Collect arguments for the parse function
            std::vector<std::string_view> args_view;
            if (args_to_consume > 0) {
                auto args_start = std::next(it);
                auto args_end = std::next(it, args_to_consume + 1); // +1 to include the option name
                args_view.assign(args_start, args_end);
                it = args_end;
            }
            else {
                ++it;
            }

            // Parse and store the value
            try {
                std::any value = option.parse_function(args_view);
                option.value = value;
                option.is_set = true;

                // Store the value for each alias
                for (const auto& name : option.names) {
                    m_values[name] = value;
                }
            }
            catch (const std::exception& e) {
                throw Exception(Status::ParsingError, Context{ Param::OptionName, join_names(option.names) } << Context{
                                                                  Param::ErrorMessage, e.what() });
            }
        }
        else {
            // Unknown option or positional argument after named options
            throw Exception(Status::UnknownOption, Context{ Param::OptionName, std::string(arg) });
        }
    }

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
