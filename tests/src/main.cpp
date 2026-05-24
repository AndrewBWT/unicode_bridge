#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <spl_unicode.hpp>
#include <tuple>
#include <windows.h>

namespace unicode_bridge_testing
{
/*!
 * @brief Helper function to convert a unicode string into a std::string.
 * @tparam CharT The character type of the underlying input string.
 * @param str_arg The unicode string to encode into the return argument.
 * @return A std::string encoded with the bytes of the input argument, after its
 * been converted to a std::u8string.
 */
template <typename CharT>
inline std::string
    convert_unicode_to_string(
        const std::basic_string<CharT>& str_arg
    )
{
    auto str_as_u8
        = unicode_bridge::to_formatted_unicode_string<char8_t>(str_arg);
    return std::string(str_as_u8.begin(), str_as_u8.end());
}

template <typename CharT>
inline std::string
    convert_ascii_to_string(
        const std::basic_string<CharT>& str_arg
    )
{
    auto str_as_u8 = unicode_bridge::to_formatted_unicode_string<char>(str_arg);
    return std::string(str_as_u8.begin(), str_as_u8.end());
}

/*!
 * @brief Checks unicode errors are exactly the same. Generic function used in
 * many testing files.
 * @param error_l_arg First unicode_error argument.
 * @param error_r_arg Second unicode_error argument.
 */
inline void
    equal_unicode_error(
        const unicode_bridge::unicode_conversion_error& error_l_arg,
        const unicode_bridge::unicode_conversion_error& error_r_arg
    )
{
    using namespace std;
    auto arr_to_str = [](const std::array<char8_t, 4>& arr_arg)
    {
        auto& [char1, char2, char3, char4]{arr_arg};
        string rv{"["};
        rv.append(convert_unicode_to_string(u8string(1, char1)));
        rv.append(", ");
        rv.append(convert_unicode_to_string(u8string(1, char2)));
        rv.append(", ");
        rv.append(convert_unicode_to_string(u8string(1, char3)));
        rv.append(", ");
        rv.append(convert_unicode_to_string(u8string(1, char4)));
        rv.append("]");
        return rv;
    };
    REQUIRE(error_l_arg.code() == error_r_arg.code());
    REQUIRE(error_l_arg.character_index() == error_r_arg.character_index());
    INFO(
        "error l arg code points = " << arr_to_str(error_l_arg.u8_code_points())
    );
    INFO(
        "error r arg code points = " << arr_to_str(error_r_arg.u8_code_points())
    );
    if (error_l_arg.u8_code_points() != error_r_arg.u8_code_points())
    {
        int x = 4;
    }
    REQUIRE(error_l_arg.u8_code_points() == error_r_arg.u8_code_points());
    if (error_l_arg.code_points_encountered()
        != error_r_arg.code_points_encountered())
    {
        int x = 4;
    }
    REQUIRE(
        error_l_arg.code_points_encountered()
        == error_r_arg.code_points_encountered()
    );
    REQUIRE(
        error_l_arg.expected_code_points_size()
        == error_r_arg.expected_code_points_size()
    );
    REQUIRE(error_l_arg.u16_code_points() == error_r_arg.u16_code_points());
    REQUIRE(error_l_arg.char32_character() == error_r_arg.char32_character());
    REQUIRE(error_l_arg.is_wchar() == error_r_arg.is_wchar());
}

/*!
 * @brief A helper function that runs a function on all different string types:
 * std::basic_string, std::basic_string_view, and the char* variant of the
 * underlying string.
 *
 * It also calls on a singular char (if the argument given has a size of exactly
 * one).
 *
 * It will only call on a char*-type argument if the string itself does not
 * contain the character literal \0.
 *
 * This is because the string will not be understood properly by the underlying
 * library code - which converts char*-like parameters into basic_string_view's.
 *
 * @tparam T The type of the argument given. It is assumed that it is
 * std::basic_string of some type.
 * @tparam F The function type that is called using all the different string
 * types as its only argument.
 * @param function_to_call The function object called on all the differenst
 * string types.
 * @param string_arg The basic_string argument, used as a basis for all the
 * string conversions.
 */
template <
    typename F,
    typename T,
    bool Run_Single_Char = true,
    bool Run_Char_Star   = true>
inline void
    run_all_string_types(
        const F function_to_call,
        T       string_arg
    )
{
    using namespace std;
    using CharT = typename T::value_type;
    if constexpr (Run_Char_Star)
    {
        if (not string_arg.contains(static_cast<CharT>('\0')))
        {
            function_to_call.template operator()(string_arg.c_str());
        }
    }
    function_to_call.template operator()(basic_string_view<CharT>(string_arg));
    function_to_call.template operator()(string_arg);
    if constexpr (Run_Single_Char)
    {
        if (string_arg.size() == 1)
        {
            function_to_call.template operator()(string_arg[0]);
        }
    }
}

/*!
 * @brief Helper function to make a unicode string from a list of bytes and an
 * optional suffix string.
 * @tparam CharT The character type.
 * @param bytes_arg The bytes to intialise the string with.
 * @param suffix_arg The optional suffix string.
 * @return A std::basic_string<CharT> constructed from the arguments.
 */
template <typename CharT>
inline std::basic_string<CharT>
    mk_unicode(
        const std::initializer_list<CharT>& bytes_arg,
        const std::basic_string<CharT>& suffix_arg = std::basic_string<CharT>{}
    )
{
    std::basic_string<CharT> result(bytes_arg);
    result.append(suffix_arg);
    return result;
}

/*!
 *
 * TODO: Can this be incorperated into mk_unicode, to create a more generalised
 * function?
 *
 * @brief
 * @param bytes_arg
 * @param suffix_arg
 * @return
 */
inline std::string
    make_str(
        const std::initializer_list<char>& bytes_arg,
        const std::string&                 suffix_arg = std::string{}
    )
{
    std::string result; // (bytes_arg);
    for (auto& character : bytes_arg)
    {
        result.push_back(static_cast<char>(character));
    }
    result.append(suffix_arg);
    return result;
}

/*!
 * @brief Given an input string and expected result, runs code that tests
 * convert_unicode_to_ascii and is_valid_Ascii return the correct results.
 * @tparam CharT The internal character type of string_arg
 * @param string_arg The input string to test.
 * @param expected_arg The expected result.
 */
template <typename CharT>
inline void
    convert_unicode_to_ascii_and_is_valid_ascii_testing_correct_result(
        const std::vector<std::pair<std::basic_string<CharT>, std::string>>&
            list_arg
    )
{
    using namespace unicode_bridge;
    using namespace std;
    size_t idx{0};
    for (auto&& [input_string, expected_output] : list_arg)
    {
        auto run_func = [&]<typename T>(const T input_arg)
        {
            // Check input_arg is always valid ascii.
            REQUIRE(is_valid_ascii(input_arg) == true);
            auto result{convert_unicode_to_ascii(input_arg)};
            if (result.has_value())
            {
                // If result has expected value, check it against expected
                // string.
                REQUIRE(result.value() == expected_output);
            }
            else
            {
                // If result has error, print it and fail the test.
                string output{"Unexpected output: \""};
                output.append(convert_unicode_to_string(result.error().message(
                    u8"<placeholder>", u8"<placeholder>", std::nullopt
                )));
                output.append("\"");
                FAIL(output);
            }
            try
            {
                // Checks that with_exception also returns the correct value.
                auto converted2
                    = convert_unicode_to_ascii_with_exception(input_arg);
                CHECK(converted2 == expected_output);
            }
            catch (const unicode_bridge_exception<unicode_to_ascii_error>&
                       unexpected_exception)
            {
                string msg = "Unexpected exception: \"";
                msg.append(convert_unicode_to_string(
                    unexpected_exception.error().message(
                        u8"<placeholder>", u8"<placeholder>", std::nullopt
                    )
                ));
                msg.append("\"");
                FAIL(msg);
            }
            catch (...)
            {
                string msg = "Unexpected, unknown exception";
                FAIL(msg);
            }
        };
        ++idx;
        INFO(
            "The input is \"" << (convert_unicode_to_string(input_string))
                              << "\""
        );
        INFO("Idx = " << idx);
        run_all_string_types(run_func, input_string);
    }
}

template <typename CharT>
inline void
    convert_unicode_to_ascii_and_is_valid_ascii_testing_failure(
        const std::vector<std::pair<
            std::basic_string<CharT>,
            unicode_bridge::unicode_to_ascii_error>>& list_arg
    )
{
    using namespace unicode_bridge;
    using namespace std;
    size_t idx{0};
    for (auto&& [input_string, ouput_error] : list_arg)
    {
        auto run_func = [&]<typename T>(const T input_arg)
        {
            // Check input is always invalid ascii.
            REQUIRE(is_valid_ascii(input_arg) == false);
            auto result{convert_unicode_to_ascii(input_arg)};
            if (result.has_value())
            {
                // If result returns valid ascii, show the error.
                string output{"Unexpected successful convert_unicode_to_ascii "
                              "conversion. std::string result is \""};
                output.append(result.value());
                output.append("\"");
                auto result22{ convert_unicode_to_ascii(input_arg) };
                FAIL(output);
            }
            else
            {
                // Otherwise, test that the failure is the one expected.
                auto& failure_result{result.error()};
                INFO(convert_unicode_to_string(failure_result.message(
                    u8"<placeholder>", u8"<placeholder>", std::nullopt
                )));
                REQUIRE(failure_result.get_enum() == ouput_error.get_enum());
                REQUIRE(failure_result.index() == ouput_error.index());
                REQUIRE(failure_result.character() == ouput_error.character());
                equal_unicode_error(
                    failure_result.error(), ouput_error.error()
                );
            }
            try
            {
                // Checks that with_exception also returns the correct value.
                auto converted2
                    = convert_unicode_to_ascii_with_exception(input_arg);
                string output{"Unexpected successful convert_unicode_to_ascii "
                              "conversion. std::string result is \""};
                output.append(result.value());
                output.append("\"");
                FAIL(output);
            }
            catch (const unicode_bridge_exception<unicode_to_ascii_error>&
                       unexpected_exception)
            {
                auto& failure_result{unexpected_exception.error()};
                INFO(convert_unicode_to_string(failure_result.message(
                    u8"<placeholder>", u8"<placeholder>", std::nullopt
                )));
                REQUIRE(failure_result.get_enum() == ouput_error.get_enum());
                REQUIRE(failure_result.index() == ouput_error.index());
                REQUIRE(failure_result.character() == ouput_error.character());
                equal_unicode_error(
                    failure_result.error(), ouput_error.error()
                );
            }
            catch (...)
            {
                string msg = "Unexpected, unknown exception";
                FAIL(msg);
            }
        };
        ++idx;
        INFO(
            "The input is \"" << (convert_unicode_to_string(input_string))
                              << "\""
        );
        INFO("Idx = " << idx);
        run_all_string_types(run_func, input_string);
    }
}
} // namespace unicode_bridge_testing

namespace unicode_bridge_test_cases
{
template <typename CharT, typename CharU>
inline std::basic_string<CharT>
    spl_convert(
        const std::basic_string<CharU>& str_arg
    )
{
    using namespace std;
    if constexpr (same_as<CharT, CharU>)
    {
        return str_arg;
    }
    else if constexpr (sizeof(CharT) == sizeof(CharU))
    {
        return basic_string<CharT>(str_arg.begin(), str_arg.end());
    }
    else
    {
        return basic_string<CharT>{};
    }
}

template <typename CharT>
inline std::basic_string<CharT>
    get_unicode(
        const std::tuple<std::u8string, std::u16string, std::u32string>&
            tuple_arg
    )
{
    using namespace std;
    auto& [u8, u16, u32]{tuple_arg};
    if constexpr (same_as<char8_t, CharT>)
    {
        return u8;
    }
    else if constexpr (same_as<char16_t, CharT>)
    {
        return u16;
    }
    else if constexpr (same_as<char32_t, CharT>)
    {
        return u32;
    }
    else if constexpr (same_as<wchar_t, CharT>)
    {
        if constexpr (sizeof(CharT) == 2)
        {
            return spl_convert<wchar_t>(u16);
        }
        else if constexpr (sizeof(CharT) == 4)
        {
            return spl_convert<wchar_t>(u32);
        }
        else
        {
            return wstring{};
        }
    }
}

template <typename CharT>
std::vector<std::pair<std::basic_string<CharT>, std::string>>
    get_valid_convert_unicode_to_ascii_and_is_valid_ascii_data()
{
    using namespace std;
    initializer_list<pair<tuple<u8string, u16string, u32string>, string>>
        valid_data_to_test = {
            {{u8"", u"", U""},       ""  },
            {{u8"hi", u"hi", U"hi"}, "hi"}
    };
    vector<pair<basic_string<CharT>, string>> rv;
    for (auto& [unicode_strs, str] : valid_data_to_test)
    {
        rv.push_back(make_pair(get_unicode<CharT>(unicode_strs), str));
    }
    return rv;
}

template <typename CharT>
std::vector<
    std::pair<std::basic_string<CharT>, unicode_bridge::unicode_to_ascii_error>>
    get_invalid_convert_unicode_to_ascii_and_is_valid_ascii_data()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge::internal;
    std::array<std::u8string, 4> arr;
    initializer_list<
        pair<tuple<u8string, u16string, u32string>, unicode_to_ascii_error>>
        invalid_ascii_errors = {
            {{u8"ℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳ⷽ",
              u"ℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳",
              U"ℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳"},
             unicode_to_ascii_error_factory::non_ascii_character_found(U'ℝ', 0)
            },
            {{u8"aaℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳ⷽ",
              u"aaℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳",
              U"aaℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳"},
             unicode_to_ascii_error_factory::non_ascii_character_found(U'ℝ', 2)}
    };
    initializer_list<pair<u8string, unicode_conversion_error>> invalid_u8_errors
        = {
            {mk_unicode<char8_t>({0xFF}, u8"hello"),
             unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0xFF)
             )},
            {mk_unicode<char8_t>({0xFE}, u8"hello"),
             unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0xFE)
             )},
            {mk_unicode<char8_t>({0x80}, u8"hello"),
             unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0x80)
             )},
            {mk_unicode<char8_t>({0xBF}, u8"hello"),
             unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0xBF)
             )},
            {u8string(u8"abc").append(mk_unicode<char8_t>({0xFF}, u8"def")),
             unicode_conversion_error_factory::invalid_leading_byte(
                 3, static_cast<char8_t>(0xFF)
             )},
            {mk_unicode<char8_t>({0xC2}),
             unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xC2), u8'\0', u8'\0', u8'\0'}, 1, 2
             )},
            {mk_unicode<char8_t>({0xE2}),
             unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xE2), u8'\0', u8'\0', u8'\0'}, 1, 3
             )},
            {mk_unicode<char8_t>({0xE2, 0x80}),
             unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x80),
                  u8'\0',
                  u8'\0'},
             2, 3
             )},
            {mk_unicode<char8_t>({0xF0}),
             unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xF0), u8'\0', u8'\0', u8'\0'}, 1, 4
             )},
            {mk_unicode<char8_t>({0xF0, 0x90}),
             unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  u8'\0',
                  u8'\0'},
             2, 4
             )},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x80}),
             unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  static_cast<char8_t>(0x80),
                  u8'\0'},
             3, 4
             )},
            {mk_unicode<char8_t>({0xE2, 0x80}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x80),
                  u8'h',
                  u8'\0'},
             3, 3
             )},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xF0, 0x90}, u8"def"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 3, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  u8'd',
                  u8'e'},
             4, 3
             )},
            {mk_unicode<char8_t>({0xC2, 0x00}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xC2),
                  static_cast<char8_t>(0x00),
                  u8'\0',
                  u8'\0'},
             2, 2
             )},
            {mk_unicode<char8_t>({0xC2, 0x20}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xC2),
                  static_cast<char8_t>(0x20),
                  u8'\0',
                  u8'\0'},
             2, 2
             )},
            {mk_unicode<char8_t>({0xC2, 0xC0}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xC2),
                  static_cast<char8_t>(0xC0),
                  u8'\0',
                  u8'\0'},
             2, 2
             )},
            {mk_unicode<char8_t>({0xE2, 0x80, 0x20}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x20),
                  u8'\0'},
             3, 3
             )},
            {mk_unicode<char8_t>({0xE2, 0x20, 0x80}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x20),
                  static_cast<char8_t>(0x80),
                  u8'\0'},
             3, 2
             )},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x20, 0x80}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  static_cast<char8_t>(0x20),
                  static_cast<char8_t>(0x80)},
             4, 3
             )},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x80, 0x20}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x20)},
             4, 4
             )},
            {mk_unicode<char8_t>({0xF0, 0x20, 0x80, 0x80}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x20),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             4, 2
             )},
            {mk_unicode<char8_t>({0xC0, 0x80}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xC0),
                  static_cast<char8_t>(0x80),
                  u8'\0',
                  u8'\0'},
             2, U'\0'
             )},
            {mk_unicode<char8_t>({0xC1, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xC1),
                  static_cast<char8_t>(0xBF),
                  u8'\0',
                  u8'\0'},
             2, U'\x7F'
             )},
            {mk_unicode<char8_t>({0xE0, 0x80, 0x80}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  u8'\0'},
             3, U'\0'
             )},
            {mk_unicode<char8_t>({0xE0, 0x81, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x81),
                  static_cast<char8_t>(0xBF),
                  u8'\0'},
             3, U'\x7F'
             )},
            {mk_unicode<char8_t>({0xE0, 0x9F, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x9F),
                  static_cast<char8_t>(0xBF),
                  u8'\0'},
             3, U'\x7FF'
             )},
            {mk_unicode<char8_t>({0xF0, 0x80, 0x80, 0x80}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             4, U'\0'
             )},
            {mk_unicode<char8_t>({0xF0, 0x80, 0x80, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0xBF)},
             4, U'\x3F'
             )},
            {mk_unicode<char8_t>({0xED, 0xA0, 0x80}, u8"hello"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xA0),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             3, U'\xD800'
                 )},
            {mk_unicode<char8_t>({0xED, 0xAF, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xAF),
                      static_cast<char8_t>(0xBF),
                      u8'\0'},
             3, U'\xDBFF'
                 )},
            {mk_unicode<char8_t>({0xED, 0xB0, 0x80}, u8"hello"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xB0),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             3, U'\xDC00'
                 )},
            {mk_unicode<char8_t>({0xED, 0xBF, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0xBF),
                      u8'\0'},
             3, U'\xDFFF'
                 )},
            {mk_unicode<char8_t>({0xF4, 0x90, 0x80, 0x80}, u8"hello"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xF4),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             4, U'\x110000'
                 )},
            {mk_unicode<char8_t>({0xF7, 0xBF, 0xBF, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xF7),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0xBF)},
             4, U'\x1FFFFF'
                 )},
            {u8string(u8"abc")
                 + mk_unicode<char8_t>({0xED, 0xA0, 0x80}, u8"def"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     3, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xA0),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             3, U'\xD800'
                 )},
    };
    initializer_list<pair<u16string, unicode_conversion_error>> invalid_u16_errors = {
        {mk_unicode<char16_t>({0xD800}),
         unicode_conversion_error_factory::
             high_surrogate_then_end_of_stream(0, u'\xD800', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xDBFF}),
         unicode_conversion_error_factory::
             high_surrogate_then_end_of_stream(0, u'\xDBFF', same_as<CharT, wchar_t>)
        },
        {u16string(u"hello") + mk_unicode<char16_t>({0xDBFF}),
         unicode_conversion_error_factory::
             high_surrogate_then_end_of_stream(5, u'\xDBFF', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xDBFF}, u"hello"),
         unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(0, u'\xDBFF', u'h', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xD800, 0xD801}),
         unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(0, u'\xD800', u'\xD801', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xDBFF, 0x4E00}),
         unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(0, u'\xDBFF', u'\x4E00', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0x0041, 0xD800, 0x0042, 0x0043}),
         unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(1, u'\xD800', u'\x0042', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xDC00}),
         unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDC00', same_as<CharT, wchar_t>)},
        {mk_unicode<char16_t>({0xDFFF}),
         unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDFFF', same_as<CharT, wchar_t>)},
        {mk_unicode<char16_t>({0x0041, 0x0042, 0xDC00}),
         unicode_conversion_error_factory::
             unexpected_low_surrogate(2, u'\xDC00', same_as<CharT, wchar_t>)},
        {mk_unicode<char16_t>({0xDC00, 0x0041, 0x0042}),
         unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDC00', same_as<CharT, wchar_t>)},
        {mk_unicode<char16_t>({0xDC00, 0xDC01}),
         unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDC00', same_as<CharT, wchar_t>)},
    };
    initializer_list<pair<u32string, unicode_conversion_error>> invalid_u32_errors = {
        {mk_unicode<char32_t>({0xD800}),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\xD800',     same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0xDBFF}),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\xDBFF',     same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0xDC00}),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\xDC00',     same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0xDFFF}),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\xDFFF',     same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0x0041, 0x0042, 0xD800}),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(2, U'\xD800',     same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0x0041, 0xD800, 0x0042}),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(1, U'\xD800',     same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0x11'0000}),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\x110000',   same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0x20'0000}),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\x200000',   same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0xFFFF'FFFF}),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\xFFFFFFFF', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char32_t>({0x0041, 0x0042, 0x11'0000}),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(2, U'\x110000',   same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0x0041, 0x11'0000, 0x0042}),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(1, U'\x110000',   same_as<CharT, wchar_t>)},
    };
    vector<pair<basic_string<CharT>, unicode_to_ascii_error>> rv;
    for (auto& [unicode_strs, err] : invalid_ascii_errors)
    {
        rv.push_back(make_pair(get_unicode<CharT>(unicode_strs), err));
    }
    auto fill_vector_func = [&]<typename T>(T list_arg)
    {
        for (auto& [unicode_str, err] : list_arg)
        {
            rv.push_back(make_pair(
                spl_convert<CharT>(unicode_str),
                unicode_to_ascii_error_factory::invalid_unicode_character(err)
            ));
        }
    };
    if constexpr (same_as<char8_t, CharT>)
    {
        fill_vector_func(invalid_u8_errors);
    }
    else if constexpr (same_as<char16_t, CharT>)
    {
        fill_vector_func(invalid_u16_errors);
    }
    else if constexpr (same_as<char32_t, CharT>)
    {
        fill_vector_func(invalid_u32_errors);
    }
    else if constexpr (same_as<wchar_t, CharT>)
    {
        if constexpr (sizeof(CharT) == 2)
        {
            fill_vector_func(invalid_u16_errors);
        }
        else if constexpr (sizeof(CharT) == 4)
        {
            fill_vector_func(invalid_u32_errors);
        }
    }
    return rv;
}
} // namespace unicode_bridge_test_cases

