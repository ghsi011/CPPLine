#include "pch.h"
#include <gtest/gtest.h>

import CPPLine;

import std;

using namespace cppline::errors;

// Helper function to measure the execution time of a callable
template<typename Callable>
double measure_execution_time(Callable&& func) {
    auto start = std::chrono::steady_clock::now();
    func();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::micro>(end - start).count();
}

// Functions to be tested
void test_function(bool should_throw, volatile int& sink) {
    if (should_throw) {
        throw Exception(
            Status::UnknownError,
            Context{} << Message::ExpectedKeyAndValue << Context{ Param::ErrorMessage, "message" }
        );
    }
    // Prevent optimization
    sink += 1;
}

ExpectedVoid try_test_function(bool should_fail, volatile int& sink) {
    if (should_fail) {
        return make_unexpected(
            Status::UnknownError,
            Context{} << Message::ExpectedKeyAndValue << Context{ Param::ErrorMessage, "message" }
        );
    }
    // Prevent optimization
    sink += 1;
    return success();
}

TEST(ErrorsTest, ExceptionsBetterOnHappyFlow) {
    constexpr int iterations = 1'000'000;
    constexpr int runs = 5;
    std::vector<double> exception_times;
    std::vector<double> expected_times;

    for (int run = 0; run < runs; ++run) {
        volatile int sink = 0;

        auto exception_time = measure_execution_time([&]() {
            for (int i = 0; i < iterations; ++i) {
                try {
                    test_function(false, sink);
                }
                catch (const Exception&) {
                    // Should not occur
                }
            }
                                                     });

        auto expected_time = measure_execution_time([&]() {
            for (int i = 0; i < iterations; ++i) {
                auto result = try_test_function(false, sink);
                if (!result.has_value()) {
                    // Should not occur
                }
            }
                                                    });

        exception_times.push_back(exception_time);
        expected_times.push_back(expected_time);
    }

    const double avg_exception_time = std::accumulate(exception_times.begin(), exception_times.end(), 0.0) / runs;
    const double avg_expected_time = std::accumulate(expected_times.begin(), expected_times.end(), 0.0) / runs;

    std::cout << "Average Exception Time (Happy Flow): " << avg_exception_time << " microseconds\n";
    std::cout << "Average Expected Time (Happy Flow): " << avg_expected_time << " microseconds\n";

    std::cout << "difference in seconds " << (avg_expected_time - avg_exception_time) / 1000000.0 << std::endl;

    EXPECT_LT(avg_exception_time, avg_expected_time) << "Exceptions should perform better on the happy path.";
}

TEST(ErrorsTest, ExpectedBetterOnSadFlow) {
    constexpr int iterations = 100'000;
    constexpr int runs = 5;
    std::vector<double> exception_times;
    std::vector<double> expected_times;

    for (int run = 0; run < runs; ++run) {
        volatile int sink = 0;

        auto exception_time = measure_execution_time([&]() {
            for (int i = 0; i < iterations; ++i) {
                try {
                    test_function(true, sink);
                }
                catch (const Exception&) {
                    // Handle the exception
                    sink += 1;
                }
            }
                                                     });

        auto expected_time = measure_execution_time([&]() {
            for (int i = 0; i < iterations; ++i) {
                auto result = try_test_function(true, sink);
                if (!result.has_value()) {
                    // Handle the error
                    sink += 1;
                }
            }
                                                    });

        exception_times.push_back(exception_time);
        expected_times.push_back(expected_time);
    }

    const double avg_exception_time = std::accumulate(exception_times.begin(), exception_times.end(), 0.0) / runs;
    const double avg_expected_time = std::accumulate(expected_times.begin(), expected_times.end(), 0.0) / runs;

    std::cout << "Average Exception Time (Sad Flow): " << avg_exception_time << " microseconds\n";
    std::cout << "Average Expected Time (Sad Flow): " << avg_expected_time << " microseconds\n";

    std::cout << "difference in seconds " << (avg_exception_time - avg_expected_time) / 1000000.0 << std::endl;

    EXPECT_LT(avg_expected_time, avg_exception_time) << "Expected should perform better on the sad path.";
}

_declspec(noinline) void recursive_function_exception(volatile int& sink, bool should_throw, int number_of_calls) {
    sink += 1;
    if (number_of_calls != 0) {
        recursive_function_exception(sink, should_throw, number_of_calls - 1);
    }

    if (should_throw) {
        throw Exception(
            Status::UnknownError,
            Context{} << Message::ExpectedKeyAndValue << Context{ Param::ErrorMessage, "Innermost exception" }
        );
    }
}

