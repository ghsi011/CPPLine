// Parser.cppm
module CPPLine;

import std;

namespace cppline {

Parser::Parser(const std::string& description)
    : description_(description) {}

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

void Parser::add_int(const std::vector<std::string>& names, const std::string& help, int default_value) {
    add_option(names, help,
               [](const std::vector<std::string_view>& args) -> std::any {
                   if (args.empty()) {
                       throw std::runtime_error("Expected a value");
                   }
                   return std::stoi(std::string(args[0]));
               },
               1); // One argument after the name
    // Set default value for all names
    for (const auto& name : names) {
        values_[name] = default_value;
    }
}

void Parser::add_string(const std::vector<std::string>& names, const std::string& help, const std::string& default_value) {
    add_option(names, help,
               [](const std::vector<std::string_view>& args) -> std::any {
                   if (args.empty()) {
                       throw std::runtime_error("Expected a value");
                   }
                   return std::string(args[0]);
               },
               1); // One argument after the name
    // Set default value for all names
    for (const auto& name : names) {
        values_[name] = default_value;
    }
}

void Parser::parse(const std::vector<std::string_view>& arguments) {
    using namespace std::ranges;

    auto args = arguments | views::common;
    auto it = args.begin();
    auto end = args.end();

    while (it != end) {
        std::string_view arg = *it;

        // Check if the argument matches any option name
        auto opt_map_it = option_map_.find(std::string(arg));
        if (opt_map_it != option_map_.end()) {
            // Option found
            Option& option = options_[opt_map_it->second];

            size_t args_to_consume = option.argument_count;
            if (std::distance(it, end) - 1 < static_cast<std::ptrdiff_t>(args_to_consume)) {
                throw std::runtime_error("Not enough arguments for option " + option.names[0]);
            }

            // Collect arguments for the parse function
            std::vector<std::string_view> args_view;
            if (args_to_consume > 0) {
                auto args_start = std::next(it);
                auto args_end = std::next(it, args_to_consume + 1); // +1 to include the option name
                args_view.assign(args_start, args_end);
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
            catch (const std::exception& e) {
                throw std::runtime_error("Error parsing option " + option.names[0] + ": " + e.what());
            }

            // Advance the iterator
            it = std::next(it, args_to_consume + 1);
        }
        else {
            // Positional argument
            positional_args_.push_back(arg);
            ++it;
        }
    }
}

const std::vector<std::string_view>& Parser::positional_arguments() const {
    return positional_args_;
}

void Parser::print_help() const {
    std::cout << description_ << "\n\n";
    std::cout << "Options:\n";

    for (const auto& option : options_) {
        std::string names_str = std::accumulate(
            option.names.begin(), option.names.end(), std::string{},
            [](const std::string& a, const std::string& b) {
                return a.empty() ? b : a + ", " + b;
            }
        );
        std::cout << "  " << names_str << "\t" << option.help << "\n";
    }
}


}