TEST_CASE(
    "convert_unicode_to_ascii and is_valid_ascii using char8_t is computed "
    "correctly",
    "[convert_unicode_to_ascii,is_valid_ascii]"
)
{
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    using namespace unicode_bridge;
    using namespace std;
    convert_unicode_to_ascii_and_is_valid_ascii_testing_correct_result(
        get_valid_convert_unicode_to_ascii_and_is_valid_ascii_data<char8_t>()
    );
    auto data_to_test
        = get_invalid_convert_unicode_to_ascii_and_is_valid_ascii_data<char8_t>(
        );
    convert_unicode_to_ascii_and_is_valid_ascii_testing_failure(data_to_test);
}

TEST_CASE(
    "convert_unicode_to_ascii and is_valid_ascii using char16_t is "
    "computed "
    "correctly",
    "[convert_unicode_to_ascii,is_valid_ascii]"
)
{
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    using namespace unicode_bridge;
    using namespace std;
    convert_unicode_to_ascii_and_is_valid_ascii_testing_correct_result(
        get_valid_convert_unicode_to_ascii_and_is_valid_ascii_data<char16_t>()
    );
    auto data_to_test
        = get_invalid_convert_unicode_to_ascii_and_is_valid_ascii_data<
            char16_t>();
    convert_unicode_to_ascii_and_is_valid_ascii_testing_failure(data_to_test);
}

TEST_CASE(
    "convert_unicode_to_ascii and is_valid_ascii using char32_t is "
    "computed "
    "correctly",
    "[convert_unicode_to_ascii,is_valid_ascii]"
)
{
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    using namespace unicode_bridge;
    using namespace std;
    convert_unicode_to_ascii_and_is_valid_ascii_testing_correct_result(
        get_valid_convert_unicode_to_ascii_and_is_valid_ascii_data<char32_t>()
    );
    auto data_to_test
        = get_invalid_convert_unicode_to_ascii_and_is_valid_ascii_data<
            char32_t>();
    convert_unicode_to_ascii_and_is_valid_ascii_testing_failure(data_to_test);
}

TEST_CASE(
    "convert_unicode_to_ascii and is_valid_ascii using wchar_t is computed "
    "correctly",
    "[convert_unicode_to_ascii,is_valid_ascii]"
)
{
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    using namespace unicode_bridge;
    using namespace std;
    convert_unicode_to_ascii_and_is_valid_ascii_testing_correct_result(
        get_valid_convert_unicode_to_ascii_and_is_valid_ascii_data<wchar_t>()
    );
    auto data_to_test
        = get_invalid_convert_unicode_to_ascii_and_is_valid_ascii_data<wchar_t>(
        );
    convert_unicode_to_ascii_and_is_valid_ascii_testing_failure(data_to_test);
}

namespace unicode_bridge_testing
{
/*!
 * @brief Generic tester for all unicode types.
 * @tparam CharT The character parameter to use for these tests.
 */
template <typename CharT>
inline void
    convert_ascii_to_unicode_and_is_valid_ascii_testing()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge::internal;
    initializer_list<pair<string, unicode_bridge::ascii_to_unicode_error>>
        invalid_results = {
            {make_str({'\x80'},                   ""),
             ascii_to_unicode_error_factory::make(0, '\x80')},
            {make_str({'\xFF'},                   ""),
             ascii_to_unicode_error_factory::make(0, '\xFF')},
            {make_str({'\xC3', '\xA9'},           ""),
             ascii_to_unicode_error_factory::make(0, '\xC3')},
            {make_str({'\x80', '\x41'},           ""),
             ascii_to_unicode_error_factory::make(0, '\x80')},
            {make_str({'\x41', '\x80'},           ""),
             ascii_to_unicode_error_factory::make(1, '\x80')},

            {make_str({'\xFF', '\xFE'},           ""),
             ascii_to_unicode_error_factory::make(0, '\xFF')},
            {make_str({'\x81', '\x82', '\x83'},   ""),
             ascii_to_unicode_error_factory::make(0, '\x81')},

            {string("hello") + make_str({'\x80'}, ""),
             ascii_to_unicode_error_factory::make(5, '\x80')},
            {make_str({'\x80'},                   "hello"),
             ascii_to_unicode_error_factory::make(0, '\x80')},
            {string("h") + make_str({'\x80'},     "ello"),
             ascii_to_unicode_error_factory::make(1, '\x80')},
    };
    // Run invalid tests.
    for (auto&& [input_string, expected_arg] : invalid_results)
    {
        auto run_func = [&]<typename T>(const T input_arg)
        {
            // Checks its valid ascii.
            REQUIRE(is_valid_ascii(input_arg) == false);
            auto result{
                unicode_bridge::convert_ascii_to_unicode<CharT>(input_arg)
            };
            if (result.has_value())
            {
                string output{"Unexpected successful convert_ascii_to_unicode "
                              "conversion. Result is \""};
                output.append(convert_unicode_to_string(result.value()));
                output.append("\"");
                FAIL(output);
            }
            else
            {
                auto& failure_result{result.error()};
                INFO(convert_unicode_to_string(failure_result.message(
                    u8"<placeholder>", u8"<placeholder>", std::nullopt
                )));
                REQUIRE(
                    failure_result.get_character()
                    == expected_arg.get_character()
                );
                REQUIRE(failure_result.get_index() == expected_arg.get_index());
            }
            try
            {
                // Checks that with_exception also returns the correct
                // value.
                auto result2
                    = convert_ascii_to_unicode_with_exception<CharT>(input_arg);
                string output{"Unexpected successful convert_ascii_to_unicode "
                              "conversion. std::string result is \""};
                output.append(convert_unicode_to_string(result2));
                output.append("\"");
                FAIL(output);
            }
            catch (const unicode_bridge_exception<ascii_to_unicode_error>&
                       unexpected_exception)
            {
                auto& failure_result{unexpected_exception.error()};
                REQUIRE(
                    failure_result.get_character()
                    == expected_arg.get_character()
                );
                REQUIRE(failure_result.get_index() == expected_arg.get_index());
            }
            catch (...)
            {
                string msg = "Unexpected, unknown exception";
                FAIL(msg);
            }
        };
        run_all_string_types(run_func, input_string);
    }
    initializer_list<pair<string, u32string>> valid_results = {
        {make_str({'\0'}, ""), u32string(1, U'\0')},
        {make_str({'\x41', '\x00', '\x41'}, ""),
         u32string(1, U'A') + u32string(1, '\0') + u32string(1, 'A')}
    };
    // Run valid tests.
    for (auto&& [input_string, u32_output_string] : valid_results)
    {
        INFO(
            "input string \"" << (convert_ascii_to_string(input_string)) << "\""
        );
        INFO(
            "Expected string \""
            << (convert_unicode_to_string(u32_output_string)) << "\""
        );
        auto run_func = [&]<typename T>(const T input_arg)
        {
            // Checks is_valid_ascii fails.
            REQUIRE(is_valid_ascii(input_arg) == true);
            auto result{
                unicode_bridge::convert_ascii_to_unicode<CharT>(input_arg)
            };
            auto output_string
                = unicode_bridge::unicode_conversion<CharT>(u32_output_string);
            if (result.has_value())
            {
                auto& result_val{result.value()};
                INFO(
                    "Output = \"" << (convert_unicode_to_string(result_val))
                                  << "\""
                );
                REQUIRE(result_val == output_string);
            }
            else
            {
                string output{"Unexpected output: \""};
                output.append(convert_unicode_to_string(result.error().message(
                    u8"<placeholder>", u8"<placeholder>", std::nullopt
                )));
                output.append("\"");
                FAIL(output);
            }
            try
            {
                // Checks that with_exception also returns the correct
                // value.
                auto converted2
                    = convert_ascii_to_unicode_with_exception<CharT>(input_arg);
                REQUIRE(converted2 == output_string);
            }
            catch (const unicode_bridge_exception<unicode_to_ascii_error>&
                       unexpected_exception)
            {
                auto&  failure_result{unexpected_exception.error()};
                string msg = "Unexpected exception: \"";
                msg.append(convert_unicode_to_string(
                    unexpected_exception.error().message(
                        u8"<placeholder>", u8"<placeholder>", std::nullopt
                    )
                ));
                msg.append("\"");
                FAIL(msg);
            }
            catch (...)
            {
                string msg = "Unexpected, unknown exception";
                FAIL(msg);
            }
        };
        run_all_string_types(run_func, input_string);
    }
}
} // namespace unicode_bridge_testing

TEST_CASE(
    "convert_ascii_to_unicode and is_valid_ascii using char8_t is computed "
    "correctly",
    "[convert_ascii_to_unicode,is_valid_ascii]"
)
{
    using namespace unicode_bridge_testing;
    convert_ascii_to_unicode_and_is_valid_ascii_testing<char8_t>();
}

TEST_CASE(
    "convert_ascii_to_unicode and is_valid_ascii using char16_t is "
    "computed "
    "correctly",
    "[convert_ascii_to_unicode,is_valid_ascii]"
)
{
    using namespace unicode_bridge_testing;
    convert_ascii_to_unicode_and_is_valid_ascii_testing<char16_t>();
}

TEST_CASE(
    "convert_ascii_to_unicode and is_valid_ascii using char32_t is "
    "computed "
    "correctly",
    "[convert_ascii_to_unicode,is_valid_ascii]"
)
{
    using namespace unicode_bridge_testing;
    convert_ascii_to_unicode_and_is_valid_ascii_testing<char32_t>();
}

TEST_CASE(
    "convert_ascii_to_unicode and is_valid_ascii using wchar_t is computed "
    "correctly",
    "[convert_ascii_to_unicode,is_valid_ascii]"
)
{
    using namespace unicode_bridge_testing;
    convert_ascii_to_unicode_and_is_valid_ascii_testing<wchar_t>();
}

namespace unicode_bridge_testing
{
/*!
 * @brief Performs tests given unicode strings, that unicode_conversion
 * can convert between the types.
 *
 * This test is only for valid unicode strings.
 *
 * @tparam CharU Type parameter of the input string.
 * @tparam CharT Type parameter of the output string.
 * @param input_string The input string.
 * @param expected_output The expected output.
 */
template <typename CharU, typename CharT>
inline void
    test_valid_unicode_conversion_and_is_valid_unicode(
        const std::basic_string<CharT>& input_string,
        const std::basic_string<CharU>& expected_output
    )
{
    using namespace std;
    using namespace unicode_bridge;
    INFO(
        "CharT = " << typeid(CharT).name()
                   << ", CharU = " << typeid(CharU).name()
    );
    INFO(
        "input string \"" << (convert_unicode_to_string(input_string)) << "\""
    );
    INFO(
        "Output string \"" << (convert_unicode_to_string(input_string)) << "\""
    );
    auto run_func = [&]<typename T>(const T input_argument)
    {
        CHECK(is_valid_unicode(input_argument) == true);
        auto converted = unicode_conversion<CharU>(input_argument);
        if (converted.has_value())
        {
            REQUIRE(converted.value() == expected_output);
        }
        else
        {
            string msg = "Unexpected error: \"";
            msg.append(convert_unicode_to_string(converted.error().message(
                u8"<placeholder>", u8"<placeholder>", std::nullopt
            )));
            msg.append("\"");
            FAIL(msg);
        }
        try
        {
            // Checks that with_exception also returns the correct
            // value.
            auto converted2
                = unicode_conversion_with_exception<CharU>(input_argument);
            CHECK(converted2 == expected_output);
        }
        catch (const unicode_bridge_exception<unicode_conversion_error>&
                   unexpected_exception)
        {
            string msg = "Unexpected exception: \"";
            msg.append(convert_unicode_to_string(
                unexpected_exception.error().message(u8"", u8"", std::nullopt)
            ));
            msg.append("\"");
            FAIL(msg);
        }
        catch (...)
        {
            string msg = "Unexpected, unknown exception";
            FAIL(msg);
        }
    };
    run_all_string_types(run_func, input_string);
}
} // namespace unicode_bridge_testing

