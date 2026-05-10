#include "CsvErrors.hh"
#include "CsvReader.hh"
#include <cstddef>
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
        std::cerr << err_msg(csv_result) << std::endl;
        return -1;
    }

    for (const auto &o : *csv_result) {
        if (!o) {
            std::cerr << err_msg(o.error()) << std::endl;
            continue;
        }

        const auto result = *o;
        std::cout << result.get<"price">() + 1 << std::endl;
    }
}
