#pragma once

#define return_on_error(expr)                               \
do {                                                        \
    auto result = expr;                                     \
    if (!result.has_value()) {                              \
        return make_unexpected(std::move(result.error()));  \
    }                                                       \
} while (false)