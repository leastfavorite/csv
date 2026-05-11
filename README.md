# csv
A C++23 comma-separated value parser. Note that this is *extremely* early in
development. Note also that I didn't know how to do any of this on Friday.

## Usage

To open a file, the static method `CsvReader<>::from_file(const std::string &filename)` is provided.
```cpp
auto csv_result = CsvReader<
    Field<"symbol", std::string>,
    Field<"venue", std::string>,
    Field<"price", double>
>::from_file("symbols.csv");
```

As with many functions in this library, this returns a result type. The `err_msg` function
is provided to parse error types into human-readable strings:

```cpp
auto not_found = CsvReader<Field<"example", std::string>>::from_file("not_found.csv");
if (!not_found) {
    std::cout << not_found.error().message() << std::endl;
}
// Example output:
// CSV Error in from_file: File not found at test.csv:0.
// Could not open file.

auto incorrect_headers = CsvReader<
    Field<"smybol", std::string>,
    Field<"venue", std::string>,
    Field<"price", double>
>::from_file("symbols.csv");
if (!incorrect_headers) {
    std::cout << not_found.error().message() << std::endl;
}

// Example output:
// CSV Error in from_file: Unexpected header entry at test.csv:1.
// Could not find header(s) [symbol] in expected set [smybol, venue, price]
```

With a valid call to `CsvReader::from_file`, you may iterate through entries:
```cpp
assert(csv_result.has_value());
for (const auto &entry_result : *csv_result) {
    if (!entry_result) {
        break;
    }

    // entry derives from std::tuple<std::string, std::string, double>
    auto &entry = *entry_result;

    // so you can access fields by index, like this:
    const std::string &symbol = std::get<0>(entry);

    // or by field name, like this:
    double price = entry.get<"price">();

    // misspelled field names will not compile:
    const std::string &venue = entry.get<"venyue">(); // compilation error
}
```

Entries return a result type too, as any line can fail for a variety of ways.
Common `CsvStreamError` messages are provided below:

```
CSV Error in operator*: Conversion Error at test.csv:4.
  4: CCCCCCCC,ven1,banana
Could not convert field 'price': got 'banana'

CSV Error in operator*: Mismatched header length at test.csv:6.
  6: TooShort,ven2
Expected 3 fields, got 2

CSV Error in operator*: Mismatched header length at test.csv:8.
  8: VeryLong,ven2,1.3,apple
Expected 3 fields, got 4
```
