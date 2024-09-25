export module ErrorHandling:Expected;

import std;
import :Exception;

export namespace cppline::errors {

using ExpectedVoid = std::expected<void, ExceptionPtr>;
using ExpectedString = std::expected<std::string, ExceptionPtr>;

ExpectedVoid success()
{
    return {};
}

template <typename T >
using Expected = std::expected<T, ExceptionPtr>;

std::unexpected<ExceptionPtr> make_unexpected(Status status,
                                              Context context = StringContext{},
                                              std::source_location location = std::source_location::current(),
                                              std::stacktrace stacktrace = std::stacktrace::current())
{
    return std::unexpected(Exception::make_exception(status, std::move(context), location, stacktrace));
}

template <typename T >
void throw_on_error(const Expected<T>& expected)
{
    if (!expected.has_value())
    {
        expected.error()->throw_self();
    }
}
}