_declspec(noinline) ExpectedVoid recursive_function_expected(volatile int& sink, bool should_fail, int number_of_calls) {
    if (number_of_calls != 0) {
        auto result = recursive_function_expected(sink, should_fail, number_of_calls - 1);
        if (!result.has_value()) {
            sink += 1;
            return result;
        }
        return success();
    }

    if (should_fail) {
        return make_unexpected(
            Status::UnknownError,
            Context{} << Message::ExpectedKeyAndValue << Context{ Param::ErrorMessage, "Innermost error" }
        );
    }
    return success();
}

static constexpr int RECURSION_DEPTH = 10;

TEST(ErrorsTest, NestedExceptionsBetterOnHappyFlow) {
    constexpr int iterations = 1'000'000;
    constexpr int runs = 5;
    std::vector<double> exception_times;
    std::vector<double> expected_times;

    for (int run = 0; run < runs; ++run) {
        volatile int sink = 0;

        // Exceptions version (happy flow, no exceptions thrown)
        auto exception_time = measure_execution_time([&]() {
            for (int i = 0; i < iterations; ++i) {
                try {
                    recursive_function_exception(sink, false, RECURSION_DEPTH); // No exception thrown
                }
                catch (const Exception& ) {
                    // Should not occur
                }
            }
                                                     });

        // Expected version (happy flow, no errors)
        auto expected_time = measure_execution_time([&]() {
            for (int i = 0; i < iterations; ++i) {
                auto result = recursive_function_expected(sink, false, RECURSION_DEPTH); // No error
                if (!result.has_value()) {
                    // Should not occur
                }
            }
                                                    });

        exception_times.push_back(exception_time);
        expected_times.push_back(expected_time);
    }

    double avg_exception_time = std::accumulate(exception_times.begin(), exception_times.end(), 0.0) / runs;
    double avg_expected_time = std::accumulate(expected_times.begin(), expected_times.end(), 0.0) / runs;

    std::cout << "Average Nested Exception Time (Happy Flow): " << avg_exception_time << " microseconds\n";
    std::cout << "Average Nested Expected Time (Happy Flow): " << avg_expected_time << " microseconds\n";

    std::cout << "difference in seconds " << (avg_expected_time - avg_exception_time) / 1000000.0 << std::endl;

    EXPECT_LT(avg_exception_time, avg_expected_time) << "Exceptions should perform better on the happy path with nesting.";
}

TEST(ErrorsTest, NestedExpectedBetterOnSadFlow) {
    constexpr int iterations = 100'000;
    constexpr int runs = 5;
    std::vector<double> exception_times;
    std::vector<double> expected_times;

    for (int run = 0; run < runs; ++run) {
        volatile int sink = 0;

        // Exceptions version (sad flow, exceptions thrown)
        auto exception_time = measure_execution_time([&]() {
            for (int i = 0; i < iterations; ++i) {
                try {
                    recursive_function_exception(sink, true, RECURSION_DEPTH); // Exception thrown
                }
                catch (const Exception&) {
                    sink += 1;
                }
            }
                                                     });

        // Expected version (sad flow, errors occur)
        auto expected_time = measure_execution_time([&]() {
            for (int i = 0; i < iterations; ++i) {
                auto result = recursive_function_expected(sink, true, RECURSION_DEPTH); // Error occurs
                if (!result.has_value()) {
                    sink += 1;
                }
            }
                                                    });

        exception_times.push_back(exception_time);
        expected_times.push_back(expected_time);
    }

    double avg_exception_time = std::accumulate(exception_times.begin(), exception_times.end(), 0.0) / runs;
    double avg_expected_time = std::accumulate(expected_times.begin(), expected_times.end(), 0.0) / runs;

    std::cout << "Average Nested Exception Time (Sad Flow): " << avg_exception_time << " microseconds\n";
    std::cout << "Average Nested Expected Time (Sad Flow): " << avg_expected_time << " microseconds\n";

    std::cout << "difference in seconds " << (avg_exception_time - avg_expected_time) / 1000000.0 << std::endl;

    EXPECT_LT(avg_expected_time, avg_exception_time) << "Expected should perform better on the sad path with nesting.";
}
