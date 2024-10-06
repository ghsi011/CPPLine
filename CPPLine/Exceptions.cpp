module ErrorHandling;

import std;
import :Context;
import :Enums;

namespace cppline::errors
{
Exception::Exception(Status status,
                     Context context,
                     std::source_location location,
                     std::stacktrace stacktrace) :
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
        << location_to_context(get_location())
        << stacktrace_to_context(get_stacktrace());

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

std::stacktrace Exception::get_stacktrace() const
{
    return m_stacktrace;
}

} // namespace cppline