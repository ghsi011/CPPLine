export module ErrorHandling:Expected;

import std;
import :Exception;

export namespace cppline::errors {

using ExpectedVoid = std::expected<void, ExceptionPtr>;
using ExpectedString = std::expected<std::string, ExceptionPtr>;

}