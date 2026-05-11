#pragma once

#include "Field.hh"
#include <format>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

template <typename CharT, FixedString Ctx, typename V, typename ...Fs>
struct CsvError {
    using atom_type = decltype(Ctx)::atom_type;

    std::string_view filename;
    std::basic_string<atom_type> line;
    std::size_t lineno;
    V error;

    std::basic_string<atom_type> message() const {
        const auto err = std::visit([](const auto &e) {
            return e.template err<Fs...>();
        }, error);

        const auto name = std::visit([]<typename T>(const T &)
                -> std::basic_string_view<atom_type> {
            return T::Name;
        }, error);

        return std::format(
                "CSV Error in {}: {} at {}:{}.\n  {}: {}\n{}",
                Ctx.view(), name, filename, lineno, lineno, line, err
        );
    }
};

template <typename CharT>
struct FileNotFound {
    static constexpr std::basic_string<CharT> Name = "File not found";

    template <is_field<CharT> ...Fs>
    std::basic_string<CharT> err() const { return "Could not open file."; }
};

template <typename CharT>
struct MismatchedHeader {
    static constexpr auto Name = "Mismatched header entries";

    std::vector<std::basic_string_view<CharT>> entries;

    template <is_field<CharT> ...Fs>
    std::basic_string<CharT> err() const {
        static constexpr auto format_span = [](const auto &s) {
            std::stringstream result;

            result << "[";
            for (size_t i = 0; i < s.size(); i++) {
                if (i > 0) {
                    result << ", ";
                }
                result << s[i];
            }
            result << "]";

            return result.str();
        };

        return std::format(
                "  Mismatch in header schema: expected {}, got {}",
                format_span(FieldAnnotations<CharT, Fs...>),
                format_span(entries)
        );
    }
};

template <typename CharT>
struct LengthMismatch {
    static constexpr auto Name = "Mismatched header length";

    size_t length;

    template <is_field<CharT> ...Fs>
    std::basic_string<CharT> err() const {
        return std::format("Expected {} fields, got {}", sizeof...(Fs), length);
    }
};

template <typename CharT>
struct CouldNotConvert {
    static constexpr auto Name = "Conversion Error";

    std::vector<std::pair<size_t, std::basic_string_view<CharT>>> failures;

    template <is_field<CharT> ...Fs>
    std::basic_string<CharT> err() const {

        std::basic_stringstream<CharT> ss;

        for (size_t i = 0; i < failures.size(); i++) {
            const auto &[idx, found] = failures[i];

            ss << std::format("Could not convert field '{}': got '{}'",
                    FieldAnnotations<Fs...>[idx], found);
            if (i < failures.size() - 1) {
                ss << "\n";
            }
        }

        return ss.str();
    }
};
