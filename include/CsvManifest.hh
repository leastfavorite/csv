#pragma once

#include "FixedString.hh"
#include <array>
#include <concepts>
#include <string_view>

template<FixedString Name, typename T>
struct CsvField {
    using value_type = T;
    static constexpr FixedString name = Name;
};


// TODO: right now this doesnt allow arbitrary char types
template<class F>
concept Field =
    requires(F const f) {
        { f.name.value } -> std::convertible_to<std::string_view>;
        typename F::value_type;
    };


template<Field ...Fs>
struct CsvManifest {
    static consteval auto names() {
        return std::array<std::string_view, sizeof...(Fs)>{ Fs::name.value... };
    }
};
