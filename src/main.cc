#include <iostream>

#include "FixedString.hh"

int main() {

    std::cout << "hi" << std::endl;
    // the pipe dream:
    // auto csv_result = CsvIterator::from_file<
    //     CsvField<"symbol", std::string>,
    //     CsvField<"venue", std::string>,
    //     CsvField<"price", double>
    // >("file.txt");
}
