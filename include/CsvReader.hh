#pragma once

#include "Field.hh"
#include <algorithm>
#include <concepts>
#include <expected>
#include <fstream>
#include <optional>
#include <ostream>
#include <ranges>
#include <sstream>
#include <string>
#include <variant>

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

template <class E>
concept ErrorLike =
    requires(E const &e) {
        { e.err() } -> std::convertible_to<std::string>;
    };

template <ErrorLike ...Es>
std::string err(const std::variant<Es...> &v) {
    return std::visit([](auto &e){ return e.err(); }, v);
}

template <FieldLike ...Fs>
class CsvReader {
public:
    using FileError = std::variant<FileNotFound, EmptyFile, MismatchedHeaders<Fs...>>;

    static std::expected<CsvReader, FileError>from_file(const std::string &filename) {
        auto ifs = std::ifstream(filename);

        if (ifs.fail()) {
            return std::unexpected(FileNotFound { filename });
        }


        // we don't fail on an empty file here--that's treated as a special case
        // of MismatchedHeaders
        std::string header;
        std::getline(ifs, header);


        // we own the strings here specifically for error-type handling.
        auto tokens = std::ranges::views::split(header, ",")
            | std::views::transform([](auto &&s) -> std::string { return std::move(s.data()); })
            | std::ranges::to<std::vector>();

        // thunk.
        //
        // TODO: it probably makes sense to split this error type
        // into its component cases (HeaderSizeMismatch, MissingHeader, ExtraHeader).
        //
        // that would bring about a reasonable API for allowing headers in the
        // CSV that don't exist in Fs, similar to Zod's looseObject:
        // https://zod.dev/api#zlooseobject
        const auto mismatched_headers = [&]() -> std::expected<CsvReader, FileError> {
            return std::unexpected(MismatchedHeaders<Fs...>{ std::move(tokens) });
        };

        static constexpr std::array<std::string_view, sizeof...(Fs)> annotations = Annotations<Fs...>;

        if (tokens.size() != annotations.size()) {
            return mismatched_headers();
        }


        // O(n^2) !!! we could speed this up with a hash table or something
        // but i expect key arrays to be tiny (<100 entries)
        auto lookup = std::array<size_t, sizeof...(Fs)>();

        auto find_lookup_idx = [&tokens](const std::string_view &s) -> std::optional<size_t> {
            for (size_t i = 0; i < tokens.size(); i++) {
                if (s == tokens[i]) return i;
            }
            return std::nullopt;
        };

        for (size_t i = 0; i < sizeof...(Fs); i++) {
            auto idx = find_lookup_idx(annotations[i]);
            if (!idx) {
                return mismatched_headers();
            }
            lookup[i] = *idx;
        }

        return CsvReader(std::move(lookup), std::move(ifs));
    }
private:
    CsvReader(std::array<size_t, sizeof...(Fs)> &&lookup_, std::ifstream &&stream_):
        lookup(std::move(lookup_)), stream(std::move(stream_)) {}

    // we support shuffled keys. lookup[k] tells us which index in the csv
    // has a header that matches Fs[k]
    std::array<size_t, sizeof...(Fs)> lookup;

    // note: likely worthwhile to switch to a templated std::istream to allow
    // reading from other input stream types.
    std::ifstream stream;
};
