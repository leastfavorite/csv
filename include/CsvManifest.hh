#include "FixedString.hh"
#include <concepts>

template<FixedString Name, typename T>
struct CsvField {
    using value_type = T;
    static constexpr FixedString name = Name;
};


template<class F>
concept Field =
    requires {
        { F::name } -> std::convertible_to<std::basic_string_view<CharT>>;
        typename F::value_type;
    };


template<Field ...Args>
struct CsvManifest {
};
