# csv
A C++23 comma-separated value parser.

## Basic Usage

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
    std::cout << err_msg(not_found) << std::endl;
}

// Example output:
// Could not open file: 'not_found.csv'

auto incorrect_headers = CsvReader<
    Field<"smybol", std::string>,
    Field<"venue", std::string>,
    Field<"price", double>
>::from_file("symbols.csv");

// Example output:
// Mismatch in header schema: expected [smybol, venue, price], got [symbol, venue, price]
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
}
```

Entries return a result type too, as any line can fail for a variety of ways.
Common `CsvStreamError` messages are provided below:
```
CsvStreamError in 'test.csv' (line 4) while parsing 'SYMBOLNAME,VENUE,foo':
  Could not convert field 'price': got 'foo'

CsvStreamError in 'test.csv' (line 6) while parsing 'SYMBOLNAME,VENUE':
  Expected 3 fields, got 2

CsvStreamError in 'test.csv' (line 8) while parsing 'SYMBOLNAME,VENUE,0.6,bar':
  Expected 3 fields, got 4
```
