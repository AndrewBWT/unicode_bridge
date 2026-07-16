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

And use the following namespace to access the functionality.

```cpp
using namespace unicode_bridge;
```

## Overview of Library

`unicode_bridge` is a small library consisting of a set of objects and functions, that can be broadly separated into the following categories:

- Printing objects. Used to allow the user to write less boilerplate when preparing Unicode strings for printing through `std::cout`
  - Only object in this category is `unicode_print`.
- Error objects. Some functions in `unicode_bridge`, process streams of Unicode characters. When these character sequences are found to be incorrect according to the Unicode standard, they are rejected by these functions. The errors are represented as individual objects in `unicode_bridge`.
  - `unicode_to_ascii_error`, `ascii_to_unicode_error`, `unicode_conversion_error`, `next_char32_error` and `prev_char32_error` are in this category.
- Exception type. There exist variants of the above mentioned functions which throw exceptions instead of returning errors.
  - `unicode_bridge_exception` is a templated type that can take any of the previously mentioned error types as arguments. It provides a generic exception type for `unicode_bridge`.
- Functions that convert either strings or characters.
  - Conversion between Unicode and the basic `std::string` type through `convert_unicode_to_ascii` and `convert_ascii_to_unicode`.
  - Conversion between different Unicode types through `unicode_conversion`.
- Functions that check strings/characters to ensure they are valid Unicode/ASCII.
  - `is_valid_unicode` and `is_valid_ascii`.
- Functions that when given a pair of iterator positions in a Unicode string, together representing a current position and an end-bound, can extract the next/previous Unicode scalar value from those iterators.
  - `next_char32`, `next_char32_no_error`, `prev_char32` and `prev_char32_no_error`.
  - There are also variants `next_char32_and_increment_iterator`, `next_char32_and_increment_iterator_no_error`, `prev_char32_and_decrement_iterator` and `prev_char32_and_decrement_iterator` which move the current iterator away/towards the other iterator representing the bound of the function.
  
In `unicode_bridge`, we include several variants of each of the functions described above. Below we show an brief overview of all of these functions. Note the function signature does not contain the concepts that ensure the functions will only accept the correct types.

