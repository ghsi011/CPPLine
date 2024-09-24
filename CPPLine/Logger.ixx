module;

#include "magic_enum.hpp"

export module ErrorHandling:Logger;

import std;
import :Enums;
import :Exception;
import :Context;

export namespace error_handling {

class Logger final
{
public:
    static void log(const std::string& message);
    static void log(const std::string& message, const Exception& exception);
    static void log(const std::string& message, const ExceptionPtr& exception);
    static void log(const Context& context,
                    const std::source_location& location = std::source_location::current());

    static void log(const std::string& message,
                    const Context& context,
                    const std::source_location& location = std::source_location::current());

    template <EnumType... Enums>
    static void log(const std::string& message, Enums... enums) {
        std::cout << std::format("Log message: {}\n{}",
                                 message,
                                 format_enums(std::forward<Enums>(enums)...)).c_str();
    }

    template <EnumType... Enums>
    static void log(Enums... enums) {
        std::cout << format_enums(std::forward<Enums>(enums)...).c_str() << std::endl;
    }

private:
    static std::string format_context(const Context& context);

    template <EnumType... Enums>
    static std::string format_enums(Enums... enums) {
        EnumsMap map = create_enum_map(std::forward<Enums>(enums)...);
        std::string message;
        for (auto [key, value] : map) {
            message += std::format("enum type: {}, enum value: {}\n", magic_enum::enum_name(key),
                                   enum_to_string(key, value));
        }

        return message;
    }

public:
    // Static class:
    Logger() = delete;
    ~Logger() = delete;
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;
};

} // namespace error_handling