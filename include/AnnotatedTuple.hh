#pragma once

#include "FixedString.hh"
#include "Field.hh"

#include <array>
#include <tuple>

template <FieldLike... Fs>
class AnnotatedTuple : public std::tuple<typename Fs::type...> {
private:
    // inherit tuple constructors
    using Tuple = std::tuple<typename Fs::type...>;
    using Tuple::Tuple;


public:
    static constexpr std::array<std::string_view, sizeof...(Fs)> annotations = { Fs::name... };

    // TODO: this one's got a lot of cousins
    template <FixedString Key>
    constexpr const typename std::tuple_element<index_of<Key, Fs...>(), Tuple>::type &get() const {
        return std::get<index_of<Key, Fs...>(), typename Fs::type...>(*this);
    }

    template <FixedString Key>
    constexpr typename std::tuple_element<index_of<Key, Fs...>(), Tuple>::type &get() {
        return std::get<index_of<Key, Fs...>(), typename Fs::type...>(*this);
    }
};
