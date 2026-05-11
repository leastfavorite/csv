#pragma once

#include "FixedString.hh"
#include <utility>

template <FixedString Name, typename Type>
struct Field {
    using value_type = Type;
    using atom_type = typename decltype(Name)::atom_type;

    static constexpr std::basic_string_view<atom_type> name = Name.view();
};

// wild that this is the easiest way to do this. it's very cool, though
template <typename F>
concept is_field = requires {
    { []<auto Name, typename Type>(Field<Name, Type>){}(std::declval<F>()) };
};

template <typename F, typename CharT>
concept is_typed_field =
    is_field<F> && std::same_as<typename F::atom_type, CharT>;

template<typename CharT, is_typed_field<CharT>... Fs>
static constexpr std::array<std::basic_string_view<CharT>, sizeof...(Fs)> Annotations
    = { Fs::name... };
