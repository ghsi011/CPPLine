#include "pch.h"

#include <gtest/gtest.h>
import CPPLine;

using namespace cppline::errors;

TEST(ParserTest, AddBoolOption) {
    try
    {
        cppline::Parser parser("Test Parser");
        parser.add_bool("--test", "Test option");

        const std::vector<std::string_view> args{ "--test" };
        parser.parse(args);

        EXPECT_TRUE(parser.get<bool>("--test"));
    }
    catch (const Exception& e)
    {
        Logger::log("Unexpected Exception occurred", e);
        FAIL();
    }
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
        Logger::log("Unexpected Exception occurred", e);
        FAIL();
    }
}

TEST(ParserTest, MissingArgument) {
    try
    {
        cppline::Parser parser("Test Parser");
        parser.add_int("--number", "Number option");

        const std::vector<std::string_view> args{ "--number" };
        EXPECT_THROW(parser.parse(args), cppline::errors::Exception);
    }
    catch (const Exception& e)
    {
        Logger::log("Unexpected Exception occurred", e);
        FAIL();
    }
}

TEST(ParserTest, UnknownOption) {
    try
    {
        cppline::Parser parser("Test Parser");

        const std::vector<std::string_view> args = { "--unknown" };
        EXPECT_THROW(parser.parse(args), cppline::errors::Exception);
    }
    catch (const Exception& e)
    {
        Logger::log("Unexpected Exception occurred", e);
        FAIL();
    }
}

TEST(ParserCustomTypeTest, CustomKeyValueOption) {
    try
    {
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
    catch (const Exception& e)
    {
        Logger::log("Unexpected Exception occurred", e);
        FAIL();
    }
}

TEST(ParserCustomTypeTest, MissingKeyValueArguments) {
    try
    {
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
    catch (const Exception& e)
    {
        Logger::log("Unexpected Exception occurred", e);
        FAIL();
    }
}

TEST(ParserTest, AddPositionalArgument) {
    try
    {
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
    catch (const Exception& e)
    {
        Logger::log("Unexpected Exception occurred", e);
        FAIL();
    }
}

TEST(ParserTest, OptionDoubleDefinition) {
    try
    {
        cppline::Parser parser("Test Parser");

        // Add an option with the same name
        parser.add_int("--number", "Number option", 0);

        // Expect an exception due to double definition
        EXPECT_THROW(parser.add_int("--number", "Number option", 0), cppline::errors::Exception);
    }
    catch (const Exception& e)
    {
        Logger::log("Unexpected Exception occurred", e);
        FAIL();
    }
}

TEST(ParserTest, OptionDoubleUsage) {
    try
    {
        cppline::Parser parser("Test Parser");

        // Add an option with the same name
        parser.add_int(std::vector<std::string>{ "--number", "-n"}, "Number option", 0);

        // Test valid input
        const std::vector<std::string_view> args = { "--number", "42", "-n", "24" };

        // Expect an exception due to double usage
        EXPECT_THROW(parser.parse(args), cppline::errors::Exception);
    }
    catch (const Exception& e)
    {
        Logger::log("Unexpected Exception occurred", e);
        FAIL();
    }
}

TEST(ParserTest, TryGetOptionSuccess) {
    cppline::Parser parser("Test Parser");
    parser.add_int("--number", "Number option", 0);

    const std::vector<std::string_view> args{ "--number", "42" };
    auto parse_result = parser.try_parse(args);
    EXPECT_TRUE(parse_result.has_value());

    auto number_result = parser.try_get<int>("--number");
    EXPECT_TRUE(number_result.has_value());
    EXPECT_EQ(number_result.value(), 42);
}

TEST(ParserTest, TryGetOptionFailure) {
    cppline::Parser parser("Test Parser");
    parser.add_int("--number", "Number option", 0);

    const std::vector<std::string_view> args{ "--number", "invalid" };
    auto parse_result = parser.try_parse(args);
    EXPECT_FALSE(parse_result.has_value());

    Logger::log("Expected parsing error", parse_result.error());
}