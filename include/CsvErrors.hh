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
        // TODO: this is perhaps a too constrictive?
        // in practice, seems we just want existence of operator<<
        { e.err() } -> std::convertible_to<std::string>;
    };

template <ErrorLike ...Es>
std::string err_msg(const std::variant<Es...> &v) {
    return std::visit([](auto &e){ return e.err(); }, v);
}

template <typename T, ErrorLike ...Es>
std::string err_msg(const std::expected<T, std::variant<Es...>> &e) {
    return err_msg(e.error());
}

// technically a misnomer--this could also occur if, say, we don't have permissions
struct FileNotFound {
    std::string filename;

    std::string err() const {
        return std::format("Could not open file: '{}'", filename);
    }
};

struct EmptyFile {
    std::string filename;
    std::string err() const {
        return std::format("Requested file was empty: '{}'", filename);
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
                "Mismatch in header schema: expected {}, got {}",
                format_span(annotations), format_span(found)
        );
    }
};