TEST_CASE(
    "unicode_conversion and is_valid_unicode is correct for valid unicode "
    "strings",
    "[unicode_conversion,is_valid_unicode]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    initializer_list<tuple<u8string, u16string, u32string, wstring>>
        valid_results = {
            {u8"",                     u"",           U"",                     L""          },

            {u8"ℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳ⷽ",
             u"ℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳ⷽ",                   U"ℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳ⷽ",
             L"ℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳ⷽ"                                                          },

            {u8"aaℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳ⷽ",
             u"aaℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳ⷽ",                 U"aaℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳ⷽ",
             L"aaℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳ⷽ"                                                        },
            {u8"\u007Fⷽ",               u"\u007Fⷽ",     U"\u007Fⷽ",               L"\u007Fⷽ"    },
            {u8"\u0080",               u"\u0080",     U"\u0080",               L"\u0080"    },
            {u8"\u07FF",               u"\u07FF",     U"\u07FF",               L"\u07FF"    },
            {u8"\u0800",               u"\u0800",     U"\u0800",               L"\u0800"    },
            {u8"\uFFFF",               u"\uFFFF",     U"\uFFFF",               L"\uFFFF"    },
            {u8"\U00010000",           u"\U00010000", U"\U00010000",           L"\U00010000"},
            {u8"\U0010FFFF",           u"\U0010FFFF", U"\U0010FFFF",           L"\U0010FFFF"},
            {u8"😀",                   u"😀",         U"😀",                   L"😀"        },
            {u8"𝄞",                    u"𝄞",          U"𝄞",                    L"𝄞"         },
            {u8"a😀b",                 u"a😀b",       U"a😀b",                 L"a😀b"      },
            {u8"ℝℝℝ",                  u"ℝℝℝ",        U"ℝℝℝ",                  L"ℝℝℝ"       },
    };
    for (auto&& [u8, u16, u32, ws] : valid_results)
    {
        test_valid_unicode_conversion_and_is_valid_unicode(u8, u8);
        test_valid_unicode_conversion_and_is_valid_unicode(u8, u16);
        test_valid_unicode_conversion_and_is_valid_unicode(u8, u32);
        test_valid_unicode_conversion_and_is_valid_unicode(u8, ws);
        test_valid_unicode_conversion_and_is_valid_unicode(u16, u8);
        test_valid_unicode_conversion_and_is_valid_unicode(u16, u16);
        test_valid_unicode_conversion_and_is_valid_unicode(u16, u32);
        test_valid_unicode_conversion_and_is_valid_unicode(u16, ws);
        test_valid_unicode_conversion_and_is_valid_unicode(u32, u8);
        test_valid_unicode_conversion_and_is_valid_unicode(u32, u16);
        test_valid_unicode_conversion_and_is_valid_unicode(u32, u32);
        test_valid_unicode_conversion_and_is_valid_unicode(u32, ws);
    }
}

namespace unicode_bridge_testing
{
/*!
 * @brief Checks unicode_conversion and is_valid_unicode return the
 * correct values for invalid unicode input arguments.
 * @tparam T
 * @param list_of_test_cases
 */
template <typename T>
inline void
    run_unicode_conversion_and_is_valid_unicode_on_invalid_strings(
        const std::vector<std::pair<
            std::basic_string<T>,
            unicode_bridge::unicode_conversion_error>>& list_of_test_cases
    )
{
    using namespace unicode_bridge;
    using namespace std;
    for (auto&& [input_string, expected_error] : list_of_test_cases)
    {
        CHECK(is_valid_unicode(input_string) == false);
        INFO("str = " << convert_unicode_to_string(input_string));
        auto test_func
            = [&]<typename TargetType, typename U>(const U input_argument)
        {
            auto converted = unicode_conversion<TargetType>(input_argument);
            if (converted.has_value())
            {
                FAIL("Unexpected successful call of unicode_conversion");
            }
            else
            {
                equal_unicode_error(converted.error(), expected_error);
            }
            try
            {
                auto converted2 = unicode_conversion_with_exception<TargetType>(
                    input_argument
                );
                FAIL("Unexpected successful call of "
                     "unicode_conversion_with_exception.");
            }
            catch (const unicode_bridge_exception<unicode_conversion_error>&
                       _exception)
            {
                equal_unicode_error(_exception.error(), expected_error);
            }
        };
        auto test_type = [&]<typename TargetType>()
        {
            auto bound_function = [&]<typename U>(const U input_argument)
            {
                test_func.template operator()<TargetType>(input_argument);
            };
            run_all_string_types(bound_function, input_string);
        };
        // Trying it with all different output types.
        test_type.template operator()<char8_t>();
        test_type.template operator()<char16_t>();
        test_type.template operator()<char32_t>();
    }
}
} // namespace unicode_bridge_testing

namespace unicode_bridge_test_cases
{
template <typename CharT>
std::vector<std::pair<
    std::basic_string<CharT>,
    unicode_bridge::unicode_conversion_error>>
    get_invalid_unicode_conversion_and_is_valid_unicode_data()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge::internal;
    std::array<std::u8string, 4>                               arr;
    initializer_list<pair<u8string, unicode_conversion_error>> invalid_u8_errors
        = {
            {mk_unicode<char8_t>({0xFF}, u8"hello"),
             unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0xFF)
             )},
            {mk_unicode<char8_t>({0xFE}, u8"hello"),
             unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0xFE)
             )},
            {mk_unicode<char8_t>({0x80}, u8"hello"),
             unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0x80)
             )},
            {mk_unicode<char8_t>({0xBF}, u8"hello"),
             unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0xBF)
             )},
            {u8string(u8"abc").append(mk_unicode<char8_t>({0xFF}, u8"def")),
             unicode_conversion_error_factory::invalid_leading_byte(
                 3, static_cast<char8_t>(0xFF)
             )},

            {mk_unicode<char8_t>({0xC2}, u8""),
             unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xC2), u8'\0', u8'\0', u8'\0'}, 1, 2
             )},
            {mk_unicode<char8_t>({0xE2}, u8""),
             unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xE2), u8'\0', u8'\0', u8'\0'}, 1, 3
             )},
            {mk_unicode<char8_t>({0xE2, 0x80}, u8""),
             unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x80),
                  u8'\0',
                  u8'\0'},
             2, 3
             )},
            {mk_unicode<char8_t>({0xF0}, u8""),
             unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xF0), u8'\0', u8'\0', u8'\0'}, 1, 4
             )},
            {mk_unicode<char8_t>({0xF0, 0x90}, u8""),
             unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  u8'\0',
                  u8'\0'},
             2, 4
             )},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x80}, u8""),
             unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  static_cast<char8_t>(0x80),
                  u8'\0'},
             3, 4
             )},

            {mk_unicode<char8_t>({0xE2, 0x80}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x80),
                  u8'h',
                  u8'\0'},
             3, 3
             )},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xF0, 0x90}, u8"def"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 3, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  u8'd',
                  u8'e'},
             4, 3
             )},
            {mk_unicode<char8_t>({0xC2, 0x00}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xC2),
                  static_cast<char8_t>(0x00),
                  u8'\0',
                  u8'\0'},
             2, 2
             )},
            {mk_unicode<char8_t>({0xC2, 0x20}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xC2),
                  static_cast<char8_t>(0x20),
                  u8'\0',
                  u8'\0'},
             2, 2
             )},
            {mk_unicode<char8_t>({0xC2, 0xC0}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xC2),
                  static_cast<char8_t>(0xC0),
                  u8'\0',
                  u8'\0'},
             2, 2
             )},
            {mk_unicode<char8_t>({0xE2, 0x80, 0x20}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x20),
                  u8'\0'},
             3, 3
             )},
            {mk_unicode<char8_t>({0xE2, 0x20, 0x80}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x20),
                  static_cast<char8_t>(0x80),
                  u8'\0'},
             3, 2
             )},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x20, 0x80}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  static_cast<char8_t>(0x20),
                  static_cast<char8_t>(0x80)},
             4, 3
             )},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x80, 0x20}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x20)},
             4, 4
             )},
            {mk_unicode<char8_t>({0xF0, 0x20, 0x80, 0x80}, u8"hello"),
             unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x20),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             4, 2
             )},

            {mk_unicode<char8_t>({0xC0, 0x80}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xC0),
                  static_cast<char8_t>(0x80),
                  u8'\0',
                  u8'\0'},
             2, U'\0'
             )},
            {mk_unicode<char8_t>({0xC1, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xC1),
                  static_cast<char8_t>(0xBF),
                  u8'\0',
                  u8'\0'},
             2, U'\x7F'
             )},
            {mk_unicode<char8_t>({0xE0, 0x80, 0x80}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  u8'\0'},
             3, U'\0'
             )},
            {mk_unicode<char8_t>({0xE0, 0x81, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x81),
                  static_cast<char8_t>(0xBF),
                  u8'\0'},
             3, U'\x7F'
             )},
            {mk_unicode<char8_t>({0xE0, 0x9F, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x9F),
                  static_cast<char8_t>(0xBF),
                  u8'\0'},
             3, U'\x7FF'
             )},
            {mk_unicode<char8_t>({0xF0, 0x80, 0x80, 0x80}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             4, U'\0'
             )},
            {mk_unicode<char8_t>({0xF0, 0x80, 0x80, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0xBF)},
             4, U'\x3F'
             )},

            {mk_unicode<char8_t>({0xED, 0xA0, 0x80}, u8"hello"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xA0),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             3, U'\xD800'
                 )},
            {mk_unicode<char8_t>({0xED, 0xAF, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xAF),
                      static_cast<char8_t>(0xBF),
                      u8'\0'},
             3, U'\xDBFF'
                 )},
            {mk_unicode<char8_t>({0xED, 0xB0, 0x80}, u8"hello"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xB0),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             3, U'\xDC00'
                 )},
            {mk_unicode<char8_t>({0xED, 0xBF, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0xBF),
                      u8'\0'},
             3, U'\xDFFF'
                 )},
            {mk_unicode<char8_t>({0xF4, 0x90, 0x80, 0x80}, u8"hello"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xF4),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             4, U'\x110000'
                 )},
            {mk_unicode<char8_t>({0xF7, 0xBF, 0xBF, 0xBF}, u8"hello"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xF7),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0xBF)},
             4, U'\x1FFFFF'
                 )},
            {u8string(u8"abc")
                 + mk_unicode<char8_t>({0xED, 0xA0, 0x80}, u8"def"),
             unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     3, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xA0),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             3, U'\xD800'
                 )},
    };
    initializer_list<pair<u16string, unicode_conversion_error>> invalid_u16_errors = {
        {mk_unicode<char16_t>({0xD800}, u""),
         unicode_conversion_error_factory::
             high_surrogate_then_end_of_stream(0, u'\xD800', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xDBFF}, u""),
         unicode_conversion_error_factory::
             high_surrogate_then_end_of_stream(0, u'\xDBFF', same_as<CharT, wchar_t>)
        },
        {u16string(u"hello") + mk_unicode<char16_t>({0xDBFF}, u""),
         unicode_conversion_error_factory::
             high_surrogate_then_end_of_stream(5, u'\xDBFF', same_as<CharT, wchar_t>)
        },

        {mk_unicode<char16_t>({0xDBFF}, u"hello"),
         unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(0, u'\xDBFF', u'h', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xD800, 0xD801}, u""),
         unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(0, u'\xD800', u'\xD801', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xDBFF, 0x4E00}, u""),
         unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(0, u'\xDBFF', u'\x4E00', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0x0041, 0xD800, 0x0042, 0x0043}, u""),
         unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(1, u'\xD800', u'\x0042', same_as<CharT, wchar_t>)
        },

        {mk_unicode<char16_t>({0xDC00}, u""),
         unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDC00', same_as<CharT, wchar_t>)},
        {mk_unicode<char16_t>({0xDFFF}, u""),
         unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDFFF', same_as<CharT, wchar_t>)},
        {mk_unicode<char16_t>({0x0041, 0x0042, 0xDC00}, u""),
         unicode_conversion_error_factory::
             unexpected_low_surrogate(2, u'\xDC00', same_as<CharT, wchar_t>)},
        {mk_unicode<char16_t>({0xDC00, 0x0041, 0x0042}, u""),
         unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDC00', same_as<CharT, wchar_t>)},
        {mk_unicode<char16_t>({0xDC00, 0xDC01}, u""),
         unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDC00', same_as<CharT, wchar_t>)},
    };
    initializer_list<pair<u32string, unicode_conversion_error>> invalid_u32_errors = {
        {mk_unicode<char32_t>({0xD800},                    U""),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\xD800',     same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0xDBFF},                    U""),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\xDBFF',     same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0xDC00},                    U""),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\xDC00',     same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0xDFFF},                    U""),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\xDFFF',     same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0x0041, 0x0042, 0xD800},    U""),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(2, U'\xD800',     same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0x0041, 0xD800, 0x0042},    U""),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(1, U'\xD800',     same_as<CharT, wchar_t>)},

        {mk_unicode<char32_t>({0x11'0000},                 U""),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\x110000',   same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0x20'0000},                 U""),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\x200000',   same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0xFFFF'FFFF},               U""),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(0, U'\xFFFFFFFF', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char32_t>({0x0041, 0x0042, 0x11'0000}, U""),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(2, U'\x110000',   same_as<CharT, wchar_t>)},
        {mk_unicode<char32_t>({0x0041, 0x11'0000, 0x0042}, U""),
         unicode_conversion_error_factory::
             invalid_utf32_code_point(1, U'\x110000',   same_as<CharT, wchar_t>)},
    };
    vector<pair<basic_string<CharT>, unicode_conversion_error>> rv;
    auto fill_vector_func = [&]<typename T>(T list_arg)
    {
        for (auto& [unicode_str, err] : list_arg)
        {
            rv.push_back(make_pair(spl_convert<CharT>(unicode_str), err));
        }
    };
    if constexpr (same_as<char8_t, CharT>)
    {
        fill_vector_func(invalid_u8_errors);
    }
    else if constexpr (same_as<char16_t, CharT>)
    {
        fill_vector_func(invalid_u16_errors);
    }
    else if constexpr (same_as<char32_t, CharT>)
    {
        fill_vector_func(invalid_u32_errors);
    }
    else if constexpr (same_as<wchar_t, CharT>)
    {
        if constexpr (sizeof(CharT) == 2)
        {
            fill_vector_func(invalid_u16_errors);
        }
        else if constexpr (sizeof(CharT) == 4)
        {
            fill_vector_func(invalid_u32_errors);
        }
    }
    return rv;
}
} // namespace unicode_bridge_test_cases

TEST_CASE(
    "unicode_conversion and is_valid_unicode is correct for invalid "
    "u8strings",
    "[unicode_conversion,is_valid_unicode]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge;
    using namespace unicode_bridge_test_cases;
    auto error_results_u8
        = get_invalid_unicode_conversion_and_is_valid_unicode_data<char8_t>();
    run_unicode_conversion_and_is_valid_unicode_on_invalid_strings(
        error_results_u8
    );
}

TEST_CASE(
    "unicode_conversion and is_valid_unicode is correct for invalid "
    "u16strings",
    "[unicode_conversion,is_valid_unicode]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge;
    using namespace unicode_bridge_test_cases;
    auto error_results_u16
        = get_invalid_unicode_conversion_and_is_valid_unicode_data<char16_t>();
    run_unicode_conversion_and_is_valid_unicode_on_invalid_strings(
        error_results_u16
    );
}

TEST_CASE(
    "unicode_conversion and is_valid_unicode is correct for invalid "
    "u32strings",
    "[unicode_conversion,is_valid_unicode]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge;
    using namespace unicode_bridge_test_cases;
    auto error_results_u32
        = get_invalid_unicode_conversion_and_is_valid_unicode_data<char32_t>();
    run_unicode_conversion_and_is_valid_unicode_on_invalid_strings(
        error_results_u32
    );
}

TEST_CASE(
    "unicode_conversion and is_valid_unicode is correct for invalid "
    "wstrings",
    "[unicode_conversion,is_valid_unicode]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge;
    using namespace unicode_bridge_test_cases;
    auto error_results_ws
        = get_invalid_unicode_conversion_and_is_valid_unicode_data<wchar_t>();
    run_unicode_conversion_and_is_valid_unicode_on_invalid_strings(
        error_results_ws
    );
}

namespace unicode_bridge_testing
{
/*!
 * @brief Checks that next_char32_t returns the correct values.
 * @tparam CharT
 * @param valid_strs_arg
 */
template <typename CharT>
inline void
    run_valid_next_char32_t_checks(
        const std::vector<std::pair<
            std::basic_string<CharT>,
            std::vector<std::pair<char32_t, std::size_t>>>>& valid_strs_arg
    )
{
    using namespace std;
    using namespace unicode_bridge;
    for (auto&& [str, expected_output] : valid_strs_arg)
    {
        INFO("str = " << convert_unicode_to_string(str));
        auto test_func = [&]<typename U>(const U input_argument)
        {
            auto   current_iterator = std::begin(input_argument);
            auto   end_iterator     = std::end(input_argument);
            size_t idx{1};
            for (auto& [unicode_char, iterator_index] : expected_output)
            {
                INFO(
                    "Expected output index = "
                    << idx << " Expected character = `"
                    << convert_unicode_to_string(
                           basic_string<CharT>(1, unicode_char)
                       )
                    << ", iterator index = " << iterator_index
                );
                ++idx;
                auto test_result_1 = [&](const auto& pair_arg)
                {
                    REQUIRE(pair_arg.first == unicode_char);
                    REQUIRE(pair_arg.second == iterator_index);
                };
                // Test with error returned.
                auto res = next_char32<true>(current_iterator, end_iterator);
                if (res.has_value())
                {
                    test_result_1(res.value());
                }
                else
                {
                    FAIL(
                        "Unexpected error \"" << convert_unicode_to_string(
                            res.error().message(u8"", u8"", std::nullopt)
                        ) << "\""
                    );
                }
                // Test without error returned.
                auto res2 = next_char32<false>(current_iterator, end_iterator);
                if (res2.has_value())
                {
                    test_result_1(res2.value());
                }
                else
                {
                    FAIL("Unexpected empty optional");
                }

                try
                {
                    // Checks that with_exception also returns the
                    // correct value.
                    auto res2a = next_char32_with_exception(
                        current_iterator, end_iterator
                    );
                    test_result_1(res2a);
                }
                catch (const unicode_bridge_exception<next_char32_error>&
                           unexpected_exception)
                {
                    string msg = "Unexpected exception: \"";
                    msg.append(convert_unicode_to_string(
                        unexpected_exception.error().message(
                            u8"", u8"", std::nullopt
                        )
                    ));
                    msg.append("\"");
                    FAIL(msg);
                }
                catch (...)
                {
                    string msg = "Unexpected, unknown exception";
                    FAIL(msg);
                }

                auto new_iterator  = current_iterator;

                auto test_result_2 = [&](const auto& character_arg)
                {
                    CHECK(character_arg == unicode_char);
                    CHECK(
                        std::distance(current_iterator, new_iterator)
                        == iterator_index
                    );
                };

                // Next with error returned and increment iterator.
                auto res3 = next_char32_and_increment_iterator<true>(
                    new_iterator, end_iterator
                );
                if (res3.has_value())
                {
                    test_result_2(res3.value());
                }
                else
                {
                    auto str_as_u8
                        = res.error().message(u8"", u8"", std::nullopt);
                    auto str_containing_u8
                        = string(str_as_u8.begin(), str_as_u8.end());
                    FAIL("Unexpected error \"" << str_containing_u8);
                }

                // Next with no error returned and increment iterator.
                // Reset iterator.
                new_iterator = current_iterator;
                auto res4    = next_char32_and_increment_iterator<false>(
                    new_iterator, end_iterator
                );
                if (res4.has_value())
                {
                    test_result_2(res4.value());
                }
                else
                {
                    FAIL("Unexpected empty optional");
                }
                new_iterator = current_iterator;
                try
                {
                    // Checks that with_exception also returns the
                    // correct value.
                    auto res4a
                        = next_char32_and_increment_iterator_with_exception(
                            new_iterator, end_iterator
                        );
                    test_result_2(res4a);
                }
                catch (const unicode_bridge_exception<next_char32_error>&
                           unexpected_exception)
                {
                    string msg = "Unexpected exception: \"";
                    msg.append(convert_unicode_to_string(
                        unexpected_exception.error().message(
                            u8"", u8"", std::nullopt
                        )
                    ));
                    msg.append("\"");
                    FAIL(msg);
                }
                catch (...)
                {
                    string msg = "Unexpected, unknown exception";
                    FAIL(msg);
                }

                // Incrmeent iterator to the next code point for hte
                // next test.
                next_char32_and_increment_iterator<false>(
                    current_iterator, end_iterator
                );
            }
        };
        // Run all string tests but without single char, or c string.
        run_all_string_types<decltype(test_func), decltype(str), false, false>(
            test_func, str
        );
    }
}
} // namespace unicode_bridge_testing

