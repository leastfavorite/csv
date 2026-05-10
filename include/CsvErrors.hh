#pragma once

#include "Field.hh"
#include <concepts>
#include <expected>
#include <format>
#include <sstream>
#include <string>
#include <variant>

template <class E>
concept ErrorLike =
    requires(E const &e) {
        // TODO: this is perhaps too constrictive?
        { e.err() } -> std::convertible_to<std::string>;
    };

template <ErrorLike ...Es>
std::string err_msg(const std::variant<Es...> &v) {
    return std::visit([](auto &e){ return e.err(); }, v);
}

template <typename T, ErrorLike ...Es>
std::string err_msg(const std::expected<T, std::variant<Es...>> &r) {
    return err_msg(r.error());
}

template <ErrorLike E>
std::string err_msg(const E &e) {
    return e.err();
}

template <typename T, ErrorLike E>
std::string err_msg(const std::expected<T, E> &r) {
    return err_msg(r.error());
}

// TODO: Streamline error types
// I like the implementation of CsvStreamError--a struct with general
// information, and a std::variant with the details of the specific error
// message. I think FileError (defined in CsvReader) deserves the same
// treatment.

// technically a misnomer--this could also occur if, say, we don't have permissions
struct FileNotFound {
    std::string filename;

    std::string err() const {
        return std::format("Could not open file: '{}'", filename);
    }
};

template <FieldLike ...Fs>
struct MismatchedHeaders {
    std::vector<std::string> found;

    std::string err() const {
        auto format_span = [](const auto &s) {
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

        static constexpr auto annotations = Annotations<Fs...>;

        return std::format(
                "  Mismatch in header schema: expected {}, got {}",
                format_span(annotations), format_span(found)
        );
    }
};

template <size_t Expected>
struct LengthMismatch {
    size_t found;
    std::string err() const {
        return std::format("  Expected {} fields, got {}", Expected, found);
    }
};

template <FieldLike ...Fs>
struct ConversionError {
    std::vector<std::pair<size_t, std::string>> failures;
    std::string err() const {
        static constexpr auto annotations = Annotations<Fs...>;

        std::stringstream ss;

        for (size_t i = 0; i < failures.size(); i++) {
            const auto &[idx, found] = failures[i];

            ss << std::format("  Could not convert field '{}': got '{}'", annotations[idx], found);
            if (i < failures.size() - 1) {
                ss << "\n";
            }
        }

        return ss.str();
    }
};


template <FieldLike ...Fs>
struct CsvStreamError {
    std::string filename;
    std::string line;
    std::size_t lineno;
    std::variant<LengthMismatch<sizeof...(Fs)>, ConversionError<Fs...>> error;

    std::string err() const {
        return std::format(
            "CsvStreamError in '{}' (line {}) while parsing '{}':\n{}",
            filename, lineno, line, err_msg(error)
        );
    }
};
