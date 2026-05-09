#pragma once
#include "Field.hh"

template<Field ...Fs>
struct CsvManifest {
    static consteval auto names() {
        return std::array<std::string_view, sizeof...(Fs)>{ Fs::name.value... };
    }
};