namespace unicode_bridge_test_cases
{
template <typename CharT>
std::vector<std::pair<
    std::basic_string<CharT>,
    std::vector<std::pair<char32_t, std::size_t>>>>
    get_valid_next_char_32_t_data()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge::internal;
    std::array<std::u8string, 4> arr;
    initializer_list<pair<u8string, vector<pair<char32_t, size_t>>>>
        valid_results_u8 = {
            // single ASCII character
            {u8"a",
             {
                 {U'a', 1},
             }},

            // all four byte-length encodings
            {u8"a£→𝄞",
             {
                 {U'a', 1},
                 {U'£', 2},
                 {U'→', 3},
                 {U'𝄞', 4},
             }},

            // all 2-byte, tests no byte-length transitions
            {u8"£©®",
             {
                 {U'£', 2},
                 {U'©', 2},
                 {U'®', 2},
             }},

            // mixed ASCII and multibyte with a 4-byte in the middle
            {u8"a😀b",
             {
                 {U'a', 1},
                 {U'😀', 4},
                 {U'b', 1},
             }},

            // modifier/combining character mixed with others
            {u8"aⷽ→",
             {
                 {U'a', 1},
                 {U'ⷽ', 3},
                 {U'→', 3},
             }},
    };
    initializer_list<pair<u16string, vector<pair<char32_t, size_t>>>>
        valid_results_u16 = {
            // single ASCII character
            {u"a",
             {
                 {U'a', 1},
             }},

            // BMP only, all 1 code unit
            {u"a→£",
             {
                 {U'a', 1},
                 {U'→', 1},
                 {U'£', 1},
             }},

            // surrogate pair only
            {u"𝄞",
             {
                 {U'𝄞', 2},
             }},

            // BMP mixed with surrogate pairs
            {u"a→𝄞£😀",
             {
                 {U'a', 1},
                 {U'→', 1},
                 {U'𝄞', 2},
                 {U'£', 1},
                 {U'😀', 2},
             }},

            // consecutive surrogate pairs
            {u"𝄞😀",
             {
                 {U'𝄞', 2},
                 {U'😀', 2},
             }},
    };
    initializer_list<pair<u32string, vector<pair<char32_t, size_t>>>>
        valid_results_u32 = {
            // single ASCII character
            {U"a",
             {
                 {U'a', 1},
             }},

            // mix of ranges, all 1 code unit
            {U"a→𝄞",
             {
                 {U'a', 1},
                 {U'→', 1},
                 {U'𝄞', 1},
             }},

            // supplementary plane characters
            {U"𝄞😀",
             {
                 {U'𝄞', 1},
                 {U'😀', 1},
             }},

            // boundary codepoints
            {U"\u007F\u0080\u07FF\u0800",
             {
                 {U'\u007F', 1},
                 {U'\u0080', 1},
                 {U'\u07FF', 1},
                 {U'\u0800', 1},
             }},

            // modifier mixed with others
            {U"aⷽ→",
             {
                 {U'a', 1},
                 {U'ⷽ', 1},
                 {U'→', 1},
             }},
    };
    vector<pair<basic_string<CharT>, vector<pair<char32_t, size_t>>>> rv;
    auto fill_vector_func = [&]<typename T>(T list_arg)
    {
        for (auto& [unicode_str, vect] : list_arg)
        {
            rv.push_back(make_pair(spl_convert<CharT>(unicode_str), vect));
        }
    };
    if constexpr (same_as<char8_t, CharT>)
    {
        fill_vector_func(valid_results_u8);
    }
    else if constexpr (same_as<char16_t, CharT>)
    {
        fill_vector_func(valid_results_u16);
    }
    else if constexpr (same_as<char32_t, CharT>)
    {
        fill_vector_func(valid_results_u32);
    }
    else if constexpr (same_as<wchar_t, CharT>)
    {
        if constexpr (sizeof(CharT) == 2)
        {
            fill_vector_func(valid_results_u16);
        }
        else if constexpr (sizeof(CharT) == 4)
        {
            fill_vector_func(valid_results_u32);
        }
    }
    return rv;
}
} // namespace unicode_bridge_test_cases

TEST_CASE(
    "next_char32_t and next_char32_t_and_increment_iterator are correct "
    "for "
    "char8_t arguments",
    "[next_char32_t,next_char32_t_and_increment_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    run_valid_next_char32_t_checks(get_valid_next_char_32_t_data<char8_t>());
}

TEST_CASE(
    "next_char32_t and next_char32_t_and_increment_iterator are correct "
    "for "
    "char16_t arguments",
    "[next_char32_t,next_char32_t_and_increment_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    run_valid_next_char32_t_checks(get_valid_next_char_32_t_data<char16_t>());
}

TEST_CASE(
    "next_char32_t and next_char32_t_and_increment_iterator are correct "
    "for "
    "char32_t arguments",
    "[next_char32_t,next_char32_t_and_increment_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    run_valid_next_char32_t_checks(get_valid_next_char_32_t_data<char32_t>());
}

TEST_CASE(
    "next_char32_t and next_char32_t_and_increment_iterator are correct "
    "for "
    "wchar_t arguments",
    "[next_char32_t,next_char32_t_and_increment_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    run_valid_next_char32_t_checks(get_valid_next_char_32_t_data<wchar_t>());
}

namespace unicode_bridge_testing
{
template <typename CharT>
inline void
    run_invalid_next_char32_t_checks(
        const std::vector<std::tuple<
            std::basic_string<CharT>,
            size_t,
            unicode_bridge::unicode_conversion_error>>& invalid_strs_arg
    )
{
    using namespace std;
    using namespace unicode_bridge;
    size_t info_idx{1};
    auto   char_printer = [&](const CharT character_arg)
    {
        return convert_unicode_to_string(basic_string<CharT>(1, character_arg));
    };
    for (auto&& [str, idx, expected_output] : invalid_strs_arg)
    {
        INFO(info_idx);
        ++info_idx;
        INFO("str = " << convert_unicode_to_string(str));
        auto test_func = [&]<typename U>(const U input_argument)
        {
            auto current_iterator  = std::begin(input_argument);
            auto end_iterator      = std::end(input_argument);
            current_iterator      += idx;

            auto res = next_char32<true>(current_iterator, end_iterator);
            if (res.has_value())
            {
                FAIL(
                    "Unexpected success. Values returned = `"
                    << char_printer(res.value().first) << "', "
                    << res.value().second
                );
            }
            else
            {
                REQUIRE(
                    res.error().get_enum()
                    == next_char32_error::next_char32_error_code::unicode_error
                );
                equal_unicode_error(res.error().error(), expected_output);
            }

            auto res2 = next_char32<false>(current_iterator, end_iterator);
            if (res2.has_value())
            {
                FAIL(
                    "Unexpected success. Values returned = `"
                    << char_printer(res.value().first) << "', "
                    << res.value().second
                );
            }
            else
            {
                SUCCEED("Returned std::nullopt, as expected");
            }

            try
            {
                auto res2a = next_char32_with_exception(
                    current_iterator, end_iterator
                );
                FAIL("Unexpected successful call of "
                     "next_char32_t_with_exception.");
            }
            catch (const unicode_bridge_exception<next_char32_error>& _exception
            )
            {
                REQUIRE(
                    res.error().get_enum()
                    == next_char32_error::next_char32_error_code::unicode_error
                );
                equal_unicode_error(
                    _exception.error().error(), expected_output
                );
            }
            catch (...)
            {
                string msg = "Unexpected, unknown exception";
                FAIL(msg);
            }

            auto new_iterator = current_iterator;
            auto res3         = next_char32_and_increment_iterator<true>(
                new_iterator, end_iterator
            );
            if (res3.has_value())
            {
                FAIL(
                    "Unexpected success. Values returned = `"
                    << char_printer(res.value().first) << "'"
                );
            }
            else
            {
                REQUIRE(
                    res.error().get_enum()
                    == next_char32_error::next_char32_error_code::unicode_error
                );
                equal_unicode_error(res.error().error(), expected_output);
                CHECK(std::distance(new_iterator, current_iterator) == 0);
            }

            new_iterator = current_iterator;
            auto res4    = next_char32_and_increment_iterator<false>(
                new_iterator, end_iterator
            );
            if (res4.has_value())
            {
                FAIL(
                    "Unexpected success. Values returned = `"
                    << char_printer(res.value().first) << "'"
                );
            }
            else
            {
                CHECK(std::distance(new_iterator, current_iterator) == 0);
            }
            try
            {
                auto res4a
                    = next_char32_and_increment_iterator_with_exception(
                        current_iterator, end_iterator
                    );
                FAIL("Unexpected successful call of "
                     "next_char32_t_with_exception.");
            }
            catch (const unicode_bridge_exception<next_char32_error>& _exception
            )
            {
                REQUIRE(
                    res.error().get_enum()
                    == next_char32_error::next_char32_error_code::unicode_error
                );
                equal_unicode_error(
                    _exception.error().error(), expected_output
                );
            }
            catch (...)
            {
                string msg = "Unexpected, unknown exception";
                FAIL(msg);
            }
        };
        run_all_string_types<decltype(test_func), decltype(str), false, false>(
            test_func, str
        );
    }
}
} // namespace unicode_bridge_testing

