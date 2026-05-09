#pragma once

#include "FixedString.hh"
#include "Field.hh"

#include <tuple>

template <FieldLike... Fs>
class AnnotatedTuple : public std::tuple<typename Fs::type...> {
    using Tuple = std::tuple<typename Fs::type...>;
    // inherit tuple constructors
    using Tuple::Tuple;

    public:
        template <FixedString Key>
        constexpr const typename std::tuple_element<index_of<Key, Fs...>(), Tuple>::type &get() {
            return std::get<index_of<Key, Fs...>(), typename Fs::type...>(*this);
        }
};
