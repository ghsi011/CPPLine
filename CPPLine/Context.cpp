module ErrorHandling;

import std;
import :Enums;

namespace error_handling
{
StringContext Context::get_string_params() const
{
    return m_string_context;
}

EnumContext Context::get_enum_params() const
{
    return m_enum_context;
}

Context::Context(StringContext str_context, EnumContext enum_context) :
    m_string_context(std::move(str_context)),
    m_enum_context(std::move(enum_context))
{
}

Context::Context(Param param, std::string message) :
    m_string_context{ {param, message} }
{
}

Context::Context(EnumContext enum_context) :
    m_enum_context(std::move(enum_context))
{
}

Context& Context::operator <<(const Context& context)
{
    if (this == &context) {
        return *this;
    }

    m_string_context.insert(context.m_string_context.begin(), context.m_string_context.end());
    m_enum_context.insert(context.m_enum_context.begin(), context.m_enum_context.end());

    return *this;
}

Context& Context::operator<<(const std::tuple<Param, std::string>& str_param)
{
    m_string_context.emplace(str_param);
    return *this;
}

Context location_to_context(const std::source_location& location)
{
    Context location_context = Context{}
        << Context{ Param::SourceFile, location.file_name() }
        << Context{ Param::SourceLine, std::to_string(location.line()) }
        << Context{ Param::SourceColumn, std::to_string(location.column())
    };

    return location_context;
}

Context stacktrace_to_context(const std::stacktrace& stacktrace)
{
    std::string stacktrace_string = "\n";
    for (auto stack_entry : stacktrace) {
        std::string entry_line = std::format("{}, file: {}, line: {}\n", stack_entry.description(), stack_entry.source_file(), stack_entry.source_line());
        stacktrace_string += entry_line;
    }

    return Context{ Param::Stacktrace, stacktrace_string };
}
}