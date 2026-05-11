#pragma once

#include "FixedString.hh"
#include "Field.hh"
#include <optional>
#include <tuple>

template <typename CharT, is_field<CharT> ...Fields>
class NamedTuple : std::tuple<typename Fields::value_type...> {
public:
    using atom_type = CharT;
    // no way to DRY this, i guess...
    using tuple_type = std::tuple<typename Fields::value_type...>;

    // use tuple constructors
    using tuple_type::tuple_type;
private:
    template <FixedString Key>
        requires is_typed_fixed_string<Key, CharT>
    static consteval std::optional<size_t> index_of() {
        std::size_t index = 0;

        bool found = (
            (Fields::name == Key.view() ? true : (++index, false))
        || ...);

        if (!found) {
            return std::nullopt;
        }

        return index;
    }
public:
    template <FixedString Key>
        requires ( is_typed_fixed_string<Key, CharT> && index_of<Key>().has_value() )
    constexpr const typename std::tuple_element<index_of<Key>().value(), tuple_type>::type &get() const noexcept {
        return std::get<NamedTuple::index_of<Key>().value()>(*this);
    }

    template <FixedString Key>
        requires ( is_typed_fixed_string<Key, CharT> && index_of<Key>().has_value() )
    constexpr typename std::tuple_element<index_of<Key>().value(), tuple_type>::type &get() noexcept {
        return std::get<NamedTuple::index_of<Key>().value()>(*this);
    }
};
