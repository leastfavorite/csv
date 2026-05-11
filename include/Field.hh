#pragma once

#include "FixedString.hh"
#include <utility>

template <FixedString Name, typename Type>
struct Field {
    using atom_type = typename decltype(Name)::atom_type;
    using value_type = Type;

    static constexpr auto name = Name.value;
};

template <typename F, typename CharT>
concept is_field = requires {
    { []<auto Name, typename Type>(Field<Name, Type>){}(std::declval<F>()) };
    std::same_as<typename F::atom_type, CharT>;
};

template<typename CharT, is_field<CharT> ...Fs>
static constexpr std::array<std::basic_string_view<CharT>, sizeof...(Fs)> FieldAnnotations = { Fs::name... };
