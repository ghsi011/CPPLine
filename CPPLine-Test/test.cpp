#include "pch.h"

#include <gtest/gtest.h>
import CPPLine;

using namespace cppline::errors;

TEST(ParserTest, AddBoolOption) {
    cppline::Parser parser("Test Parser");
    parser.add_bool("--test", "Test option");

    const std::vector<std::string_view> args{ "--test" };
    parser.parse(args);

    EXPECT_TRUE(parser.get<bool>("--test"));
}

TEST(ParserTest, AddIntOption) {
    try
    {
        cppline::Parser parser("Test Parser");
        parser.add_int("--number", "Number option", 0);

        const std::vector<std::string_view> args{ "--number", "42" };
        parser.parse(args);

        EXPECT_EQ(parser.get<int>("--number"), 42);
    }
    catch (const Exception& e)
    {
        Logger::log("Exception occurred", e);
        FAIL();
    }
}

TEST(ParserTest, MissingArgument) {
    cppline::Parser parser("Test Parser");
    parser.add_int("--number", "Number option");

    const std::vector<std::string_view> args{ "--number" };
    EXPECT_THROW(parser.parse(args), cppline::errors::Exception);
}

TEST(ParserTest, UnknownOption) {
    cppline::Parser parser("Test Parser");

    const std::vector<std::string_view> args = { "--unknown" };
    EXPECT_THROW(parser.parse(args), cppline::errors::Exception);
}

TEST(ParserCustomTypeTest, CustomKeyValueOption) {
    cppline::Parser parser("Test Parser");

    // Add a custom key-value pair option
    parser.add_option("--keyvalue", "Set a key-value pair",
                      [](const std::vector<std::string_view>& args) -> std::any {
                          if (args.size() < 2) {
                              throw cppline::errors::Exception(
                                  cppline::errors::Status::MissingArgument,
                                  cppline::errors::Context{ cppline::errors::Param::ErrorMessage, "Expected key and value" });
                          }
                          return std::make_pair(std::string(args[0]), std::string(args[1]));
                      }, 2);

    // Test valid key-value input
    const std::vector<std::string_view> args{ "--keyvalue", "myKey", "myValue" };
    parser.parse(args);

    // Get the parsed key-value pair
    auto key_value = parser.get<std::pair<std::string, std::string>>("--keyvalue");

    // Verify key and value
    EXPECT_EQ(key_value.first, "myKey");
    EXPECT_EQ(key_value.second, "myValue");
}

TEST(ParserCustomTypeTest, MissingKeyValueArguments) {
    cppline::Parser parser("Test Parser");

    // Add a custom key-value pair option
    parser.add_option("--keyvalue", "Set a key-value pair",
                      [](const std::vector<std::string_view>& args) -> std::any {
                          if (args.size() < 2) {
                              throw cppline::errors::Exception(
                                  cppline::errors::Status::MissingArgument,
                                  cppline::errors::Context{ cppline::errors::Param::ErrorMessage, "Expected key and value" });
                          }
                          return std::make_pair(std::string(args[0]), std::string(args[1]));
                      }, 2);

    // Test with missing value
    const std::vector<std::string_view> args = { "--keyvalue", "myKey" };

    // Expect an exception due to missing value argument
    EXPECT_THROW(parser.parse(args), cppline::errors::Exception);
}

TEST(ParserTest, AddPositionalArgument) {
    cppline::Parser parser("Test Parser");

    // Add a positional argument
    parser.add_int("Number argument");
    parser.add_string("String argument");

    // Test valid positional argument
    const std::vector<std::string_view> args = { "42", "str" };
    parser.parse(args);

    // Get the parsed positional argument
    auto number = parser.get_positional<int>(0);
    auto str = parser.get_positional<std::string>(1);

    // Verify the parsed value
    EXPECT_EQ(number, 42);
    EXPECT_EQ(str, "str");
}

TEST(ParserTest, OptionDoubleDefinition) {
    cppline::Parser parser("Test Parser");

    // Add an option with the same name
    parser.add_int("--number", "Number option", 0);

    // Expect an exception due to double definition
    EXPECT_THROW(parser.add_int("--number", "Number option", 0), cppline::errors::Exception);
}

TEST(ParserTest, OptionDoubleUsage) {
    cppline::Parser parser("Test Parser");

    // Add an option with the same name
    parser.add_int(std::vector<std::string>{ "--number" , "-n"}, "Number option", 0);

    // Test valid input
    const std::vector<std::string_view> args = { "--number", "42", "-n", "24" };

    // Expect an exception due to double usage
    EXPECT_THROW(parser.parse(args), cppline::errors::Exception);
}