| Function Signature         | Return Type                                         | Exceptions |
|----------------------------|-----------------------------------------------------|------------|
| `template<typename Unicode_String_Object> convert_unicode_to_ascii(Unicode_String_Object)` | `std::expected<std::string,unicode_to_ascii_error>` | None       |
| `template<typename Unicode_String_Object> convert_unicode_to_ascii_append(Unicode_String_Object, std::string&)` | `std::optional<unicode_to_ascii_error>` | None       |
| `template<typename Unicode_String_Object> convert_unicode_to_ascii_no_error(Unicode_String_Object)` | `std::optional<std::string>` | None       |
| `template<typename Unicode_String_Object> convert_unicode_to_ascii_append_no_error(Unicode_String_Object, std::string&)` | `bool` | None       |
| `template<typename Unicode_Char> convert_unicode_to_ascii(const Unicode_Char)` | `std::expected<std::string,unicode_to_ascii_error>` | None       |
| `template<typename Unicode_Char> convert_unicode_to_ascii_append(const Unicode_Char, std::string&)` | `std::optional<unicode_to_ascii_error>` | None       |
| `template<typename Unicode_Char> convert_unicode_to_ascii_no_error(const Unicode_Char)` | `std::optional<std::string>` | None       |
| `template<typename Unicode_Char> convert_unicode_to_ascii_append_no_error(const Unicode_Char, std::string&)` | `bool` | None       |
| `template<typename Unicode_String_Object> convert_unicode_to_ascii_with_exception(Unicode_String_Object)` | `std::string` | `unicode_bridge_exception<unicode_to_ascii_error>`       |
| `template<typename Unicode_String_Object> convert_unicode_to_ascii_append_with_exception(Unicode_String_Object, std::string&)` | `void` | `unicode_bridge_exception<unicode_to_ascii_error>`       |
| `template<typename Unicode_Char> convert_unicode_to_ascii_with_exception(const Unicode_Char)` | `std::string` | `unicode_bridge_exception<unicode_to_ascii_error>`       |
| `template<typename Unicode_Char> convert_unicode_to_ascii_append_with_exception(const Unicode_Char, std::string&)` | `void` | `unicode_bridge_exception<unicode_to_ascii_error>`       |
| `template<typename OutputChar> convert_ascii_to_unicode(const std::string_view)` | `std::expected<std::basic_string<OutputChar>,ascii_to_unicode_error>` | None       |
| `template<typename OutputChar> convert_ascii_to_unicode_append(const std::string_view, std::basic_string<OutputChar>&)` | `std::optional<ascii_to_unicode_error>` | None       |
| `template<typename OutputChar> convert_ascii_to_unicode_no_error(const std::string_view)` | `std::optional<std::basic_string<OutputChar>>` | None       |
| `template<typename OutputChar> convert_ascii_to_unicode_append_no_error(const std::string_view, std::basic_string<OutputChar>&)` | `bool` | None       |
| `template<typename OutputChar> convert_ascii_to_unicode(const char)` | `std::expected<std::basic_string<OutputChar>,ascii_to_unicode_error>` | None       |
| `template<typename OutputChar> convert_ascii_to_unicode_append(const char, std::basic_string<OutputChar>&)` | `std::optional<ascii_to_unicode_error>` | None       |
| `template<typename OutputChar> convert_ascii_to_unicode_no_error(const char)` | `std::optional<std::basic_string<OutputChar>>` | None       |
| `template<typename OutputChar> convert_ascii_to_unicode_append_no_error(const char, std::basic_string<OutputChar>&)` | `bool` | None       |
| `template<typename OutputChar> convert_ascii_to_unicode_with_exception(const std::string_view)` | `std::basic_string<OutputChar>` | `unicode_bridge_exception<unicode_to_ascii_error>`       |
| `template<typename OutputChar> convert_ascii_to_unicode_append_with_exception(const std::string_view, std::basic_string<OutputChar>&)` | `void` | `unicode_bridge_exception<unicode_to_ascii_error>`       |
| `template<typename OutputChar> convert_ascii_to_unicode_with_exception(const char)` | `std::basic_string<OutputChar>` | `unicode_bridge_exception<ascii_to_unicode_error>`       |
| `template<typename OutputChar> convert_ascii_to_unicode_append_with_exception(const char, std::basic_string<OutputChar>&)` | `void` | `unicode_bridge_exception<unicode_to_ascii_error>`       |
| `template<typename OutputChar, typename Unicode_String_Object> unicode_conversion(Unicode_String_Object)` | `std::expected<std::basic_string<OutputChar>,unicode_conversion_error>` | None       |
| `template<typename OutputChar, typename Unicode_Char> unicode_conversion(const Unicode_Char)` | `std::expected<std::basic_string<OutputChar>,unicode_conversion_error>` | None       |
| `template<typename OutputChar, typename Unicode_String_Object> unicode_conversion_with_exception(Unicode_String_Object)` | `std::basic_string<OutputChar>` | `unicode_bridge_exception<unicode_conversion_error>` |
| `template<typename OutputChar, typename Unicode_Char> unicode_conversion_with_exception(const Unicode_Char)` | `std::basic_string<OutputChar>` | `unicode_bridge_exception<unicode_conversion_error>` |
| `template<typename Unicode_String_Object> is_valid_unicode(Unicode_String_Object)` | `bool` | None |
| `template<typename Unicode_Char> is_valid_unicode(const Unicode_Char)` | `bool` | None |
| `template<typename String_Object> is_valid_ascii(String_Object)` | `bool` | None |
| `template<typename Char_Type> is_valid_ascii(const Char_Type)` | `bool` | None |
| `template<typename Iterator_Type> next_char32(const Iterator_Type, const Iterator_Type)` | `std::expected<std::pair<char32_t,std::size_t>,next_char32_error>` | None       |
| `template<typename Iterator_Type> next_char32_no_error(const Iterator_Type, const Iterator_Type)` | `std::optional<std::pair<char32_t,std::size_t>>` | None       |
| `template<typename Iterator_Type> next_char32_with_exception(const Iterator_Type, const Iterator_Type)` | `std::pair<char32_t,std::size_t>` | `unicode_bridge_exception<next_char32_error>`|
| `template<typename Iterator_Type> next_char32_and_increment_iterator_with_exception(const Iterator_Type, const Iterator_Type)` | `std::pair<char32_t,std::size_t>` | `unicode_bridge_exception<next_char32_error>`|
| `template<typename Iterator_Type> prev_char32(const Iterator_Type, const Iterator_Type)` | `std::expected<std::pair<char32_t,std::size_t>,prev_char32_error>` | None       |
| `template<typename Iterator_Type> prev_char32_and_increment_iterator_no_error(Iterator_Type&, const Iterator_Type)` | `std::optional<std::pair<char32_t,std::size_t>>` | None       |
| `template<typename Iterator_Type> prev_char32_with_exception(const Iterator_Type, const Iterator_Type)` | `std::pair<char32_t,std::size_t>` | `unicode_bridge_exception<prev_char32_error>`|
| `template<typename Iterator_Type> prev_char32_and_increment_iterator_with_exception(const Iterator_Type, const Iterator_Type)` | `std::pair<char32_t,std::size_t>` | `unicode_bridge_exception<prev_char32_error>`|

