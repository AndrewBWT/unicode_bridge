# unicode_bridge

A primarily `constexpr` C++23 library for Unicode conversion and validation.

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

`unicode_print` is an object used to facilitate the printing of Unicode characters through `std::cout` through an overload of `operator<<`. 

To be very clear, its premise is simple; if the Unicode string is not in UTF-8 then convert to UTF-8. Afterwards, cast the UTF-8 string to a `std::string` object, then print through `std::cout`. If the environment is set up to process UTF-8, then the correct characters should be printed to the console.

`unicode_print` also includes a `str()` function that allows the user to construct the string, instead of it being sent to `std::cout`. 

```cpp
std::cout << unicode_print(u8"Hello World! 😀") << std::endl;
std::cout << unicode_print(u"hello world! 😀") << std::endl;
std::cout << unicode_print(L"hello world! 😀") << std::endl;
std::cout << unicode_print(U"hello world! 😀") << std::endl;

std::string str1 = unicode_print(u8"unicode_print doesn't own the strings").str();
std::cout << str1 << std::endl;
```

Note that the constructor to `unicode_print` does not own the argument - if it goes out of scope, then you will get undefined behaviour.

The conversion from UTF-16/32 to UTF-8 performs few checks. If the UTF-16 is invalid, or the UTF-32 characters are in the surrogate range, they are still converted to UTF-8 (though it would technically be invalid UTF-8). UTF-32 characters outside the Unicode range of U+10FFFF become U+uFFFD replacement characters.

No validation is performed on the UTF-8 strings - they are passed as-is to the console.

### Convert Unicode to ASCII

Though it may seem trivial (and it is), we include a set of functions to convert Unicode strings to ASCII strings. If any of the characters do not fit in the basic ASCII range, the function returns an error type. The return type itself is encoded as an std::expected type.

```cpp
auto result = convert_unicode_to_ascii(u8"hello");
if (result.has_value())
{
    std::cout << result << std::endl;
}
else
{
    std::cout << result.error().message() << std::endl;
}
```

It can be used with any Unicode-based character type in C++.

```cpp
auto result_u8 = convert_unicode_to_ascii(u8"hello");
auto result_u16 = convert_unicode_to_ascii(u"hello");
auto result_u32 = convert_unicode_to_ascii(U"hello");
auto result_wchar = convert_unicode_to_ascii(L"hello");
```

As well as single characters (the result will be a std::string).

```cpp
auto result_char = convert_unicode_to_ascii(u8'h');
```

There also include variants that throw an exception

```cpp
auto result_u8 = convert_unicode_to_ascii_with_exception(u8"hello");
std::cout << result_u8 << std::endl;
```

This functionality is seen across unicode_bridge; all functions that deal with processing a string in some way that can fail include two variants; one that creates an std::expected, and one that throws an exception.

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