#include <iostream>

#include "FixedString.hh"
#include "CsvManifest.hh"

int main() {

    CsvManifest<
        CsvField<"symbol", std::string>,
        CsvField<"venue", std::string>,
        CsvField<"price", double>
    > k;

    auto l = k.names();

    for (const auto i : l) {
        std::cout << i << std::endl;
    }

    std::cout << "hi" << std::endl;
    // the pipe dream:
    // auto csv_result = CsvIterator::from_file<
    //     CsvField<"symbol", std::string>,
    //     CsvField<"venue", std::string>,
    //     CsvField<"price", double>
    // >("file.txt");
}
