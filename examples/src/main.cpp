#define NOMINMAX
#include "unicode_bridge.hpp"

#include <Windows.h>
#include <fstream>
#include <iostream>
#include <string>

int
    main(
        int   argc,
        char* argv[]
    )
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    using namespace unicode_bridge;

    // unicode_print examples
    {
        std::cout << unicode_print(u8"Hello World! 😀") << std::endl;
        std::cout << unicode_print(u"hello world! 😀") << std::endl;
        std::cout << unicode_print(L"hello world! 😀") << std::endl;
        std::cout << unicode_print(U"hello world! 😀") << std::endl;

        std::string str1
            = unicode_print(u8"unicode_print doesn't own the strings").str();
        std::cout << str1 << std::endl;
    }
    // convert_unicode_to_ascii examples
    {
        auto str_to_check = u8"hello 😀";
        auto result_1     = convert_unicode_to_ascii(str_to_check);
        if (result_1.has_value())
        {
            std::cout << result_1.value() << std::endl;
        }
        else
        {
            std::cout << unicode_print(result_1.error().message()) << std::endl;
            std::cout << unicode_print(result_1.error().message(str_to_check))
                      << std::endl;
        }
        auto result_u16 = convert_unicode_to_ascii(u'h');
        std::cout
            << (result_u16.has_value()
                    ? result_u16.value()
                    : unicode_print(result_u16.error().message(u'h')).str())
            << std::endl;
        try
        {
            auto result_u32 = convert_unicode_to_ascii_with_exception(U"hello");
            auto result_wchar = convert_unicode_to_ascii_with_exception(L'h');
        }
        catch (const unicode_bridge_exception<unicode_to_ascii_error>&
                   exception_arg)
        {
            std::cout << exception_arg.what() << std::endl;
        }
    }
    // convert_ascii_to_unicode_examples
    {
        std::expected<std::u8string, ascii_to_unicode_error> res_u8
            = convert_ascii_to_unicode<char8_t>("hello");
        std::expected<std::u16string, ascii_to_unicode_error> res_16
            = convert_ascii_to_unicode<char16_t>('h');
        std::u32string res_32
            = convert_ascii_to_unicode_with_exception<char32_t>("hello");
        std::wstring res_wchar
            = convert_ascii_to_unicode_with_exception<wchar_t>('h');
        if (res_16.has_value())
        {
            std::cout << unicode_print(res_16.value()) << std::endl;
        }
        else
        {
            std::cout << unicode_print(res_16.error().message('h'))
                      << std::endl;
        }
    }
    // unicode_conversion results
    {
        std::expected<std::u8string, unicode_conversion_error> res_u8
            = unicode_conversion<char8_t>(U"hello world");
        std::expected<std::u16string, unicode_conversion_error> res_16
            = unicode_conversion<char16_t>(U'h');
        std::u32string res_32
            = unicode_conversion_with_exception<char32_t>(u8"hello world");
        std::wstring res_wchar
            = unicode_conversion_with_exception<wchar_t>(u8'h');
        if (res_16.has_value())
        {
            std::cout << unicode_print(res_16.value()) << std::endl;
        }
        else
        {
            std::cout << unicode_print(res_16.error().message(U'h'))
                      << std::endl;
        }
    }
    // is_valid_unicode/is_valid_ascii
    {
        bool r1 = is_valid_unicode(U"😀");
        // r2 should be false.
        bool r2 = is_valid_unicode(static_cast<char8_t>(0xFF));
        bool r3 = is_valid_ascii(u8"should be all fine");
        bool r4 = is_valid_ascii("as should this");
    }
    // next_char32/prev_char32
    {
        std::u8string_view str = u8"the string to check";
        std::expected<std::pair<char32_t, std::size_t>, next_char32_error> res
            = next_char32(std::begin(str), std::end(str));
        std::optional<std::pair<char32_t, std::size_t>> res2
            = next_char32_no_error(std::begin(str), std::end(str));
        std::pair<char32_t, std::size_t> res3
            = next_char32_with_exception(std::begin(str), std::end(str));

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
    }
    {
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
    }
}