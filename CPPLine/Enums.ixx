export module ErrorHandling:Enums;

import std;
import "magic_enum.hpp";

namespace cppline::errors
{
export template <typename T>
concept EnumType = std::is_enum_v<T>;

export enum class Message {
    Hello = 0,
    Godbye = 1,
    SendSignal = 1,
    SignalRecieved = 1,
};

export enum class EnumTypes
{
    EnumTypes = 0,
    Status,
    Message,
};

export using EnumsMap = std::map<EnumTypes, uint32_t>;

export template <EnumType Enum>
EnumTypes enum_type_helper(Enum enumerator);

export template <EnumType Enum>
EnumTypes enum_type(Enum enumerator) {
    return enum_type_helper(enumerator);
}

export template <EnumType... Enums>
EnumsMap create_enum_map(Enums... enumValues)
{
    EnumsMap map;
    create_enum_map_helper(map, std::forward<Enums>(enumValues)...);
    return map;
}

export template <EnumType Enum, EnumType... Enums>
void create_enum_map_helper(EnumsMap& map, Enum value, Enums... enumValues)
{
    map[enum_type(value)] = static_cast<uint32_t>(value);
    create_enum_map_helper(map, std::forward<Enums>(enumValues)...);
}

export template <EnumType Enum>
void create_enum_map_helper(EnumsMap& map, Enum value)
{
    map[enum_type(value)] = static_cast<uint32_t>(value);
}

export enum class Status {
    Success,
    UnknownOption,
    MissingArgument,
    ParsingError,
    InvalidValue,
    NotEnoughArguments,
    OptionAlreadySet,
    OptionNotFound,
    IndexOutOfRange,
    UnknownEnum,
    UnknownError,
    OptionAlreadyDefined
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
    Stacktrace,
    EnumValue,
    EnumType,
    Index,
};

std::string enum_to_string(EnumTypes enum_type, uint32_t enum_value);

}