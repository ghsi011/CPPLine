module ErrorHandling;

import "magic_enum.hpp";
import std;
import :Exception;

#define logger_add_enum(enum_name) \
template <> \
EnumTypes enum_type_helper<enum_name>(enum_name) { \
    return EnumTypes::enum_name; \
}

namespace cppline::errors {

logger_add_enum(Status)
logger_add_enum(Message)
logger_add_enum(EnumTypes)

std::string enum_to_string(EnumTypes enum_type, uint32_t enum_value)
{
    switch (enum_type)
    {
    case EnumTypes::EnumTypes:
        return magic_enum::enum_name(static_cast<EnumTypes>(enum_value)).data();
    case EnumTypes::Status:
        return magic_enum::enum_name(static_cast<Status>(enum_value)).data();
    case EnumTypes::Message:
        return magic_enum::enum_name(static_cast<Message>(enum_value)).data();
    default:
        throw Exception(Status::UnknownEnum,
                        Context{ Param::EnumType, std::to_string(static_cast<uint32_t>(enum_type)) } <<
                        Context{ Param::EnumValue, std::to_string(enum_value) });
    }
}

}