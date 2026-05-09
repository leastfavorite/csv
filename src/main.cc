#include "CsvReader.hh"
#include <cstddef>
#include <iostream>

int main() {
    auto csv_result = CsvReader<
        Field<"symbol", std::string>,
        Field<"venue", std::string>,
        Field<"price", double>
    >::from_file("file.txt");

    std::cout << err(csv_result.error()) << std::endl;

    // the pipe dream:
    // auto csv_result = CsvIterator::from_file<
    //     CsvField<"symbol", std::string>,
    //     CsvField<"venue", std::string>,
    //     CsvField<"price", double>
    // >("file.txt");
    // auto entry = csv.next()
    // entry.get<"symbol">() -> std::string
    // entry.get<"price">() -> double
}
