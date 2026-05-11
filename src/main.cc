#include "NamedTuple.hh"
#include <iostream>
#include "CsvErrors.hh"

template<class CharT, class ...Fs>
using FromFileError = CsvError<CharT, "from_file", std::variant<
    FileNotFound<CharT>, LengthMismatch<CharT>, MismatchedHeader<CharT>>,
Fs...>;

int main() {
    auto n = NamedTuple<
        char,
        Field<"symbol", std::string>,
        Field<"test", double>
    >();

    auto error = FromFileError<char,
        Field<"symbol", std::string>,
        Field<"test", double>
    >();
    std::cout << error.message() << std::endl;

    std::cout << n.get<"symbol">() << std::endl;
}

// int main(int argc, char *argv[]) {
//
//     if (argc < 2) {
//         std::cerr << "Usage: csv <file.csv>" << std::endl;
//         return -1;
//     }
//
//     auto csv_result = CsvReader<char,
//         Field<"symbol", std::string>,
//         Field<"venue", std::string>,
//         Field<"price", double>
//     >::from_file(argv[1]);
//
//     if (!csv_result) {
//         std::cerr << err_msg(csv_result) << std::endl;
//         return -1;
//     }
//
//     for (const auto &entry_result : **csv_result) {
//         if (!entry_result) {
//             std::cerr << "\n" << err_msg(entry_result) << "\n" << std::endl;
//             continue;
//         }
//
//         // entry derives std::tuple<std::string, std::string, double>...
//         const auto &entry = *entry_result;
//
//         // ...with additional compile-time .get() to retrieve values by name
//         std::cout << std::format("symbol: '{}', venue: '{}', price * 2: {}",
//             entry.get<"symbol">(),
//             entry.get<"venue">(),
//             entry.get<"price">() * 2) << std::endl;
//     }
// }
