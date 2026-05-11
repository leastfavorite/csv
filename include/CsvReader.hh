
#include "CsvErrors.hh"
#include "Field.hh"
#include <expected>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>

template <typename CharT, is_field<CharT> ...Fs>
class BasicCsvReader : public std::enable_shared_from_this<BasicCsvReader<CharT, Fs...>> {
public:
    template <typename CharTT, is_field<CharTT> ...Fss>
    friend bool operator==(BasicCsvReader<CharTT, Fss...> const &, std::default_sentinel_t const & );

    static std::expected<
        std::shared_ptr<BasicCsvReader>,
        FromFileError<CharT, Fs...>>
    from_file(const std::string &filename);

private:
    std::array<size_t, sizeof...(Fs)> lookup;

    std::basic_ifstream<CharT> stream;
    std::basic_string<CharT> line;

    const std::string filename;
    size_t lineno = 1;
};

template <typename CharT, is_field<CharT> ...Fs>
bool operator==( BasicCsvReader<CharT, Fs...> const & lhs, std::default_sentinel_t const & ) {
    return lhs.stream == nullptr;
};
