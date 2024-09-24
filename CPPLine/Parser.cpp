// Parser.cppm
module CPPLine;

import std;
import ErrorHandling; // Import the error handling module

using namespace error_handling;

namespace cppline {

Parser::Parser(const std::string& description)
    : description_(description) {}

// General method to add an option with multiple names
void Parser::add_option(const std::vector<std::string>& names,
                        const std::string& help,
                        std::function<std::any(const std::vector<std::string_view>&)> parse_function,
                        size_t argument_count) {
    Option option{ names, help, argument_count, parse_function, std::any{}, false };
    options_.push_back(option);

    size_t index = options_.size() - 1;
    for (const auto& name : names) {
        option_map_[name] = index;
    }
}

// Overload for single name
void Parser::add_option(const std::string& name,
                        const std::string& help,
                        std::function<std::any(const std::vector<std::string_view>&)> parse_function,
                        size_t argument_count) {
    add_option(std::vector<std::string>{ name }, help, parse_function, argument_count);
}

// Overload for positional argument (no names)
void Parser::add_option(const std::string& help,
                        std::function<std::any(const std::vector<std::string_view>&)> parse_function,
                        size_t argument_count) {
    Option option{ {}, help, argument_count, parse_function, std::any{}, false };
    options_.push_back(option);

    positional_options_.push_back(&options_.back());
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
        values_[name] = false; // Default is false
    }
}

void Parser::add_bool(const std::string& name, const std::string& help) {
    add_bool(std::vector<std::string>{ name }, help);
}

void Parser::add_bool(const std::string& help) {
    add_option(help,
               [](const std::vector<std::string_view>&) -> std::any {
                   return true; // Presence implies true
               },
               0); // No arguments after the positional argument
    // For positional arguments, default value is false
    positional_values_.push_back(false);
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
        values_[name] = default_value;
    }
}

void Parser::add_int(const std::string& name, const std::string& help, int default_value) {
    add_int(std::vector<std::string>{ name }, help, default_value);
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
    positional_values_.push_back(default_value);
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
        values_[name] = default_value;
    }
}

void Parser::add_string(const std::string& name, const std::string& help, const std::string& default_value) {
    add_string(std::vector<std::string>{ name }, help, default_value);
}

void Parser::add_string(const std::string& help, const std::string& default_value) {
    add_option(help,
               [](const std::vector<std::string_view>& args) -> std::any {
                   if (args.empty()) {
                       throw Exception(Status::MissingArgument, Context{ Param::ErrorMessage, "Expected a value" });
                   }
                   return std::string(args[0]);
               },
               1); // One argument after the positional argument
    // For positional arguments, store default value
    positional_values_.push_back(default_value);
}

void Parser::parse(const std::vector<std::string_view>& arguments) {
    using namespace std::ranges;

    auto args = arguments | views::common;
    auto it = args.begin();
    auto end = args.end();

    // First process positional arguments
    positional_index_ = 0;
    while (it != end && positional_index_ < positional_options_.size()) {
        Option* option = positional_options_[positional_index_];
        size_t args_to_consume = option->argument_count;

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
            std::any value = option->parse_function(args_view);
            option->value = value;
            option->is_set = true;

            // Store the value
            if (positional_index_ < positional_values_.size()) {
                positional_values_[positional_index_] = value;
            }
            else {
                positional_values_.push_back(value);
            }
        }
        catch (const Exception& e) {
            throw e;
        }
        catch (const std::exception& e) {
            throw Exception(Status::ParsingError, Context{ Param::ErrorMessage, e.what() });
        }

        ++positional_index_;
    }

    // Now process named options
    while (it != end) {
        std::string_view arg = *it;

        // Check if the argument matches any option name
        auto opt_map_it = option_map_.find(std::string(arg));
        if (opt_map_it != option_map_.end()) {
            // Option found
            Option& option = options_[opt_map_it->second];

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
                    values_[name] = value;
                }
            }
            catch (const Exception& e) {
                throw e;
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
    std::cout << description_ << "\n\n";
    std::cout << "Options:\n";

    for (const auto& option : options_) {
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

const std::vector<std::any>& Parser::positional_arguments() const {
    return positional_values_;
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
