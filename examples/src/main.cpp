#define NOMINMAX
#include <Windows.h>
#include <iostream>

#include "unicode_bridge.hpp"

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
    std::cout << unicode_print(u8"Hello World! 😀") << std::endl;
    std::cout << unicode_print(u"hello world! 😀") << std::endl;
    std::cout << unicode_print(L"hello world! 😀") << std::endl;
    std::cout << unicode_print(U"hello world! 😀") << std::endl;

    std::string str1 = unicode_print(u8"unicode_print doesn't own the strings").str();
    std::cout << str1 << std::endl;

    auto result = convert_unicode_to_ascii(u8"hello 😀");
    if (result.has_value())
    {
        std::cout << result.value() << std::endl;
    }
    else
    {
        std::cout << unicode_print(result.error().message()) << std::endl;
    }
    auto kx = unicode_print(u8"sv").str();

}