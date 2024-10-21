module ErrorHandling;
import :Enums;
import :Exception;
import :Context;

import std;
import "magic_enum.hpp";

namespace cppline::errors
{

void Logger::log(const std::string& message)
{
    std::cout << message.c_str() << std::endl;
}

void Logger::log(const std::string& message, const Exception& exception)
{
    std::cout << message.c_str() << std::endl;
    const std::string exception_info
        = std::format("Exception:\n{}", format_context(exception.get_context()));

    std::cout << exception_info << std::endl;
}

void Logger::log(const Context& context, const std::source_location& location)
{
    const std::string context_info
        = std::format("Context:\n{}", format_context(location_to_context(location) << context));

    std::cout << context_info << std::endl;
}

void Logger::log(const std::string& message, const Context& context, const std::source_location& location)
{
    const std::string context_info = std::format("Log Message: {}\n{}",
                                                 message,
                                                 format_context(location_to_context(location) << context));

    std::cout << context_info << std::endl;
}

std::string Logger::format_context(const Context& context)
{
    std::string context_string = "Context: {\n";
    for (auto [key, value] : context.get_enum_params())
    {
        context_string += std::format("\t[{} : {}]\n", magic_enum::enum_name(key), enum_to_string(key, value));
    }

    for (auto [key, value] : context.get_string_params())
    {
        context_string += std::format("\t[{} : {}]\n", magic_enum::enum_name(key), value);
    }

    context_string += "}";

    return context_string;
}

}
