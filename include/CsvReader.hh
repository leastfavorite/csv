#pragma once

#include "Field.hh"
#include <concepts>
#include <expected>
#include <fstream>
#include <ostream>
#include <span>
#include <sstream>
#include <variant>

// technically a misnomer--this could also occur if, say, we don't have permissions
struct FileNotFound {
    std::string filename;

    const std::string err() const {
        return std::format("Could not open file: '{}'", filename);
    }
};

template <FieldLike ...Fs>
struct HeaderMismatch {
    std::vector<std::string> found;

    const std::string err() const {
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

        static constexpr std::array<std::string_view, sizeof...(Fs)> annotations = { Fs::name... };

        return std::format(
                "Mismatch in header schema: expected {}, got {}",
                format_span(annotations), format_span(found)
        );
    }
};


template <class E>
concept ErrorLike =
    requires(E const &e) {
        { e.err() } -> std::convertible_to<std::string_view>;
    };

template <ErrorLike ...Es>
std::string err(const std::variant<Es...> &v) {
    return std::visit([](auto &e){ return e.err(); }, v);
}

template <FieldLike ...Fs>
class CsvReader {
public:
    using FileError = std::variant<FileNotFound, HeaderMismatch<Fs...>>;
    static std::expected<CsvReader, FileError>from_file(const std::string &filename) {
        auto ifs = std::ifstream(filename);

        if (ifs.fail()) {
            return std::unexpected(FileNotFound { filename });
        }

        return std::unexpected(HeaderMismatch<Fs...> {{"a", "b", "c"}});
    }
private:
    std::istream stream;
};
