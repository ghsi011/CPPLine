export module ErrorHandling:Expected;

import std;
import :Exception;

export namespace cppline::errors {

using ExpectedVoid = std::expected<void, Exception>;
using ExpectedString = std::expected<std::string, Exception>;

ExpectedVoid success()
{
    return {};
}

template <typename T >
using Expected = std::expected<T, Exception>;

inline std::unexpected<Exception> make_unexpected(const Status status,
                                           Context context = StringContext{},
                                           const std::source_location& location = std::source_location::current(),
                                           const std::stacktrace& stacktrace = std::stacktrace::current())
{
    return std::unexpected(Exception(status, std::move(context), location, stacktrace));
}

inline std::unexpected<Exception> make_unexpected(Exception&& exception)
{
    return std::unexpected(std::move(exception));
}

template <typename T >
void throw_on_error(const Expected<T>& expected)
{
    if (!expected.has_value())
    {
        expected.error().throw_self();
    }
}
}
