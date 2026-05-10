#pragma once

#include "AnnotatedTuple.hh"
#include "CsvErrors.hh"
#include "Field.hh"
#include <algorithm>
#include <expected>
#include <fstream>
#include <iterator>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>
#include <variant>

// technically a misnomer--this could also occur if, say, we don't have permissions
// TODO: enforce in concept--Tuple must be trivially default initializable, see operator*
template <FieldLike ...Fs>
class CsvReader {
public:
    using Tuple = AnnotatedTuple<Fs...>;
    using FileError = std::variant<FileNotFound, EmptyFile, MismatchedHeaders<Fs...>>;

    // so, gut feeling is that of all the wrong things i'm doing, this here
    // is probably the wrong-est.
    class Iterator {
    public:
        Iterator (CsvReader<Fs...> *ptr_): ptr(ptr_) {}

        using value_type = std::expected<Tuple, CsvStreamError<Fs...>>;
        using difference_type = std::size_t;

        Iterator &operator++() {
            ptr->next();
            return *this;
        }

        Iterator &operator++(int) {
            auto result = *this;
            ptr->next();
            return result;
        }

        bool operator==( std::default_sentinel_t const & ) {
            return ( ptr->stream.eof() );
        }

        value_type operator*() {
            auto tokens = std::string_view(ptr->line)
                | std::views::split(',')
                | std::ranges::to<std::vector>();

            if (tokens.size() != sizeof...(Fs)) {
                return std::unexpected(CsvStreamError<Fs...> {
                    .line = ptr->line,
                    .lineno = ptr->lineno,
                    .filename = ptr->filename,
                    .error = LengthMismatch<sizeof...(Fs)> { tokens.size() }
                });
            }

            Tuple result = {};
            std::vector<std::pair<size_t, std::string>> failed_conversions {};

            // some rather ugly syntax for compile-time loop unrolling--
            // 'template for' sure will be nice in C++26 :)
            //
            // https://stackoverflow.com/questions/71586051/enumerating-a-pack
            // https://youtu.be/15etE6WcvBY?t=2670
            auto apply = [&]<size_t Idx>() {
                const auto &s = tokens[ptr->lookup[Idx]];

                // TODO: this istringstream necessitates a copy.
                // it's likely worthwhile to create some sort of partially
                // specialized converter function that can deal with both
                // string types (views and std::strings) as well as
                // integral and float types (using std::from_chars).
                std::istringstream ss(std::string(s.begin(), s.end()));
                ss >> std::get<Idx>(result);

                bool success = ss.eof() && !ss.fail();
                if (!success) {
                    failed_conversions.emplace_back(
                            Idx, std::string(s.begin(), s.end()));
                }
                return success;
            };

            auto apply_all = []<size_t ...Idxs>(decltype(apply) &f, std::index_sequence<Idxs...>) {
                return (f.template operator()<Idxs>() && ...);
            };
            if (!apply_all(apply, std::make_index_sequence<sizeof...(Fs)> {})) {
                // WARNING - Tuple result is partially constructed here
                return std::unexpected(CsvStreamError<Fs...> {
                    .line = ptr->line,
                    .lineno = ptr->lineno,
                    .filename = ptr->filename,
                    .error = ConversionError<Fs...> { failed_conversions }
                });
            }

            return result;
        }

    private:
        CsvReader<Fs...> *ptr;
    };

    // i think the prickliest part of this whole implementation for me has been
    // this iterator--a weird combination of old and new C++. eager to talk
    // to someone smarter than me about better options for impl here.

    Iterator begin() {
        if (lineno < 2) {
            next();
        }
        return Iterator { this };
    };

    std::default_sentinel_t end() { return {}; }


    void next() {
        if (stream) {
            getline(stream, line);
            lineno++;
        }
    }

    static std::expected<CsvReader, FileError> from_file(const std::string &filename) {
        auto ifs = std::ifstream(filename);

        if (ifs.fail()) {
            return std::unexpected(FileNotFound { filename });
        }

        // we don't fail on an empty file here--that's treated as a special case
        // of MismatchedHeaders
        std::string header;
        std::getline(ifs, header);

        // we copy the strings here specifically for error-type handling.
        // it's cheap, we only do it once
        auto tokens = std::string_view(header)
            | std::views::split(',')
            | std::views::transform([](auto &&s){ return std::string(s.begin(), s.end()); })
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

        return CsvReader(
            std::move(lookup),
            std::move(ifs),
            filename
        );
    }

    auto iter() {
    }

    template <FieldLike ...Fss>
    friend bool operator==( CsvReader<Fss...> const & lhs, std::default_sentinel_t const & );

private:
    CsvReader(std::array<size_t, sizeof...(Fs)> &&lookup_, std::ifstream &&stream_, const std::string &filename_):
        lookup(std::move(lookup_)), stream(std::move(stream_)), filename(filename_) {}

    // we support shuffled keys. lookup[k] tells us which index in the csv
    // has a header that matches Fs[k]
    std::array<size_t, sizeof...(Fs)> lookup;

    // TODO: switch to a templated istream.
    // that way we can support csv from strings
    std::ifstream stream;
    std::string line;

    // for errors
    std::string filename;
    size_t lineno = 1;
};

template <FieldLike ...Fs>
bool operator==( CsvReader<Fs...> const & lhs, std::default_sentinel_t const & ) {
    return lhs.stream == nullptr;
}
