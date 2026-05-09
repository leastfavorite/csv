// https://ctrpeach.io/posts/cpp20-string-literal-template-parameters/
// https://github.com/mpusz/mp-units/blob/master/src/core/include/mp-units/ext/fixed_string.h
#pragma once

#include <algorithm>
#include <cstddef>

template<typename CharT, size_t Extent>
class FixedString {
public:
    using value_type = CharT;

    static constexpr auto extent = Extent;

    constexpr FixedString(const CharT (&str)[Extent]) {
        std::copy_n(str, Extent, value);
    }

    // must be public to be NTTP
    CharT value[Extent];
};