namespace unicode_bridge_test_cases
{
template <typename CharT>
std::vector<std::tuple<
    std::basic_string<CharT>,
    std::size_t,
    unicode_bridge::unicode_conversion_error>>
    get_invalid_next_char32_t_data()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge::internal;
    std::array<std::u8string, 4> arr;
    initializer_list<tuple<u8string, size_t, unicode_conversion_error>>
        invalid_u8_errors = {
            // invalid leading byte at offset 0
            {mk_unicode<char8_t>({0xFF}),
             0, unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0xFF)
             )},
            {mk_unicode<char8_t>({0x80}),
             0, unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0x80)
             )},
            {mk_unicode<char8_t>({0xBF}),
             0, unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0xBF)
             )},

            // invalid leading byte mid-string
            {u8string(u8"a") + mk_unicode<char8_t>({0xFF}),
             1, unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0xFF)
             )},
            {u8string(u8"ab") + mk_unicode<char8_t>({0x80}),
             2, unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0x80)
             )},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xFE}),
             3, unicode_conversion_error_factory::invalid_leading_byte(
                 0, static_cast<char8_t>(0xFE)
             )},

            // truncated sequence at offset 0
            {mk_unicode<char8_t>({0xC2}),
             0, unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xC2), u8'\0', u8'\0', u8'\0'}, 1, 2
             )},
            {mk_unicode<char8_t>({0xE2}),
             0, unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xE2), u8'\0', u8'\0', u8'\0'}, 1, 3
             )},
            {mk_unicode<char8_t>({0xE2, 0x80}),
             0, unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x80),
                  u8'\0',
                  u8'\0'},
             2, 3
             )},
            {mk_unicode<char8_t>({0xF0}),
             0, unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xF0), u8'\0', u8'\0', u8'\0'}, 1, 4
             )},
            {mk_unicode<char8_t>({0xF0, 0x90}),
             0, unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  u8'\0',
                  u8'\0'},
             2, 4
             )},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x80}),
             0, unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  static_cast<char8_t>(0x80),
                  u8'\0'},
             3, 4
             )},
            // truncated sequence mid-string
            {u8string(u8"a") + mk_unicode<char8_t>({0xC2}),
             1, unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xC2), u8'\0', u8'\0', u8'\0'}, 1, 2
             )},
            {u8string(u8"ab") + mk_unicode<char8_t>({0xE2, 0x80}),
             2, unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x80),
                  u8'\0',
                  u8'\0'},
             2, 3
             )},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xF0, 0x90, 0x80}),
             3, unicode_conversion_error_factory::truncated_sequence(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  static_cast<char8_t>(0x80),
                  u8'\0'},
             3, 4
             )},

            // invalid continuation byte at offset 0
            {mk_unicode<char8_t>({0xC2, 0x20}),
             0, unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xC2),
                  static_cast<char8_t>(0x20),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             2, 2
             )},
            {mk_unicode<char8_t>({0xE2, 0x21, 0x80}),
             0, unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x21),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             3, 2
             )},
            {mk_unicode<char8_t>({0xE2, 0x80, 0x20}),
             0, unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x20),
                  static_cast<char8_t>(0x00)},
             3, 3
             )},
            {mk_unicode<char8_t>({0xF0, 0x20, 0x80, 0x80}),
             0, unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x20),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             4, 2
             )},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x20, 0x80}),
             0, unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  static_cast<char8_t>(0x20),
                  static_cast<char8_t>(0x80)},
             4, 3
             )},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x80, 0x20}),
             0, unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x20)},
             4, 4
             )},
            // invalid continuation byte mid-string
            {u8string(u8"a") + mk_unicode<char8_t>({0xC2, 0x20}),
             1, unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xC2),
                  static_cast<char8_t>(0x20),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             2, 2
             )},
            {u8string(u8"ab") + mk_unicode<char8_t>({0xE2, 0x80, 0x20}),
             2, unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xE2),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x20),
                  static_cast<char8_t>(0x00)},
             3, 3
             )},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xF0, 0x90, 0x80, 0x20}),
             3, unicode_conversion_error_factory::invalid_continuation_byte(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x90),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x20)},
             4, 4
             )},
            // overlong at offset 0
            {mk_unicode<char8_t>({0xC0, 0x80}),
             0, unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xC0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             2, U'\0'
             )},
            {mk_unicode<char8_t>({0xC1, 0xBF}),
             0, unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xC1),
                  static_cast<char8_t>(0xBF),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             2, U'\u007F'
             )},
            {mk_unicode<char8_t>({0xE0, 0x9F, 0xBF}),
             0, unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x9F),
                  static_cast<char8_t>(0xBF),
                  static_cast<char8_t>(0x00)},
             3, U'\u07FF'
             )},
            {mk_unicode<char8_t>({0xF0, 0x80, 0x80, 0x80}),
             0, unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             4, U'\0'
             )},

            // overlong mid-string
            {u8string(u8"a") + mk_unicode<char8_t>({0xC0, 0x80}),
             1, unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xC0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             2, U'\0'
             )},
            {u8string(u8"ab") + mk_unicode<char8_t>({0xE0, 0x9F, 0xBF}),
             2, unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x9F),
                  static_cast<char8_t>(0xBF),
                  static_cast<char8_t>(0x00)},
             3, U'\u07FF'
             )},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xF0, 0x80, 0x80, 0x80}),
             3, unicode_conversion_error_factory::overlong_encoding(
                 0, {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             4, U'\0'
             )},
            // surrogates / out of range at offset 0
            {mk_unicode<char8_t>({0xED, 0xA0, 0x80}),
             0, unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xA0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x00)},
             3, U'\xD800'
                 )},
            {mk_unicode<char8_t>({0xED, 0xBF, 0xBF}),
             0, unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0x00)},
             3, U'\xDFFF'
                 )},
            {mk_unicode<char8_t>({0xF4, 0x90, 0x80, 0x80}),
             0, unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xF4),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             4, U'\x110000'
                 )},
            // surrogates / out of range mid-string
            {u8string(u8"a") + mk_unicode<char8_t>({0xED, 0xA0, 0x80}),
             1, unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xA0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x00)},
             3, U'\xD800'
                 )},
            {u8string(u8"ab") + mk_unicode<char8_t>({0xED, 0xBF, 0xBF}),
             2, unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0x00)},
             3, U'\xDFFF'
                 )},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xF4, 0x90, 0x80, 0x80}),
             3, unicode_conversion_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     0, {static_cast<char8_t>(0xF4),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             4, U'\x110000'
                 )},
            // advance into the middle of a 2-byte sequence
            {u8"£",
             1, unicode_conversion_error_factory::invalid_leading_byte(0, '\xA3')},

            // advance into the middle of a 3-byte sequence
            {u8"→",
             1, unicode_conversion_error_factory::invalid_leading_byte(0, '\x86')},
            {u8"→",
             2, unicode_conversion_error_factory::invalid_leading_byte(0, '\x92')},

            // advance into the middle of a 4-byte sequence
            {u8"𝄞",
             1, unicode_conversion_error_factory::invalid_leading_byte(0, '\x9D')},
            {u8"𝄞",
             2, unicode_conversion_error_factory::invalid_leading_byte(0, '\x84')},
            {u8"𝄞",
             3, unicode_conversion_error_factory::invalid_leading_byte(0, '\x9E')},

            // advance past ASCII into middle of multibyte
            {u8"a£",
             2, unicode_conversion_error_factory::invalid_leading_byte(0, '\xA3')},
            {u8"a→",
             2, unicode_conversion_error_factory::invalid_leading_byte(0, '\x86')},
            {u8"a→",
             3, unicode_conversion_error_factory::invalid_leading_byte(0, '\x92')},
            {u8"a𝄞",
             2, unicode_conversion_error_factory::invalid_leading_byte(0, '\x9D')},

            // advance past a valid multibyte into middle of another
            {u8"£→",
             3, unicode_conversion_error_factory::invalid_leading_byte(0, '\x86')},
            {u8"→𝄞",
             4, unicode_conversion_error_factory::invalid_leading_byte(0, '\x9D')},
            {u8"→𝄞",
             5, unicode_conversion_error_factory::invalid_leading_byte(0, '\x84')},
    };

    initializer_list<tuple<u16string, size_t, unicode_conversion_error>> invalid_u16_errors = {
        // high surrogate then end of stream at offset 0
        {mk_unicode<char16_t>({0xD800}),
         0, unicode_conversion_error_factory::
             high_surrogate_then_end_of_stream(0, u'\xD800', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xDBFF}),
         0, unicode_conversion_error_factory::
             high_surrogate_then_end_of_stream(0, u'\xDBFF', same_as<CharT, wchar_t>)
        },

        // high surrogate then end of stream mid-string
        {u16string(u"a") + mk_unicode<char16_t>({0xD800}),
         1, unicode_conversion_error_factory::
             high_surrogate_then_end_of_stream(0, u'\xD800', same_as<CharT, wchar_t>)
        },
        {u16string(u"ab") + mk_unicode<char16_t>({0xDBFF}),
         2, unicode_conversion_error_factory::
             high_surrogate_then_end_of_stream(0, u'\xDBFF', same_as<CharT, wchar_t>)
        },
        {u16string(u"abc") + mk_unicode<char16_t>({0xD800}),
         3, unicode_conversion_error_factory::
             high_surrogate_then_end_of_stream(0, u'\xD800', same_as<CharT, wchar_t>)
        },

        // high surrogate not followed by low surrogate at offset 0
        {mk_unicode<char16_t>({0xD800, 0xD801}),
         0, unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(0, u'\xD800', u'\xD801', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xDBFF, 0x4E00}),
         0, unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(0, u'\xDBFF', u'\x4E00', same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xDBFF}, u"hello"),
         0, unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(0, u'\xDBFF', u'h', same_as<CharT, wchar_t>)
        },

        // high surrogate not followed by low surrogate mid-string
        {u16string(u"a") + mk_unicode<char16_t>({0xD800, 0xD801}),
         1, unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(0, u'\xD800', u'\xD801', same_as<CharT, wchar_t>)
        },
        {u16string(u"ab") + mk_unicode<char16_t>({0xDBFF, 0x4E00}),
         2, unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(0, u'\xDBFF', u'\x4E00', same_as<CharT, wchar_t>)
        },
        {u16string(u"abc") + mk_unicode<char16_t>({0xDBFF}, u"hello"),
         3, unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(0, u'\xDBFF', u'h', same_as<CharT, wchar_t>)
        },

        // unexpected low surrogate at offset 0
        {mk_unicode<char16_t>({0xDC00}),
         0, unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDC00', same_as<CharT, wchar_t>)},
        {mk_unicode<char16_t>({0xDFFF}),
         0, unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDFFF', same_as<CharT, wchar_t>)},
        {mk_unicode<char16_t>({0xDC00, 0xDC01}),
         0, unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDC00', same_as<CharT, wchar_t>)},

        // unexpected low surrogate mid-string
        {u16string(u"a") + mk_unicode<char16_t>({0xDC00}),
         1, unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDC00', same_as<CharT, wchar_t>)},
        {u16string(u"ab") + mk_unicode<char16_t>({0xDFFF}),
         2, unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDFFF', same_as<CharT, wchar_t>)},
        {u16string(u"abc") + mk_unicode<char16_t>({0xDC00}),
         3, unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDC00', same_as<CharT, wchar_t>)},
        // advance into the low surrogate of a pair
        {u"𝄞",
         1, unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDD1E', same_as<CharT, wchar_t>)},
        {u"😀",
         1, unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDE00', same_as<CharT, wchar_t>)},

        // advance past BMP character into low surrogate
        {u"a𝄞",
         2, unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDD1E', same_as<CharT, wchar_t>)},
        {u"→𝄞",
         2, unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDD1E', same_as<CharT, wchar_t>)},

        // advance past one surrogate pair into low surrogate of
        // next
        {u"𝄞😀",
         3, unicode_conversion_error_factory::
             unexpected_low_surrogate(0, u'\xDE00', same_as<CharT, wchar_t>)},
    };
    initializer_list<tuple<u32string, size_t, unicode_conversion_error>>
        invalid_u32_errors = {
            // at offset 0
            {mk_unicode<char32_t>({0xD800}),
             0, unicode_conversion_error_factory::
                 invalid_utf32_code_point(0, U'\xD800',     same_as<CharT, wchar_t>)
            },
            {mk_unicode<char32_t>({0xDFFF}),
             0, unicode_conversion_error_factory::
                 invalid_utf32_code_point(0, U'\xDFFF',     same_as<CharT, wchar_t>)
            },
            {mk_unicode<char32_t>({0x11'0000}),
             0, unicode_conversion_error_factory::
                 invalid_utf32_code_point(0, U'\x110000',   same_as<CharT, wchar_t>)
            },
            {mk_unicode<char32_t>({0xFFFF'FFFF}),
             0, unicode_conversion_error_factory::
                 invalid_utf32_code_point(0, U'\xFFFFFFFF', same_as<CharT, wchar_t>)
            },

            // mid-string
            {u32string(U"a") + mk_unicode<char32_t>({0xD800}),
             1, unicode_conversion_error_factory::
                 invalid_utf32_code_point(0, U'\xD800',     same_as<CharT, wchar_t>)
            },
            {u32string(U"ab") + mk_unicode<char32_t>({0xDFFF}),
             2, unicode_conversion_error_factory::
                 invalid_utf32_code_point(0, U'\xDFFF',     same_as<CharT, wchar_t>)
            },
            {u32string(U"abc") + mk_unicode<char32_t>({0x11'0000}),
             3, unicode_conversion_error_factory::
                 invalid_utf32_code_point(0, U'\x110000',   same_as<CharT, wchar_t>)
            },
            {u32string(U"a") + mk_unicode<char32_t>({0xFFFF'FFFF}),
             1, unicode_conversion_error_factory::
                 invalid_utf32_code_point(0, U'\xFFFFFFFF', same_as<CharT, wchar_t>)
            },
    };
    vector<tuple<basic_string<CharT>, size_t, unicode_conversion_error>> rv;
    auto fill_vector_func = [&]<typename T>(T list_arg)
    {
        for (auto& [unicode_str, size, err] : list_arg)
        {
            rv.push_back(make_tuple(spl_convert<CharT>(unicode_str), size, err)
            );
        }
    };
    if constexpr (same_as<char8_t, CharT>)
    {
        fill_vector_func(invalid_u8_errors);
    }
    else if constexpr (same_as<char16_t, CharT>)
    {
        fill_vector_func(invalid_u16_errors);
    }
    else if constexpr (same_as<char32_t, CharT>)
    {
        fill_vector_func(invalid_u32_errors);
    }
    else if constexpr (same_as<wchar_t, CharT>)
    {
        if constexpr (sizeof(CharT) == 2)
        {
            fill_vector_func(invalid_u16_errors);
        }
        else if constexpr (sizeof(CharT) == 4)
        {
            fill_vector_func(invalid_u32_errors);
        }
    }
    return rv;
}
} // namespace unicode_bridge_test_cases

TEST_CASE(
    "next_char32_t and next_char32_t_and_increment_iterator are correct "
    "for "
    "invalid char8_t arguments",
    "[next_char32_t,next_char32_t_and_increment_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge;
    using namespace unicode_bridge_test_cases;
    auto error_results_u8 = get_invalid_next_char32_t_data<char8_t>();
    run_invalid_next_char32_t_checks(error_results_u8);
}

TEST_CASE(
    "next_char32_t and next_char32_t_and_increment_iterator are correct "
    "for "
    "invalid char16_t arguments",
    "[next_char32_t,next_char32_t_and_increment_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge;
    using namespace unicode_bridge_test_cases;
    auto error_results_u16 = get_invalid_next_char32_t_data<char16_t>();
    run_invalid_next_char32_t_checks(error_results_u16);
}

TEST_CASE(
    "next_char32_t and next_char32_t_and_increment_iterator are correct "
    "for "
    "invalid char32_t arguments",
    "[next_char32_t,next_char32_t_and_increment_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge;
    using namespace unicode_bridge_test_cases;
    auto error_results_u32 = get_invalid_next_char32_t_data<char32_t>();
    run_invalid_next_char32_t_checks(error_results_u32);
}

TEST_CASE(
    "next_char32_t and next_char32_t_and_increment_iterator are correct "
    "for "
    "invalid wchar_t arguments",
    "[next_char32_t,next_char32_t_and_increment_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge;
    using namespace unicode_bridge_test_cases;
    auto error_results_ws = get_invalid_next_char32_t_data<wchar_t>();
    run_invalid_next_char32_t_checks(error_results_ws);
}

namespace unicode_bridge_testing
{
/*!
 * @brief Tests that test_unicode_to_formatted_u8string returns the
 * correct output.
 * @tparam CharT The character type of the input parameter.
 * @param list_arg The list of value pairs to test.
 */
template <typename CharT>
inline void
    test_to_formatted_unicode_string(
        const std::vector<std::pair<std::basic_string<CharT>, std::u8string>>&
            list_arg
    )
{
    using namespace unicode_bridge;
    using namespace std;
    for (auto& [str, output_arg] : list_arg)
    {
        INFO(
            "input string = \"" << convert_unicode_to_string(str)
                                << "\", expected output = \""
                                << convert_unicode_to_string(output_arg) << "\""
        );
        auto test_func = [&]<typename T>(T input_arg)
        {
            auto u16_output
                = unicode_conversion_with_exception<char16_t>(output_arg);
            auto u32_output
                = unicode_conversion_with_exception<char32_t>(output_arg);
            auto ws_output
                = unicode_conversion_with_exception<wchar_t>(output_arg);
            std::u8string result1 = to_formatted_unicode_string<char8_t>(input_arg);
            REQUIRE(result1 == output_arg);
            auto result2 = to_formatted_unicode_string<char16_t>(input_arg);
            REQUIRE(result2 == u16_output);
            auto result3 = to_formatted_unicode_string<char32_t>(input_arg);
            REQUIRE(result3 == u32_output);
            auto result4 = to_formatted_unicode_string<wchar_t>(input_arg);
            REQUIRE(result4 == ws_output);
        };
        run_all_string_types(test_func, str);
    }
}
} // namespace unicode_bridge_testing

namespace unicode_bridge_test_cases
{
template <typename CharT>
std::vector<std::pair<std::basic_string<CharT>, std::u8string>>
    get_to_formatted_unicode_string_data()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge::internal;
    initializer_list<pair<u8string, u8string>> input_output_pairs_u8 = {
        {u8"",                                                               u8""                },
        {u8"hello",                                                          u8"hello"           },
        {u8"a£→𝄞",                                                           u8"a£→𝄞"            },
        {u8"ℝℝℝ",                                                            u8"ℝℝℝ"             },
        {u8"aⷽ→",                                                             u8"aⷽ→"              },
        {u8"😀",                                                             u8"😀"              },
        {u8"a😀b",                                                           u8"a😀b"            },
        // single characters
        {mk_unicode<char8_t>({0x0}),                                         u8"\\0"             },
        {u8"\a",                                                             u8"\\a"             },
        {u8"\b",                                                             u8"\\b"             },
        {u8"\t",                                                             u8"\\t"             },
        {u8"\n",                                                             u8"\\n"             },
        {u8"\v",                                                             u8"\\v"             },
        {u8"\f",                                                             u8"\\f"             },
        {u8"\r",                                                             u8"\\r"             },
        {u8"\"",                                                             u8"\\\""            },
        {u8"\'",                                                             u8"\\'"             },
        {u8"\\",                                                             u8"\\\\"            },

        // embedded in normal strings
        {u8"a\tb",                                                           u8"a\\tb"           },
        {u8"a\nb",                                                           u8"a\\nb"           },
        {u8"a\rb",                                                           u8"a\\rb"           },
        {u8"a\\b",                                                           u8"a\\\\b"          },
        {u8"a\"b",                                                           u8"a\\\"b"          },
        {u8"a\'b",                                                           u8"a\\'b"           },

        // multiple escape sequences
        {u8"\t\n",                                                           u8"\\t\\n"          },
        {u8"\r\n",                                                           u8"\\r\\n"          },
        {u8"a\t\nb",                                                         u8"a\\t\\nb"        },

        // escape sequences with unicode
        {u8"£\t→",                                                           u8"£\\t→"           },
        {u8"😀\n😀",                                                         u8"😀\\n😀"         },
        // control characters (< 0x20, excluding escape sequences)
        {u8"\u0001",                                                         u8"\\u0001"         },
        {u8"\u0002",                                                         u8"\\u0002"         },
        {u8"\u0010",                                                         u8"\\u0010"         },
        {u8"\u001F",                                                         u8"\\u001F"         },

        // 0x7F-0x9F range
        {u8"\u007F",                                                         u8"\\u007F"         },
        {u8"\u0080",                                                         u8"\\u0080"         }, // U+0080
        {u8"\u009F",                                                         u8"\\u009F"         }, // U+009F

        // special unicode characters
        {u8"\u00A0",                                                         u8"\\u00A0"         }, // non-breaking space
        {u8"\u1680",                                                         u8"\\u1680"         }, // ogham space mark
        {u8"\u2006",                                                         u8"\\u2006"         }, // six-per-em space
        {u8"\u200B",                                                         u8"\\u200B"         }, // zero width space
        {u8"\u2028",                                                         u8"\\u2028"         }, // line separator
        {u8"\u2029",                                                         u8"\\u2029"         }, // paragraph separator
        {u8"\u3000",                                                         u8"\\u3000"         }, // ideographic space
        {u8"\uFEFF",                                                         u8"\\uFEFF"         }, // byte order mark
        {u8"\uFFFD",                                                         u8"\\uFFFD"         }, // replacement character

        // embedded in normal strings
        {u8"a\u00A0b",                                                       u8"a\\u00A0b"       }, // non-breaking space
        {u8"a\u2006b",                                                       u8"a\\u2006b"       }, // six-per-em space
        {u8"£\uFEFF→",                                                       u8"£\\uFEFF→"       },

        // multiple special characters
        {u8"\u00A0\u00A0",                                                   u8"\\u00A0\\u00A0"  },
        {u8"\u2006\u200B",                                                   u8"\\u2006\\u200B"  },
        // single invalid leading bytes
        {mk_unicode<char8_t>({0xFF}),                                        u8"\\xFF"           },
        {mk_unicode<char8_t>({0xFE}),                                        u8"\\xFE"           },
        {mk_unicode<char8_t>({0x80}),                                        u8"\\x80"           },
        {mk_unicode<char8_t>({0xBF}),                                        u8"\\xBF"           },

        // truncated sequences
        {mk_unicode<char8_t>({0xC2}),                                        u8"\\xC2"           },
        {mk_unicode<char8_t>({0xE2}),                                        u8"\\xE2"           },
        {mk_unicode<char8_t>({0xE2, 0x80}),                                  u8"\\xE2\\x80"      },
        {mk_unicode<char8_t>({0xF0}),                                        u8"\\xF0"           },
        {mk_unicode<char8_t>({0xF0, 0x90}),                                  u8"\\xF0\\x90"      },
        {mk_unicode<char8_t>({0xF0, 0x90, 0x80}),                            u8"\\xF0\\x90\\x80" },

        // invalid continuation bytes
        {mk_unicode<char8_t>({0xC2, 0x20}),                                  u8"\\xC2 "          },
        {mk_unicode<char8_t>({0xE2, 0x20, 0x80}),                            u8"\\xE2 \\x80"     },
        {mk_unicode<char8_t>({0xE2, 0x80, 0x20}),                            u8"\\xE2\\x80 "     },
        {mk_unicode<char8_t>({0xF0, 0x20, 0x80, 0x80}),                      u8"\\xF0 \\x80\\x80"},

        // overlong encodings - keep as hex, do not decode to code
        // point.
        {mk_unicode<char8_t>({0xC0, 0x80}),                                  u8"\\xC0\\x80"
        }, // decodes to U+0000
        {mk_unicode<char8_t>({0xC1, 0xBF}),                                  u8"\\xC1\\xBF"
        }, // decodes to U+007F
        {mk_unicode<char8_t>({0xE0, 0x9F, 0xBF}),                            u8"\\xE0\\x9F\\xBF"
        }, // decodes to U+07FF

        // surrogates - invalid utf8, so keep as hex.
        {mk_unicode<char8_t>({0xED, 0xA0, 0x80}),                            u8"\\xED\\xA0\\x80" },
        {mk_unicode<char8_t>({0xED, 0xBF, 0xBF}),                            u8"\\xED\\xBF\\xBF" },

        // malformed embedded in valid string
        {u8string(u8"a") + mk_unicode<char8_t>({0xFF}),                      u8"a\\xFF"          },
        {u8string(u8"a") + mk_unicode<char8_t>({0xC2}),                      u8"a\\xC2"          },
        {u8string(u8"a") + mk_unicode<char8_t>({0xC2, 0x20}),                u8"a\\xC2 "         },
        {u8string(u8"ab") + mk_unicode<char8_t>({0xE2, 0x80}),               u8"ab\\xE2\\x80"
        },
        // normal + escape
        {u8"hello\tworld",                                                   u8"hello\\tworld"   },
        {u8"£\n→",                                                           u8"£\\n→"           },

        // normal + hex codepoint
        {u8"a" + mk_unicode<char8_t>({0xC2, 0xA0}) + u8"b",                  u8"a\\u00A0b"       },
        {u8"→" + mk_unicode<char8_t>({0xEF, 0xBB, 0xBF}) + u8"£",            u8"→\\uFEFF£"
        },

        // normal + malformed
        {u8string(u8"hello") + mk_unicode<char8_t>({0xFF}),                  u8"hello\\xFF"      },
        {u8string(u8"£→") + mk_unicode<char8_t>({0xC2}),                     u8"£→\\xC2"         },

        // escape + hex codepoint
        {u8"\t\u00A0",                                                       u8"\\t\\u00A0"      },
        {u8"\u00A0\n",                                                       u8"\\u00A0\\n"      },

        // escape + malformed
        {u8string(u8"\t") + mk_unicode<char8_t>({0xFF}),                     u8"\\t\\xFF"        },
        {mk_unicode<char8_t>({0xFF}) + u8string(u8"\n"),                     u8"\\xFF\\n"        },

        // hex codepoint + malformed
        {u8string(u8"\u00A0") + mk_unicode<char8_t>({0xFF}),                 u8"\\u00A0\\xFF"    },
        {mk_unicode<char8_t>({0xFF, 0xC2, 0xA0}),                            u8"\\xFF\\u00A0"    },

        // all four categories
        {u8string(u8"a\t\u00A0") + mk_unicode<char8_t>({0xFF}),
         u8"a\\t\\u00A0\\xFF"                                                                    },
        {mk_unicode<char8_t>({0xC2}) + u8string(u8"\n£\uFEFF"),
         u8"\\xC2\\n£\\uFEFF"                                                                    },
        {u8string(u8"hello\n£") + u8"\u00A0"
             + mk_unicode<char8_t>({0xED, 0xA0, 0x80}) + u8"world",
         u8"hello\\n£\\u00A0\\xED\\xA0\\x80world"                                                },
    };
    initializer_list<pair<u16string, u8string>> input_output_pairs_u16 = {
        {u"", u8""},
        {u"hello", u8"hello"},
        {u"a£→𝄞", u8"a£→𝄞"},
        {u"ℝℝℝ", u8"ℝℝℝ"},
        {u"aⷽ→", u8"aⷽ→"},
        {u"😀", u8"😀"},
        {u"a😀b", u8"a😀b"},
        // single characters
        {mk_unicode<char16_t>({0x0}), u8"\\0"},
        {u"\a", u8"\\a"},
        {u"\b", u8"\\b"},
        {u"\t", u8"\\t"},
        {u"\n", u8"\\n"},
        {u"\v", u8"\\v"},
        {u"\f", u8"\\f"},
        {u"\r", u8"\\r"},
        {u"\"", u8"\\\""},
        {u"\'", u8"\\'"},
        {u"\\", u8"\\\\"},

        // embedded in normal strings
        {u"a\tb", u8"a\\tb"},
        {u"a\nb", u8"a\\nb"},
        {u"a\rb", u8"a\\rb"},
        {u"a\\b", u8"a\\\\b"},
        {u"a\"b", u8"a\\\"b"},
        {u"a\'b", u8"a\\'b"},

        // multiple escape sequences
        {u"\t\n", u8"\\t\\n"},
        {u"\r\n", u8"\\r\\n"},
        {u"a\t\nb", u8"a\\t\\nb"},

        // escape sequences with unicode
        {u"£\t→", u8"£\\t→"},
        {u"😀\n😀", u8"😀\\n😀"},
        // control characters (< 0x20, excluding escape sequences)
        {u"\u0001", u8"\\u0001"},
        {u"\u0002", u8"\\u0002"},
        {u"\u0010", u8"\\u0010"},
        {u"\u001F", u8"\\u001F"},

        // 0x7F-0x9F range
        {u"\u007F", u8"\\u007F"},
        {u"\u0080", u8"\\u0080"},
        {u"\u009F", u8"\\u009F"},

        // special unicode characters
        {u"\u00A0", u8"\\u00A0"}, // non-breaking space
        {u"\u1680", u8"\\u1680"}, // ogham space mark
        {u"\u2006", u8"\\u2006"}, // six-per-em space
        {u"\u200B", u8"\\u200B"}, // zero width space
        {u"\u2028", u8"\\u2028"}, // line separator
        {u"\u2029", u8"\\u2029"}, // paragraph separator
        {u"\u3000", u8"\\u3000"}, // ideographic space
        {u"\uFEFF", u8"\\uFEFF"}, // byte order mark
        {u"\uFFFD", u8"\\uFFFD"}, // replacement character

        // embedded in normal strings
        {u"a\u00A0b", u8"a\\u00A0b"}, // non-breaking space
        {u"a\u2006b", u8"a\\u2006b"}, // six-per-em space
        {u"£\uFEFF→", u8"£\\uFEFF→"},

        // multiple special characters
        {u"\u00A0\u00A0", u8"\\u00A0\\u00A0"},
        {u"\u2006\u200B", u8"\\u2006\\u200B"},
        // lone high surrogate at offset 0
        {mk_unicode<char16_t>({0xD800}), u8"\\uD800"},
        {mk_unicode<char16_t>({0xDBFF}), u8"\\uDBFF"},

        // lone high surrogate - end of stream
        {u16string(u"a") + mk_unicode<char16_t>({0xD800}), u8"a\\uD800"},
        {u16string(u"ab") + mk_unicode<char16_t>({0xDBFF}), u8"ab\\uDBFF"},

        // high surrogate followed by high surrogate
        {mk_unicode<char16_t>({0xD800, 0xD801}), u8"\\uD800\\uD801"},
        {mk_unicode<char16_t>({0xDBFF, 0xD800}), u8"\\uDBFF\\uD800"},

        // high surrogate followed by non-surrogate
        {mk_unicode<char16_t>({0xD800, 0x0041}), u8"\\uD800A"},
        {mk_unicode<char16_t>({0xDBFF}, u"中"), u8"\\uDBFF中"},

        // lone low surrogate at offset 0
        {mk_unicode<char16_t>({0xDC00}), u8"\\uDC00"},
        {mk_unicode<char16_t>({0xDFFF}), u8"\\uDFFF"},
        {mk_unicode<char16_t>({0xDC00, 0xDC01}), u8"\\uDC00\\uDC01"},

        // lone low surrogate mid-string
        {u16string(u"a") + mk_unicode<char16_t>({0xDC00}), u8"a\\uDC00"},
        {u16string(u"ab") + mk_unicode<char16_t>({0xDFFF}), u8"ab\\uDFFF"},
        {u16string(u"abc") + mk_unicode<char16_t>({0xDC00}), u8"abc\\uDC00"},

        // malformed embedded with valid content after
        {mk_unicode<char16_t>({0xD800}) + u16string(u"hello"), u8"\\uD800hello"
        },
        {mk_unicode<char16_t>({0xDC00}) + u16string(u"hello"), u8"\\uDC00hello"
        },
        // normal + escape
        {u"hello\tworld", u8"hello\\tworld"},
        {u"£\n→", u8"£\\n→"},

        // normal + hex codepoint
        {u"a\u00A0b", u8"a\\u00A0b"},
        {u"→\uFEFF£", u8"→\\uFEFF£"},

        // normal + malformed
        {u16string(u"hello") + mk_unicode<char16_t>({0xD800}), u8"hello\\uD800"
        },
        {u16string(u"£→") + mk_unicode<char16_t>({0xDC00}), u8"£→\\uDC00"},

        // escape + hex codepoint
        {u"\t\u00A0", u8"\\t\\u00A0"},
        {u"\u00A0\n", u8"\\u00A0\\n"},

        // escape + malformed
        {u16string(u"\t") + mk_unicode<char16_t>({0xD800}), u8"\\t\\uD800"},
        {mk_unicode<char16_t>({0xDC00}) + u16string(u"\n"), u8"\\uDC00\\n"},

        // hex codepoint + malformed
        {u16string(u"\u00A0") + mk_unicode<char16_t>({0xD800}),
         u8"\\u00A0\\uD800"},
        {mk_unicode<char16_t>({0xDC00}) + u16string(u"\u00A0"),
         u8"\\uDC00\\u00A0"},

        // all four categories
        {u16string(u"a\t") + u"\u00A0" + mk_unicode<char16_t>({0xD800}),
         u8"a\\t\\u00A0\\uD800"},
        {mk_unicode<char16_t>({0xDC00}) + u16string(u"\n£\uFEFF"),
         u8"\\uDC00\\n£\\uFEFF"},
        {u16string(u"hello\n£") + u"\u00A0"
             + mk_unicode<char16_t>({0xD800, 0xD801}) + u"world",
         u8"hello\\n£\\u00A0\\uD800\\uD801world"},
    };
    initializer_list<pair<u32string, u8string>> input_output_pairs_u32 = {
        {U"",                                                            u8""              },
        {U"hello",                                                       u8"hello"         },
        {U"a£→𝄞",                                                        u8"a£→𝄞"          },
        {U"ℝℝℝ",                                                         u8"ℝℝℝ"           },
        {U"aⷽ→",                                                          u8"aⷽ→"            },
        {U"😀",                                                          u8"😀"            },
        {U"a😀b",                                                        u8"a😀b"          },
        // single characters
        {mk_unicode<char32_t>({0x0}),                                    u8"\\0"           },
        {U"\a",                                                          u8"\\a"           },
        {U"\b",                                                          u8"\\b"           },
        {U"\t",                                                          u8"\\t"           },
        {U"\n",                                                          u8"\\n"           },
        {U"\v",                                                          u8"\\v"           },
        {U"\f",                                                          u8"\\f"           },
        {U"\r",                                                          u8"\\r"           },
        {U"\"",                                                          u8"\\\""          },
        {U"\'",                                                          u8"\\'"           },
        {U"\\",                                                          u8"\\\\"          },

        // embedded in normal strings
        {U"a\tb",                                                        u8"a\\tb"         },
        {U"a\nb",                                                        u8"a\\nb"         },
        {U"a\rb",                                                        u8"a\\rb"         },
        {U"a\\b",                                                        u8"a\\\\b"        },
        {U"a\"b",                                                        u8"a\\\"b"        },
        {U"a\'b",                                                        u8"a\\'b"         },

        // multiple escape sequences
        {U"\t\n",                                                        u8"\\t\\n"        },
        {U"\r\n",                                                        u8"\\r\\n"        },
        {U"a\t\nb",                                                      u8"a\\t\\nb"      },

        // escape sequences with unicode
        {U"£\t→",                                                        u8"£\\t→"         },
        {U"😀\n😀",                                                      u8"😀\\n😀"       },
        // control characters (< 0x20, excluding escape sequences)
        {U"\u0001",                                                      u8"\\u0001"       },
        {U"\u0002",                                                      u8"\\u0002"       },
        {U"\u0010",                                                      u8"\\u0010"       },
        {U"\u001F",                                                      u8"\\u001F"       },

        // 0x7F-0x9F range
        {U"\u007F",                                                      u8"\\u007F"       },
        {U"\u0080",                                                      u8"\\u0080"       },
        {U"\u009F",                                                      u8"\\u009F"       },

        // special unicode characters
        {U"\u00A0",                                                      u8"\\u00A0"       }, // non-breaking space
        {U"\u1680",                                                      u8"\\u1680"       }, // ogham space mark
        {U"\u2006",                                                      u8"\\u2006"       }, // six-per-em space
        {U"\u200B",                                                      u8"\\u200B"       }, // zero width space
        {U"\u2028",                                                      u8"\\u2028"       }, // line separator
        {U"\u2029",                                                      u8"\\u2029"       }, // paragraph separator
        {U"\u3000",                                                      u8"\\u3000"       }, // ideographic space
        {U"\uFEFF",                                                      u8"\\uFEFF"       }, // byte order mark
        {U"\uFFFD",                                                      u8"\\uFFFD"       }, // replacement character

        // embedded in normal strings
        {U"a\u00A0b",                                                    u8"a\\u00A0b"     }, // non-breaking space
        {U"a\u2006b",                                                    u8"a\\u2006b"     }, // six-per-em space
        {U"£\uFEFF→",                                                    u8"£\\uFEFF→"     },

        // multiple special characters
        {U"\u00A0\u00A0",                                                u8"\\u00A0\\u00A0"},
        {U"\u2006\u200B",                                                u8"\\u2006\\u200B"},
        // surrogates - use \uNNNN
        {mk_unicode<char32_t>({0xD800}),                                 u8"\\uD800"       },
        {mk_unicode<char32_t>({0xDBFF}),                                 u8"\\uDBFF"       },
        {mk_unicode<char32_t>({0xDC00}),                                 u8"\\uDC00"       },
        {mk_unicode<char32_t>({0xDFFF}),                                 u8"\\uDFFF"       },

        // surrogates mid-string
        {u32string(U"a") + mk_unicode<char32_t>({0xD800}),               u8"a\\uD800"      },
        {u32string(U"ab") + mk_unicode<char32_t>({0xDC00}),              u8"ab\\uDC00"     },
        {u32string(U"abc") + mk_unicode<char32_t>({0xDFFF}),             u8"abc\\uDFFF"    },
        {mk_unicode<char32_t>({0xD800}) + u32string(U"hello"),           u8"\\uD800hello"
        },
        {mk_unicode<char32_t>({0xDC00}) + u32string(U"hello"),           u8"\\uDC00hello"
        },

        // consecutive surrogates
        {mk_unicode<char32_t>({0xD800, 0xDC00}),                         u8"\\uD800\\uDC00"},
        {mk_unicode<char32_t>({0xDBFF, 0xDFFF}),                         u8"\\uDBFF\\uDFFF"},

        // out of range - use \UNNNNNNNN
        {mk_unicode<char32_t>({0x11'0000}),                              u8"\\U00110000"   },
        {mk_unicode<char32_t>({0x20'0000}),                              u8"\\U00200000"   },
        {mk_unicode<char32_t>({0xFFFF'FFFF}),                            u8"\\UFFFFFFFF"   },

        // out of range mid-string
        {u32string(U"a") + mk_unicode<char32_t>({0x11'0000}),            u8"a\\U00110000"  },
        {u32string(U"ab") + mk_unicode<char32_t>({0xFFFF'FFFF}),
         u8"ab\\UFFFFFFFF"                                                                 },
        {mk_unicode<char32_t>({0x11'0000}) + u32string(U"hello"),
         u8"\\U00110000hello"                                                              },

        // consecutive out of range
        {mk_unicode<char32_t>({0x11'0000, 0x20'0000}),
         u8"\\U00110000\\U00200000"                                                        },
        // normal + escape
        {U"hello\tworld",                                                u8"hello\\tworld" },
        {U"£\n→",                                                        u8"£\\n→"         },

        // normal + hex codepoint
        {U"a\u00A0b",                                                    u8"a\\u00A0b"     },
        {U"→\uFEFF£",                                                    u8"→\\uFEFF£"     },

        // normal + surrogate
        {u32string(U"hello") + mk_unicode<char32_t>({0xD800}),           u8"hello\\uD800"
        },
        {u32string(U"£→") + mk_unicode<char32_t>({0xDC00}),              u8"£→\\uDC00"     },

        // normal + out of range
        {u32string(U"hello") + mk_unicode<char32_t>({0x11'0000}),
         u8"hello\\U00110000"                                                              },
        {u32string(U"£→") + mk_unicode<char32_t>({0xFFFF'FFFF}),
         u8"£→\\UFFFFFFFF"                                                                 },

        // escape + hex codepoint
        {U"\t\u00A0",                                                    u8"\\t\\u00A0"    },
        {U"\u00A0\n",                                                    u8"\\u00A0\\n"    },

        // escape + surrogate
        {u32string(U"\t") + mk_unicode<char32_t>({0xD800}),              u8"\\t\\uD800"    },
        {mk_unicode<char32_t>({0xDC00}) + u32string(U"\n"),              u8"\\uDC00\\n"    },

        // escape + out of range
        {u32string(U"\t") + mk_unicode<char32_t>({0x11'0000}),
         u8"\\t\\U00110000"                                                                },
        {mk_unicode<char32_t>({0x11'0000}) + u32string(U"\n"),
         u8"\\U00110000\\n"                                                                },

        // hex codepoint + surrogate
        {u32string(U"\u00A0") + mk_unicode<char32_t>({0xD800}),
         u8"\\u00A0\\uD800"                                                                },
        {mk_unicode<char32_t>({0xDC00}) + u32string(U"\u00A0"),
         u8"\\uDC00\\u00A0"                                                                },

        // hex codepoint + out of range
        {u32string(U"\u00A0") + mk_unicode<char32_t>({0x11'0000}),
         u8"\\u00A0\\U00110000"                                                            },
        {mk_unicode<char32_t>({0x11'0000}) + u32string(U"\u00A0"),
         u8"\\U00110000\\u00A0"                                                            },

        // all categories
        {u32string(U"a\t") + U"\u00A0" + mk_unicode<char32_t>({0xD800}),
         u8"a\\t\\u00A0\\uD800"                                                            },
        {mk_unicode<char32_t>({0xDC00}) + u32string(U"\n£\uFEFF"),
         u8"\\uDC00\\n£\\uFEFF"                                                            },
        {u32string(U"hello\n£") + U"\u00A0" + mk_unicode<char32_t>({0xD800})
             + U"world",
         u8"hello\\n£\\u00A0\\uD800world"                                                  },
        {u32string(U"a\t") + U"\u00A0" + mk_unicode<char32_t>({0xD800})
             + mk_unicode<char32_t>({0x11'0000}) + U"b",
         u8"a\\t\\u00A0\\uD800\\U00110000b"                                                },
    };
    initializer_list<pair<string, u8string>> input_output_pairs_ascii = {
        {"",                                                    u8""                    },
        {"hello",                                               u8"hello"               },
        {"abc",                                                 u8"abc"                 },
        {"Hello World",                                         u8"Hello World"         },
        {"0123456789",                                          u8"0123456789"          },
        {"!@#$%^&*()",                                          u8"!@#$%^&*()"          },
        // single characters
        {make_str({0x0}),                                       u8"\\0"                 },
        {"\a",                                                  u8"\\a"                 },
        {"\b",                                                  u8"\\b"                 },
        {"\t",                                                  u8"\\t"                 },
        {"\n",                                                  u8"\\n"                 },
        {"\v",                                                  u8"\\v"                 },
        {"\f",                                                  u8"\\f"                 },
        {"\r",                                                  u8"\\r"                 },
        {"\"",                                                  u8"\\\""                },
        {"\'",                                                  u8"\\'"                 },
        {"\\",                                                  u8"\\\\"                },

        // embedded in normal strings
        {"a\tb",                                                u8"a\\tb"               },
        {"a\nb",                                                u8"a\\nb"               },
        {"a\rb",                                                u8"a\\rb"               },
        {"a\\b",                                                u8"a\\\\b"              },
        {"a\"b",                                                u8"a\\\"b"              },
        {"a\'b",                                                u8"a\\'b"               },

        // multiple escape sequences
        {"\t\n",                                                u8"\\t\\n"              },
        {"\r\n",                                                u8"\\r\\n"              },
        {"a\t\nb",                                              u8"a\\t\\nb"            },

        // escape sequences with normal ascii
        {"hello\tworld",                                        u8"hello\\tworld"       },
        {"line1\nline2",                                        u8"line1\\nline2"       },
        // control characters (< 0x20, excluding escape sequences)
        {"\x01",                                                u8"\\u0001"             },
        {"\x02",                                                u8"\\u0002"             },
        {"\x10",                                                u8"\\u0010"             },
        {"\x1F",                                                u8"\\u001F"             },

        // 0x7F
        {"\x7F",                                                u8"\\u007F"             },

        // embedded in normal strings
        {"a\x01"
         "b",                                          u8"a\\u0001b"           },
        {"a\x1F"
         "b",                                          u8"a\\u001Fb"           },
        {"a\x7F"
         "b",                                          u8"a\\u007Fb"           },

        // multiple control characters
        {"\x01\x02",                                            u8"\\u0001\\u0002"      },
        {"\x1F\x7F",                                            u8"\\u001F\\u007F"      },
        // single invalid bytes
        {make_str({'\x80'}),                                    u8"\\x80"               },
        {make_str({'\xFF'}),                                    u8"\\xFF"               },
        {make_str({'\xFE'}),                                    u8"\\xFE"               },
        {make_str({'\xA0'}),                                    u8"\\xA0"               },

        // invalid bytes mid-string
        {string("a") + make_str({'\x80'}),                      u8"a\\x80"              },
        {string("ab") + make_str({'\xFF'}),                     u8"ab\\xFF"             },
        {string("abc") + make_str({'\x80'}),                    u8"abc\\x80"            },
        {make_str({'\x80'}) + string("hello"),                  u8"\\x80hello"          },
        {make_str({'\xFF'}) + string("hello"),                  u8"\\xFFhello"          },

        // consecutive invalid bytes
        {make_str({'\x80', '\xFF'}),                            u8"\\x80\\xFF"          },
        {make_str({'\xFe', '\xFF'}),                            u8"\\xFE\\xFF"          },

        // invalid bytes surrounding valid string
        {make_str({'\x80'}) + string("a") + make_str({'\xFF'}), u8"\\x80a\\xFF"
        },
        // normal + escape
        {"hello\tworld",                                        u8"hello\\tworld"       },
        {"line1\nline2",                                        u8"line1\\nline2"       },

        // normal + control character
        {"a\x01"
         "b",                                          u8"a\\u0001b"           },
        {"a\x7F"
         "b",                                          u8"a\\u007Fb"           },

        // normal + invalid byte
        {string("hello") + make_str({'\xFF'}),                  u8"hello\\xFF"          },
        {string("abc") + make_str({'\x80'}),                    u8"abc\\x80"            },

        // escape + control character
        {"\t\x01",                                              u8"\\t\\u0001"          },
        {"\x01\n",                                              u8"\\u0001\\n"          },

        // escape + invalid byte
        {string("\t") + make_str({'\xFF'}),                     u8"\\t\\xFF"            },
        {make_str({'\x80'}) + string("\n"),                     u8"\\x80\\n"            },

        // control character + invalid byte
        {string("\x01") + make_str({'\xFF'}),                   u8"\\u0001\\xFF"        },
        {make_str({'\xFF'}) + string("\x01"),                   u8"\\xFF\\u0001"        },

        // all four categories
        {string("a\t\x01") + make_str({'\xFF'}),                u8"a\\t\\u0001\\xFF"    },
        {make_str({'\x80'}) + string("\nhello\x01"),            u8"\\x80\\nhello\\u0001"},
        {string("hello\t") + string("\x01") + make_str({'\x80'})
             + string("world"),
         u8"hello\\t\\u0001\\x80world"                                                  },
    };
    vector<pair<basic_string<CharT>, u8string>> rv;
    auto fill_vector_func = [&]<typename T>(T list_arg)
    {
        for (auto& [unicode_str, u8str] : list_arg)
        {
            rv.push_back(make_tuple(spl_convert<CharT>(unicode_str), u8str));
        }
    };
    if constexpr (same_as<char, CharT>)
    {
        fill_vector_func(input_output_pairs_ascii);
    }
    else if constexpr (same_as<char8_t, CharT>)
    {
        fill_vector_func(input_output_pairs_u8);
    }
    else if constexpr (same_as<char16_t, CharT>)
    {
        fill_vector_func(input_output_pairs_u16);
    }
    else if constexpr (same_as<char32_t, CharT>)
    {
        fill_vector_func(input_output_pairs_u32);
    }
    else if constexpr (same_as<wchar_t, CharT>)
    {
        if constexpr (sizeof(CharT) == 2)
        {
            fill_vector_func(input_output_pairs_u16);
        }
        else if constexpr (sizeof(CharT) == 4)
        {
            fill_vector_func(input_output_pairs_u32);
        }
    }
    return rv;
}
} // namespace unicode_bridge_test_cases

TEST_CASE(
    "to_formatted_unicode_string correct for std::string arguments",
    "[to_formatted_unicode_string]"
)
{
    using namespace unicode_bridge;
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    auto input_output_pairs = get_to_formatted_unicode_string_data<char>();
    test_to_formatted_unicode_string(input_output_pairs);
}

TEST_CASE(
    "to_formatted_unicode_string correct for u8string arguments",
    "[to_formatted_unicode_string]"
)
{
    using namespace unicode_bridge;
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    auto input_output_pairs = get_to_formatted_unicode_string_data<char8_t>();
    test_to_formatted_unicode_string(input_output_pairs);
}

TEST_CASE(
    "to_formatted_unicode_string correct for u16string arguments",
    "[to_formatted_unicode_string]"
)
{
    using namespace unicode_bridge;
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    auto input_output_pairs = get_to_formatted_unicode_string_data<char16_t>();
    test_to_formatted_unicode_string(input_output_pairs);
}

TEST_CASE(
    "to_formatted_unicode_string correct for u32string arguments",
    "[to_formatted_unicode_string]"
)
{
    using namespace unicode_bridge;
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    auto input_output_pairs = get_to_formatted_unicode_string_data<char32_t>();
    test_to_formatted_unicode_string(input_output_pairs);
}

TEST_CASE(
    "to_formatted_unicode_string correct for wchar_t arguments",
    "[to_formatted_unicode_string]"
)
{
    using namespace unicode_bridge;
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    auto input_output_pairs = get_to_formatted_unicode_string_data<wchar_t>();
    test_to_formatted_unicode_string(input_output_pairs);
}

namespace unicode_bridge_testing
{
template <typename T, typename U>
inline void
    test_errors(
        const std::vector<std::pair<T, std::basic_string<U>>>&
            input_output_pairs_arg
    )
{
    using namespace std;
    for (auto&& [error, expected_msg] : input_output_pairs_arg)
    {
        auto result = error.message(
            u8"<function name>",
            u8"<additional message>",
            optional<u8string_view>(u8"<input string>")
        );
        if (result != expected_msg)
        {
            INFO(
                "error.message() = \""
                << convert_unicode_to_string(result) << "\", expected_msg = \""
                << convert_unicode_to_string(expected_msg) << "\""
            );
            REQUIRE(result == expected_msg);
        }
        REQUIRE(result == expected_msg);
    }
}
} // namespace unicode_bridge_testing

TEST_CASE(
    "unicode_error message tests",
    "[unicode_error]"
)
{
    using namespace unicode_bridge;
    using namespace unicode_bridge::internal;
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    vector<pair<unicode_conversion_error, u8string>> input_output_pairs = {
        {unicode_conversion_error_factory::invalid_leading_byte(10, '\xF8'),
         u8"The 11th code unit (0xF8) in the UTF-8 input passed to "
         u8"<function "
         u8"name> "
         u8"(\"<input string>\")"
         u8" was found to be an invalid leading byte. A valid leading byte "
         u8"must "
         u8"be inclusively within one of the following ranges: 0x00 to "
         u8"0x7F "
         u8"(single-byte sequence), 0xC0 to 0xDF (two-byte sequence), 0xE0 "
         u8"to 0xEF (three-byte sequence), or 0xF0 to 0xF7 (four-byte "
         u8"sequence). As 0xF8 falls outside all of these ranges, it cannot "
         u8"begin a sequence representing a valid Unicode scalar "
         u8"value<additional message>."},
        {unicode_conversion_error_factory::truncated_sequence(
             10, {static_cast<char8_t>(0xC2), u8'\0', u8'\0', u8'\0'}, 1, 2
         ), u8"The 11th code unit (0xC2) in the UTF-8 input passed to "
         u8"<function name> (\"<input string>\") was found to be a valid "
         u8"leading byte, indicating the "
         u8"start of a two-byte "
         u8"sequence. However, the input "
         u8"ended after the first code unit "
         u8"— 1 continuation byte was "
         u8"expected but was not present. As "
         u8"the sequence is incomplete, it cannot represent a valid Unicode "
         u8"scalar value<additional message>."},
        {unicode_conversion_error_factory::truncated_sequence(
             10, {static_cast<char8_t>(0xE2), u8'\0', u8'\0', u8'\0'}, 1, 3
         ), u8"The 11th code unit (0xE2) in the UTF-8 input passed to "
         u8"<function name> (\"<input string>\") was found to be a valid "
         u8"leading byte, indicating the start of a three-byte "
         u8"sequence. However, the input ended after the first code unit — 2 "
         u8"continuation bytes were expected but none were present. As the "
         u8"sequence is incomplete, it cannot represent a valid Unicode scalar "
         u8"value<additional message>."},
        {unicode_conversion_error_factory::truncated_sequence(
             10, {static_cast<char8_t>(0xE2), u8'\0', u8'\0', u8'\0'}, 1, 4
         ), u8"The 11th code unit (0xE2) in the UTF-8 input passed to "
         u8"<function name> (\"<input string>\") was found to be a valid "
         u8"leading byte, indicating the start of a four-byte "
         u8"sequence. However, the input ended after the first code unit — 3 "
         u8"continuation bytes were expected but none were present. As the "
         u8"sequence is incomplete, it cannot represent a valid Unicode scalar "
         u8"value<additional message>."},
        {unicode_conversion_error_factory::truncated_sequence(
             10, {static_cast<char8_t>(0xE2),
              static_cast<char8_t>(0x80),
              u8'\0',
              u8'\0'},
         2, 3
         ), u8"The 11th and 12th code units ([0xE2, 0x80]) in the UTF-8 input "
         u8"passed to <function name> (\"<input string>\") form the start of a "
         u8"three-byte sequence. However, the input ended after the second "
         u8"code unit — 1 further continuation byte was expected but was not "
         u8"present. As the sequence is incomplete, it cannot represent a "
         u8"valid Unicode scalar value<additional message>."},
        {unicode_conversion_error_factory::truncated_sequence(
             10, {static_cast<char8_t>(0xF0),
              static_cast<char8_t>(0x90),
              u8'\0',
              u8'\0'},
         2, 4
         ), u8"The 11th and 12th code units ([0xF0, 0x90]) in the UTF-8 input "
         u8"passed to <function name> (\"<input string>\") form the start of a "
         u8"four-byte sequence. However, the input ended after the second "
         u8"code unit — 2 further continuation bytes were expected but none "
         u8"were present. As the sequence is incomplete, it cannot represent a "
         u8"valid Unicode scalar value<additional message>."},
        {unicode_conversion_error_factory::truncated_sequence(
             10, {static_cast<char8_t>(0xF0),
              static_cast<char8_t>(0x90),
              static_cast<char8_t>(0x80),
              u8'\0'},
         3, 4
         ), u8"The 11th, 12th and 13th code units ([0xF0, 0x90, 0x80]) in the "
         u8"UTF-8 input passed to <function name> (\"<input string>\") form "
         u8"the start of a four-byte sequence. However, the input ended after "
         u8"the third code unit — 1 further continuation byte was expected but "
         u8"was not present. As the sequence is incomplete, it cannot "
         u8"represent a valid Unicode scalar value<additional message>."},

        {unicode_conversion_error_factory::invalid_continuation_byte(
             10, {static_cast<char8_t>(0xC2),
              static_cast<char8_t>(0x20),
              static_cast<char8_t>(0x00),
              u8'\0'},
         2, 0
         ), u8"The 11th and 12th code units ([0xC2, 0x20]) in the UTF-8 input "
         u8"passed to <function name> (\"<input string>\") form the start of a "
         u8"two-byte sequence. The second code unit (0x20) was expected to be "
         u8"a continuation byte, but was not — a valid continuation byte must "
         u8"be inclusively between 0x80 and 0xBF. As 0x20 falls outside this "
         u8"range, the sequence cannot represent a valid Unicode scalar "
         u8"value<additional "
         u8"message>."},
        {unicode_conversion_error_factory::invalid_continuation_byte(
             10, {static_cast<char8_t>(0xE2),
              static_cast<char8_t>(0x20),
              static_cast<char8_t>(0x80),
              u8'\0'},
         3, 1
         ), u8"The 11th, 12th and 13th code units ([0xE2, 0x20, 0x80]) in the "
         u8"UTF-8 input passed to <function name> (\"<input string>\") form "
         u8"the start of a three-byte sequence. The second code unit (0x20) "
         u8"was expected to be a continuation byte, but was not — a valid "
         u8"continuation byte must be inclusively between 0x80 and 0xBF. As "
         u8"0x20 falls outside this range, the sequence cannot represent a "
         u8"valid Unicode scalar value<additional "
         u8"message>."},
        {unicode_conversion_error_factory::invalid_continuation_byte(
             10, {static_cast<char8_t>(0xE2),
              static_cast<char8_t>(0x80),
              static_cast<char8_t>(0x20),
              u8'\0'},
         3, 2
         ), u8"The 11th, 12th and 13th code units ([0xE2, 0x80, 0x20]) in the "
         u8"UTF-8 input passed to <function name> (\"<input string>\") form "
         u8"the start of a three-byte sequence. The third code unit (0x20) was "
         u8"expected to be a continuation byte, but was not — a valid "
         u8"continuation byte must be inclusively between 0x80 and 0xBF. As "
         u8"0x20 falls outside this range, the sequence cannot represent a "
         u8"valid Unicode scalar value<additional "
         u8"message>."},
        {unicode_conversion_error_factory::invalid_continuation_byte(
             10, {static_cast<char8_t>(0xE2),
              static_cast<char8_t>(0x20),
              static_cast<char8_t>(0x21),
              u8'\0'},
         3, 3
         ), u8"The 11th, 12th and 13th code units ([0xE2, 0x20, 0x21]) in the "
         u8"UTF-8 input passed to <function name> (\"<input string>\") form "
         u8"the start of a three-byte sequence. The second and third code "
         u8"units (0x20 and 0x21) were expected to be continuation bytes, but "
         u8"were not — a valid "
         u8"continuation byte must be inclusively between 0x80 and 0xBF. As "
         u8"both are outside this range, the sequence cannot represent a valid "
         u8"Unicode scalar value<additional message>."},
        {unicode_conversion_error_factory::invalid_continuation_byte(
             10, {static_cast<char8_t>(0xF0),
              static_cast<char8_t>(0x20),
              static_cast<char8_t>(0x80),
              static_cast<char8_t>(0x80)},
         4, 4
         ), u8"The 11th, 12th, 13th and 14th code units ([0xF0, 0x20, 0x80, "
         u8"0x80]) in the UTF-8 input passed to <function name> (\"<input "
         u8"string>\") form the start of a four-byte sequence. The second code "
         u8"unit (0x20) was expected to be a continuation byte, but was not — "
         u8"a valid "
         u8"continuation byte must be inclusively between 0x80 and 0xBF. As "
         u8"0x20 falls outside this range, the sequence cannot represent a "
         u8"valid Unicode scalar value<additional message>."},
        {unicode_conversion_error_factory::invalid_continuation_byte(
             10, {static_cast<char8_t>(0xF0),
              static_cast<char8_t>(0x90),
              static_cast<char8_t>(0x20),
              static_cast<char8_t>(0x80)},
         4, 5
         ), u8"The 11th, 12th, 13th and 14th code units ([0xF0, 0x90, 0x20, "
         u8"0x80]) in the UTF-8 input passed to <function name> (\"<input "
         u8"string>\") form the start of a four-byte sequence. The third code "
         u8"unit (0x20) was expected to be a continuation byte, but was not — "
         u8"a valid "
         u8"continuation byte must be inclusively between 0x80 and 0xBF. As "
         u8"0x20 falls outside this range, the sequence cannot represent a "
         u8"valid Unicode scalar value<additional message>."},
        {unicode_conversion_error_factory::invalid_continuation_byte(
             10, {static_cast<char8_t>(0xF0),
              static_cast<char8_t>(0x90),
              static_cast<char8_t>(0x80),
              static_cast<char8_t>(0x20)},
         4, 6
         ), u8"The 11th, 12th, 13th and 14th code units ([0xF0, 0x90, 0x80, "
         u8"0x20]) in the UTF-8 input passed to <function name> (\"<input "
         u8"string>\") form the start of a four-byte sequence. The fourth code "
         u8"unit (0x20) was expected to be a continuation byte, but was not — "
         u8"a valid continuation byte must be inclusively between 0x80 and "
         u8"0xBF. As 0x20 falls outside this range, the sequence cannot "
         u8"represent a valid Unicode scalar value<additional message>."},
        {unicode_conversion_error_factory::invalid_continuation_byte(
             10, {static_cast<char8_t>(0xF0),
              static_cast<char8_t>(0x20),
              static_cast<char8_t>(0x21),
              static_cast<char8_t>(0x80)},
         4, 7
         ), u8"The 11th, 12th, 13th and 14th code units ([0xF0, 0x20, 0x21, "
         u8"0x80]) in the UTF-8 input passed to <function name> (\"<input "
         u8"string>\") form the start of a four-byte sequence. The second and "
         u8"third code units (0x20 and 0x21) were expected to be continuation "
         u8"bytes, but were not — a valid "
         u8"continuation byte must be inclusively between 0x80 and 0xBF. As "
         u8"both are outside this range, the sequence cannot represent a valid "
         u8"Unicode scalar value<additional message>."},
        {unicode_conversion_error_factory::invalid_continuation_byte(
             10, {static_cast<char8_t>(0xF0),
              static_cast<char8_t>(0x90),
              static_cast<char8_t>(0x20),
              static_cast<char8_t>(0x21)},
         4, 9
         ), u8"The 11th, 12th, 13th and 14th code units ([0xF0, 0x90, 0x20, "
         u8"0x21]) in the UTF-8 input passed to <function name> (\"<input "
         u8"string>\") form the start of a four-byte sequence. The third and "
         u8"fourth code units (0x20 and 0x21) were expected to be continuation "
         u8"bytes, but were not — a valid "
         u8"continuation byte must be inclusively between 0x80 and 0xBF. As "
         u8"both are outside this range, the sequence cannot represent a valid "
         u8"Unicode scalar value<additional message>."},
        {unicode_conversion_error_factory::invalid_continuation_byte(
             10, {static_cast<char8_t>(0xF0),
              static_cast<char8_t>(0x20),
              static_cast<char8_t>(0x80),
              static_cast<char8_t>(0x21)},
         4, 8
         ), u8"The 11th, 12th, 13th and 14th code units ([0xF0, 0x20, 0x80, "
         u8"0x21]) in the UTF-8 input passed to <function name> (\"<input "
         u8"string>\") form the start of a four-byte sequence. The second and "
         u8"fourth code units (0x20 and 0x21) were expected to be continuation "
         u8"bytes, but were not — a valid "
         u8"continuation byte must be inclusively between 0x80 and 0xBF. As "
         u8"both are outside this range, the sequence cannot represent a valid "
         u8"Unicode scalar value<additional message>."},
        {unicode_conversion_error_factory::invalid_continuation_byte(
             10, {static_cast<char8_t>(0xF0),
              static_cast<char8_t>(0x20),
              static_cast<char8_t>(0x21),
              static_cast<char8_t>(0x22)},
         4, 10
         ), u8"The 11th, 12th, 13th and 14th code units ([0xF0, 0x20, 0x21, "
         u8"0x22]) in the UTF-8 input passed to <function name> (\"<input "
         u8"string>\") form the start of a four-byte sequence. The second, "
         u8"third and fourth code units (0x20, 0x21 and 0x22) were expected to "
         u8"be continuation bytes, but were not — a valid "
         u8"continuation byte must be inclusively between 0x80 and 0xBF. As "
         u8"all three are "
         u8"outside "
         u8"this range, the sequence cannot represent a valid Unicode scalar "
         u8"value<additional message>."},

        {unicode_conversion_error_factory::overlong_encoding(
             10, {static_cast<char8_t>(0xC0),
              static_cast<char8_t>(0x80),
              static_cast<char8_t>(0x00),
              static_cast<char8_t>(0x00)},
         2, static_cast<char32_t>(0x0)
         ), u8"The 11th and 12th code units ([0xC0, 0x80]) in the UTF-8 input "
         u8"passed to <function name> (\"<input string>\") form a two-byte "
         u8"sequence encoding U+0000. This is an overlong encoding — "
         u8"U+0000 can be represented using a single byte (0x00), which is the "
         u8"shortest valid UTF-8 representation. The UTF-8 standard requires "
         u8"that code points are always encoded using the shortest possible "
         u8"sequence. As this requirement is not met, the sequence does not "
         u8"represent a valid Unicode scalar value<additional message>."},
        {unicode_conversion_error_factory::overlong_encoding(
             10, {static_cast<char8_t>(0xE0),
              static_cast<char8_t>(0x80),
              static_cast<char8_t>(0x80),
              static_cast<char8_t>(0x00)},
         3, static_cast<char32_t>(0x0)
         ), u8"The 11th, 12th and 13th code units ([0xE0, 0x80, 0x80]) in the "
         u8"UTF-8 input passed to <function name> (\"<input string>\") form a "
         u8"three-byte sequence encoding U+0000. This is an overlong "
         u8"encoding — U+0000 can be represented using a single byte (0x00), "
         u8"which is the shortest valid UTF-8 representation. The UTF-8 "
         u8"standard requires that code points are always encoded using the "
         u8"shortest possible sequence. As this requirement is not met, the "
         u8"sequence does not represent a valid Unicode scalar "
         u8"value<additional message>."},
        {unicode_conversion_error_factory::overlong_encoding(
             10, {static_cast<char8_t>(0xE0),
              static_cast<char8_t>(0x9F),
              static_cast<char8_t>(0xBF),
              static_cast<char8_t>(0x00)},
         3, static_cast<char32_t>(0x7FF)
         ), u8"The 11th, 12th and 13th code units ([0xE0, 0x9F, 0xBF]) in the UTF"
         u8"-8 input passed to <function name> (\"<input string>\") form a "
         u8"three-byte sequence encoding U+07FF. This is an overlong "
         u8"encoding — U+07FF can be represented using two bytes ([0xDF, "
         u8"0xBF]), which is the shortest valid UTF-8 representation. The UTF"
         u8"-8 standard requires that code points are always encoded using "
         u8"the shortest possible sequence. As this requirement is not met, "
         u8"the sequence does not represent a valid Unicode scalar "
         u8"value<additional message>."},
        {unicode_conversion_error_factory::overlong_encoding(
             10, {static_cast<char8_t>(0xF0),
              static_cast<char8_t>(0x80),
              static_cast<char8_t>(0x80),
              static_cast<char8_t>(0x80)},
         4, static_cast<char32_t>(0x0)
         ), u8"The 11th, 12th, 13th and 14th code units ([0xF0, 0x80, 0x80, "
         u8"0x80]) in the UTF-8 input passed to <function name> (\"<input "
         u8"string>\") form a four-byte sequence encoding U+0000. This "
         u8"is an overlong encoding — U+0000 can be represented using a single "
         u8"byte (0x00), which is the shortest valid UTF-8 representation. The "
         u8"UTF-8 standard requires that code points are always encoded using "
         u8"the shortest possible sequence. As this requirement is not met, "
         u8"the sequence does not represent a valid Unicode scalar "
         u8"value<additional message>."},
        {unicode_conversion_error_factory::overlong_encoding(
             10, {static_cast<char8_t>(0xF0),
              static_cast<char8_t>(0x80),
              static_cast<char8_t>(0x80),
              static_cast<char8_t>(0xBF)},
         4, static_cast<char32_t>(0x3F)
         ), u8"The 11th, 12th, 13th and 14th code units ([0xF0, 0x80, 0x80, "
         u8"0xBF]) in the UTF-8 input passed to <function name> (\"<input "
         u8"string>\") form a four-byte sequence encoding U+003F. This "
         u8"is an overlong encoding — U+003F can be represented using a single "
         u8"byte (0x3F), which is the shortest valid UTF-8 representation. The "
         u8"UTF-8 standard requires that code points are always encoded using "
         u8"the shortest possible sequence. As this requirement is not met, "
         u8"the sequence does not represent a valid Unicode scalar "
         u8"value<additional message>."},
        {unicode_conversion_error_factory::overlong_encoding(
             10, {static_cast<char8_t>(0xF0),
              static_cast<char8_t>(0x80),
              static_cast<char8_t>(0xBF),
              static_cast<char8_t>(0xBF)},
         4, static_cast<char32_t>(0x0FFF)
         ), u8"The 11th, 12th, 13th and 14th code units ([0xF0, 0x80, 0xBF, "
         u8"0xBF]) in the UTF-8 input passed to <function name> (\"<input "
         u8"string>\") form a four-byte sequence encoding U+0FFF. "
         u8"This is an overlong encoding — U+0FFF can be represented using "
         u8"three bytes ([0xE0, 0xBF, 0xBF]), which is the shortest valid "
         u8"UTF-8 representation. The UTF-8 standard requires that code points "
         u8"are always encoded using the shortest possible sequence. As this "
         u8"requirement is not met, the sequence does not represent a valid "
         u8"Unicode scalar value<additional "
         u8"message>."},

        {unicode_conversion_error_factory::
             invalid_utf32_code_point_after_utf8_conversion(
                 10, {static_cast<char8_t>(0xED),
                  static_cast<char8_t>(0xA0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
         3, static_cast<char32_t>(0xD800)
             ), u8"The 11th, 12th and 13th code units ([0xED, 0xA0, 0x80]) in the "
         u8"UTF-8 input passed to <function name> (\"<input string>\") form a "
         u8"three-byte sequence encoding U+D800. However, U+D800 falls outside "
         u8"the valid Unicode range — valid Unicode scalar values must be "
         u8"inclusively between U+0000 and U+10FFFF, excluding the surrogate "
         u8"range U+D800 to U+DFFF. As U+D800 falls within the surrogate "
         u8"range, it cannot represent a valid Unicode scalar value<additional "
         u8"message>."},
        {unicode_conversion_error_factory::
             invalid_utf32_code_point_after_utf8_conversion(
                 10, {static_cast<char8_t>(0xF4),
                  static_cast<char8_t>(0x90),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
         4, static_cast<char32_t>(0x11'0000)
             ), u8"The 11th, 12th, 13th and 14th code units ([0xF4, 0x90, 0x80, "
         u8"0x80]) in the UTF-8 input passed to <function name> (\"<input "
         u8"string>\") form a four-byte sequence encoding U+110000. "
         u8"However, U+110000 falls outside the valid Unicode range — valid "
         u8"Unicode scalar values must be inclusively between U+0000 and "
         u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. As "
         u8"U+110000 exceeds the maximum valid codepoint, it cannot represent "
         u8"a valid Unicode scalar value<additional "
         u8"message>."},

        {unicode_conversion_error_factory::high_surrogate_then_end_of_stream(
             10, static_cast<char16_t>(0xD800), false
         ), u8"The 11th code unit (0xD800) in the UTF-16 input passed to "
         u8"<function name> (\"<input string>\") is a high surrogate, "
         u8"indicating the start of a surrogate pair. However, the input ended "
         u8"after this code unit — a low surrogate was expected to follow but "
         u8"was not present. As the surrogate pair is incomplete, it cannot "
         u8"represent a valid Unicode scalar value<additional "
         u8"message>."},
        {unicode_conversion_error_factory::high_surrogate_then_end_of_stream(
             10, static_cast<char16_t>(0xD800), true
         ), u8"The 11th code unit (0xD800) in the UTF-16 input (encoded using "
         u8"wchar_t) passed to "
         u8"<function name> (\"<input string>\") is a high surrogate, "
         u8"indicating the start of a surrogate pair. However, the input ended "
         u8"after this code unit — a low surrogate was expected to follow but "
         u8"was not present. As the surrogate pair is incomplete, it cannot "
         u8"represent a valid Unicode scalar value<additional "
         u8"message>."},

        {unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(
                 10, static_cast<char16_t>(0xDBFF),
         static_cast<char16_t>(0xD800),
         false
             ), u8"The 11th and 12th code units ([0xDBFF, 0xD800]) in the UTF-16 "
         u8"input passed to <function name> (\"<input string>\") form the "
         u8"start of a surrogate pair. The first code unit (0xDBFF) is a high "
         u8"surrogate, which must be followed by a low surrogate inclusively "
         u8"between 0xDC00 and 0xDFFF. However, the second code unit (0xD800) "
         u8"falls outside this range, and therefore the two code units cannot "
         u8"represent a valid Unicode scalar value<additional "
         u8"message>."},
        {unicode_conversion_error_factory::
             high_surrogate_not_followed_by_low_surrogate(
                 10, static_cast<char16_t>(0xDBFF),
         static_cast<char16_t>(0xD800),
         true
             ), u8"The 11th and 12th code units ([0xDBFF, 0xD800]) in the UTF-16 "
         u8"input (encoded using wchar_t) passed to <function name> (\"<input "
         u8"string>\") form the "
         u8"start of a surrogate pair. The first code unit (0xDBFF) is a high "
         u8"surrogate, which must be followed by a low surrogate inclusively "
         u8"between 0xDC00 and 0xDFFF. However, the second code unit (0xD800) "
         u8"falls outside this range, and therefore the two code units cannot "
         u8"represent a valid Unicode scalar value<additional "
         u8"message>."},

        {unicode_conversion_error_factory::unexpected_low_surrogate(
             10, static_cast<char16_t>(0xDC00), false
         ), u8"The 11th code unit (0xDC00) in the UTF-16 input passed to "
         u8"<function name> (\"<input string>\") is a low surrogate. Low "
         u8"surrogates must always be preceded by a high surrogate "
         u8"(inclusively between 0xD800 and 0xDBFF) as the second part of a "
         u8"surrogate pair. As this low surrogate appears without a preceding "
         u8"high surrogate, it cannot represent a valid Unicode scalar "
         u8"value<additional "
         u8"message>."},
        {unicode_conversion_error_factory::unexpected_low_surrogate(
             10, static_cast<char16_t>(0xDC00), true
         ), u8"The 11th code unit (0xDC00) in the UTF-16 input (encoded using "
         u8"wchar_t) passed to <function name> (\"<input string>\") is a low "
         u8"surrogate. Low surrogates must always be preceded by a high "
         u8"surrogate (inclusively between 0xD800 and 0xDBFF) as the second "
         u8"part of a surrogate pair. As this low surrogate appears without a "
         u8"preceding high surrogate, it cannot represent a valid Unicode "
         u8"scalar value<additional "
         u8"message>."},

        {unicode_conversion_error_factory::invalid_utf32_code_point(
             10, static_cast<char32_t>(0xD800), false
         ), u8"The 11th code unit (0xD800) in the UTF-32 input passed to "
         u8"<function name> (\"<input string>\") decodes to U+D800. However, "
         u8"U+D800 falls outside the valid Unicode range — valid Unicode "
         u8"scalar "
         u8"values must be inclusively between U+0000 and U+10FFFF, excluding "
         u8"the surrogate range U+D800 to U+DFFF. As U+D800 falls within the "
         u8"surrogate range, it cannot represent a valid Unicode scalar "
         u8"value<additional message>."},
        {unicode_conversion_error_factory::invalid_utf32_code_point(
             10, static_cast<char32_t>(0x11'0000), false
         ), u8"The 11th code unit (0x110000) in the UTF-32 input passed to "
         u8"<function name> (\"<input string>\") decodes to U+110000. However, "
         u8"U+110000 falls outside the valid Unicode range — valid Unicode "
         u8"scalar "
         u8"values must be inclusively between U+0000 and U+10FFFF, excluding "
         u8"the surrogate range U+D800 to U+DFFF. As U+110000 exceeds the "
         u8"maximum valid codepoint, it cannot represent a valid Unicode "
         u8"scalar "
         u8"value<additional message>."},
        {unicode_conversion_error_factory::invalid_utf32_code_point(
             10, static_cast<char32_t>(0xD800), true
         ), u8"The 11th code unit (0xD800) in the UTF-32 input (encoded using "
         u8"wchar_t) passed to <function name> (\"<input string>\") decodes to "
         u8"U+D800. However, U+D800 falls outside the valid Unicode range — "
         u8"valid Unicode scalar values must be inclusively between U+0000 and "
         u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. As "
         u8"U+D800 "
         u8"falls within the surrogate range, it cannot represent a valid "
         u8"Unicode "
         u8"scalar value<additional message>."},
        {unicode_conversion_error_factory::invalid_utf32_code_point(
             10, static_cast<char32_t>(0x11'0000), true
         ), u8"The 11th code unit (0x110000) in the UTF-32 input (encoded using "
         u8"wchar_t) passed to <function name> (\"<input string>\") decodes to "
         u8"U+110000. However, U+110000 falls outside the valid Unicode range "
         u8"— "
         u8"valid Unicode scalar values must be inclusively between U+0000 and "
         u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. As "
         u8"U+110000 exceeds the maximum valid codepoint, it cannot represent "
         u8"a "
         u8"valid Unicode scalar value<additional message>."},
    };


    test_errors(input_output_pairs);
}

int
    main(
        int   argc,
        char* argv[]
    )
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    return Catch::Session().run(argc, argv);
}