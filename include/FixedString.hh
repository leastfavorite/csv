// adapted from
// https://ctrpeach.io/posts/cpp20-string-literal-template-parameters/
//
// additional reading:
// https://github.com/mpusz/mp-units/blob/4161608a92e6d997d49b78ccd795550191fc80e3/src/core/include/mp-units/ext/fixed_string.h

#pragma once

#include <algorithm>
#include <cstddef>

// TODO: FixedString has the ability to use different char atoms,
// but all specialization in this library assumes convertible_to<string_view>,
// which locks us into chars.
//
// To get real specialization, it'd be worthwhile to follow the basic_[] pattern
// used by built-in string types.
//
// Also perhaps worthwhile when doing that: user-defined literal syntax,
// seen in section 5.6 of the class-type NTTP proposal:
//
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0732r2.pdf

// A compile-time string.
template<typename CharT, size_t Extent>
class FixedString {
public:
    using value_type = CharT;

    static constexpr auto extent = Extent;
    static constexpr auto size = Extent - 1;

    constexpr FixedString(const CharT (&str)[Extent]) {
        std::copy_n(str, Extent, value);
    }

    // must be public to be NTTP
    CharT value[Extent];
};
