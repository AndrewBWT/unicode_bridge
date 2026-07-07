# unicode_bridge

A primarily constexpr C++23 library for Unicode conversion and validation.

## Requirements

- C++23
- MSVC (tested. GCC/Clang untested)

## Usage

Single header include:

```cpp
#include "unicode_bridge.hpp"
```

## Examples

Unicode bridge has a simple set of exported functions. Many of them come in different flavours depending on your use-case.

### Convert Unicode to ASCII

Though it may seem trivial (and it is), we include a set of functions to convert Unicode strings to ASCII strings. If any of the characters do not fit in the basic ASCII range, the function returns an error type. The return type itself is encoded as an std::expected type.

```
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

```
auto result_u8 = convert_unicode_to_ascii(u8"hello");
auto result_u16 = convert_unicode_to_ascii(u"hello");
auto result_u32 = convert_unicode_to_ascii(U"hello");
auto result_wchar = convert_unicode_to_ascii(L"hello");
```

As well as single characters (the result will be a std::string).

```
auto result_char = convert_unicode_to_ascii(u8'h');
```

There also include variants that throw an exception

```
auto result_u8 = convert_unicode_to_ascii_with_exception(u8"hello");
std::cout << result_u8 << std::endl;
```

This functionality is seen across unicode_bridge; all functions that deal with processing a string in some way that can fail include two variants; one that creates an std::expected, and one that throws an exception.

### convert_ascii_to_unicode

The opposite of `convert_unicode_to_ascii`, this function will take any argument convertible to a `std::string_view` and process it into a Unicode string. Any invalid `char`'s (outside the basic ASCII range) will ensure the function fails. A template type is used to denote what the result type should be. As before, the return type is an `std::expected`.

```
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

```
auto r1 = unicode_conversion<char8_t>(u"hello world! 😀");
auto r2 = unicode_conversion_with_exception<char16_t>(L"hello world! 😀");
auto r3 = unicode_conversion<char32_t>(U'😀');
auto r4 = unicode_conversion_with_exception<wchar_t>(U'😀');
```

### is_valid_unicode/is_valid_ascii

These two functions return a bool denoting whether the input is valid Unicode or ASCII.

`is_valid_unicode` will only accept Unicode strings/characters as input, ensuring they are valid Unicode.

`is_valid_ascii` will take any character type, checking if the string/characters are within the ASCII range.

```
auto r1 = is_valid_unicode(U"😀");
// r2 should be false.
auto r2 = is_valid_unicode(static_cast<char8_t>(0xFF));
auto r3 = is_valid_ascii(u8"should be all fine");
auto r4 = is_valid_ascii("as should this");
```

### next_char32

This set of functions, when given two iterators representing some current position and the end of the string-like object, it will return a pair. This pair consists of the next Unicode scalar value from the iterators, and the number of elements of the iterator which were consumed to create that scalar value. If it cannot find a Unicode scalar value, then an error is returned instead.

```
auto str = u8"the string to check";
auto res = next_char32(std::begin(str), std::end(str));
```