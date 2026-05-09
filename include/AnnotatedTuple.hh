#include "CsvManifest.hh"
#include "FixedString.hh"
#include <tuple>

template <Field... Fs>
class AnnotatedTuple : public std::tuple<typename Fs::value_type...> {
    using tuple_type = std::tuple<typename Fs::value_type...>;

    public:
        template <FixedString Key>
            requires (
                (index_of<Key, Fs...>() < SIZE_T_MAX)
            )
        constexpr const typename std::tuple_element<index_of<Key, Fs...>, tuple_type> &get() {
            return std::get<index_of<Key, Fs...>()>(*this);
        }
};
