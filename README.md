# unicode_bridge

A C++ library for Unicode conversion and validation.

- Primarily `constexpr`, C++23
- Contains detailed error messages.

## Requirements

- C++23
- MSVC (tested. GCC/Clang untested)

## Usage

Single header include:

```cpp
#include "unicode_bridge.hpp"
```

And

```cpp
using namespace unicode_bridge;
```

When required.

## Examples and Basic Documentation

The examples below are taken from file `examples/src/main.cpp`. That file, under MSVC and Windows, uses `SetConsoleOutputCP(CP_UTF8)` to ensure the output sent to `std::cout` is rendered properly. 

Unicode bridge has a simple set of exported functions. There are several versions of each function, designed for different circumstances. 

### `unicode_print` object

`unicode_print` is an object used to facilitate the printing of Unicode characters through `std::cout`, using an overload of `operator<<`. 

To be very clear, the premise of the object is simple; if the Unicode string is not in UTF-8 then convert it to UTF-8. Afterwards, cast the UTF-8 string to a `std::string` object, then print it using `std::cout`. If the environment is set up to process UTF-8, then the correct characters should be printed to the console.

`unicode_print` also includes a `str()` function that allows the user to construct the `std::string` object, instead of having it sent to `std::cout`. 

```cpp
std::cout << unicode_print(u8"Hello World! 😀") << std::endl;
std::cout << unicode_print(u"hello world! 😀") << std::endl;
std::cout << unicode_print(L"hello world! 😀") << std::endl;
std::cout << unicode_print(U"hello world! 😀") << std::endl;

std::string str1 = unicode_print(u8"unicode_print doesn't own the strings").str();
std::cout << str1 << std::endl;
```

Note that the constructor to `unicode_print` does not own the argument - if it goes out of scope, then you will get undefined behaviour.

The conversion from UTF-16/32 to UTF-8 performs few checks. If the UTF-16 is invalid, or the UTF-32 characters are in the surrogate range, they are still converted to UTF-8 (though it would technically be invalid UTF-8). UTF-32 characters outside the Unicode range of U+10FFFF become U+FFFD replacement characters.

No validation is performed on the UTF-8 strings - they are passed as-is to the console.

### Convert Unicode to ASCII

Though it may seem trivial (and it is), we include a set of functions to convert Unicode strings to ASCII strings. The return type of `convert_unicode_to_ascii` is  `std::expected<std::string,unicode_to_ascii_error>`, with `std::string` representing success and `unicode_to_ascii_error` representing failure. If any of the input characters do not fit in the basic ASCII range, then a `unicode_to_ascii_error` is returned. 

Below is an example of its use.

```cpp
auto result_1 = convert_unicode_to_ascii(u8"hello 😀");
if (result_1.has_value())
{
    std::cout << result_1.value() << std::endl;
}
else
{
    std::cout << unicode_print(result_1.error().message()) << std::endl;
}
```

It can be used with any Unicode-based character type in C++. There is also a variants that accepts a single character. Of these base two functions, there are a further two variants that return an exception instead of an `std::expected` value.

```cpp
auto result_u16 = convert_unicode_to_ascii(u'h');
auto result_u32 = convert_unicode_to_ascii_with_exception(U"hello");
auto result_wchar = convert_unicode_to_ascii_with_exception(L'h');
```

Below we discuss error types and exceptions in `unicode_bridge`, as they are consistent across all functions. Also note that, across the conversion functions in `unicode_bridge`, this pattern is seen regularly; there are variants of functions for single characters, and variants that throw exceptions instead of returning `std::expected` values.

### Errors and exceptions

Each of the conversion or processing functions in `unicode_bridge` return errors, used for when the input does not adhere to either the Unicode standard, or is outside the range of the expected characters.

The return types of the functions in `unicode_bridge` are generally consistent; they return `std::expected` objects, where the second element is the error. There are also variants that exist which return exceptions.

These exceptions are based on `unicode_bridge_exception`, a templated type which can take any of the error types from `unicode_bridge`. 

### convert_ascii_to_unicode

The opposite of `convert_unicode_to_ascii`, this function will take any argument convertible to a `std::string_view` and process it into a Unicode string. Any invalid `char`'s (outside the basic ASCII range) will ensure the function fails. A template type is used to denote what the result type should be. As before, the return type is an `std::expected`.

```cpp
auto result_u8 = convert_ascii_to_unicode<char8_t>("hello");
auto result_u16 = convert_ascii_to_unicode<char16_t>('h');
auto result_u32 = convert_ascii_to_unicode_with_exception<char32_t>("hello");
auto result_wchar = convert_ascii_to_unicode_with_exception<wchar_t>('h');
if (result_u8.has_value() && result_u16.has_value())
{
    std::cout << "They all passed!" << std::endl;
}
```

### unicode_conversion

Probably the core reason I created this library. This function allows conversion between all the different Unicode types. If the input is invalid Unicode, it returns an error type, in the form of an `std::expected`. 

```cpp
auto r1 = unicode_conversion<char8_t>(u"hello world! 😀");
auto r2 = unicode_conversion_with_exception<char16_t>(L"hello world! 😀");
auto r3 = unicode_conversion<char32_t>(U'😀');
auto r4 = unicode_conversion_with_exception<wchar_t>(U'😀');
```

### is_valid_unicode/is_valid_ascii

These two functions return a bool denoting whether the input is valid Unicode or ASCII.

`is_valid_unicode` will only accept Unicode strings/characters as input, ensuring they are valid Unicode.

`is_valid_ascii` will take any character type, checking if the string/characters are within the ASCII range.

```cpp
auto r1 = is_valid_unicode(U"😀");
// r2 should be false.
auto r2 = is_valid_unicode(static_cast<char8_t>(0xFF));
auto r3 = is_valid_ascii(u8"should be all fine");
auto r4 = is_valid_ascii("as should this");
```

### next_char32

This set of functions, when given two iterators representing some current position and the end of the string-like object, it will return a pair. This pair consists of the next Unicode scalar value from the iterators, and the number of elements of the iterator which were consumed to create that scalar value. If it cannot find a Unicode scalar value, then an error is returned instead.

```cpp
auto str = u8"the string to check";
auto res = next_char32(std::begin(str), std::end(str));
```

### prev_char32

This set of functions, given two iterators representing one-past the current position, and a beginning position, will return a pairo. The pair consists of the previous Unicode scalar value from the iterators, and the number of elements of the iteratr which were consumed to create that scalar value. If it cannot find a Unicode scalar value, then an error is returned. 