#include <iostream>

#include "AnnotatedTuple.hh"

int main() {

    AnnotatedTuple<
        CsvField<"symbol", std::string>,
        CsvField<"venue", std::string>,
        CsvField<"price", double>
    > k;

    auto l = k.get<"symbol">();

    for (const auto i : l) {
        std::cout << i << std::endl;
    }

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