## Examples and Basic Documentation

The examples below are taken from file `examples/src/main.cpp`. That file, under MSVC and Windows, uses `SetConsoleOutputCP(CP_UTF8)` to ensure the output sent to `std::cout` is rendered properly. 

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

Note that `unicode_print` does not own the string - internally it is a `std::basic_string_view`.  If the `basic_string_view` goes out of scope, then you will get undefined behaviour. It really is only designed to be used when printing using `std::cout`, and not having to write `reinterpret_cast` when converting from UTF-8 to a `std::string`, or other conversion functions for UTF-16/32.

The conversion from UTF-16/32 to UTF-8 in `unicode_print` does not check extensively for Unicode compliance. If the UTF-16 is invalid, or the UTF-32 characters are in the surrogate range, they are still converted to UTF-8 (what is created would be invalid UTF-8). UTF-32 characters above U+10FFFF become U+FFFD replacement characters.

No validation is performed on the UTF-8 strings - they are passed as-is to the console.

### Errors and exceptions

As seen previously, many of the conversion functions take on similar forms; with there being a specified error type, and variants of the functions that throw `unicode_bridge_exception`, templated around that error type.

Each error type has a `message()` function that creates a textual representation of the error. It is written in UTF-8. `unicode_bridge_exception`'s `what()` function encodes this error message in a `std::string`, which is returned as a `char*` using `what()`.

There also exist other `message` functions that can augment the error message with the original string, however there is no uniform calling convention.

In the subsequent sections we show how each of the functions in `unicode_bridge` can be used, and show specific examples concerning how to print the error messages when they fail.

### Convert Unicode to ASCII

Though it may seem trivial (and it is), we include a set of functions to convert Unicode strings to ASCII strings. The return type of `convert_unicode_to_ascii` is  `std::expected<std::string,unicode_to_ascii_error>`, with `std::string` representing success and `unicode_to_ascii_error` representing failure. If any of the input characters do not fit in the basic ASCII range, then a `unicode_to_ascii_error` object is returned. 

Below is an example of its use.

```cpp
auto str_to_check = u8"hello 😀";
auto result_1 = convert_unicode_to_ascii(str_to_check);
if (result_1.has_value())
{
    std::cout << result_1.value() << std::endl;
}
else
{
    std::cout << unicode_print(result_1.error().message()) << std::endl;
    std::cout << unicode_print(result_1.error().message(str_to_check)) << std::endl;
}
```

Though it may not be obvious from the above example, but this function (and all other similar functions in `unicode_bridge`) work with any type that can be converted to a `std::basic_string_view<CharT>`, where `CharT` is a Unicode character type. So the above example uses a `const char8_t*` string. 

We have shown two ways of producing error messages using `unicode_to_ascii_error`; one that uses the original input string, and one that doesn't. This type of pattern is mirrored across all error types in `unicode_bridge`.

`convert_unicode_to_ascii` has three other variants; the first is for single Unicode characters. The other two are variants of the string/single character forms, which throw exceptions instead of `std::expected` objects. Below we show examples of their use.

```cpp
auto result_u16 = convert_unicode_to_ascii(u'h');
std::cout
    << (result_u16.has_value()
            ? result_u16.value()
            : unicode_print(result_u16.error().message()).str())
    << std::endl;
try
{
    auto result_u32   = convert_unicode_to_ascii_with_exception(U"hello");
    auto result_wchar = convert_unicode_to_ascii_with_exception(L'h');
}
catch (const unicode_bridge_exception<unicode_to_ascii_error>& exception_arg)
{
    std::cout << exception_arg.what() << std::endl;
}
```

### convert_ascii_to_unicode

The opposite of `convert_unicode_to_ascii`, this function will take any argument convertible to a `std::string_view` and process it into a Unicode string. Any invalid `char`'s (outside the basic ASCII range) will ensure the function fails. A template type is used to denote what the result type should be. Similar to before, the return type is `std::expected<std::basic_string<OutputChar>,ascii_to_unicode_error>`.

Below we show the various variants and uses of it.

```cpp
std::expected<std::u8string,ascii_to_unicode_error> res_u8 = convert_ascii_to_unicode<char8_t>("hello");
std::expected<std::u16string, ascii_to_unicode_error> res_16 = convert_ascii_to_unicode<char16_t>('h');
std::u32string res_32
    = convert_ascii_to_unicode_with_exception<char32_t>("hello");
std::wstring res_wchar = convert_ascii_to_unicode_with_exception<wchar_t>('h');
```

