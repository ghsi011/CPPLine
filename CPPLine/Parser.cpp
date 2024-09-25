module CPPLine;

import std;
import ErrorHandling;

using namespace cppline::errors;

namespace cppline {

Parser::Parser(const std::string& description)
    : m_description(description) {}

ExpectedVoid Parser::try_add_option(const std::vector<std::string>& names, const std::string& help,
                                    ParseFunctionType parse_function, const size_t argument_count, std::any default_value)
{
    if (std::ranges::any_of(names, [this](const std::string& name) { return m_option_map.contains(name); })) {
        return make_unexpected(Status::OptionAlreadyDefined, Context{ Param::OptionName, join_names(names) });
    }

    Option option{ names, help, argument_count, parse_function, default_value, false };
    m_options.push_back(option);

    size_t index = m_options.size() - 1;
    for (const auto& name : names) {
        m_option_map[name] = index;
    }

    return success();
}

ExpectedVoid Parser::try_add_option(const std::string& name, const std::string& help, ParseFunctionType parse_function,
                                    size_t argument_count, std::any default_value)
{
    return try_add_option(std::vector{ name }, help, parse_function, argument_count, default_value);
}

ExpectedVoid Parser::try_add_option(const std::string& help, ParseFunctionType parse_function, size_t argument_count,
                                    std::any default_value)
{
    Option option{ {}, help, argument_count, parse_function, default_value, false };

    m_positional_options.push_back(option);

    return success();
}

ExpectedVoid Parser::try_add_bool(const std::vector<std::string>& names, const std::string& help) {
    return try_add_option(names, help,
                          parse_bool,
                          0,
                          false); // No arguments after the option name
}

ExpectedVoid Parser::try_add_bool(const std::string& name, const std::string& help) {
    return try_add_bool(std::vector{ name }, help);
}

ExpectedVoid Parser::try_add_bool(const std::string& help) {
    return try_add_option(help,
                          parse_bool,
                          0,
                          false); // No arguments after the positional argument
}

ExpectedVoid Parser::try_add_int(const std::vector<std::string>& names, const std::string& help, int default_value) {
    return try_add_option(names, help,
                          parse_int_factory(names),
                          1,
                          default_value); // One argument after the name
}

ExpectedVoid Parser::try_add_int(const std::string& name, const std::string& help, int default_value) {
    return try_add_int(std::vector{ name }, help, default_value);
}

ExpectedVoid Parser::try_add_int(const std::string& help) {
    return try_add_option(help,
                          parse_int_factory({}),
                          1); // One argument after the positional argument
}

ExpectedVoid Parser::try_add_string(const std::vector<std::string>& names, const std::string& help, const std::string& default_value) {
    return try_add_option(names, help,
                          parse_string_factory(names),
                          1,
                          default_value); // One argument after the name
}

ExpectedVoid Parser::try_add_string(const std::string& name, const std::string& help, const std::string& default_value) {
    return try_add_string(std::vector{ name }, help, default_value);
}

ExpectedVoid Parser::try_add_string(const std::string& help) {
    return try_add_option(help,
                          parse_string_factory({}),
                          1); // One argument after the positional argument
}


void Parser::parse(const std::vector<std::string_view>& arguments) {
    auto non_positional_arguments = parse_positional(arguments);

    parse_non_positional(non_positional_arguments);
}

void Parser::print_help() const {

    std::string usage = "Usage: ";
    std::string help;

    for (const auto& positional_option : m_positional_options) {
        usage += std::format("<{}> ", positional_option.help);
        help += std::format("{} \n", positional_option.help);
    }

    help += "Options:\n";

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
        usage += std::format("[{}] ", names_str);
        help += std::format("  {} \t{}\n", names_str, option.help);
    }

    std::cout << m_description << std::endl;
    std::cout << usage << std::endl;
    std::cout << help << std::endl;
}

std::vector<std::string_view> Parser::parse_positional(const std::vector<std::string_view>& arguments)
{
    std::vector<std::string_view> remaining_args = arguments;

    for (const auto& [positional_index, option] : std::views::enumerate(m_positional_options))
    {
        const size_t args_to_consume = option.argument_count;

        if (remaining_args.size() < args_to_consume) {
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

std::any Parser::parse_bool(const std::vector<std::string_view>&)
{
    return true; // Presence implies true
}

ParseFunctionType Parser::parse_int_factory(const std::vector<std::string>& names)
{
    return [names](const std::vector<std::string_view>& args) -> std::any {
        if (args.empty()) {
            throw Exception(Status::MissingArgument, Context{ Param::OptionName, join_names(names) });
        }
        return std::stoi(std::string(args[0]));
        };
}

ParseFunctionType Parser::parse_string_factory(const std::vector<std::string>& names)
{
    return [names](const std::vector<std::string_view>& args) -> std::any {
        if (args.empty()) {
            throw Exception(Status::MissingArgument, Context{ Param::OptionName, join_names(names) });
        }
        return std::string(args[0]);
        };
}
} // namespace cppline
