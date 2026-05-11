#pragma once

#include "Field.hh"
#include <format>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

// TODO: it might makes more sense to have .message() and its associate .err()s
// supply an ostream << operator instead.

template <typename CharT, typename T>
consteval std::string __format_span(const T &s) {
    std::basic_stringstream<CharT> ss;
    ss << '[';

    for (size_t i = 0; i < s.size(); i++) {
        if (i > 0) {
            ss << ", ";
        }
        ss << s[i];
    }

    ss << ']';

    return ss.str();
};

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
struct UnknownHeader {
    static constexpr auto Name = "Unexpected header entry";
    std::vector<std::basic_string_view<CharT>> failures;

    template <is_field<CharT> ...Fs>
    std::basic_string<CharT> err() const {
        return std::format("Could not find header(s) {} in expected set {}",
            __format_span(failures),
            __format_span(FieldAnnotations<CharT, Fs...>));
    }
};

template <typename CharT>
struct HeaderNotFound {
    static constexpr std::basic_string<CharT> Name = "Required headers not found";

    std::vector<size_t> indices;

    template <is_field<CharT> ...Fs>
    std::basic_string<CharT> err() const {
        std::basic_stringstream<CharT> ss;
        for (size_t i = 0; i < indices.size(); i++) {
            ss << "Could not find header '"
               << FieldAnnotations<CharT, Fs...>[indices[i]];
            if (i < indices.size() - 1) {
                ss << "\n";
            }
        }
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

template<typename CharT, is_field<CharT> ...Fs>
using FromFileError = CsvError<
        CharT,
        "from_file",
        std::variant<FileNotFound<CharT>, UnknownHeader<CharT>, HeaderNotFound<CharT>>,
        Fs...>;

template<typename CharT, is_field<CharT> ...Fs>
using DereferenceError = CsvError<
        CharT,
        "operator*",
        std::variant<LengthMismatch<CharT>, CouldNotConvert<CharT>>,
        Fs...>;
