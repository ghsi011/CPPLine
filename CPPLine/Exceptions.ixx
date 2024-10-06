export module ErrorHandling:Exception;

import std;
import :Context;
import :Enums;

export namespace cppline::errors {
class Exception;

class Exception final
{
public:
    explicit Exception(Status status,
                       Context context = StringContext{},
                       std::source_location location = std::source_location::current(),
                       std::stacktrace stacktrace = std::stacktrace::current());
    ~Exception() = default;

    Context get_context() const;
    void throw_self() const; // To allow ExceptionPtr to throw DerivedException.

    Status get_error() const;
    std::source_location get_location() const;
    std::stacktrace get_stacktrace() const;

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
} // namespace cppline::errors