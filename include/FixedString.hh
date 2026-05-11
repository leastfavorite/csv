#pragma once

#include <algorithm>
#include <cstddef>
#include <string_view>

// NOTE: some very cool work is being done to make this type irrelevant.
// see P3094.
template <typename CharT, size_t N>
struct FixedString {
    using atom_type = CharT;

    static constexpr size_t Extent = N;
    static constexpr size_t Length = N - 1;

    constexpr FixedString(const CharT (&str)[N]) {
        std::copy_n(str, N, value);
    };

    constexpr std::basic_string_view<CharT> view() const {
        return std::basic_string_view<CharT>(value, N);
    }

    CharT value[N];
};

template <FixedString S, typename CharT>
concept is_typed_fixed_string = std::same_as<typename decltype(S)::atom_type, CharT>;
