module cppline;

import std;

namespace cppline {

void OptionBase::parse(const std::string& argument) {
    is_set = true;
    parse_impl(argument);
}

constexpr Parser::Parser(const std::string& desc) : m_description(desc) {}

void Parser::parse(const std::vector<std::string>& arguments)
{
    for (auto [index, argument] : std::ranges::enumerate_view(arguments)) {
        for (auto& option : m_options) {
            if (option->name == argument) {
                if (index + 1 < arguments.size()) {
                    option->parse(arguments[index + 1]);
                }
            }
        }
    }
}

// Factory function
constexpr Parser create_parser(const std::string& description) {
    return Parser(description);
}

} // namespace cppline
