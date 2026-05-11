#include "NamedTuple.hh"
#include "CsvErrors.hh"
#include "Field.hh"
#include <expected>
#include <fstream>
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
        using value_type = std::expected<
            NamedTuple<CharT, Fs...>, DereferenceError<CharT, Fs...>>;
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
        value_type operator*();
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

        static constexpr
            std::array<std::string_view, sizeof...(Fs)> annotations =
                FieldAnnotations<CharT, Fs...>;

        std::vector<std::basic_string_view<CharT>> unknown_entries;

        std::array<bool, sizeof ...(Fs)> found_annotation;
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
                    .lineno = 0,
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
                        .lineno = 0,
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
