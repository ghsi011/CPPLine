export module cppline;

import std;

namespace cppline {

int parse_argument(const std::string& argument, std::optional<int> default_value = std::nullopt) {
    return std::stoi(argument);
}

struct OptionBase {
    std::string name;
    std::string help;
    bool is_set = false;

    void parse(const std::string& argument);

    OptionBase(const std::string& name_, const std::string& help_) : name{ name_ }, help{ help_ } {}

private:
    virtual void parse_impl(const std::string& argument) = 0;
};

template <typename T>
struct Option : public OptionBase{
    std::optional<T> value;

    constexpr Option(const std::string& name_, const std::string& help_, std::optional<T> default_val = std::nullopt) :
        OptionBase{ name_, help_ }, value{ default_val } {}

private:
    void parse_impl(const std::string& argument) override {
        value = parse_argument(argument, value);
    }
};

export class Parser {
public:
    constexpr Parser(const std::string& desc);

    void parse(const std::vector<std::string>& arguments);
    
    template <typename T>
    constexpr void add_option(const std::string& name, const std::string& help, std::optional<T> default_val = std::nullopt) {
        m_options.emplace_back(std::make_unique<Option<T>>(name, help, default_val));
    }

    template <typename T>
    constexpr T get(const std::string& name) {
        for (auto& option : m_options) {
            if (option->name == name) {
                auto opt = reinterpret_cast<Option<T>*>(option.get());
                if (opt) {
                    return *opt->value;
                }
            }
        }
        throw std::runtime_error("Option not found");
    }

private:
    std::string m_description;
    std::vector<std::unique_ptr<OptionBase>> m_options;

};

// Function to create a parser
constexpr Parser create_parser(const std::string& description);

}
