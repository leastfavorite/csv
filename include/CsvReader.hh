#include "NamedTuple.hh"
#include "CsvErrors.hh"
#include "Field.hh"
#include <expected>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <string>
#include <ranges>

template <typename CharT, is_field<CharT> ...Fs>
class BasicCsvReader : public std::enable_shared_from_this<BasicCsvReader<CharT, Fs...>> {
private:
    using csv_ptr = std::shared_ptr<BasicCsvReader<CharT, Fs...>>;

    std::array<size_t, sizeof...(Fs)> lookup;

    std::basic_ifstream<CharT> ifs;
    std::basic_string<CharT> line;

    const std::string filename;
    size_t lineno = 1;

    struct Private { explicit Private() = default; };

    void next() {
        if (ifs) {
            std::getline(ifs, line);
            lineno++;
        }
    }

public:
    class Iterator {
    public:
        using tuple_type = NamedTuple<CharT, Fs...>;
        using value_type = std::expected<
            tuple_type, DereferenceError<CharT, Fs...>>;
        using difference_type = std::size_t;

        Iterator(csv_ptr &&ptr_): ptr(std::move(ptr_)) {}

        Iterator &operator++() {
            ptr->next(); return *this;
        }
        // functionally identical to ++it
        Iterator &operator++(int) {
            auto result = *this;
            ptr->next();
            return result;
        }
        bool operator==( std::default_sentinel_t const & ) {
            return ( ptr->ifs.eof() || ptr->ifs.fail() );
        }

        // TODO
        value_type operator*() {
            auto tokens = std::string_view(ptr->line)
                | std::views::split(',')
                | std::ranges::to<std::vector>();

            if (tokens.size() != sizeof...(Fs)) {
                return std::unexpected(
                    DereferenceError<CharT, Fs...> {
                        .filename = ptr->filename,
                        .line = ptr->line,
                        .lineno = ptr->lineno,
                        .error = LengthMismatch<CharT, Fs...> {
                            tokens.size()
                        }
                    }
                );
            }

            tuple_type result = {};

            std::vector<std::pair<size_t, std::basic_string_view<CharT>>>
                failed_conversions;

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
                std::basic_istringstream<CharT> ss(
                        std::basic_string<CharT>(s.begin(), s.end()));
                ss >> std::get<Idx>(result);

                bool success = ss.eof() && !ss.fail();
                if (!success) {
                    failed_conversions.emplace_back(
                            Idx, std::basic_string_view<CharT>(s));
                }
                return success;
            };

            auto apply_all = []<size_t ...Idxs>(decltype(apply) &f, std::index_sequence<Idxs...>) {
                return (f.template operator()<Idxs>() && ...);
            };

            if (!apply_all(apply, std::make_index_sequence<sizeof...(Fs)> {})) {
                return std::unexpected(
                    DereferenceError<CharT, Fs...> {
                        .filename = ptr->filename,
                        .line = ptr->line,
                        .lineno = ptr->lineno,
                        .error = CouldNotConvert<CharT, Fs...> {
                            failed_conversions
                        }
                    }
                );
            }

            return result;
        }
    private:
        csv_ptr ptr;
    };

    Iterator begin() {
        if (lineno < 2) {
            next();
        }
        return Iterator { this->shared_from_this() };
    }
    std::default_sentinel_t end() { return {}; }

    BasicCsvReader(
        std::array<size_t, sizeof...(Fs)> &&lookup_,
        std::basic_ifstream<CharT> ifs_,
        const std::string &filename_,
        Private
    ): lookup(std::move(lookup_)), ifs(std::move(ifs_)), filename(filename_) {}

    static std::expected<csv_ptr, FromFileError<CharT, Fs...>>
    from_file(const std::string &filename) {
        auto ifs = std::basic_ifstream<CharT>(filename);

        if (ifs.fail()) {
            return std::unexpected(
                FromFileError<CharT, Fs...> {
                    .filename = filename,
                    .line = "",
                    .lineno = 0,
                    .error = FileNotFound<CharT>{}
                }
            );
        }

        // we don't fail on an empty file here--that's treated as a special case
        // of MismatchedHeaders
        std::basic_string<CharT> header;
        std::getline(ifs, header);

        auto entries = std::basic_string_view<CharT>(header)
            | std::views::split(',')
            | std::ranges::to<std::vector>();

        // we don't check for length matching. this removes an early-out case
        // but gives us better errors

        static constexpr auto annotations = FieldAnnotations<CharT, Fs...>;

        std::vector<std::basic_string<CharT>> unknown_entries;

        std::array<bool, sizeof ...(Fs)> found_annotation;
        found_annotation.fill(false);

        std::array<std::size_t, sizeof...(Fs)> lookup;

        for (size_t i = 0; i < entries.size(); i++) {
            const auto &entry = std::basic_string_view<CharT>(entries[i]);

            bool found = false;
            for (size_t ann_idx = 0; ann_idx < sizeof...(Fs); ann_idx++) {
                if (annotations[ann_idx] == entry) {
                    found_annotation[ann_idx] = true;
                    lookup[ann_idx] = i;
                    found = true;
                    break;
                }
            }

            if (!found) {
                unknown_entries.emplace_back(entry);
            }
        }

        if (unknown_entries.size() > 0) {
            // TODO: we don't return a string_view to the header line because
            // we're going to discard its owner at the end of this function.
            // it likely makes sense for FromFileError to have a distinct
            // structure to DereferenceError so it can own this information
            return std::unexpected(
                FromFileError<CharT, Fs...>{
                    .filename = filename,
                    .line = "",
                    .lineno = 1,
                    .error = UnknownHeader<CharT, Fs...> { unknown_entries }
                }
            );
        }

        for (bool found : found_annotation) {
            if (!found) {
                return std::unexpected(
                    FromFileError<CharT, Fs...>{
                        .filename = filename,
                        .line = "",
                        .lineno = 1,
                        .error = HeaderNotFound<CharT, Fs...>
                            { found_annotation }
                    }
                );
            }
        }

        return std::make_shared<BasicCsvReader<CharT, Fs...>>(
            std::move(lookup),
            std::move(ifs),
            filename,
            Private()
        );
    };

};

template<is_field<char> ...Fs>
using CsvReader = BasicCsvReader<char, Fs...>;

template<is_field<wchar_t> ...Fs>
using WideCsvReader = BasicCsvReader<wchar_t, Fs...>;
