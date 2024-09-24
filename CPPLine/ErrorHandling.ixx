module;

#include "magic_enum.hpp"

// ErrorHandling.ixx
export module ErrorHandling;

import std;

export namespace error_handling {

template <typename T>
concept EnumType = std::is_enum_v<T>;

enum class Message {
    Hello = 0,
    Godbye = 1,
    SendSignal = 1,
    SignalRecieved = 1,
};

enum class SystemErrors
{
    ErrorFileNotFound,
    ErrorAccessDenied,
    ErrorFileExists,
};

enum class FileAccess
{
    Read,
    Write,
};

enum class EnumTypes
{
    EnumTypes = 0,
    Status,
    Message,
    SystemErrors,
    FileAccess,
    NewEnum,
};

enum class NewEnum
{
    Value1,
    Value2,
};

using EnumsMap = std::map<EnumTypes, uint32_t>;

std::string enum_to_string(EnumTypes enum_type, uint32_t enum_value);

template <EnumType Enum>
EnumTypes enum_type_helper(Enum enumerator);

template <EnumType Enum>
EnumTypes enum_type(Enum enumerator) {
    return enum_type_helper(enumerator);
}

template <EnumType... Enums>
EnumsMap create_enum_map(Enums... enumValues)
{
    EnumsMap map;
    create_enum_map_helper(map, std::forward<Enums>(enumValues)...);
    return map;
}

template <EnumType Enum, EnumType... Enums>
void create_enum_map_helper(EnumsMap& map, Enum value, Enums... enumValues)
{
    map[enum_type(value)] = static_cast<uint32_t>(value);
    create_enum_map_helper(map, std::forward<Enums>(enumValues)...);
}

template <EnumType Enum>
void create_enum_map_helper(EnumsMap& map, Enum value)
{
    map[enum_type(value)] = static_cast<uint32_t>(value);
}

#define logger_add_enum(enum_name) \
template <> \
EnumTypes enum_type_helper<enum_name>(enum_name) { \
    return EnumTypes::enum_name; \
}

export enum class Status {
    Success,
    UnknownOption,
    MissingArgument,
    ParsingError,
    InvalidValue,
    NotEnoughArguments,
    OptionAlreadySet,
    UnknownError
};

export enum class Param {
    OptionName,
    ArgumentValue,
    ExpectedArgumentCount,
    ReceivedArgumentCount,
    HelpMessage,
    ErrorMessage,
    SourceFile,
    SourceLine,
    SourceColumn,
    Stacktrace
};

using StringContext = std::map<Param, std::string>;
using StringPair = std::tuple<Param, std::string>;

using EnumContext = EnumsMap;

export class Context final
{
public:
    Context() = default;
    ~Context() = default;
    StringContext get_string_params() const;
    EnumContext get_enum_params() const;

    Context(StringContext str_context, EnumContext enum_context = {});
    Context(Param param, std::string message);
    Context(EnumContext enum_context);

    template <EnumType... Enums>
    explicit Context(Enums... enums) :
        m_enum_context(create_enum_map(std::forward<Enums>(enums)...))
    {
    }

    Context& operator<< (const Context& context);

    Context& operator<< (const std::tuple<Param, std::string>& str_param);

    template <EnumType Enum>
    Context& operator<< (Enum enum_value);

private:
    StringContext m_string_context;
    EnumContext m_enum_context;
};

template <EnumType Enum>
Context& Context::operator<<(Enum enum_value)
{
    m_enum_context.emplace(enum_type(enum_value), static_cast<uint32_t>(enum_value));
    return *this;
}

Context location_to_context(const std::source_location& location);
Context stacktrace_to_context(const std::stacktrace& stacktrace);

export class Exception;
using ExceptionPtr = std::unique_ptr<Exception>;

class Exception
{
public:
    explicit Exception(Status status,
                       Context context = StringContext{},
                       std::source_location location = std::source_location::current(),
                       std::stacktrace stacktrace = std::stacktrace::current());
    virtual ~Exception() = default;

    virtual Context get_context() const;

    Status get_error() const;
    std::source_location get_location() const;
    std::stacktrace get_stacktrace() const;

    static ExceptionPtr make_exception(Status status,
                                       Context context = StringContext{},
                                       std::source_location location = std::source_location::current(),
                                       std::stacktrace stacktrace = std::stacktrace::current());

    Exception(const Exception&) = default;             // Copy Ctor
    Exception(Exception&&) = default;                  // Move Ctor
    Exception& operator= (const Exception&) = default; // Copy Assignment Operator
    Exception& operator= (Exception&&) = default;      // Move Assignment Operator

private:
    Status m_status;
    Context m_context;
    std::source_location m_location;
    std::stacktrace m_stacktrace;
};

export class Logger final
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

} // namespace cppline
