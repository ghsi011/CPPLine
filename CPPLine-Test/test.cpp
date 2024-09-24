#include "pch.h"

#include <gtest/gtest.h>
import CPPLine;

TEST(ParserTest, AddBoolOption) {
    cppline::Parser parser("Test Parser");
    parser.add_bool("--test", "Test option");

    const char* argv[] = { "--test" };
    parser.parse({ argv, argv + 1 });

    EXPECT_TRUE(parser.get<bool>("--test"));
}

TEST(ParserTest, AddIntOption) {
    cppline::Parser parser("Test Parser");
    parser.add_int("--number", "Number option", 0);

    const char* argv[] = { "--number", "42" };
    parser.parse({ argv, argv + 2 });

    EXPECT_EQ(parser.get<int>("--number"), 42);
}

TEST(ParserTest, MissingArgument) {
    cppline::Parser parser("Test Parser");
    parser.add_int("--number", "Number option");

    const char* argv[] = { "--number" };
    EXPECT_THROW(parser.parse({ argv, argv + 1 }), cppline::errors::Exception);
}

TEST(ParserTest, UnknownOption) {
    cppline::Parser parser("Test Parser");

    const char* argv[] = { "--unknown" };
    EXPECT_THROW(parser.parse({ argv, argv + 1 }), cppline::errors::Exception);
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
    const char* argv[] = { "--keyvalue", "myKey", "myValue" };
    parser.parse({ argv, argv + 3 });

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
    const char* argv[] = { "--keyvalue", "myKey" };

    // Expect an exception due to missing value argument
    EXPECT_THROW(parser.parse({ argv, argv + 2 }), cppline::errors::Exception);
}