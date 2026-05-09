#include <cstddef>
#include <iostream>

#include "AnnotatedTuple.hh"
#include "Field.hh"

int main() {
    auto g = AnnotatedTuple<
        Field<"firstName", std::string>,
        Field<"lastName", std::string>,
        Field<"age", size_t>
    >("Bill", "Nye", 50);

    // mapping is resolved at compile time!!
    std::cout << g.get<"firstName">() << std::endl;


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
