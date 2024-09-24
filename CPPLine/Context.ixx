export module ErrorHandling:Context;

import std;
import :Enums;

export namespace error_handling
{

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
}
