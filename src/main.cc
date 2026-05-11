#include "CsvReader.hh"
#include <iostream>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: csv <file.csv>" << std::endl;
        return -1;
    }

    auto csv_result = CsvReader<
        Field<"symbol", std::string>,
        Field<"venue", std::string>,
        Field<"price", double>
    >::from_file(argv[1]);

    if (!csv_result) {
        std::cerr << csv_result.error().message() << std::endl;
        return -1;
    }

    for (const auto &entry_result : **csv_result) {
        if (!entry_result) {
            std::cerr << "\n" << entry_result.error().message() << "\n" << std::endl;
            continue;
        }

        // entry derives std::tuple<std::string, std::string, double>...
        const auto &entry = *entry_result;

        // ...with additional compile-time .get() to retrieve values by name
        std::cout << std::format("symbol: '{}', venue: '{}', price * 2: {}",
            entry.get<"symbol">(),
            entry.get<"venue">(),
            entry.get<"price">() * 2) << std::endl;
    }
}
