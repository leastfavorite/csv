#pragma once

#include "FixedString.hh"
#include <concepts>
#include <string_view>

template<FixedString Name, typename T>
struct Field {
    using type = T;
    static constexpr auto name = Name.value;
};

// TODO: allow arbitrary char types
template<class F>
concept FieldLike =
    requires(F const f) {
        { f.name } -> std::convertible_to<std::string_view>;
        typename F::type;
    };

template <FixedString Key, FieldLike... Fs>
consteval std::size_t index_of() {
    std::size_t index = 0;

    bool found = (
        (std::string_view(Fs::name) == std::string_view(Key.value)
        ? true : (++index, false))
    || ...);

    if (!found) {
        return SIZE_T_MAX;
    }

    return index;
}

template<FieldLike ...Fs>
static constexpr std::array<std::string_view, sizeof...(Fs)> Annotations = { Fs::name... };
