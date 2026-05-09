#include "FixedString.hh"
#include <concepts>
#include <string_view>

template<FixedString Name, typename T>
struct CsvField {
    using value_type = T;
    static constexpr FixedString name = Name;
};

// FIXME: right now this doesnt allow arbitrary char types
template<class F>
concept Field =
    requires(F const f) {
        { f.name.value } -> std::convertible_to<std::string_view>;
        typename F::value_type;
    };

template <FixedString Key, Field... Fs>
consteval std::size_t index_of() {
    std::size_t index = 0;

    bool found = ((Fs::name.value == Key.value ? true : (++index, false)) || ...);

    if (!found) {
        return SIZE_T_MAX;
    }

    return index;
}