### unicode_conversion

`unicode_conversion` provides the core funtionality of `unicode_bridge`. It allows for conversion between different Unicode types. If the input is invalid Unicode, it returns a `unicode_conversion_error`, otherwise it is the converted Unicode string.

```cpp
std::expected<std::u8string,unicode_conversion_error> res_u8 = unicode_conversion<char8_t>(U"hello world");
std::expected<std::u16string, unicode_conversion_error> res_16 = unicode_conversion<char16_t>(U'h');
std::u32string res_32 = unicode_conversion_with_exception<char32_t>(u8"hello world");
std::wstring res_wchar = unicode_conversion_with_exception<wchar_t>(u8'h');
```

### is_valid_unicode/is_valid_ascii

These two functions return a `bool` denoting whether the input is valid Unicode or ASCII.

`is_valid_unicode` will only accept Unicode strings/characters as input, telling the user whether they are valid Unicode.

`is_valid_ascii` will take any character type, checking if the string/characters are within the ASCII range.

```cpp
bool r1 = is_valid_unicode(U"😀");
// r2 should be false.
bool r2 = is_valid_unicode(static_cast<char8_t>(0xFF));
bool r3 = is_valid_ascii(u8"should be all fine");
bool r4 = is_valid_ascii("as should this");
```

### next_char32/prev_char32

This set of functions, when given two iterators representing some current position and end position in a string-like object, get the next or previous Unicode scalar value from that string-like object.

To begin, let us concentrate on the series of functions represented by `next_char32`.

At their most basic level, these functions return a Unicode scalar value and an integer representing the number of bytes processed to produce that value. This is in the form `std::pair<char32_t,std::size_t>`.

A `bool` template parameter can change the return type. This parameter, called `Return_Reason`, if set to `true` returns an `std::expected<std::pair<char32_t,std::size_t>,next_char32_error>`. If set to `false`, it returns an `std::optional<std::pair<char32_t,std::size_t>`. In essence, one returns the error, the other fails with an empty optional. There is also a variant that will throw an exception, which does not require a `bool` template parameter.

Below we show an example of this.

```cpp
std::u8string_view str = u8"the string to check";
std::expected<std::pair<char32_t, std::size_t>, next_char32_error> res
    = next_char32(std::begin(str), std::end(str));
std::optional<std::pair<char32_t, std::size_t>> res2
    = next_char32_no_error(std::begin(str), std::end(str));
std::pair<char32_t, std::size_t> res3
    = next_char32_with_exception(std::begin(str), std::end(str));
```

The iterators given as arguments are `const`, so they are not changed.

`unicode_bridge` also provide versions which move the iterator towards the bounds iterator by the number of bytes consumed to create the character, allowing the user to easily move through a string, extracting each Unicode character.

```cpp
std::u8string_view str2         = u8"the string to check";
auto current_itt = std::begin(str2);
auto end_itt     = std::end(str2);
auto next_result
    = next_char32_and_increment_iterator(current_itt, end_itt);
while (next_result.has_value())
{
    auto character = next_result.value();
    // Do stuff with the character.
    next_result    = next_char32_and_increment_iterator(
        current_itt, end_itt
    );
}
auto with_no_error
    = next_char32_and_increment_iterator_no_error(current_itt, end_itt);
auto thros_exception
    = next_char32_and_increment_iterator_with_exception(
        current_itt, end_itt
    );
```

`prev_char32` works in a similar way to `next_char32`, except it gets the previous Unicode character from the string.

```cpp
std::u8string_view str = u8"the string to check";
std::expected<std::pair<char32_t, std::size_t>, prev_char32_error> res
    = prev_char32(std::end(str), std::begin(str));
std::optional<std::pair<char32_t, std::size_t>> res2
    = prev_char32_no_error(std::end(str), std::begin(str));
std::pair<char32_t, std::size_t> res3
    = prev_char32_with_exception(std::end(str), std::begin(str));

std::u8string_view str2 = u8"the string to check";
auto current_itt = std::end(str2);
auto end_itt     = std::begin(str2);
auto next_result
    = prev_char32_and_decrement_iterator(current_itt, end_itt);
while (next_result.has_value())
{
    auto character = next_result.value();
    // Do stuff with the character.
    next_result    = prev_char32_and_decrement_iterator(
        current_itt, end_itt
    );
}
auto with_no_error
    = prev_char32_and_decrement_iterator_no_error(current_itt, end_itt);
auto thros_exception
    = prev_char32_and_decrement_iterator_with_exception(
        current_itt, end_itt
    );
```