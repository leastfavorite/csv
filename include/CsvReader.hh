#include "NamedTuple.hh"
#include "CsvErrors.hh"
#include "Field.hh"
#include <expected>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>

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
    from_file(const std::string &filename);

};
