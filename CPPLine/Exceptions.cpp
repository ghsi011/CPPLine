module ErrorHandling;

import std;
import :Context;
import :Enums;

namespace cppline::errors
{
Exception::Exception(Status status,
                     Context context,
                     std::source_location location,
                     std::optional<std::stacktrace> stacktrace) :
    m_status(status),
    m_context(std::move(context)),
    m_location(std::move(location)),
    m_stacktrace(std::move(stacktrace)) {}

Status Exception::get_error() const
{
    return m_status;
}
Context Exception::get_context() const
{
    auto exception_context = Context(m_status)
        << location_to_context(get_location());

    if (const std::optional<std::stacktrace> stack_trace = get_stacktrace(); stack_trace.has_value())
    {
        exception_context << stacktrace_to_context(stack_trace.value());
    }

    return exception_context << m_context;
}
void errors::Exception::throw_self() const
{
    throw* this;
}
std::source_location Exception::get_location() const
{
    return m_location;
}

std::optional<std::stacktrace> Exception::get_stacktrace() const
{
    return m_stacktrace;
}

} // namespace cppline