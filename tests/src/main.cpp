#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <tuple>
#include <unicode_bridge.hpp>
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

inline void
    equal_basic_unicode_error(
        const unicode_bridge::basic_unicode_error& error_l_arg,
        const unicode_bridge::basic_unicode_error& error_r_arg
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
    REQUIRE(error_l_arg.char32_character() == error_r_arg.char32_character());
    if (error_l_arg.auxillery_data() != error_r_arg.auxillery_data())
    {
        int x = 4;
    }
    REQUIRE(error_l_arg.auxillery_data() == error_r_arg.auxillery_data());
    if (error_l_arg.u16_code_points() != error_r_arg.u16_code_points())
    {
        int x = 4;
    }
    REQUIRE(error_l_arg.u16_code_points() == error_r_arg.u16_code_points());
    REQUIRE(error_l_arg.is_wchar() == error_r_arg.is_wchar());
}

inline void
    equal_forward_unicode_error(
        const unicode_bridge::forward_scan_unicode_error& error_l_arg,
        const unicode_bridge::forward_scan_unicode_error& error_r_arg
    )
{
    REQUIRE(error_l_arg.code() == error_l_arg.code());
    equal_basic_unicode_error(error_l_arg.error(), error_r_arg.error());
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
    REQUIRE(error_l_arg.character_index() == error_r_arg.character_index());
    equal_forward_unicode_error(error_l_arg.error(), error_r_arg.error());
}

inline void
    equal_next_char32_error(
        const unicode_bridge::next_char32_error& error_l_arg,
        const unicode_bridge::next_char32_error& error_r_arg
    )
{
    if (error_l_arg.get_enum() != error_r_arg.get_enum())
    {
        int x = 4;
    }
    REQUIRE(error_l_arg.get_enum() == error_r_arg.get_enum());
    equal_forward_unicode_error(error_l_arg.error(), error_r_arg.error());
}

inline void
    equal_prev_char32_error(
        const unicode_bridge::prev_char32_error& error_l_arg,
        const unicode_bridge::prev_char32_error& error_r_arg
    )
{
    if (error_l_arg.get_enum() != error_r_arg.get_enum())
    {
        int x = 4;
    }
    REQUIRE(error_l_arg.get_enum() == error_r_arg.get_enum());
    equal_basic_unicode_error(error_l_arg.error(), error_r_arg.error());
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
            auto check_func
                = [&](const std::tuple<
                      // Used to catch non-caught exceptions. False is "major
                      // unknown failure".
                      bool,
                      // Result and optional error.
                      std::expected<std::string, unicode_to_ascii_error>>&
                          result_arg)
            {
                if (get<0>(result_arg) == false)
                {
                    INFO("Unknown functio failure");
                    FAIL();
                }
                if (get<1>(result_arg).has_value())
                {
                    REQUIRE((get<1>(result_arg).value()) == expected_output);
                }
                else
                {
                    string output{"Unexpected output: \""};
                    output.append(convert_unicode_to_string(
                        get<1>(result_arg).error().message()
                    ));
                    output.append("\"");
                    FAIL(output);
                }
            };
            // Check input_arg is always valid ascii.
            REQUIRE(is_valid_ascii(input_arg) == true);
            auto result{convert_unicode_to_ascii(input_arg)};
            check_func({true, result});
            {
                tuple<bool, expected<string, unicode_to_ascii_error>>
                    exception_args = {true, ""};
                try
                {
                    // Checks that with_exception also returns the correct
                    // value.
                    auto converted
                        = convert_unicode_to_ascii_with_exception(input_arg);
                    get<1>(exception_args) = converted;
                }
                catch (const unicode_bridge_exception<unicode_to_ascii_error>&
                           unexpected_exception)
                {
                    get<1>(exception_args)
                        = unexpected(unexpected_exception.error());
                }
                catch (...)
                {
                    get<0>(exception_args) = false;
                }
                check_func(exception_args);
            }
            {
                string str;
                auto   res = convert_unicode_to_ascii_append(input_arg, str);
                tuple<bool, expected<string, unicode_to_ascii_error>> test_args
                    = {true, ""};
                if (res.has_value())
                {
                    get<1>(test_args) = unexpected(res.value());
                }
                else
                {
                    get<1>(test_args) = str;
                }
            }
            {
                tuple<bool, expected<string, unicode_to_ascii_error>> test_args
                    = {true, ""};
                try
                {
                    // Checks that with_exception also returns the correct
                    // value.
                    string str;
                    convert_unicode_to_ascii_append_with_exception(
                        input_arg, str
                    );
                    get<1>(test_args) = str;
                }
                catch (const unicode_bridge_exception<unicode_to_ascii_error>&
                           unexpected_exception)
                {
                    get<1>(test_args)
                        = unexpected(unexpected_exception.error());
                }
                catch (...)
                {
                    get<0>(test_args) = false;
                }
                check_func(test_args);
            }
            {
                tuple<bool, expected<string, unicode_to_ascii_error>> test_args
                    = {true, ""};
                auto res = convert_unicode_to_ascii_no_error(input_arg);
                if (res.has_value())
                {
                    get<1>(test_args) = res.value();
                }
                else
                {
                    get<0>(test_args) = false;
                }
                check_func(test_args);
            }
            {
                string                                                str;
                tuple<bool, expected<string, unicode_to_ascii_error>> test_args
                    = {true, ""};
                auto res
                    = convert_unicode_to_ascii_append_no_error(input_arg, str);
                if (res.has_value())
                {
                    get<0>(test_args) = false;
                }
                else
                {
                    get<1>(test_args) = str;
                }
                check_func(test_args);
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
        const std::vector<std::tuple<
            std::basic_string<CharT>,
            unicode_bridge::unicode_to_ascii_error,
            std::string>>& list_arg
    )
{
    using namespace unicode_bridge;
    using namespace std;
    size_t idx{0};
    for (auto&& [input_string, output_error, partial_str_res] : list_arg)
    {
        auto run_func = [&]<typename T>(const T input_arg)
        {
            auto check_func
                = [&](const std::tuple<
                      // Used to catch non-caught exceptions. False is "major
                      // unknown failure".
                      bool,
                      // Catches the half-created output.
                      std::optional<std::pair<std::string, std::size_t>>,
                      // Result and optional error.
                      std::expected<
                          std::string,
                          optional<unicode_to_ascii_error>>>& result_arg)
            {
                if (get<0>(result_arg) == false)
                {
                    INFO("Unknown functio failure");
                    FAIL();
                }
                if (get<2>(result_arg).has_value())
                {
                    string output{
                        "Unexpected successful convert_unicode_to_ascii "
                        "conversion. std::string result is \""
                    };
                    output.append(get<2>(result_arg).value());
                    output.append("\"");
                    FAIL(output);
                }
                else
                {
                    // Otherwise, test that the failure is the one expected.
                    if (get<2>(result_arg).error().has_value())
                    {
                        auto& failure_result{get<2>(result_arg).error().value()
                        };
                        INFO(unicode_print(failure_result.message(input_string))
                        );
                        REQUIRE(
                            failure_result.get_enum() == output_error.get_enum()
                        );
                        equal_unicode_error(
                            failure_result.error(), output_error.error()
                        );
                        if (get<1>(result_arg).has_value())
                        {
                            REQUIRE(
                                (get<1>(result_arg).value().first)
                                == partial_str_res
                            );
                            REQUIRE(
                                (get<1>(result_arg).value().second)
                                == (get<2>(result_arg)
                                        .error()
                                        .value()
                                        .error()
                                        .partial_output_size())
                            );
                        }
                    }
                    else
                    {
                        SUCCEED();
                    }
                }
            };
            // Check input is always invalid ascii.
            REQUIRE(is_valid_ascii(input_arg) == false);
            {
                auto result{convert_unicode_to_ascii(input_arg)};
                check_func({true, std::nullopt, result});
            }
            {
                tuple<
                    bool,
                    optional<pair<string, size_t>>,
                    expected<string, optional<unicode_to_ascii_error>>>
                    to_check_arg = {true, std::nullopt, ""};

                try
                {
                    // Checks that with_exception also returns the correct
                    // value.
                    auto converted2
                        = convert_unicode_to_ascii_with_exception(input_arg);
                    get<2>(to_check_arg) = converted2;
                }
                catch (const unicode_bridge_exception<unicode_to_ascii_error>&
                           unexpected_exception)
                {
                    get<2>(to_check_arg)
                        = unexpected(unexpected_exception.error());
                }
                catch (...)
                {
                    get<0>(to_check_arg) = false;
                }
                check_func(to_check_arg);
            }
            {
                string str_result;
                auto   result{
                    convert_unicode_to_ascii_append(input_arg, str_result)
                };
                check_func(
                    {true,
                     make_optional(make_pair(
                         str_result,
                         result.value().error().partial_output_size()
                     )),
                     not result.has_value()
                         ? expected<string, unicode_to_ascii_error>(str_result)
                         : unexpected(result.value())}
                );
            }
            {
                auto result = convert_unicode_to_ascii_no_error(input_arg);
                check_func(
                    {true,
                     std::nullopt,
                     result.has_value()
                         ? expected<string, optional<unicode_to_ascii_error>>(
                               result.value()
                           )
                         : unexpected(std::nullopt)}
                );
            }
            {
                string str_res;
                auto   result = convert_unicode_to_ascii_append_no_error(
                    input_arg, str_res
                );
                check_func(
                    {true,
                     make_pair(str_res, result.value()),
                     result.has_value()
                         ? unexpected(std::nullopt)
                         : expected<string, optional<unicode_to_ascii_error>>(
                               str_res
                           )}
                );
            }
            {
                tuple<
                    bool,
                    optional<pair<string, size_t>>,
                    expected<string, optional<unicode_to_ascii_error>>>
                       to_check_arg = {true, std::nullopt, ""};
                string str;
                try
                {
                    // Checks that with_exception also returns the correct
                    // value.
                    convert_unicode_to_ascii_append_with_exception(
                        input_arg, str
                    );
                    get<2>(to_check_arg) = str;
                }
                catch (const unicode_bridge_exception<unicode_to_ascii_error>&
                           unexpected_exception)
                {
                    get<2>(to_check_arg)
                        = unexpected(unexpected_exception.error());
                    get<1>(to_check_arg) = make_pair(
                        str,
                        unexpected_exception.error()
                            .error()
                            .partial_output_size()
                    );
                }
                catch (...)
                {
                    get<0>(to_check_arg) = false;
                }
                check_func(to_check_arg);
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
std::vector<std::tuple<
    std::basic_string<CharT>,
    unicode_bridge::unicode_to_ascii_error,
    std::string>>
    get_invalid_convert_unicode_to_ascii_and_is_valid_ascii_data()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge::internal;
    std::array<std::u8string, 4> arr;
    initializer_list<tuple<
        tuple<u8string, u16string, u32string>,
        unicode_to_ascii_error,
        string>>
        invalid_ascii_errors = {
            {{u8"ℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳ⷽ",
              u"ℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳",
              U"ℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳"},
             unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf32(U'ℝ', 0, true, 0),
             ""  },
            {{u8"aaℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳ⷽ",
              u"aaℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳",
              U"aaℝ┹ⶬ⯿⎣⊚⻀→⇮⸣⽛╡⽛ⱕ⩳"},
             unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf32(U'ℝ', 2, true, 0),
             "aa"}
    };
    initializer_list<tuple<u8string, unicode_to_ascii_error, string>>
        invalid_u8_errors = {
            {mk_unicode<char8_t>({0xFF}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xFF)
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xFE}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xFE)
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0x80}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0x80)
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xBF}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xBF)
                 ), 0
             ), ""},
            {u8string(u8"abc").append(mk_unicode<char8_t>({0xFF}, u8"def")),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 3, forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xFF)
                 ), 3
             ), "abc"},
            {mk_unicode<char8_t>({0xC2}),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xC2), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_2_found_1
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xE2}),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xE2), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_3_found_1
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xE2, 0x80}),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x80),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_3_found_2
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xF0}),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xF0), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_1
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xF0, 0x90}),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_2
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x80}),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_3
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xE2, 0x80}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x80),
                      u8'h',
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_3_invalid_indexes_2
                 ), 0
             ), ""},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xF0, 0x90}, u8"def"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 3, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      u8'd',
                      u8'e'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_2_3
                 ), 3
             ), "abc"},
            {mk_unicode<char8_t>({0xC2, 0x00}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xC2),
                      static_cast<char8_t>(0x00),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_2_invalid_indexes_1
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xC2, 0x20}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xC2),
                      static_cast<char8_t>(0x20),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_2_invalid_indexes_1
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xC2, 0xC0}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xC2),
                      static_cast<char8_t>(0xC0),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_2_invalid_indexes_1
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xE2, 0x80, 0x20}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x20),
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_3_invalid_indexes_2
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xE2, 0x20, 0x80}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_3_invalid_indexes_1
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x20, 0x80}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x80)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_2
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x80, 0x20}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x20)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_3
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xF0, 0x20, 0x80, 0x80}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_1
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xC0, 0x80}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xC0),
                      static_cast<char8_t>(0x80),
                      u8'\0',
                      u8'\0'},
             2, U'\0'
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xC1, 0xBF}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xC1),
                      static_cast<char8_t>(0xBF),
                      u8'\0',
                      u8'\0'},
             2, U'\x7F'
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xE0, 0x80, 0x80}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xE0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             3, U'\0'
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xE0, 0x81, 0xBF}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xE0),
                      static_cast<char8_t>(0x81),
                      static_cast<char8_t>(0xBF),
                      u8'\0'},
             3, U'\x7F'
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xE0, 0x9F, 0xBF}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xE0),
                      static_cast<char8_t>(0x9F),
                      static_cast<char8_t>(0xBF),
                      u8'\0'},
             3, U'\x7FF'
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xF0, 0x80, 0x80, 0x80}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             4, U'\0'
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xF0, 0x80, 0x80, 0xBF}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0xBF)},
             4, U'\x3F'
                 ), 0
             ), ""},
            {mk_unicode<char8_t>({0xED, 0xA0, 0x80}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xA0),
                          static_cast<char8_t>(0x80),
                          u8'\0'},
             3, U'\xD800'
                     ), 0
             ), ""},
            {mk_unicode<char8_t>({0xED, 0xAF, 0xBF}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xAF),
                          static_cast<char8_t>(0xBF),
                          u8'\0'},
             3, U'\xDBFF'
                     ), 0
             ), ""},
            {mk_unicode<char8_t>({0xED, 0xB0, 0x80}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xB0),
                          static_cast<char8_t>(0x80),
                          u8'\0'},
             3, U'\xDC00'
                     ), 0
             ), ""},
            {mk_unicode<char8_t>({0xED, 0xBF, 0xBF}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xBF),
                          static_cast<char8_t>(0xBF),
                          u8'\0'},
             3, U'\xDFFF'
                     ), 0
             ), ""},
            {mk_unicode<char8_t>({0xF4, 0x90, 0x80, 0x80}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xF4),
                          static_cast<char8_t>(0x90),
                          static_cast<char8_t>(0x80),
                          static_cast<char8_t>(0x80)},
             4, U'\x110000'
                     ), 0
             ), ""},
            {mk_unicode<char8_t>({0xF7, 0xBF, 0xBF, 0xBF}, u8"hello"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 0, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xF7),
                          static_cast<char8_t>(0xBF),
                          static_cast<char8_t>(0xBF),
                          static_cast<char8_t>(0xBF)},
             4, U'\x1FFFFF'
                     ), 0
             ), ""},
            {u8string(u8"abc")
                 + mk_unicode<char8_t>({0xED, 0xA0, 0x80}, u8"def"),
             unicode_to_ascii_error_factory::invalid_unicode_character(
                 3, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xA0),
                          static_cast<char8_t>(0x80),
                          u8'\0'},
             3, U'\xD800'
                     ), 3
             ), "abc"},
            {u8"😂",
             unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf8(
                     U'😂', {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x9F),
                      static_cast<char8_t>(0x98),
                      static_cast<char8_t>(0x82)},
             0, 4,
             0
                 ), ""},
            {u8"hello😂goodbye",
             unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf8(
                     U'😂', {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x9F),
                      static_cast<char8_t>(0x98),
                      static_cast<char8_t>(0x82)},
             5, 4,
             5
                 ), "hello"},
            {u8"€",
             unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf8(
                     U'€', {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x82),
                      static_cast<char8_t>(0xAC),
                      static_cast<char8_t>(0x00)},
             0, 3,
             0
                 ), ""},
            {u8"hello€goodbye",
             unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf8(
                     U'€', {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x82),
                      static_cast<char8_t>(0xAC),
                      static_cast<char8_t>(0x00)},
             5, 3,
             0
                 ), "hello"},
            {u8"é",
             unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf8(
                     U'é', {static_cast<char8_t>(0xC3),
                      static_cast<char8_t>(0xA9),
                      static_cast<char8_t>(0x00),
                      static_cast<char8_t>(0x00)},
             0, 2,
             0
                 ), ""},
            {u8"helloégoodbye",
             unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf8(
                     U'é', {static_cast<char8_t>(0xC3),
                      static_cast<char8_t>(0xA9),
                      static_cast<char8_t>(0x00),
                      static_cast<char8_t>(0x00)},
             5, 2,
             0
                 ), "hello"},
    };
    initializer_list<tuple<u16string, unicode_to_ascii_error, string>> invalid_u16_errors = {
        {mk_unicode<char16_t>({0xD800}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0, forward_scan_unicode_error_factory::
                 high_surrogate_then_end_of_stream(u'\xD800', same_as<CharT, wchar_t>),
         0
         ), ""},
        {mk_unicode<char16_t>({0xDBFF}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0, forward_scan_unicode_error_factory::
                 high_surrogate_then_end_of_stream(u'\xDBFF', same_as<CharT, wchar_t>),
         0
         ), ""},
        {u16string(u"hello") + mk_unicode<char16_t>({0xDBFF}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             5, forward_scan_unicode_error_factory::
                 high_surrogate_then_end_of_stream(u'\xDBFF', same_as<CharT, wchar_t>),
         5
         ), "hello"},
        {mk_unicode<char16_t>({0xDBFF}, u"hello"),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0, forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xDBFF', u'h', same_as<CharT, wchar_t>),
         0
         ), ""},
        {mk_unicode<char16_t>({0xD800, 0xD801}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0, forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xD800', u'\xD801', same_as<CharT, wchar_t>),
         0
         ), ""},
        {mk_unicode<char16_t>({0xDBFF, 0x4E00}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0, forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xDBFF', u'\x4E00', same_as<CharT, wchar_t>),
         0
         ), ""},
        {mk_unicode<char16_t>({0x0041, 0xD800, 0x0042, 0x0043}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             1, forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xD800', u'\x0042', same_as<CharT, wchar_t>),
         1
         ), "A"},
        {mk_unicode<char16_t>({0xDC00}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0, forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDC00', same_as<CharT, wchar_t>),
         0
         ), ""},
        {mk_unicode<char16_t>({0xDFFF}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0, forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDFFF', same_as<CharT, wchar_t>),
         0
         ), ""},
        {mk_unicode<char16_t>({0x0041, 0x0042, 0xDC00}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             2, forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDC00', same_as<CharT, wchar_t>),
         2
         ), "AB"},
        {mk_unicode<char16_t>({0xDC00, 0x0041, 0x0042}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0, forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDC00', same_as<CharT, wchar_t>),
         0
         ), ""},
        {mk_unicode<char16_t>({0xDC00, 0xDC01}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0, forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDC00', same_as<CharT, wchar_t>),
         0
         ), ""},
        {u"é",
         unicode_to_ascii_error_factory::non_ascii_character_found_from_utf16(
             U'é', {static_cast<char16_t>(0x00E9), static_cast<char16_t>(0x0)},
         0, 1,
         std::same_as<CharT, wchar_t>,
         0
         ), ""},
        {u"helloégoodbye",
         unicode_to_ascii_error_factory::non_ascii_character_found_from_utf16(
             U'é', {static_cast<char16_t>(0x00E9), static_cast<char16_t>(0x0)},
         5, 1,
         std::same_as<CharT, wchar_t>,
         5
         ), "hello"},
        {u"😂",
         unicode_to_ascii_error_factory::non_ascii_character_found_from_utf16(
             U'😂', {static_cast<char16_t>(0xD83D), static_cast<char16_t>(0xDE02)},
         0, 2,
         std::same_as<CharT, wchar_t>,
         0
         ), ""},
        {u"hello😂goodbye",
         unicode_to_ascii_error_factory::non_ascii_character_found_from_utf16(
             U'😂', {static_cast<char16_t>(0xD83D), static_cast<char16_t>(0xDE02)},
         5, 2,
         std::same_as<CharT, wchar_t>,
         5
         ), "hello"},
    };
    initializer_list<tuple<u32string, unicode_to_ascii_error, string>> invalid_u32_errors = {
        {mk_unicode<char32_t>({0xD800}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0,     forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xD800',     same_as<CharT,      wchar_t>),
         0
         ),                                                                                                      ""     },
        {mk_unicode<char32_t>({0xDBFF}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0,     forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xDBFF',     same_as<CharT,      wchar_t>),
         0
         ),                                                                                                      ""     },
        {mk_unicode<char32_t>({0xDC00}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0,     forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xDC00',     same_as<CharT,      wchar_t>),
         0
         ),                                                                                                      ""     },
        {mk_unicode<char32_t>({0xDFFF}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0,     forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xDFFF',     same_as<CharT,      wchar_t>),
         0
         ),                                                                                                      ""     },
        {mk_unicode<char32_t>({0x0041, 0x0042, 0xD800}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             2,     forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xD800',     same_as<CharT,      wchar_t>),
         2
         ),                                                                                                      "AB"   },
        {mk_unicode<char32_t>({0x0041, 0xD800, 0x0042}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             1,     forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xD800',     same_as<CharT,      wchar_t>),
         1
         ),                                                                                                      "A"    },
        {mk_unicode<char32_t>({0x11'0000}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0,     forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\x110000',   same_as<CharT,      wchar_t>),
         0
         ),                                                                                                      ""     },
        {mk_unicode<char32_t>({0x20'0000}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0,     forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\x200000',   same_as<CharT,      wchar_t>),
         0
         ),                                                                                                      ""     },
        {mk_unicode<char32_t>({0xFFFF'FFFF}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             0,     forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xFFFFFFFF', same_as<CharT,      wchar_t>),
         0
         ),                                                                                                      ""     },
        {mk_unicode<char32_t>({0x0041, 0x0042, 0x11'0000}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             2,     forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\x110000',   same_as<CharT,      wchar_t>),
         2
         ),                                                                                                      "AB"   },
        {mk_unicode<char32_t>({0x0041, 0x11'0000, 0x0042}),
         unicode_to_ascii_error_factory::invalid_unicode_character(
             1,     forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\x110000',   same_as<CharT,      wchar_t>),
         1
         ),                                                                                                      "A"    },
        {U"😂",
         unicode_to_ascii_error_factory::non_ascii_character_found_from_utf32(
             U'😂', 0,                                                  std::same_as<CharT, wchar_t>,  0
         ), ""     },
        {U"hello😂goodbye",
         unicode_to_ascii_error_factory::non_ascii_character_found_from_utf32(
             U'😂', 5,                                                  std::same_as<CharT, wchar_t>,  5
         ), "hello"},
    };
    vector<tuple<basic_string<CharT>, unicode_to_ascii_error, string>> rv;
    for (auto& [unicode_strs, err, partial_str] : invalid_ascii_errors)
    {
        // rv.push_back(make_tuple(get_unicode<CharT>(unicode_strs), err,
        // partial_str));
    }
    auto fill_vector_func = [&]<typename T>(T list_arg)
    {
        for (auto& [unicode_str, err, partial_str] : list_arg)
        {
            rv.push_back(
                make_tuple(spl_convert<CharT>(unicode_str), err, partial_str)
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
    initializer_list<tuple<
        string,
        unicode_bridge::ascii_to_unicode_error,
        basic_string<CharT>>>
        invalid_results = {
            {make_str({'\x80'},                   ""),
             ascii_to_unicode_error(0, '\x80'),
             unicode_conversion_with_exception<CharT>(u8"")     },
            {make_str({'\xFF'},                   ""),
             ascii_to_unicode_error(0, '\xFF'),
             unicode_conversion_with_exception<CharT>(u8"")     },
            {make_str({'\xC3', '\xA9'},           ""),
             ascii_to_unicode_error(0, '\xC3'),
             unicode_conversion_with_exception<CharT>(u8"")     },
            {make_str({'\x80', '\x41'},           ""),
             ascii_to_unicode_error(0, '\x80'),
             unicode_conversion_with_exception<CharT>(u8"")     },
            {make_str({'\x41', '\x80'},           ""),
             ascii_to_unicode_error(1, '\x80'),
             unicode_conversion_with_exception<CharT>(u8"A")     },

            {make_str({'\xFF', '\xFE'},           ""),
             ascii_to_unicode_error(0, '\xFF'),
             unicode_conversion_with_exception<CharT>(u8"")     },
            {make_str({'\x81', '\x82', '\x83'},   ""),
             ascii_to_unicode_error(0, '\x81'),
             unicode_conversion_with_exception<CharT>(u8"")     },

            {string("hello") + make_str({'\x80'}, "hello"),
             ascii_to_unicode_error(5, '\x80'),
             unicode_conversion_with_exception<CharT>(u8"hello")},
            {make_str({'\x80'},                   "hello"),
             ascii_to_unicode_error(0, '\x80'),
             unicode_conversion_with_exception<CharT>(u8"")     },
            {string("h") + make_str({'\x80'},     "ello"),
             ascii_to_unicode_error(1, '\x80'),
             unicode_conversion_with_exception<CharT>(u8"h")    },
    };
    // Run invalid tests.
    for (auto&& [input_string, expected_arg, partial_str_res] : invalid_results)
    {
        INFO("Input string " << input_string);
        auto run_func = [&]<typename T>(const T input_arg)
        {
            // Checks its valid ascii.
            REQUIRE(is_valid_ascii(input_arg) == false);
            auto check_func
                = [&](const std::tuple<
                      // Used to catch non-caught exceptions. False is "major
                      // unknown failure".
                      bool,
                      // Catches the half-created output.
                      std::optional<pair<basic_string<CharT>, size_t>>,
                      // Result and optional error.
                      std::expected<
                          std::basic_string<CharT>,
                          optional<ascii_to_unicode_error>>>& result_arg)
            {
                if (get<0>(result_arg) == false)
                {
                    INFO("Unknown functio failure");
                    FAIL();
                }
                if (get<2>(result_arg).has_value())
                {
                    string output{
                        "Unexpected successful convert_unicode_to_ascii "
                        "conversion. std::string result is \""
                    };
                    output.append(
                        unicode_print(get<2>(result_arg).value()).str()
                    );
                    output.append("\"");
                    FAIL(output);
                }
                else
                {
                    // Otherwise, test that the failure is the one expected.
                    if (get<2>(result_arg).error().has_value())
                    {
                        auto& failure_result{get<2>(result_arg).error().value()
                        };
                        INFO(unicode_print(failure_result.message(input_string))
                        );
                        REQUIRE(
                            failure_result.get_character()
                            == expected_arg.get_character()
                        );
                        REQUIRE(
                            failure_result.get_index()
                            == expected_arg.get_index()
                        );
                        if (get<1>(result_arg).has_value())
                        {
                            if ((
                                (get<1>(result_arg).value().first)
                                != partial_str_res
                                ))
                            {
                                int x = 4;
                            }
                            REQUIRE(
                                (get<1>(result_arg).value().first)
                                == partial_str_res
                            );
                            REQUIRE(
                                (get<1>(result_arg).value().second)
                                == expected_arg.get_index()
                            );
                        }
                    }
                    else
                    {
                        SUCCEED();
                    }
                }
            };
            tuple<
                bool,
                optional<pair<basic_string<CharT>,size_t>>,
                expected<basic_string<CharT>, optional<ascii_to_unicode_error>>>
                test_arg;
            {
                auto result
                    = unicode_bridge::convert_ascii_to_unicode<CharT>(input_arg
                    );
                check_func({true, std::nullopt, result});
            }
            {
                test_arg
                    = {true,
                       std::nullopt,
                       unicode_conversion_with_exception<CharT>(u8"")};
                try
                {
                    // Checks that with_exception also returns the correct
                    // value.
                    auto result
                        = convert_ascii_to_unicode_with_exception<CharT>(
                            input_arg
                        );
                    get<2>(test_arg) = result;
                }
                catch (const unicode_bridge_exception<ascii_to_unicode_error>&
                           unexpected_exception)
                {
                    get<2>(test_arg) = unexpected(unexpected_exception.error());
                }
                catch (...)
                {
                    get<0>(test_arg) = false;
                }
                check_func(test_arg);
            }
            {
                test_arg
                    = {true,
                       std::nullopt,
                       unicode_conversion_with_exception<CharT>(u8"")};
                basic_string<CharT> str;
                auto result = unicode_bridge::convert_ascii_to_unicode_append(
                    input_arg, str
                );
                if (result.has_value())
                {
                    get<2>(test_arg) = unexpected(result.value());
                }
                else
                {
                    get<2>(test_arg) = str;
                }
                check_func(test_arg);
            }
            {
                test_arg
                    = {true,
                       std::nullopt,
                       unicode_conversion_with_exception<CharT>(u8"")};
                auto result
                    = unicode_bridge::convert_ascii_to_unicode_no_error<CharT>(
                        input_arg
                    );
                if (result.has_value())
                {
                    get<2>(test_arg) = result.value();
                }
                else
                {
                    get<2>(test_arg) = unexpected(std::nullopt);
                }
                check_func(test_arg);
            }
            {
                test_arg
                    = {true,
                       std::nullopt,
                       unicode_conversion_with_exception<CharT>(u8"")};
                basic_string<CharT> str;
                auto                result
                    = unicode_bridge::convert_ascii_to_unicode_append_no_error(
                        input_arg, str
                    );
                if (result.has_value())
                {
                    get<1>(test_arg) = make_pair(str,result.value());
                    get<2>(test_arg) = unexpected(std::nullopt);
                }
                else
                {
                    get<2>(test_arg) = str;
                }
                check_func(test_arg);
            }
            {
                basic_string<CharT> str;
                test_arg
                    = {true,
                       std::nullopt,
                       unicode_conversion_with_exception<CharT>(u8"")};
                try
                {
                    // Checks that with_exception also returns the correct
                    // value.
                    convert_ascii_to_unicode_append_with_exception<CharT>(
                        input_arg, str
                    );
                    get<2>(test_arg) = str;
                }
                catch (const unicode_bridge_exception<ascii_to_unicode_error>&
                           unexpected_exception)
                {
                    get<1>(test_arg) = make_pair(str, unexpected_exception.error().get_index());
                    get<2>(test_arg) = unexpected(unexpected_exception.error());
                }
                catch (...)
                {
                    get<0>(test_arg) = false;
                }
                check_func(test_arg);
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
            auto check_func = [&](const std::tuple<
                                  // Used to catch non-caught exceptions. False
                                  // is "major unknown failure".
                                  bool,
                                  // Result and optional error.
                                  std::expected<
                                      std::basic_string<CharT>,
                                      ascii_to_unicode_error>>& result_arg)
            {
                if (get<0>(result_arg) == false)
                {
                    INFO("Unknown functio failure");
                    FAIL();
                }
                if (get<1>(result_arg).has_value())
                {
                    if (not (
                            (get<1>(result_arg).value()) ==

                            unicode_conversion_with_exception<CharT>(
                                u32_output_string
                            )
                        ))
                    {
                        int  x = 4;
                        auto result{
                            unicode_bridge::convert_ascii_to_unicode<CharT>(
                                input_arg
                            )
                        };
                    }
                    auto resk = unicode_conversion_with_exception<CharT>(
                        u32_output_string
                    );
                    REQUIRE(
                        (get<1>(result_arg).value()) ==

                        resk
                    );
                }
                else
                {
                    string output{"Unexpected output: \""};
                    output.append(convert_unicode_to_string(
                        get<1>(result_arg).error().message()
                    ));
                    output.append("\"");
                    FAIL(output);
                }
            };
            tuple<bool, expected<basic_string<CharT>, ascii_to_unicode_error>>
                test_args;
            // Checks is_valid_ascii fails.
            REQUIRE(is_valid_ascii(input_arg) == true);
            {
                auto result{
                    unicode_bridge::convert_ascii_to_unicode<CharT>(input_arg)
                };
                check_func({true, result});
            }
            {
                test_args = {true, basic_string<CharT>()};
                basic_string<CharT> str;
                auto result{unicode_bridge::convert_ascii_to_unicode_append(
                    input_arg, str
                )};
                if (result.has_value())
                {
                    get<1>(test_args) = unexpected(result.value());
                }
                else
                {
                    get<1>(test_args) = str;
                }
                check_func(test_args);
            }
            {
                test_args = {true, basic_string<CharT>()};
                basic_string<CharT> str;
                auto                result{
                    unicode_bridge::convert_ascii_to_unicode_no_error<CharT>(
                        input_arg
                    )
                };
                if (result.has_value())
                {
                    get<1>(test_args) = result.value();
                }
                else
                {
                    get<0>(test_args) = false;
                }
                check_func(test_args);
            }
            {
                test_args = {true, basic_string<CharT>()};
                basic_string<CharT> str;
                auto                result{
                    unicode_bridge::convert_ascii_to_unicode_append_no_error(
                        input_arg, str
                    )
                };
                if (result.has_value())
                {
                    get<0>(test_args) = false;
                }
                else
                {
                    get<1>(test_args) = str;
                }
                check_func(test_args);
            }

            {
                test_args = {true, basic_string<CharT>()};
                basic_string<CharT> str;
                try
                {
                    // Checks that with_exception also returns the correct
                    // value.
                    auto result
                        = convert_ascii_to_unicode_with_exception<CharT>(
                            input_arg
                        );
                    get<1>(test_args) = result;
                }
                catch (const unicode_bridge_exception<ascii_to_unicode_error>&
                           unexpected_exception)
                {
                    get<1>(test_args)
                        = unexpected(unexpected_exception.error());
                }
                catch (...)
                {
                    get<0>(test_args) = false;
                }
                check_func(test_args);
            }

            {
                test_args = {true, basic_string<CharT>()};
                basic_string<CharT> str;
                try
                {
                    // Checks that with_exception also returns the correct
                    // value.
                    convert_ascii_to_unicode_append_with_exception(
                        input_arg, str
                    );
                    get<1>(test_args) = str;
                }
                catch (const unicode_bridge_exception<ascii_to_unicode_error>&
                           unexpected_exception)
                {
                    get<1>(test_args)
                        = unexpected(unexpected_exception.error());
                }
                catch (...)
                {
                    get<0>(test_args) = false;
                }
                check_func(test_args);
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
        auto check_func = [&](const std::tuple<
                              // Used to catch non-caught exceptions. False
                              // is "major unknown failure".
                              bool,
                              // Result and optional error.
                              std::expected<
                                  std::basic_string<CharU>,
                                  unicode_conversion_error>>& result_arg)
        {
            if (get<0>(result_arg) == false)
            {
                INFO("Unknown functio failure");
                FAIL();
            }
            if (get<1>(result_arg).has_value())
            {
                if ((get<1>(result_arg).value()) == expected_output)
                {
                    int  x         = 4;
                    auto converted = unicode_conversion<CharU>(input_argument);
                }
                REQUIRE((get<1>(result_arg).value()) == expected_output);
            }
            else
            {
                string output{"Unexpected output: \""};
                output.append(
                    unicode_print(get<1>(result_arg).error().message()).str()
                );
                output.append("\"");
                FAIL(output);
            }
        };
        tuple<bool, expected<basic_string<CharU>, unicode_conversion_error>>
            test_args;
        {
            auto converted = unicode_conversion<CharU>(input_argument);
            check_func({true, converted});
        }
        {
            test_args = {true, basic_string<CharU>()};
            basic_string<CharU> str;
            auto res = unicode_conversion_append(input_argument, str);
            if (res.has_value())
            {
                get<1>(test_args) = unexpected(res.value());
            }
            else
            {
                get<1>(test_args) = str;
            }
            check_func(test_args);
        }
        {
            test_args = {true, basic_string<CharU>()};
            auto res  = unicode_conversion_no_error<CharU>(input_argument);
            if (res.has_value())
            {
                get<1>(test_args) = res.value();
            }
            else
            {
                get<0>(test_args) = false;
            }
            check_func(test_args);
        }
        {
            test_args = {true, basic_string<CharU>()};
            basic_string<CharU> str;
            auto res = unicode_conversion_append_no_error(input_argument, str);
            if (res.has_value())
            {
                get<0>(test_args) = false;
            }
            else
            {
                get<1>(test_args) = str;
            }
            check_func(test_args);
        }
        {
            test_args = {true, basic_string<CharU>()};
            try
            {
                auto res
                    = unicode_conversion_with_exception<CharU>(input_argument);
                get<1>(test_args) = res;
            }
            catch (const unicode_bridge_exception<unicode_conversion_error>&
                       unexpected_exception)
            {
                get<1>(test_args) = unexpected(unexpected_exception.error());
            }
            catch (...)
            {
                get<0>(test_args) = false;
            }
        }
        {
            test_args = {true, basic_string<CharU>()};
            try
            {
                basic_string<CharU> str;
                unicode_conversion_append_with_exception<CharU>(
                    input_argument, str
                );
                get<1>(test_args) = str;
            }
            catch (const unicode_bridge_exception<unicode_conversion_error>&
                       unexpected_exception)
            {
                get<1>(test_args) = unexpected(unexpected_exception.error());
            }
            catch (...)
            {
                get<0>(test_args) = false;
            }
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
        const std::vector<std::tuple<
            std::basic_string<T>,
            unicode_bridge::unicode_conversion_error,
            std::basic_string<T>>>& list_of_test_cases
    )
{
    using namespace unicode_bridge;
    using namespace std;
    for (auto&& [input_string, expected_error, partial_str_res] :
         list_of_test_cases)
    {
        CHECK(is_valid_unicode(input_string) == false);
        INFO("str = " << convert_unicode_to_string(input_string));
        auto test_func
            = [&]<typename TargetType, typename U>(const U input_argument)
        {
            auto check_func =
                [&](const std::tuple<
                    // Used to catch non-caught exceptions. False is "major
                    // unknown failure".
                    bool,
                    // Catches the half-created output.
                    optional<std::pair<basic_string<TargetType>, std::size_t>>,
                    // Result and optional error.
                    std::expected<
                        std::basic_string<TargetType>,
                        optional<unicode_conversion_error>>>& result_arg)
            {
                if (get<0>(result_arg) == false)
                {
                    INFO("Unknown functio failure");
                    FAIL();
                }
                if (get<2>(result_arg).has_value())
                {
                    string output{"Unexpected successful unicode_conversion "
                                  "conversion. result is \""};
                    output.append(
                        unicode_print(get<2>(result_arg).value()).str()
                    );
                    output.append("\"");
                    FAIL(output);
                }
                else
                {
                    // Otherwise, test that the failure is the one expected.
                    if (get<2>(result_arg).error().has_value())
                    {
                        auto& failure_result{get<2>(result_arg).error().value()
                        };
                        INFO(unicode_print(failure_result.message(input_string))
                        );
                        equal_unicode_error(failure_result, expected_error);
                        auto normalised_res
                            = unicode_conversion_with_exception<TargetType>(
                                partial_str_res
                            );
                        if (get<1>(result_arg).has_value())
                        {
                            REQUIRE(
                                (get<1>(result_arg).value().first)
                                == normalised_res
                            );
                            REQUIRE(
                                (get<1>(result_arg).value().second)
                                == expected_error.partial_output_size()
                            );
                        }
                    }
                    else
                    {
                        SUCCEED();
                    }
                }
            };
            tuple<
                bool,
                optional<std::pair<basic_string<TargetType>, std::size_t>>,
                expected<
                    basic_string<TargetType>,
                    optional<unicode_conversion_error>>>
                test_arg;
            {
                test_arg = {true, std::nullopt, basic_string<TargetType>()};
                auto converted = unicode_conversion<TargetType>(input_argument);
                get<2>(test_arg) = converted;
                check_func(test_arg);
            }

            {
                test_arg = {true, std::nullopt, basic_string<TargetType>()};
                auto converted
                    = unicode_conversion_no_error<TargetType>(input_argument);
                if (converted.has_value())
                {
                    get<2>(test_arg) = converted.value();
                }
                else
                {
                    get<2>(test_arg) = unexpected(std::nullopt);
                }
                check_func(test_arg);
            }

            {
                test_arg = {true, std::nullopt, basic_string<TargetType>()};
                basic_string<TargetType> str;
                auto converted = unicode_conversion_append(input_argument, str);
                if (converted.has_value())
                {
                    get<2>(test_arg) = unexpected(converted.value());
                    get<1>(test_arg
                    ) = make_pair(str, converted.value().partial_output_size());
                }
                else
                {
                    get<2>(test_arg) = str;
                }
                check_func(test_arg);
            }
            {
                test_arg = {true, std::nullopt, basic_string<TargetType>()};
                basic_string<TargetType> str;
                auto                     converted
                    = unicode_conversion_append_no_error(input_argument, str);
                if (converted.has_value())
                {
                    get<2>(test_arg) = unexpected(std::nullopt);
                    get<1>(test_arg) = make_pair(str, converted.value());
                }
                else
                {
                    get<2>(test_arg) = str;
                }
                check_func(test_arg);
            }

            {
                test_arg = {true, std::nullopt, basic_string<TargetType>()};
                try
                {
                    auto converted
                        = unicode_conversion_with_exception<TargetType>(
                            input_argument
                        );
                    get<2>(test_arg) = converted;
                }
                catch (const unicode_bridge_exception<unicode_conversion_error>&
                           _exception)
                {
                    get<2>(test_arg) = unexpected(_exception.error());
                }
                catch (...)
                {
                    get<0>(test_arg) = false;
                }
                check_func(test_arg);
            }

            {
                test_arg = {true, std::nullopt, basic_string<TargetType>()};
                basic_string<TargetType> str;
                try
                {
                    unicode_conversion_append_with_exception<TargetType>(
                        input_argument, str
                    );
                    get<2>(test_arg) = str;
                }
                catch (const unicode_bridge_exception<unicode_conversion_error>&
                           _exception)
                {
                    get<1>(test_arg) = make_pair(
                        str, _exception.error().partial_output_size()
                    );
                    get<2>(test_arg) = unexpected(_exception.error());
                }
                catch (...)
                {
                    get<0>(test_arg) = false;
                }
                check_func(test_arg);
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
std::vector<std::tuple<
    std::basic_string<CharT>,
    unicode_bridge::unicode_conversion_error,
    std::basic_string<CharT>>>
    get_invalid_unicode_conversion_and_is_valid_unicode_data()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge::internal;
    std::array<std::u8string, 4> arr;
    initializer_list<tuple<u8string, unicode_conversion_error, u8string>>
        invalid_u8_errors = {
            {mk_unicode<char8_t>({0xFF}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xFF)
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xFE}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xFE)
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0x80}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0x80)
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xBF}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xBF)
                 ), 0
             ), u8""},
            {u8string(u8"abc").append(mk_unicode<char8_t>({0xFF}, u8"def")),
             unicode_conversion_error(
                 3, forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xFF)
                 ), 3
             ), u8"abc"},

            {mk_unicode<char8_t>({0xC2}, u8""),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xC2), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_2_found_1
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xE2}, u8""),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xE2), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_3_found_1
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xE2, 0x80}, u8""),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x80),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_3_found_2
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xF0}, u8""),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xF0), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_1
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xF0, 0x90}, u8""),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_2
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x80}, u8""),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_3
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xE2, 0x80}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x80),
                      u8'h',
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_3_invalid_indexes_2
                 ), 0
             ), u8""},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xF0, 0x90}, u8"def"),
             unicode_conversion_error(
                 3, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      u8'd',
                      u8'e'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_2_3
                 ), 3
             ), u8"abc"},
            {mk_unicode<char8_t>({0xC2, 0x00}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xC2),
                      static_cast<char8_t>(0x00),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_2_invalid_indexes_1
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xC2, 0x20}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xC2),
                      static_cast<char8_t>(0x20),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_2_invalid_indexes_1
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xC2, 0xC0}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xC2),
                      static_cast<char8_t>(0xC0),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_2_invalid_indexes_1
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xE2, 0x80, 0x20}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x20),
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_3_invalid_indexes_2
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xE2, 0x20, 0x80}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_3_invalid_indexes_1
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x20, 0x80}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x80)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_2
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x80, 0x20}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x20)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_3
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xF0, 0x20, 0x80, 0x80}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_1
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xC0, 0x80}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xC0),
                      static_cast<char8_t>(0x80),
                      u8'\0',
                      u8'\0'},
             2, U'\0'
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xC1, 0xBF}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xC1),
                      static_cast<char8_t>(0xBF),
                      u8'\0',
                      u8'\0'},
             2, U'\x7F'
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xE0, 0x80, 0x80}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xE0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             3, U'\0'
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xE0, 0x81, 0xBF}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xE0),
                      static_cast<char8_t>(0x81),
                      static_cast<char8_t>(0xBF),
                      u8'\0'},
             3, U'\x7F'
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xE0, 0x9F, 0xBF}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xE0),
                      static_cast<char8_t>(0x9F),
                      static_cast<char8_t>(0xBF),
                      u8'\0'},
             3, U'\x7FF'
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xF0, 0x80, 0x80, 0x80}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             4, U'\0'
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xF0, 0x80, 0x80, 0xBF}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0xBF)},
             4, U'\x3F'
                 ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xED, 0xA0, 0x80}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xA0),
                          static_cast<char8_t>(0x80),
                          u8'\0'},
             3, U'\xD800'
                     ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xED, 0xAF, 0xBF}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xAF),
                          static_cast<char8_t>(0xBF),
                          u8'\0'},
             3, U'\xDBFF'
                     ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xED, 0xB0, 0x80}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xB0),
                          static_cast<char8_t>(0x80),
                          u8'\0'},
             3, U'\xDC00'
                     ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xED, 0xBF, 0xBF}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xBF),
                          static_cast<char8_t>(0xBF),
                          u8'\0'},
             3, U'\xDFFF'
                     ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xF4, 0x90, 0x80, 0x80}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xF4),
                          static_cast<char8_t>(0x90),
                          static_cast<char8_t>(0x80),
                          static_cast<char8_t>(0x80)},
             4, U'\x110000'
                     ), 0
             ), u8""},
            {mk_unicode<char8_t>({0xF7, 0xBF, 0xBF, 0xBF}, u8"hello"),
             unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xF7),
                          static_cast<char8_t>(0xBF),
                          static_cast<char8_t>(0xBF),
                          static_cast<char8_t>(0xBF)},
             4, U'\x1FFFFF'
                     ), 0
             ), u8""},
            {u8string(u8"abc")
                 + mk_unicode<char8_t>({0xED, 0xA0, 0x80}, u8"def"),
             unicode_conversion_error(
                 3, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xA0),
                          static_cast<char8_t>(0x80),
                          u8'\0'},
             3, U'\xD800'
                     ), 3
             ), u8"abc"},
    };
    initializer_list<tuple<u16string, unicode_conversion_error, u16string>> invalid_u16_errors = {
        {mk_unicode<char16_t>({0xD800}, u""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 high_surrogate_then_end_of_stream(u'\xD800', same_as<CharT, wchar_t>),
         0
         ), u""},
        {mk_unicode<char16_t>({0xDBFF}, u""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 high_surrogate_then_end_of_stream(u'\xDBFF', same_as<CharT, wchar_t>),
         0
         ), u""},
        {u16string(u"hello") + mk_unicode<char16_t>({0xDBFF}, u""),
         unicode_conversion_error(
             5, forward_scan_unicode_error_factory::
                 high_surrogate_then_end_of_stream(u'\xDBFF', same_as<CharT, wchar_t>),
         5
         ), u"hello"},
        {mk_unicode<char16_t>({0xDBFF}, u"hello"),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xDBFF', u'h', same_as<CharT, wchar_t>),
         0
         ), u""},
        {mk_unicode<char16_t>({0xD800, 0xD801}, u""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xD800', u'\xD801', same_as<CharT, wchar_t>),
         0
         ), u""},
        {mk_unicode<char16_t>({0xDBFF, 0x4E00}, u""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xDBFF', u'\x4E00', same_as<CharT, wchar_t>),
         0
         ), u""},
        {mk_unicode<char16_t>({0x0041, 0xD800, 0x0042, 0x0043}, u""),
         unicode_conversion_error(
             1, forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xD800', u'\x0042', same_as<CharT, wchar_t>),
         1
         ), u"A"},
        {mk_unicode<char16_t>({0xDC00}, u""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDC00', same_as<CharT, wchar_t>),
         0
         ), u""},
        {mk_unicode<char16_t>({0xDFFF}, u""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDFFF', same_as<CharT, wchar_t>),
         0
         ), u""},
        {mk_unicode<char16_t>({0x0041, 0x0042, 0xDC00}, u""),
         unicode_conversion_error(
             2, forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDC00', same_as<CharT, wchar_t>),
         2
         ), u"AB"},
        {mk_unicode<char16_t>({0xDC00, 0x0041, 0x0042}, u""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDC00', same_as<CharT, wchar_t>),
         0
         ), u""},
        {mk_unicode<char16_t>({0xDC00, 0xDC01}, u""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDC00', same_as<CharT, wchar_t>),
         0
         ), u""},
    };
    initializer_list<tuple<u32string, unicode_conversion_error, u32string>> invalid_u32_errors = {
        {mk_unicode<char32_t>({0xD800},                    U""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xD800',     same_as<CharT, wchar_t>),
         0
         ), U""  },
        {mk_unicode<char32_t>({0xDBFF},                    U""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xDBFF',     same_as<CharT, wchar_t>),
         0
         ), U""  },
        {mk_unicode<char32_t>({0xDC00},                    U""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xDC00',     same_as<CharT, wchar_t>),
         0
         ), U""  },
        {mk_unicode<char32_t>({0xDFFF},                    U""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xDFFF',     same_as<CharT, wchar_t>),
         0
         ), U""  },
        {mk_unicode<char32_t>({0x0041, 0x0042, 0xD800},    U""),
         unicode_conversion_error(
             2, forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xD800',     same_as<CharT, wchar_t>),
         2
         ), U"AB"},
        {mk_unicode<char32_t>({0x0041, 0xD800, 0x0042},    U""),
         unicode_conversion_error(
             1, forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xD800',     same_as<CharT, wchar_t>),
         1
         ), U"A" },
        {mk_unicode<char32_t>({0x11'0000},                 U""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\x110000',   same_as<CharT, wchar_t>),
         0
         ), U""  },
        {mk_unicode<char32_t>({0x20'0000},                 U""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\x200000',   same_as<CharT, wchar_t>),
         0
         ), U""  },
        {mk_unicode<char32_t>({0xFFFF'FFFF},               U""),
         unicode_conversion_error(
             0, forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xFFFFFFFF', same_as<CharT, wchar_t>),
         0
         ), U""  },
        {mk_unicode<char32_t>({0x0041, 0x0042, 0x11'0000}, U""),
         unicode_conversion_error(
             2, forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\x110000',   same_as<CharT, wchar_t>),
         2
         ), U"AB"},
        {mk_unicode<char32_t>({0x0041, 0x11'0000, 0x0042}, U""),
         unicode_conversion_error(
             1, forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\x110000',   same_as<CharT, wchar_t>),
         1
         ), U"A" },
    };
    vector<tuple<
        basic_string<CharT>,
        unicode_conversion_error,
        basic_string<CharT>>>
         rv;
    auto fill_vector_func = [&]<typename T>(T list_arg)
    {
        for (auto& [unicode_str, err, partial_str] : list_arg)
        {
            rv.push_back(make_tuple(
                spl_convert<CharT>(unicode_str),
                err,
                spl_convert<CharT>(partial_str)
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
            auto   begin_iterator   = std::begin(input_argument);
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
                    if (pair_arg.first != unicode_char
                        || pair_arg.second != iterator_index)
                    {
                        int x = 4;
                    }
                    REQUIRE(pair_arg.first == unicode_char);
                    REQUIRE(pair_arg.second == iterator_index);
                };
                // Test with error returned.
                auto res = next_char32(current_iterator, end_iterator);
                if (res.has_value())
                {
                    test_result_1(res.value());
                }
                else
                {
                    FAIL(
                        "Unexpected error \""
                        << convert_unicode_to_string(res.error().message())
                        << "\""
                    );
                }
                // Test without error returned.
                auto res2
                    = next_char32_no_error(current_iterator, end_iterator);
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
                auto res3 = next_char32_and_increment_iterator(
                    new_iterator, end_iterator
                );
                if (res3.has_value())
                {
                    test_result_2(res3.value());
                }
                else
                {
                    auto str_as_u8 = res.error().message();
                    auto str_containing_u8
                        = string(str_as_u8.begin(), str_as_u8.end());
                    FAIL("Unexpected error \"" << str_containing_u8);
                }

                // Next with no error returned and increment iterator.
                // Reset iterator.
                new_iterator = current_iterator;
                auto res4    = next_char32_and_increment_iterator_no_error(
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
                        unexpected_exception.error().message()
                    ));
                    msg.append("\"");
                    FAIL(msg);
                }
                catch (...)
                {
                    string msg = "Unexpected, unknown exception";
                    FAIL(msg);
                }

                // Now test all the backwards versions too.
                auto next_iterator = current_iterator + iterator_index;
                auto res5          = prev_char32(next_iterator, begin_iterator);
                if (res5.has_value())
                {
                    test_result_1(res5.value());
                }
                else
                {
                    FAIL(
                        "Unexpected error \""
                        << convert_unicode_to_string(res5.error().message())
                        << "\""
                    );
                }
                // Test without error returned.
                auto res6 = prev_char32_no_error(next_iterator, begin_iterator);
                if (res6.has_value())
                {
                    test_result_1(res6.value());
                }
                else
                {
                    FAIL("Unexpected empty optional");
                }

                try
                {
                    // Checks that with_exception also returns the
                    // correct value.
                    auto res6a = prev_char32_with_exception(
                        next_iterator, begin_iterator
                    );
                    test_result_1(res6a);
                }
                catch (const unicode_bridge_exception<prev_char32_error>&
                           unexpected_exception)
                {
                    string msg = "Unexpected exception: \"";
                    msg.append(convert_unicode_to_string(
                        unexpected_exception.error().message(

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
                new_iterator = next_iterator;
                auto res7    = prev_char32_and_decrement_iterator(
                    new_iterator, begin_iterator
                );
                auto test_result_3 = [&](const auto& character_arg)
                {
                    if (not (
                            std::distance(new_iterator, next_iterator)
                            == (iterator_index)
                        ))
                    {
                        int x = 4;
                    }
                    CHECK(character_arg == unicode_char);
                    CHECK(
                        std::distance(new_iterator, next_iterator)
                        == (iterator_index)
                    );
                };
                if (res7.has_value())
                {
                    test_result_3(res7.value());
                }
                else
                {
                    auto str_as_u8 = res7.error().message();
                    auto str_containing_u8
                        = string(str_as_u8.begin(), str_as_u8.end());
                    FAIL("Unexpected error \"" << str_containing_u8);
                }

                // Next with no error returned and increment iterator.
                // Reset iterator.
                new_iterator = next_iterator;
                auto res8    = prev_char32_and_decrement_iterator_no_error(
                    new_iterator, begin_iterator
                );
                if (res8.has_value())
                {
                    test_result_3(res8.value());
                }
                else
                {
                    FAIL("Unexpected empty optional");
                }
                new_iterator = next_iterator;
                try
                {
                    // Checks that with_exception also returns the
                    // correct value.
                    auto res4a
                        = prev_char32_and_decrement_iterator_with_exception(
                            new_iterator, begin_iterator
                        );
                    test_result_3(res4a);
                }
                catch (const unicode_bridge_exception<prev_char32_error>&
                           unexpected_exception)
                {
                    string msg = "Unexpected exception: \"";
                    msg.append(convert_unicode_to_string(
                        unexpected_exception.error().message()
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
                next_char32_and_increment_iterator_no_error(
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
    "next_char32_t, next_char32_t_and_increment_iterator, prev_char32_t and "
    "prev_char32_t_and_decrement_iterator are correct "
    "for char8_t arguments",
    "[next_char32_t,next_char32_t_and_increment_iterator,prev_char32_t,prev_"
    "char32_t_and_decrement_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    run_valid_next_char32_t_checks(get_valid_next_char_32_t_data<char8_t>());
}

TEST_CASE(
    "next_char32_t, next_char32_t_and_increment_iterator, prev_char32_t and "
    "prev_char32_t_and_decrement_iterator are correct for "
    "char16_t arguments",
    "[next_char32_t,next_char32_t_and_increment_iterator,prev_char32_t,prev_"
    "char32_t_and_decrement_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    run_valid_next_char32_t_checks(get_valid_next_char_32_t_data<char16_t>());
}

TEST_CASE(
    "next_char32_t, next_char32_t_and_increment_iterator, prev_char32_t and "
    "prev_char32_t_and_decrement_iterator are correct for "
    "char32_t arguments",
    "[next_char32_t,next_char32_t_and_increment_iterator,prev_char32_t,prev_"
    "char32_t_and_decrement_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    run_valid_next_char32_t_checks(get_valid_next_char_32_t_data<char32_t>());
}

TEST_CASE(
    "next_char32_t, next_char32_t_and_increment_iterator, prev_char32_t and "
    "prev_char32_t_and_decrement_iterator are correct for "
    "wchar_t arguments",
    "[next_char32_t,next_char32_t_and_increment_iterator,prev_char32_t,prev_"
    "char32_t_and_decrement_iterator]"
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
            unicode_bridge::next_char32_error>>& invalid_strs_arg
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

            auto res = next_char32(current_iterator, end_iterator);
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
                equal_next_char32_error(res.error(), expected_output);
            }

            auto res2 = next_char32_no_error(current_iterator, end_iterator);
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
                equal_next_char32_error(_exception.error(), expected_output);
            }
            catch (...)
            {
                string msg = "Unexpected, unknown exception";
                FAIL(msg);
            }

            auto new_iterator = current_iterator;
            auto res3         = next_char32_and_increment_iterator(
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
                equal_next_char32_error(res.error(), expected_output);
                CHECK(std::distance(new_iterator, current_iterator) == 0);
            }

            new_iterator = current_iterator;
            auto res4    = next_char32_and_increment_iterator_no_error(
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
                auto res4a = next_char32_and_increment_iterator_with_exception(
                    current_iterator, end_iterator
                );
                FAIL("Unexpected successful call of "
                     "next_char32_t_with_exception.");
            }
            catch (const unicode_bridge_exception<next_char32_error>& _exception
            )
            {
                equal_next_char32_error(_exception.error(), expected_output);
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
    unicode_bridge::next_char32_error>>
    get_invalid_next_char32_t_data()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge::internal;
    std::array<std::u8string, 4> arr;
    initializer_list<tuple<u8string, size_t, next_char32_error>>
        invalid_u8_errors = {
            // invalid leading byte at offset 0
            {mk_unicode<char8_t>({0xFF}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xFF)
                 )
             )},
            {mk_unicode<char8_t>({0x80}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0x80)
                 )
             )},
            {mk_unicode<char8_t>({0xBF}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xBF)
                 )
             )},
            {mk_unicode<char8_t>({0xFF}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xFF)
                 )
             )},
            // invalid leading byte mid-string
            {u8string(u8"a") + mk_unicode<char8_t>({0xFF}),
             1, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xFF)
                 )
             )},
            {u8string(u8"ab") + mk_unicode<char8_t>({0x80}),
             2, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0x80)
                 )
             )},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xFE}),
             3, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte(
                     static_cast<char8_t>(0xFE)
                 )
             )},

            // truncated sequence at offset 0
            {mk_unicode<char8_t>({0xC2}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xC2), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_2_found_1
                 )
             )},
            {mk_unicode<char8_t>({0xE2}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xE2), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_3_found_1
                 )
             )},
            {mk_unicode<char8_t>({0xE2, 0x80}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x80),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_3_found_2
                 )
             )},
            {mk_unicode<char8_t>({0xF0}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xF0), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_1
                 )
             )},
            {mk_unicode<char8_t>({0xF0, 0x90}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_2
                 )
             )},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x80}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_3
                 )
             )},
            // truncated sequence mid-string
            {u8string(u8"a") + mk_unicode<char8_t>({0xC2}),
             1, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xC2), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_2_found_1
                 )
             )},
            {u8string(u8"ab") + mk_unicode<char8_t>({0xE2, 0x80}),
             2, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x80),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_3_found_2
                 )
             )},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xF0, 0x90, 0x80}),
             3, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_3
                 )
             )},

            // invalid continuation byte at offset 0
            {mk_unicode<char8_t>({0xC2, 0x20}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xC2),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x00),
                      static_cast<char8_t>(0x00)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_2_invalid_indexes_1
                 )
             )},
            {mk_unicode<char8_t>({0xE2, 0x21, 0x80}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x21),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x00)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_3_invalid_indexes_1
                 )
             )},
            {mk_unicode<char8_t>({0xE2, 0x80, 0x20}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x00)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_3_invalid_indexes_2
                 )
             )},
            {mk_unicode<char8_t>({0xF0, 0x20, 0x80, 0x80}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_1
                 )
             )},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x20, 0x80}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x80)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_2
                 )
             )},
            {mk_unicode<char8_t>({0xF0, 0x90, 0x80, 0x20}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x20)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_3
                 )
             )},
            // invalid continuation byte mid-string
            {u8string(u8"a") + mk_unicode<char8_t>({0xC2, 0x20}),
             1, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xC2),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x00),
                      static_cast<char8_t>(0x00)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_2_invalid_indexes_1
                 )
             )},
            {u8string(u8"ab") + mk_unicode<char8_t>({0xE2, 0x80, 0x20}),
             2, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x00)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_3_invalid_indexes_2
                 )
             )},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xF0, 0x90, 0x80, 0x20}),
             3, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x20)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_3
                 )
             )},
            // overlong at offset 0
            {mk_unicode<char8_t>({0xC0, 0x80}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xC0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x00),
                      static_cast<char8_t>(0x00)},
             2, U'\0'
                 )
             )},
            {mk_unicode<char8_t>({0xC1, 0xBF}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xC1),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0x00),
                      static_cast<char8_t>(0x00)},
             2, U'\u007F'
                 )
             )},
            {mk_unicode<char8_t>({0xE0, 0x9F, 0xBF}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xE0),
                      static_cast<char8_t>(0x9F),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0x00)},
             3, U'\u07FF'
                 )
             )},
            {mk_unicode<char8_t>({0xF0, 0x80, 0x80, 0x80}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             4, U'\0'
                 )
             )},

            // overlong mid-string
            {u8string(u8"a") + mk_unicode<char8_t>({0xC0, 0x80}),
             1, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xC0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x00),
                      static_cast<char8_t>(0x00)},
             2, U'\0'
                 )
             )},
            {u8string(u8"ab") + mk_unicode<char8_t>({0xE0, 0x9F, 0xBF}),
             2, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xE0),
                      static_cast<char8_t>(0x9F),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0x00)},
             3, U'\u07FF'
                 )
             )},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xF0, 0x80, 0x80, 0x80}),
             3, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             4, U'\0'
                 )
             )},
            // surrogates / out of range at offset 0
            {mk_unicode<char8_t>({0xED, 0xA0, 0x80}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xA0),
                          static_cast<char8_t>(0x80),
                          static_cast<char8_t>(0x00)},
             3, U'\xD800'
                     )
             )},
            {mk_unicode<char8_t>({0xED, 0xBF, 0xBF}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xBF),
                          static_cast<char8_t>(0xBF),
                          static_cast<char8_t>(0x00)},
             3, U'\xDFFF'
                     )
             )},
            {mk_unicode<char8_t>({0xF4, 0x90, 0x80, 0x80}),
             0, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xF4),
                          static_cast<char8_t>(0x90),
                          static_cast<char8_t>(0x80),
                          static_cast<char8_t>(0x80)},
             4, U'\x110000'
                     )
             )},
            // surrogates / out of range mid-string
            {u8string(u8"a") + mk_unicode<char8_t>({0xED, 0xA0, 0x80}),
             1, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xA0),
                          static_cast<char8_t>(0x80),
                          static_cast<char8_t>(0x00)},
             3, U'\xD800'
                     )
             )},
            {u8string(u8"ab") + mk_unicode<char8_t>({0xED, 0xBF, 0xBF}),
             2, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xBF),
                          static_cast<char8_t>(0xBF),
                          static_cast<char8_t>(0x00)},
             3, U'\xDFFF'
                     )
             )},
            {u8string(u8"abc") + mk_unicode<char8_t>({0xF4, 0x90, 0x80, 0x80}),
             3, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xF4),
                          static_cast<char8_t>(0x90),
                          static_cast<char8_t>(0x80),
                          static_cast<char8_t>(0x80)},
             4, U'\x110000'
                     )
             )},
            // advance into the middle of a 2-byte sequence
            {u8"£",
             1, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\xA3'
                 )
             )},

            // advance into the middle of a 3-byte sequence
            {u8"→",
             1, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\x86'
                 )
             )},
            {u8"→",
             2, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\x92'
                 )
             )},

            // advance into the middle of a 4-byte sequence
            {u8"𝄞",
             1, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\x9D'
                 )
             )},
            {u8"𝄞",
             2, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\x84'
                 )
             )},
            {u8"𝄞",
             3, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\x9E'
                 )
             )},

            // advance past ASCII into middle of multibyte
            {u8"a£",
             2, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\xA3'
                 )
             )},
            {u8"a→",
             2, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\x86'
                 )
             )},
            {u8"a→",
             3, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\x92'
                 )
             )},
            {u8"a𝄞",
             2, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\x9D'
                 )
             )},

            // advance past a valid multibyte into middle of another
            {u8"£→",
             3, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\x86'
                 )
             )},
            {u8"→𝄞",
             4, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\x9D'
                 )
             )},
            {u8"→𝄞",
             5, next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_leading_byte('\x84'
                 )
             )},
    };

    initializer_list<tuple<u16string, size_t, next_char32_error>> invalid_u16_errors = {
        // high surrogate then end of stream at offset 0
        {mk_unicode<char16_t>({0xD800}),
         0, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 high_surrogate_then_end_of_stream(u'\xD800', same_as<CharT, wchar_t>)
         )},
        {mk_unicode<char16_t>({0xDBFF}),
         0, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 high_surrogate_then_end_of_stream(u'\xDBFF', same_as<CharT, wchar_t>)
         )},

        // high surrogate then end of stream mid-string
        {u16string(u"a") + mk_unicode<char16_t>({0xD800}),
         1, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 high_surrogate_then_end_of_stream(u'\xD800', same_as<CharT, wchar_t>)
         )},
        {u16string(u"ab") + mk_unicode<char16_t>({0xDBFF}),
         2, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 high_surrogate_then_end_of_stream(u'\xDBFF', same_as<CharT, wchar_t>)
         )},
        {u16string(u"abc") + mk_unicode<char16_t>({0xD800}),
         3, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 high_surrogate_then_end_of_stream(u'\xD800', same_as<CharT, wchar_t>)
         )},

        // high surrogate not followed by low surrogate at offset 0
        {mk_unicode<char16_t>({0xD800, 0xD801}),
         0, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xD800', u'\xD801', same_as<CharT, wchar_t>)
         )},
        {mk_unicode<char16_t>({0xDBFF, 0x4E00}),
         0, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xDBFF', u'\x4E00', same_as<CharT, wchar_t>)
         )},
        {mk_unicode<char16_t>({0xDBFF}, u"hello"),
         0, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xDBFF', u'h', same_as<CharT, wchar_t>)
         )},

        // high surrogate not followed by low surrogate mid-string
        {u16string(u"a") + mk_unicode<char16_t>({0xD800, 0xD801}),
         1, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xD800', u'\xD801', same_as<CharT, wchar_t>)
         )},
        {u16string(u"ab") + mk_unicode<char16_t>({0xDBFF, 0x4E00}),
         2, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xDBFF', u'\x4E00', same_as<CharT, wchar_t>)
         )},
        {u16string(u"abc") + mk_unicode<char16_t>({0xDBFF}, u"hello"),
         3, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 high_surrogate_not_followed_by_low_surrogate(u'\xDBFF', u'h', same_as<CharT, wchar_t>)
         )},

        // unexpected low surrogate at offset 0
        {mk_unicode<char16_t>({0xDC00}),
         0, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDC00', same_as<CharT, wchar_t>)
         )},
        {mk_unicode<char16_t>({0xDFFF}),
         0, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDFFF', same_as<CharT, wchar_t>)
         )},
        {mk_unicode<char16_t>({0xDC00, 0xDC01}),
         0, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDC00', same_as<CharT, wchar_t>)
         )},

        // unexpected low surrogate mid-string
        {u16string(u"a") + mk_unicode<char16_t>({0xDC00}),
         1, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDC00', same_as<CharT, wchar_t>)
         )},
        {u16string(u"ab") + mk_unicode<char16_t>({0xDFFF}),
         2, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDFFF', same_as<CharT, wchar_t>)
         )},
        {u16string(u"abc") + mk_unicode<char16_t>({0xDC00}),
         3, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDC00', same_as<CharT, wchar_t>)
         )},
        // advance into the low surrogate of a pair
        {u"𝄞",
         1, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDD1E', same_as<CharT, wchar_t>)
         )},
        {u"😀",
         1, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDE00', same_as<CharT, wchar_t>)
         )},

        // advance past BMP character into low surrogate
        {u"a𝄞",
         2, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDD1E', same_as<CharT, wchar_t>)
         )},
        {u"→𝄞",
         2, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDD1E', same_as<CharT, wchar_t>)
         )},

        // advance past one surrogate pair into low surrogate of
        // next
        {u"𝄞😀",
         3, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 unexpected_low_surrogate(u'\xDE00', same_as<CharT, wchar_t>)
         )},
    };
    initializer_list<tuple<u32string, size_t, next_char32_error>> invalid_u32_errors = {
        // at offset 0
        {mk_unicode<char32_t>({0xD800}),
         0, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xD800',     same_as<CharT, wchar_t>)
         )},
        {mk_unicode<char32_t>({0xDFFF}),
         0, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xDFFF',     same_as<CharT, wchar_t>)
         )},
        {mk_unicode<char32_t>({0x11'0000}),
         0, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\x110000',   same_as<CharT, wchar_t>)
         )},
        {mk_unicode<char32_t>({0xFFFF'FFFF}),
         0, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xFFFFFFFF', same_as<CharT, wchar_t>)
         )},

        // mid-string
        {u32string(U"a") + mk_unicode<char32_t>({0xD800}),
         1, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xD800',     same_as<CharT, wchar_t>)
         )},
        {u32string(U"ab") + mk_unicode<char32_t>({0xDFFF}),
         2, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xDFFF',     same_as<CharT, wchar_t>)
         )},
        {u32string(U"abc") + mk_unicode<char32_t>({0x11'0000}),
         3, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\x110000',   same_as<CharT, wchar_t>)
         )},
        {u32string(U"a") + mk_unicode<char32_t>({0xFFFF'FFFF}),
         1, next_char32_error_factory::mk_forward_error(
             forward_scan_unicode_error_factory::
                 invalid_utf32_code_point(U'\xFFFFFFFF', same_as<CharT, wchar_t>)
         )},
    };
    vector<tuple<basic_string<CharT>, size_t, next_char32_error>> rv;
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
template <typename CharT>
inline void
    run_invalid_prev_char32_t_checks(
        const std::vector<std::tuple<
            std::basic_string<CharT>,
            size_t,
            unicode_bridge::prev_char32_error>>& invalid_strs_arg
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
            auto current_iterator  = std::end(input_argument);
            auto end_iterator      = std::begin(input_argument);
            current_iterator      -= idx;

            auto res = prev_char32(current_iterator, end_iterator);
            if (res.has_value())
            {
                auto res356 = prev_char32(current_iterator, end_iterator);
                FAIL(
                    "Unexpected success. Values returned = `"
                    << char_printer(res.value().first) << "', "
                    << res.value().second
                );
            }
            else
            {
                equal_prev_char32_error(res.error(), expected_output);
                auto res55 = prev_char32(current_iterator, end_iterator);
            }

            auto res2 = prev_char32_no_error(current_iterator, end_iterator);
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
                auto res2a = prev_char32_with_exception(
                    current_iterator, end_iterator
                );
                FAIL("Unexpected successful call of "
                     "prev_char32_t_with_exception.");
            }
            catch (const unicode_bridge_exception<prev_char32_error>& _exception
            )
            {
                equal_prev_char32_error(_exception.error(), expected_output);
            }
            catch (...)
            {
                string msg = "Unexpected, unknown exception";
                FAIL(msg);
            }

            auto new_iterator = current_iterator;
            auto res3         = prev_char32_and_decrement_iterator(
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
                equal_prev_char32_error(res.error(), expected_output);
                CHECK(std::distance(new_iterator, current_iterator) == 0);
            }

            new_iterator = current_iterator;
            auto res4    = prev_char32_and_decrement_iterator_no_error(
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
                auto res4a = prev_char32_and_decrement_iterator_with_exception(
                    current_iterator, end_iterator
                );
                FAIL("Unexpected successful call of "
                     "prev_char32_t_with_exception.");
            }
            catch (const unicode_bridge_exception<prev_char32_error>& _exception
            )
            {
                equal_prev_char32_error(_exception.error(), expected_output);
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
    unicode_bridge::prev_char32_error>>
    get_invalid_prev_char32_t_data()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge::internal;
    std::array<std::u8string, 4> arr;
    initializer_list<tuple<u8string, size_t, prev_char32_error>>
        invalid_u8_errors = {
            {mk_unicode<char8_t>({0x80, 0xA9, 0x80, 0xBF}),
             0, prev_char32_error_factory::no_valid_leading_byte_found(
                 {static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0xA9),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0xBF)}
             )},
            {u8string(u8"hello world"
             ) + mk_unicode<char8_t>({0x80, 0xA9, 0x80, 0xBF}, u8"hello world"),
             11, prev_char32_error_factory::no_valid_leading_byte_found(
                 {static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0xA9),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0xBF)}
             )},
            {mk_unicode<char8_t>({0x41, 0x80}),
             0, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0x41),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             1
             )},
            {u8string(u8"hello world")
                 + mk_unicode<char8_t>({0x41, 0x80}, u8"hello world"),
             11, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0x41),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             1
             )},
            {mk_unicode<char8_t>({0x41, 0x80, 0x80}),
             0, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0x41),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             2
             )},
            {u8string(u8"hello world")
                 + mk_unicode<char8_t>({0x41, 0x80, 0x80}, u8"hello world"),
             11, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0x41),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             2
             )},
            {mk_unicode<char8_t>({0x41, 0x80, 0x80, 0x80}),
             0, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0x41),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             3
             )},
            {u8string(u8"hello world"
             ) + mk_unicode<char8_t>({0x41, 0x80, 0x80, 0x80}, u8"hello world"),
             11, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0x41),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             3
             )},
            {mk_unicode<char8_t>({0xC3}),
             0, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xC3),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             0
             )},
            {u8string(u8"hello world")
                 + mk_unicode<char8_t>({0xC3}, u8"hello world"),
             11, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xC3),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             0
             )},
            {mk_unicode<char8_t>({0xC3, 0x80, 0x80}),
             0, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xC3),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             2
             )},
            {u8string(u8"hello world")
                 + mk_unicode<char8_t>({0xC3, 0x80, 0x80}, u8"hello world"),
             11, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xC3),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             2
             )},
            {mk_unicode<char8_t>({0xC3, 0x80, 0x80, 0x80}),
             0, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xC3),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             3
             )},
            {u8string(u8"hello world"
             ) + mk_unicode<char8_t>({0xC3, 0x80, 0x80, 0x80}, u8"hello world"),
             11, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xC3),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             3
             )},
            {mk_unicode<char8_t>({0xE0}),
             0, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             0
             )},
            {u8string(u8"hello world")
                 + mk_unicode<char8_t>({0xE0}, u8"hello world"),
             11, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             0
             )},
            {mk_unicode<char8_t>({0xE0, 0x80}),
             0, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             1
             )},
            {u8string(u8"hello world")
                 + mk_unicode<char8_t>({0xE0, 0x80}, u8"hello world"),
             11, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             1
             )},
            {mk_unicode<char8_t>({0xE0, 0x80, 0x80, 0x80}),
             0, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             3
             )},
            {u8string(u8"hello world"
             ) + mk_unicode<char8_t>({0xE0, 0x80, 0x80, 0x80}, u8"hello world"),
             11, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             3
             )},
            {mk_unicode<char8_t>({0xF0}),
             0, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             0
             )},
            {u8string(u8"hello world")
                 + mk_unicode<char8_t>({0xF0}, u8"hello world"),
             11, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             0
             )},
            {mk_unicode<char8_t>({0xF0, 0x80}),
             0, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             1
             )},
            {u8string(u8"hello world")
                 + mk_unicode<char8_t>({0xF0, 0x80}, u8"hello world"),
             11, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             1
             )},
            {mk_unicode<char8_t>({0xF0, 0x80, 0x80}),
             0, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             2
             )},
            {u8string(u8"hello world")
                 + mk_unicode<char8_t>({0xF0, 0x80, 0x80}, u8"hello world"),
             11, prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             2
             )},
            {mk_unicode<char8_t>({0xFF, 0x80, 0x80}),
             0, prev_char32_error_factory::invalid_utf8_byte(
                 {static_cast<char8_t>(0xFF),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             2
             )},
            {u8string(u8"hello world")
                 + mk_unicode<char8_t>({0xFF, 0x80, 0x80}, u8"hello world"),
             11, prev_char32_error_factory::invalid_utf8_byte(
                 {static_cast<char8_t>(0xFF),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             2
             )},
            {mk_unicode<char8_t>({0xFF}),
             0, prev_char32_error_factory::invalid_utf8_byte(
                 {static_cast<char8_t>(0xFF),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             0
             )},
            {u8string(u8"hello world")
                 + mk_unicode<char8_t>({0xFF}, u8"hello world"),
             11, prev_char32_error_factory::invalid_utf8_byte(
                 {static_cast<char8_t>(0xFF),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             0
             )},
            {mk_unicode<char8_t>({0xED, 0xA0, 0x80}),
             0, prev_char32_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xA0),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             3, U'\xD800'
                 )},
            {u8string(u8"hello world")
                 + mk_unicode<char8_t>({0xED, 0xA0, 0x80}, u8"hello world"),
             11, prev_char32_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xA0),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             3, U'\xD800'
                 )},
            {mk_unicode<char8_t>({0x80}),
             0, prev_char32_error_factory::
                 iterator_begin_reached_before_leading_byte(
                     {static_cast<char8_t>(0x80), u8'\0', u8'\0', u8'\0'}, 1
                 )},
            {mk_unicode<char8_t>({0x80}, u8"hello world"),
             11, prev_char32_error_factory::
                 iterator_begin_reached_before_leading_byte(
                     {static_cast<char8_t>(0x80), u8'\0', u8'\0', u8'\0'}, 1
                 )},
            {mk_unicode<char8_t>({0x80, 0xBF}),
             0, prev_char32_error_factory::
                 iterator_begin_reached_before_leading_byte(
                     {static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0xBF),
                      u8'\0',
                      u8'\0'},
             2
                 )},
            {mk_unicode<char8_t>({0x80, 0xBF}, u8"hello world"),
             11, prev_char32_error_factory::
                 iterator_begin_reached_before_leading_byte(
                     {static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0xBF),
                      u8'\0',
                      u8'\0'},
             2
                 )},
            {mk_unicode<char8_t>({0x80, 0xA0, 0xBF}),
             0, prev_char32_error_factory::
                 iterator_begin_reached_before_leading_byte(
                     {static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0xA0),
                      static_cast<char8_t>(0xBF),
                      u8'\0'},
             3
                 )},
            {mk_unicode<char8_t>({0x80, 0xA0, 0xBF}, u8"hello world"),
             11, prev_char32_error_factory::
                 iterator_begin_reached_before_leading_byte(
                     {static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0xA0),
                      static_cast<char8_t>(0xBF),
                      u8'\0'},
             3
                 )},
    };
    initializer_list<tuple<u16string, size_t, prev_char32_error>> invalid_u16_errors = {
        {mk_unicode<char16_t>({0xDC00}),
         0, prev_char32_error_factory::
             low_surrogate_then_start_of_stream(static_cast<char16_t>(0xDC00), same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xDC00}, u"hello world"),
         11, prev_char32_error_factory::
             low_surrogate_then_start_of_stream(static_cast<char16_t>(0xDC00), same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0x0041, 0xDC00}),
         0, prev_char32_error_factory::
             low_surrogate_not_preceded_by_high_surrogate(static_cast<char16_t>(0x0041), static_cast<char16_t>(0xDC00), same_as<CharT, wchar_t>)
        },
        {u16string(u"hello world")
             + mk_unicode<char16_t>({0x0041, 0xDC00}, u"hello world"),
         11, prev_char32_error_factory::
             low_surrogate_not_preceded_by_high_surrogate(static_cast<char16_t>(0x0041), static_cast<char16_t>(0xDC00), same_as<CharT, wchar_t>)
        },
        {mk_unicode<char16_t>({0xD800}),
         0, prev_char32_error_factory::
             unexpected_high_surrogate(static_cast<char16_t>(0xD800), same_as<CharT, wchar_t>)
        },
        {u16string(u"hello world")
             + mk_unicode<char16_t>({0xD800}, u"hello world"),
         11, prev_char32_error_factory::
             unexpected_high_surrogate(static_cast<char16_t>(0xD800), same_as<CharT, wchar_t>)
        },
    };
    initializer_list<tuple<u32string, size_t, prev_char32_error>>
        invalid_u32_errors = {
            // at offset 0
            {mk_unicode<char32_t>({0xD800}),
             0, prev_char32_error_factory::
                 invalid_utf32_code_point(U'\xD800', same_as<CharT, wchar_t>)},
            {mk_unicode<char32_t>({0xDFFF}),
             0, prev_char32_error_factory::
                 invalid_utf32_code_point(U'\xDFFF', same_as<CharT, wchar_t>)},
            {mk_unicode<char32_t>({0x11'0000}),
             0, prev_char32_error_factory::
                 invalid_utf32_code_point(U'\x110000', same_as<CharT, wchar_t>)
            },
            {mk_unicode<char32_t>({0xFFFF'FFFF}),
             0, prev_char32_error_factory::
                 invalid_utf32_code_point(U'\xFFFFFFFF', same_as<CharT, wchar_t>)
            },

            // mid-string
            {u32string(U"a") + mk_unicode<char32_t>({0xD800}),
             0, prev_char32_error_factory::
                 invalid_utf32_code_point(U'\xD800', same_as<CharT, wchar_t>)},
            {u32string(U"ab") + mk_unicode<char32_t>({0xDFFF}),
             0, prev_char32_error_factory::
                 invalid_utf32_code_point(U'\xDFFF', same_as<CharT, wchar_t>)},
            {u32string(U"abc") + mk_unicode<char32_t>({0x11'0000}),
             0, prev_char32_error_factory::
                 invalid_utf32_code_point(U'\x110000', same_as<CharT, wchar_t>)
            },
            {u32string(U"a") + mk_unicode<char32_t>({0xFFFF'FFFF}),
             0, prev_char32_error_factory::
                 invalid_utf32_code_point(U'\xFFFFFFFF', same_as<CharT, wchar_t>)
            },

            // End of string.
            {u32string(U"a") + mk_unicode<char32_t>({0xD800}, U"a"),
             1, prev_char32_error_factory::
                 invalid_utf32_code_point(U'\xD800', same_as<CharT, wchar_t>)},
            {u32string(U"ab") + mk_unicode<char32_t>({0xDFFF}, U"ab"),
             2, prev_char32_error_factory::
                 invalid_utf32_code_point(U'\xDFFF', same_as<CharT, wchar_t>)},
            {u32string(U"abc") + mk_unicode<char32_t>({0x11'0000}, U"abcd"),
             4, prev_char32_error_factory::
                 invalid_utf32_code_point(U'\x110000', same_as<CharT, wchar_t>)
            },
            {u32string(U"abcdef")
                 + mk_unicode<char32_t>({0xFFFF'FFFF}, U"abcdefghij"),
             10, prev_char32_error_factory::
                 invalid_utf32_code_point(U'\xFFFFFFFF', same_as<CharT, wchar_t>)
            },
    };
    vector<tuple<basic_string<CharT>, size_t, prev_char32_error>> rv;
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
    "prev_char32_t and prev_char32_t_and_increment_iterator are correct "
    "for "
    "invalid char8_t arguments",
    "[prev_char32_t,prev_char32_t_and_increment_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge;
    using namespace unicode_bridge_test_cases;
    auto error_results_u8 = get_invalid_prev_char32_t_data<char8_t>();
    run_invalid_prev_char32_t_checks(error_results_u8);
}

TEST_CASE(
    "prev_char32_t and prev_char32_t_and_increment_iterator are correct "
    "for "
    "invalid char16_t arguments",
    "[prev_char32_t,prev_char32_t_and_increment_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge;
    using namespace unicode_bridge_test_cases;
    auto error_results_u16 = get_invalid_prev_char32_t_data<char16_t>();
    run_invalid_prev_char32_t_checks(error_results_u16);
}

TEST_CASE(
    "prev_char32_t and prev_char32_t_and_increment_iterator are correct "
    "for "
    "invalid char32_t arguments",
    "[prev_char32_t,prev_char32_t_and_increment_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge;
    using namespace unicode_bridge_test_cases;
    auto error_results_u32 = get_invalid_prev_char32_t_data<char32_t>();
    run_invalid_prev_char32_t_checks(error_results_u32);
}

TEST_CASE(
    "prev_char32_t and prev_char32_t_and_increment_iterator are correct "
    "for "
    "invalid wchar_t arguments",
    "[prev_char32_t,prev_char32_t_and_increment_iterator]"
)
{
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge;
    using namespace unicode_bridge_test_cases;
    auto error_results_ws = get_invalid_prev_char32_t_data<wchar_t>();
    run_invalid_prev_char32_t_checks(error_results_ws);
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
            std::u8string result1
                = to_formatted_unicode_string<char8_t>(input_arg);
            INFO("Actual output " << convert_unicode_to_string(result1));
            if (result1 != output_arg)
            {
                int           x = 4;
                std::u8string result4
                    = to_formatted_unicode_string<char8_t>(input_arg);
            }
            REQUIRE(result1 == output_arg);

            auto result2 = to_formatted_unicode_string<char16_t>(input_arg);
            if (result2 != u16_output)
            {
                std::u8string result4
                    = to_formatted_unicode_string<char8_t>(input_arg);
            }
            REQUIRE(result2 == u16_output);
            auto result3 = to_formatted_unicode_string<char32_t>(input_arg);
            REQUIRE(result3 == u32_output);
            auto result4 = to_formatted_unicode_string<wchar_t>(input_arg);
            REQUIRE(result4 == ws_output);
            auto test_from_formatted = [&]<typename U>(U test_arg)
            {
                auto result_var = from_formatted_unicode_string<CharT>(test_arg);
                if (result_var.has_value())
                {
                    if (not (str == result_var.value()))
                    {
                        auto result_var = from_formatted_unicode_string<CharT>(test_arg);
                    }
                    REQUIRE(str == result_var.value());
                }
                else
                {
                    auto result_var = from_formatted_unicode_string<CharT>(test_arg);
                    FAIL();
                }
            };
            test_from_formatted(output_arg);
            test_from_formatted(u16_output);
            test_from_formatted(u32_output);
            test_from_formatted(ws_output);
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
        {u8"\u0001",                                                         u8"\\x01"           },
        {u8"\u0002",                                                         u8"\\x02"           },
        {u8"\u0010",                                                         u8"\\x10"           },
        {u8"\u001F",                                                         u8"\\x1F"           },

        // 0x7F-0x9F range
        {u8"\u007F",                                                         u8"\\x7F"           },
        {u8"\u0080",                                                         u8"\\u0080"           }, // U+0080
        {u8"\u009F",                                                         u8"\\u009F"           }, // U+009F

        // special unicode characters
        {u8"\u00A0",                                                         u8"\\u00A0"           }, // non-breaking space
        {u8"\u1680",                                                         u8"\\u1680"         }, // ogham space mark
        {u8"\u2006",                                                         u8"\\u2006"         }, // six-per-em space
        {u8"\u200B",                                                         u8"\\u200B"         }, // zero width space
        {u8"\u2028",                                                         u8"\\u2028"         }, // line separator
        {u8"\u2029",                                                         u8"\\u2029"         }, // paragraph separator
        {u8"\u3000",                                                         u8"\\u3000"         }, // ideographic space
        {u8"\uFEFF",                                                         u8"\\uFEFF"         }, // byte order mark
        {u8"\uFFFD",                                                         u8"\\uFFFD"         }, // replacement character

        // embedded in normal strings
        {u8"a\u00A0b",                                                       u8"a\\u00A0b"         }, // non-breaking space
        {u8"a\u2006b",                                                       u8"a\\u2006b"       }, // six-per-em space
        {u8"£\uFEFF→",                                                       u8"£\\uFEFF→"       },

        // multiple special characters
        {u8"\u00A0\u00A0",                                                   u8"\\u00A0\\u00A0"      },
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
        {u8"a" + mk_unicode<char8_t>({0xC2, 0xA0}) + u8"b",                  u8"a\\u00A0b"         },
        {u8"→" + mk_unicode<char8_t>({0xEF, 0xBB, 0xBF}) + u8"£",            u8"→\\uFEFF£"
        },

        // normal + malformed
        {u8string(u8"hello") + mk_unicode<char8_t>({0xFF}),                  u8"hello\\xFF"      },
        {u8string(u8"£→") + mk_unicode<char8_t>({0xC2}),                     u8"£→\\xC2"         },

        // escape + hex codepoint
        {u8"\t\u00A0",                                                       u8"\\t\\u00A0"        },
        {u8"\u00A0\n",                                                       u8"\\u00A0\\n"        },

        // escape + malformed
        {u8string(u8"\t") + mk_unicode<char8_t>({0xFF}),                     u8"\\t\\xFF"        },
        {mk_unicode<char8_t>({0xFF}) + u8string(u8"\n"),                     u8"\\xFF\\n"        },

        // hex codepoint + malformed
        {u8string(u8"\u00A0") + mk_unicode<char8_t>({0xFF}),                 u8"\\u00A0\\xFF"      },
        {mk_unicode<char8_t>({0xFF, 0xC2, 0xA0}),                            u8"\\xFF\\u00A0"      },

        // all four categories
        {u8string(u8"a\t\u00A0") + mk_unicode<char8_t>({0xFF}),
         u8"a\\t\\u00A0\\xFF"                                                                      },
        {mk_unicode<char8_t>({0xC2}) + u8string(u8"\n£\uFEFF"),
         u8"\\xC2\\n£\\uFEFF"                                                                    },
        {u8string(u8"hello\n£") + u8"\u00A0"
             + mk_unicode<char8_t>({0xED, 0xA0, 0x80}) + u8"world",
         u8"hello\\n£\\u00A0\\xED\\xA0\\x80world"                                                  },
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
        {u"\u0001", u8"\\x01"},
        {u"\u0002", u8"\\x02"},
        {u"\u0010", u8"\\x10"},
        {u"\u001F", u8"\\x1F"},

        // 0x7F-0x9F range
        {u"\u007F", u8"\\x7F"},
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
        {u16string(u"\u00A0") + mk_unicode<char16_t>({0xD800}), u8"\\u00A0\\uD800"
        },
        {mk_unicode<char16_t>({0xDC00}) + u16string(u"\u00A0"), u8"\\uDC00\\u00A0"
        },

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
        {U"\u0001",                                                      u8"\\x01"         },
        {U"\u0002",                                                      u8"\\x02"         },
        {U"\u0010",                                                      u8"\\x10"         },
        {U"\u001F",                                                      u8"\\x1F"         },

        // 0x7F-0x9F range
        {U"\u007F",                                                      u8"\\x7F"         },
        {U"\u0080",                                                      u8"\\u0080"         },
        {U"\u009F",                                                      u8"\\u009F"         },

        // special unicode characters
        {U"\u00A0",                                                      u8"\\u00A0"         }, // non-breaking space
        {U"\u1680",                                                      u8"\\u1680"       }, // ogham space mark
        {U"\u2006",                                                      u8"\\u2006"       }, // six-per-em space
        {U"\u200B",                                                      u8"\\u200B"       }, // zero width space
        {U"\u2028",                                                      u8"\\u2028"       }, // line separator
        {U"\u2029",                                                      u8"\\u2029"       }, // paragraph separator
        {U"\u3000",                                                      u8"\\u3000"       }, // ideographic space
        {U"\uFEFF",                                                      u8"\\uFEFF"       }, // byte order mark
        {U"\uFFFD",                                                      u8"\\uFFFD"       }, // replacement character

        // embedded in normal strings
        {U"a\u00A0b",                                                    u8"a\\u00A0b"       }, // non-breaking space
        {U"a\u2006b",                                                    u8"a\\u2006b"     }, // six-per-em space
        {U"£\uFEFF→",                                                    u8"£\\uFEFF→"     },

        // multiple special characters
        {U"\u00A0\u00A0",                                                u8"\\u00A0\\u00A0"    },
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
        {U"a\u00A0b",                                                    u8"a\\u00A0b"       },
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
        {U"\t\u00A0",                                                    u8"\\t\\u00A0"      },
        {U"\u00A0\n",                                                    u8"\\u00A0\\n"      },

        // escape + surrogate
        {u32string(U"\t") + mk_unicode<char32_t>({0xD800}),              u8"\\t\\uD800"    },
        {mk_unicode<char32_t>({0xDC00}) + u32string(U"\n"),              u8"\\uDC00\\n"    },

        // escape + out of range
        {u32string(U"\t") + mk_unicode<char32_t>({0x11'0000}),
         u8"\\t\\U00110000"                                                                },
        {mk_unicode<char32_t>({0x11'0000}) + u32string(U"\n"),
         u8"\\U00110000\\n"                                                                },

        // hex codepoint + surrogate
        {u32string(U"\u00A0") + mk_unicode<char32_t>({0xD800}),          u8"\\u00A0\\uD800"
        },
        {mk_unicode<char32_t>({0xDC00}) + u32string(U"\u00A0"),          u8"\\uDC00\\u00A0"
        },

        // hex codepoint + out of range
        {u32string(U"\u00A0") + mk_unicode<char32_t>({0x11'0000}),
         u8"\\u00A0\\U00110000"                                                              },
        {mk_unicode<char32_t>({0x11'0000}) + u32string(U"\u00A0"),
         u8"\\U00110000\\u00A0"                                                              },

        // all categories
        {u32string(U"a\t") + U"\u00A0" + mk_unicode<char32_t>({0xD800}),
         u8"a\\t\\u00A0\\uD800"                                                              },
        {mk_unicode<char32_t>({0xDC00}) + u32string(U"\n£\uFEFF"),
         u8"\\uDC00\\n£\\uFEFF"                                                            },
        {u32string(U"hello\n£") + U"\u00A0" + mk_unicode<char32_t>({0xD800})
             + U"world",
         u8"hello\\n£\\u00A0\\uD800world"                                                    },
        {u32string(U"a\t") + U"\u00A0" + mk_unicode<char32_t>({0xD800})
             + mk_unicode<char32_t>({0x11'0000}) + U"b",
         u8"a\\t\\u00A0\\uD800\\U00110000b"                                                  },
    };
    initializer_list<pair<string, u8string>> input_output_pairs_ascii = {
        {"",                                                    u8""                  },
        {"hello",                                               u8"hello"             },
        {"abc",                                                 u8"abc"               },
        {"Hello World",                                         u8"Hello World"       },
        {"0123456789",                                          u8"0123456789"        },
        {"!@#$%^&*()",                                          u8"!@#$%^&*()"        },
        // single characters
        {make_str({0x0}),                                       u8"\\0"               },
        {"\a",                                                  u8"\\a"               },
        {"\b",                                                  u8"\\b"               },
        {"\t",                                                  u8"\\t"               },
        {"\n",                                                  u8"\\n"               },
        {"\v",                                                  u8"\\v"               },
        {"\f",                                                  u8"\\f"               },
        {"\r",                                                  u8"\\r"               },
        {"\"",                                                  u8"\\\""              },
        {"\'",                                                  u8"\\'"               },
        {"\\",                                                  u8"\\\\"              },

        // embedded in normal strings
        {"a\tb",                                                u8"a\\tb"             },
        {"a\nb",                                                u8"a\\nb"             },
        {"a\rb",                                                u8"a\\rb"             },
        {"a\\b",                                                u8"a\\\\b"            },
        {"a\"b",                                                u8"a\\\"b"            },
        {"a\'b",                                                u8"a\\'b"             },

        // multiple escape sequences
        {"\t\n",                                                u8"\\t\\n"            },
        {"\r\n",                                                u8"\\r\\n"            },
        {"a\t\nb",                                              u8"a\\t\\nb"          },

        // escape sequences with normal ascii
        {"hello\tworld",                                        u8"hello\\tworld"     },
        {"line1\nline2",                                        u8"line1\\nline2"     },
        // control characters (< 0x20, excluding escape sequences)
        {"\x01",                                                u8"\\x01"             },
        {"\x02",                                                u8"\\x02"             },
        {"\x10",                                                u8"\\x10"             },
        {"\x1F",                                                u8"\\x1F"             },

        // 0x7F
        {"\x7F",                                                u8"\\x7F"             },

        // embedded in normal strings
        {"a\x01"
         "b",                                          u8"a\\x01b"           },
        {"a\x1F"
         "b",                                          u8"a\\x1Fb"           },
        {"a\x7F"
         "b",                                          u8"a\\x7Fb"           },

        // multiple control characters
        {"\x01\x02",                                            u8"\\x01\\x02"        },
        {"\x1F\x7F",                                            u8"\\x1F\\x7F"        },
        // single invalid bytes
        {make_str({'\x80'}),                                    u8"\\x80"             },
        {make_str({'\xFF'}),                                    u8"\\xFF"             },
        {make_str({'\xFE'}),                                    u8"\\xFE"             },
        {make_str({'\xA0'}),                                    u8"\\xA0"             },

        // invalid bytes mid-string
        {string("a") + make_str({'\x80'}),                      u8"a\\x80"            },
        {string("ab") + make_str({'\xFF'}),                     u8"ab\\xFF"           },
        {string("abc") + make_str({'\x80'}),                    u8"abc\\x80"          },
        {make_str({'\x80'}) + string("hello"),                  u8"\\x80hello"        },
        {make_str({'\xFF'}) + string("hello"),                  u8"\\xFFhello"        },

        // consecutive invalid bytes
        {make_str({'\x80', '\xFF'}),                            u8"\\x80\\xFF"        },
        {make_str({'\xFe', '\xFF'}),                            u8"\\xFE\\xFF"        },

        // invalid bytes surrounding valid string
        {make_str({'\x80'}) + string("a") + make_str({'\xFF'}), u8"\\x80a\\xFF"
        },
        // normal + escape
        {"hello\tworld",                                        u8"hello\\tworld"     },
        {"line1\nline2",                                        u8"line1\\nline2"     },

        // normal + control character
        {"a\x01"
         "b",                                          u8"a\\x01b"           },
        {"a\x7F"
         "b",                                          u8"a\\x7Fb"           },

        // normal + invalid byte
        {string("hello") + make_str({'\xFF'}),                  u8"hello\\xFF"        },
        {string("abc") + make_str({'\x80'}),                    u8"abc\\x80"          },

        // escape + control character
        {"\t\x01",                                              u8"\\t\\x01"          },
        {"\x01\n",                                              u8"\\x01\\n"          },

        // escape + invalid byte
        {string("\t") + make_str({'\xFF'}),                     u8"\\t\\xFF"          },
        {make_str({'\x80'}) + string("\n"),                     u8"\\x80\\n"          },

        // control character + invalid byte
        {string("\x01") + make_str({'\xFF'}),                   u8"\\x01\\xFF"        },
        {make_str({'\xFF'}) + string("\x01"),                   u8"\\xFF\\x01"        },

        // all four categories
        {string("a\t\x01") + make_str({'\xFF'}),                u8"a\\t\\x01\\xFF"    },
        {make_str({'\x80'}) + string("\nhello\x01"),            u8"\\x80\\nhello\\x01"},
        {string("hello\t") + string("\x01") + make_str({'\x80'})
             + string("world"),
         u8"hello\\t\\x01\\x80world"                                                  },
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
template <typename T, typename U, typename V>
inline void
    test_errors(
        const std::vector<std::tuple<
            T,
            std::basic_string<U>,
            std::optional<std::basic_string<V>>>>& input_output_pairs_arg
    )
{
    using namespace std;
    for (auto&& [error, expected_msg, input_msg] : input_output_pairs_arg)
    {
        const optional<basic_string_view<V>> vi = input_msg;
        u8string                             result
            = vi.has_value() ? error.message(vi.value()) : error.message();
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

namespace unicode_bridge_test_cases
{
template <typename CharT>
std::vector<std::tuple<
    unicode_bridge::unicode_conversion_error,
    std::u8string,
    std::optional<std::basic_string<CharT>>>>
    get_unicode_conversion_error_message()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge::internal;
    using namespace unicode_bridge_testing;
    vector<tuple<unicode_conversion_error, u8string, optional<u8string>>>
        u8_input_output_pairs = {
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_leading_byte('\xF8'
                 ), 0
             ), u8"The 11th code unit (0xF8) in the UTF-8 input "
             u8"passed to the function was found to be an invalid leading "
             u8"byte. A valid leading "
             u8"byte "
             u8"must "
             u8"be inclusively within one of the following ranges: 0x00 to "
             u8"0x7F "
             u8"(single-byte sequence), 0xC0 to 0xDF (two-byte sequence), 0xE0 "
             u8"to 0xEF (three-byte sequence), or 0xF0 to 0xF7 (four-byte "
             u8"sequence). As 0xF8 falls outside all of these ranges, it "
             u8"cannot "
             u8"begin a sequence representing a valid Unicode scalar "
             u8"value, and the function was terminated.", std::nullopt},
            {unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xC2), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_2_found_1
                 ), 0
             ), u8"The 1st code unit (0xC2) in the UTF-8 input (\"A very lo...\") "
             u8"passed to "
             u8"the function was found to be a valid "
             u8"leading byte, indicating the "
             u8"start of a two-byte "
             u8"sequence. However, the input "
             u8"ended after the first code unit "
             u8"— one continuation byte was "
             u8"expected but was not present. As "
             u8"the sequence is incomplete, it cannot represent a valid "
             u8"Unicode "
             u8"scalar value, and the function was terminated.", u8"A very long string that will be cut off at the end"},
            {unicode_conversion_error(
                 17, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xE2), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_3_found_1
                 ), 0
             ), u8"The 18th code unit (0xE2) in the UTF-8 input (\"... the "
             u8"start\") "
             u8"passed to "
             u8"the function was found to be a valid "
             u8"leading byte, indicating the start of a three-byte "
             u8"sequence. However, the input ended after the first code unit — "
             u8"two "
             u8"continuation bytes were expected but none were present. As the "
             u8"sequence is incomplete, it cannot represent a valid Unicode "
             u8"scalar "
             u8"value, and the function was terminated.", u8"Cutoff at the start"},
            {unicode_conversion_error(
                 14, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xE2), u8'\0', u8'\0', u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_1
                 ), 0
             ), u8"The 15th code unit (0xE2) in the UTF-8 input (\"...    in the "
             u8"middle...\") "
             u8"passed to "
             u8"the function was found to be a valid "
             u8"leading byte, indicating the start of a four-byte "
             u8"sequence. However, the input ended after the first code unit — "
             u8"three "
             u8"continuation bytes were expected but none were present. As the "
             u8"sequence is incomplete, it cannot represent a valid Unicode "
             u8"scalar "
             u8"value, and the function was terminated.", u8"Cutoff    in the middle       but nowhere else"},
            {unicode_conversion_error(
                 8, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x80),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_3_found_2
                 ), 0
             ), u8"The 9th and 10th code units ([0xE2, 0x80]) in the UTF-8 input "
             u8"(\"maximum size12345\") "
             u8"passed to the function form the start of a "
             u8"three-byte sequence. However, the input ended after the "
             u8"second "
             u8"code unit — one further continuation byte was expected but was "
             u8"not "
             u8"present. As the sequence is incomplete, it cannot represent a "
             u8"valid Unicode scalar value, and the function was terminated.", u8"maximum size12345"},
            {unicode_conversion_error(
                 8, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      u8'\0',
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_2
                 ), 0
             ), u8"The 9th and 10th code units ([0xF0, 0x90]) in the UTF-8 input "
             u8"(\"maximum sizé12345\") "
             u8"passed to the function form the start of a "
             u8"four-byte sequence. However, the input ended after the second "
             u8"code unit — two further continuation bytes were expected but "
             u8"none "
             u8"were present. As the sequence is incomplete, it cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was terminated.", u8"maximum sizé12345"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::truncated_sequence(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             forward_scan_unicode_error::truncated_sequence_sub_error::
                         expected_4_found_3
                 ), 0
             ), u8"The 11th to 13th code units ([0xF0, 0x90, 0x80]) in the "
             u8"UTF-8 input (\"éééééééééééééé...\") passed to the function "
             u8"form the start of a four-byte sequence. However, the input "
             u8"ended after the third code unit — one further continuation "
             u8"byte was expected but was not present. As the sequence is "
             u8"incomplete, it cannot represent a valid Unicode scalar value, "
             u8"and the function was terminated.", u8"ééééééééééééééffff"},
            // For above case: 10-12 is 3 bytes. the 1 byte after that is the
            // 1st element, then 7 chars after.
            {unicode_conversion_error(
                 20, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xC2),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x00),
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_2_invalid_indexes_1
                 ), 0
             ), u8"The 21st and 22nd code units ([0xC2, 0x20]) in the UTF-8 input "
             u8"(\"...ééééééééééééééééé...\") "
             u8"passed to the function form the start of a "
             u8"two-byte sequence. The second code unit (0x20) was expected to "
             u8"be "
             u8"a continuation byte, but was not — a valid continuation byte "
             u8"must "
             u8"be inclusively between 0x80 and 0xBF. As 0x20 falls outside "
             u8"this "
             u8"range, the sequence cannot represent a valid Unicode scalar "
             u8"value, and the function was terminated.", u8"éééééééééééééééééééééééééé"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_3_invalid_indexes_1
                 ), 0
             ), u8"The 11th to 13th code units ([0xE2, 0x20, 0x80]) in the "
             u8"UTF-8 input "
             u8"(\"... \\xE2\\x80 \\xE2\\x80 hello\") passed "
             u8"to the function form "
             u8"the start of a three-byte sequence. The second code unit "
             u8"(0x20) "
             u8"was expected to be a continuation byte, but was not — a valid "
             u8"continuation byte must be inclusively between 0x80 and 0xBF. "
             u8"As "
             u8"0x20 falls outside this range, the sequence cannot represent a "
             u8"valid Unicode scalar value, and the function was terminated.", mk_unicode<char8_t>(
                 {0xE2, 0x80, 0x20, 0xE2, 0x80, 0x20, 0xE2, 0x80, 0x20},
             u8"hello"
             )},
            {unicode_conversion_error(
                 0, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x20),
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_3_invalid_indexes_2
                 ), 0
             ), u8"The 1st to 3rd code units ([0xE2, 0x80, 0x20]) in the "
             u8"UTF-8 input (\"hello\\xE2\\x80 \\xE2\\x80 ...\") passed to the "
             u8"function form "
             u8"the start of a three-byte sequence. The third code unit (0x20) "
             u8"was "
             u8"expected to be a continuation byte, but was not — a valid "
             u8"continuation byte must be inclusively between 0x80 and 0xBF. "
             u8"As "
             u8"0x20 falls outside this range, the sequence cannot represent a "
             u8"valid Unicode scalar value, and the function was terminated.", u8string(u8"hello")
                 + mk_unicode<char8_t>(
                     {0xE2, 0x80, 0x20, 0xE2, 0x80, 0x20, 0xE2, 0x80, 0x20}
                 )},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x21),
                      u8'\0'},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_3_invalid_indexes_1_2
                 ), 0
             ), u8"The 11th to 13th code units ([0xE2, 0x20, 0x21]) in the "
             u8"UTF-8 input (\"...\\xC3 superb owls hello\") passed to the "
             u8"function form "
             u8"the start of a three-byte sequence. The second and third code "
             u8"units (0x20 and 0x21) were expected to be continuation bytes, "
             u8"but "
             u8"were not — a valid "
             u8"continuation byte must be inclusively between 0x80 and 0xBF. "
             u8"As "
             u8"both are outside this range, the sequence cannot represent a "
             u8"valid "
             u8"Unicode scalar value, and the function was terminated.", mk_unicode<char8_t>(
                 {0xC3, 0x20, 0xC3, 0x20}, u8"superb owls hello"
             )},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_1
                 ), 0
             ), u8"The 11th to 14th code units ([0xF0, 0x20, 0x80, "
             u8"0x80]) in the UTF-8 input (\"<string>\") passed to the "
             u8"function form the start of a four-byte sequence. The second "
             u8"code "
             u8"unit (0x20) was expected to be a continuation byte, but was "
             u8"not — "
             u8"a valid "
             u8"continuation byte must be inclusively between 0x80 and 0xBF. "
             u8"As "
             u8"0x20 falls outside this range, the sequence cannot represent a "
             u8"valid Unicode scalar value, and the function was terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x80)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_2
                 ), 0
             ), u8"The 11th to 14th code units ([0xF0, 0x90, 0x20, "
             u8"0x80]) in the UTF-8 input (\"<string>\") passed to the "
             u8"function form the start of a four-byte sequence. The third "
             u8"code "
             u8"unit (0x20) was expected to be a continuation byte, but was "
             u8"not — "
             u8"a valid "
             u8"continuation byte must be inclusively between 0x80 and 0xBF. "
             u8"As "
             u8"0x20 falls outside this range, the sequence cannot represent a "
             u8"valid Unicode scalar value, and the function was terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x20)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_3
                 ), 0
             ), u8"The 11th to 14th code units ([0xF0, 0x90, 0x80, "
             u8"0x20]) in the UTF-8 input (\"<string>\") passed to the "
             u8"function form the start of a four-byte sequence. The fourth "
             u8"code "
             u8"unit (0x20) was expected to be a continuation byte, but was "
             u8"not — "
             u8"a valid continuation byte must be inclusively between 0x80 and "
             u8"0xBF. As 0x20 falls outside this range, the sequence cannot "
             u8"represent a valid Unicode scalar value, and the function was "
             u8"terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x21),
                      static_cast<char8_t>(0x80)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_1_2
                 ), 0
             ), u8"The 11th to 14th code units ([0xF0, 0x20, 0x21, "
             u8"0x80]) in the UTF-8 input (\"<string>\") passed to the "
             u8"function form the start of a four-byte sequence. The second "
             u8"and "
             u8"third code units (0x20 and 0x21) were expected to be "
             u8"continuation "
             u8"bytes, but were not — a valid "
             u8"continuation byte must be inclusively between 0x80 and 0xBF. "
             u8"As "
             u8"both are outside this range, the sequence cannot represent a "
             u8"valid "
             u8"Unicode scalar value, and the function was terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x90),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x21)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_2_3
                 ), 0
             ), u8"The 11th to 14th code units ([0xF0, 0x90, 0x20, "
             u8"0x21]) in the UTF-8 input (\"<string>\") passed to the "
             u8"function form the start of a four-byte sequence. The third and "
             u8"fourth code units (0x20 and 0x21) were expected to be "
             u8"continuation "
             u8"bytes, but were not — a valid "
             u8"continuation byte must be inclusively between 0x80 and 0xBF. "
             u8"As "
             u8"both are outside this range, the sequence cannot represent a "
             u8"valid "
             u8"Unicode scalar value, and the function was terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x21)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_1_3
                 ), 0
             ), u8"The 11th to 14th code units ([0xF0, 0x20, 0x80, "
             u8"0x21]) in the UTF-8 input (\"<string>\") passed to the "
             u8"function form the start of a four-byte sequence. The second "
             u8"and "
             u8"fourth code units (0x20 and 0x21) were expected to be "
             u8"continuation "
             u8"bytes, but were not — a valid "
             u8"continuation byte must be inclusively between 0x80 and 0xBF. "
             u8"As "
             u8"both are outside this range, the sequence cannot represent a "
             u8"valid "
             u8"Unicode scalar value, and the function was terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x21),
                      static_cast<char8_t>(0x22)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_1_2_3
                 ), 0
             ), u8"The 11th to 14th code units ([0xF0, 0x20, 0x21, "
             u8"0x22]) in the UTF-8 input (\"<string>\") passed to the "
             u8"function form the start of a four-byte sequence. The second, "
             u8"third and fourth code units (0x20, 0x21 and 0x22) were "
             u8"expected to "
             u8"be continuation bytes, but were not — a valid "
             u8"continuation byte must be inclusively between 0x80 and 0xBF. "
             u8"As "
             u8"all three are "
             u8"outside "
             u8"this range, the sequence cannot represent a valid Unicode "
             u8"scalar "
             u8"value, and the function was terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xC0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x00),
                      static_cast<char8_t>(0x00)},
             2, static_cast<char32_t>(0x0)
                 ), 0
             ), u8"The 11th and 12th code units ([0xC0, 0x80]) in the UTF-8 input "
             u8"(\"<string>\") "
             u8"passed to the function form a two-byte "
             u8"sequence encoding U+0000. This is an overlong encoding — "
             u8"U+0000 can be represented using a single byte (0x00), which is "
             u8"the "
             u8"shortest valid UTF-8 representation. The UTF-8 standard "
             u8"requires "
             u8"that code points are always encoded using the shortest "
             u8"possible "
             u8"sequence. As this requirement is not met, the sequence does "
             u8"not "
             u8"represent a valid Unicode scalar value, and the function was "
             u8"terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xE0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x00)},
             3, static_cast<char32_t>(0x0)
                 ), 0
             ), u8"The 11th to 13th code units ([0xE0, 0x80, 0x80]) in the "
             u8"UTF-8 input (\"<string>\") passed to the function form a "
             u8"three-byte sequence encoding U+0000. This is an overlong "
             u8"encoding — U+0000 can be represented using a single byte "
             u8"(0x00), "
             u8"which is the shortest valid UTF-8 representation. The UTF-8 "
             u8"standard requires that code points are always encoded using "
             u8"the "
             u8"shortest possible sequence. As this requirement is not met, "
             u8"the "
             u8"sequence does not represent a valid Unicode scalar "
             u8"value, and the function was terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xE0),
                      static_cast<char8_t>(0x9F),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0x00)},
             3, static_cast<char32_t>(0x7FF)
                 ), 0
             ), u8"The 11th to 13th code units ([0xE0, 0x9F, 0xBF]) in the "
             u8"UTF"
             u8"-8 input (\"<string>\") passed to the function form a "
             u8"three-byte sequence encoding U+07FF. This is an overlong "
             u8"encoding — U+07FF can be represented using two bytes ([0xDF, "
             u8"0xBF]), which is the shortest valid UTF-8 representation. The "
             u8"UTF"
             u8"-8 standard requires that code points are always encoded using "
             u8"the shortest possible sequence. As this requirement is not "
             u8"met, "
             u8"the sequence does not represent a valid Unicode scalar "
             u8"value, and the function was terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80)},
             4, static_cast<char32_t>(0x0)
                 ), 0
             ), u8"The 11th to 14th code units ([0xF0, 0x80, 0x80, "
             u8"0x80]) in the UTF-8 input (\"<string>\") passed to the "
             u8"function form a four-byte sequence encoding U+0000. This "
             u8"is an overlong encoding — U+0000 can be represented using a "
             u8"single "
             u8"byte (0x00), which is the shortest valid UTF-8 representation. "
             u8"The "
             u8"UTF-8 standard requires that code points are always encoded "
             u8"using "
             u8"the shortest possible sequence. As this requirement is not "
             u8"met, "
             u8"the sequence does not represent a valid Unicode scalar "
             u8"value, and the function was terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0xBF)},
             4, static_cast<char32_t>(0x3F)
                 ), 0
             ), u8"The 11th to 14th code units ([0xF0, 0x80, 0x80, "
             u8"0xBF]) in the UTF-8 input (\"<string>\") passed to the "
             u8"function form a four-byte sequence encoding U+003F. This "
             u8"is an overlong encoding — U+003F can be represented using a "
             u8"single "
             u8"byte (0x3F), which is the shortest valid UTF-8 representation. "
             u8"The "
             u8"UTF-8 standard requires that code points are always encoded "
             u8"using "
             u8"the shortest possible sequence. As this requirement is not "
             u8"met, "
             u8"the sequence does not represent a valid Unicode scalar "
             u8"value, and the function was terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::overlong_encoding(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0xBF)},
             4, static_cast<char32_t>(0x0FFF)
                 ), 0
             ), u8"The 11th to 14th code units ([0xF0, 0x80, 0xBF, "
             u8"0xBF]) in the UTF-8 input (\"<string>\") passed to the "
             u8"function form a four-byte sequence encoding U+0FFF. "
             u8"This is an overlong encoding — U+0FFF can be represented using "
             u8"three bytes ([0xE0, 0xBF, 0xBF]), which is the shortest valid "
             u8"UTF-8 representation. The UTF-8 standard requires that code "
             u8"points "
             u8"are always encoded using the shortest possible sequence. As "
             u8"this "
             u8"requirement is not met, the sequence does not represent a "
             u8"valid "
             u8"Unicode scalar value, and the function was terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xED),
                          static_cast<char8_t>(0xA0),
                          static_cast<char8_t>(0x80),
                          static_cast<char8_t>(0x00)},
             3, static_cast<char32_t>(0xD800)
                     ), 0
             ), u8"The 11th to 13th code units ([0xED, 0xA0, 0x80]) in the "
             u8"UTF-8 input (\"<string>\") passed to the function form a "
             u8"three-byte sequence encoding U+D800. However, U+D800 falls "
             u8"outside "
             u8"the valid Unicode range — valid Unicode scalar values must be "
             u8"inclusively between U+0000 and U+10FFFF, excluding the "
             u8"surrogate "
             u8"range U+D800 to U+DFFF. As U+D800 falls within the surrogate "
             u8"range, it cannot represent a valid Unicode scalar value, and "
             u8"the "
             u8"function was terminated.", u8"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::
                     invalid_utf32_code_point_after_utf8_conversion(
                         {static_cast<char8_t>(0xF4),
                          static_cast<char8_t>(0x90),
                          static_cast<char8_t>(0x80),
                          static_cast<char8_t>(0x80)},
             4, static_cast<char32_t>(0x11'0000)
                     ), 0
             ), u8"The 11th to 14th code units ([0xF4, 0x90, 0x80, "
             u8"0x80]) in the UTF-8 input (\"<string>\") passed to the "
             u8"function form a four-byte sequence encoding U+110000. "
             u8"However, U+110000 falls outside the valid Unicode range — "
             u8"valid "
             u8"Unicode scalar values must be inclusively between U+0000 and "
             u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. As "
             u8"U+110000 exceeds the maximum valid codepoint, it cannot "
             u8"represent "
             u8"a valid Unicode scalar value, and the function was terminated.", u8"<string>"},
    };
    vector<tuple<unicode_conversion_error, u8string, optional<u16string>>>
        u16_input_output_pairs = {
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::
                     high_surrogate_then_end_of_stream(
                         static_cast<char16_t>(0xD800), false
                     ), 0
             ), u8"The 11th code unit (0xD800) in the UTF-16 input (\"hello\") "
             u8"passed to the function is a high surrogate, "
             u8"indicating the start of a surrogate pair. However, the input "
             u8"ended "
             u8"after this code unit — a low surrogate was expected to follow "
             u8"but "
             u8"was not present. As the surrogate pair is incomplete, it "
             u8"cannot "
             u8"represent a valid Unicode scalar value, and the function was "
             u8"terminated.", u"hello"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::
                     high_surrogate_then_end_of_stream(
                         static_cast<char16_t>(0xD800), true
                     ), 0
             ), u8"The 11th code unit (0xD800) in the UTF-16 input (encoded using "
             u8"wchar_t) passed to the function is a high surrogate, "
             u8"indicating the start of a surrogate pair. However, the input "
             u8"ended "
             u8"after this code unit — a low surrogate was expected to follow "
             u8"but "
             u8"was not present. As the surrogate pair is incomplete, it "
             u8"cannot "
             u8"represent a valid Unicode scalar value, and the function was "
             u8"terminated.", std::nullopt},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::
                     high_surrogate_not_followed_by_low_surrogate(
                         static_cast<char16_t>(0xDBFF),
             static_cast<char16_t>(0xD800),
             false
                     ), 0
             ), u8"The 11th and 12th code units ([0xDBFF, 0xD800]) in the UTF-16 "
             u8"input (\"\\uD800\") passed to the function form the "
             u8"start of a surrogate pair. The first code unit (0xDBFF) is a "
             u8"high "
             u8"surrogate, which must be followed by a low surrogate "
             u8"inclusively "
             u8"between 0xDC00 and 0xDFFF. However, the second code unit "
             u8"(0xD800) "
             u8"falls outside this range, and therefore the two code units "
             u8"cannot "
             u8"represent a valid Unicode scalar value, and the function was "
             u8"terminated.", mk_unicode<char16_t>({0xD800})},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::
                     high_surrogate_not_followed_by_low_surrogate(
                         static_cast<char16_t>(0xDBFF),
             static_cast<char16_t>(0xD800),
             true
                     ), 0
             ), u8"The 11th and 12th code units ([0xDBFF, 0xD800]) in the UTF-16 "
             u8"input (\"<string>\", encoded using wchar_t) passed to "
             u8"the "
             u8"function form the "
             u8"start of a surrogate pair. The first code unit (0xDBFF) is a "
             u8"high "
             u8"surrogate, which must be followed by a low surrogate "
             u8"inclusively "
             u8"between 0xDC00 and 0xDFFF. However, the second code unit "
             u8"(0xD800) "
             u8"falls outside this range, and therefore the two code units "
             u8"cannot "
             u8"represent a valid Unicode scalar value, and the function was "
             u8"terminated.", u"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::unexpected_low_surrogate(
                     static_cast<char16_t>(0xDC00), false
                 ), 0
             ), u8"The 11th code unit (0xDC00) in the UTF-16 input (\"<"
             u8"string>\") passed to "
             u8"the function is a low surrogate. Low "
             u8"surrogates must always be preceded by a high surrogate "
             u8"(inclusively between 0xD800 and 0xDBFF) as the second part of "
             u8"a "
             u8"surrogate pair. As this low surrogate appears without a "
             u8"preceding "
             u8"high surrogate, it cannot represent a valid Unicode scalar "
             u8"value, and the function was terminated.", u"<string>"},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::unexpected_low_surrogate(
                     static_cast<char16_t>(0xDC00), true
                 ), 0
             ), u8"The 11th code unit (0xDC00) in the UTF-16 input (\"<"
             u8"string>\", encoded using "
             u8"wchar_t) passed to the function is a low "
             u8"surrogate. Low surrogates must always be preceded by a high "
             u8"surrogate (inclusively between 0xD800 and 0xDBFF) as the "
             u8"second "
             u8"part of a surrogate pair. As this low surrogate appears "
             u8"without a "
             u8"preceding high surrogate, it cannot represent a valid Unicode "
             u8"scalar value, and the function was terminated.", u"<string>"},
    };
    vector<tuple<unicode_conversion_error, u8string, optional<u32string>>>
        u32_input_output_pairs = {
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_utf32_code_point(
                     static_cast<char32_t>(0xD800),    false
                 ),    0
             ),    u8"The 11th code unit (0xD800) in the UTF-32 input (\"\\uD800\") "
             u8"passed to "
             u8"the function decodes to U+D800. However, "
             u8"U+D800 falls outside the valid Unicode range — valid Unicode "
             u8"scalar "
             u8"values must be inclusively between U+0000 and U+10FFFF, "
             u8"excluding "
             u8"the surrogate range U+D800 to U+DFFF. As U+D800 falls within "
             u8"the "
             u8"surrogate range, it cannot represent a valid Unicode scalar "
             u8"value, and the function was terminated.",                         mk_unicode<char32_t>({0xD800})},
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_utf32_code_point(
                     static_cast<char32_t>(0x11'0000), false
                 ), 0
             ), u8"The 11th code unit (0x110000) in the UTF-32 input (\"<"
             u8"string>\") passed to "
             u8"the function decodes to U+110000. However, "
             u8"U+110000 falls outside the valid Unicode range — valid Unicode "
             u8"scalar "
             u8"values must be inclusively between U+0000 and U+10FFFF, "
             u8"excluding "
             u8"the surrogate range U+D800 to U+DFFF. As U+110000 exceeds the "
             u8"maximum valid codepoint, it cannot represent a valid Unicode "
             u8"scalar "
             u8"value, and the function was terminated.",                      U"<string>"                   },
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_utf32_code_point(
                     static_cast<char32_t>(0xD800),    true
                 ),    0
             ),    u8"The 11th code unit (0xD800) in the UTF-32 input (\"<"
             u8"string>\", encoded using "
             u8"wchar_t) passed to the function decodes to "
             u8"U+D800. However, U+D800 falls outside the valid Unicode range "
             u8"— "
             u8"valid Unicode scalar values must be inclusively between U+0000 "
             u8"and "
             u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. As "
             u8"U+D800 "
             u8"falls within the surrogate range, it cannot represent a valid "
             u8"Unicode "
             u8"scalar value, and the function was terminated.",                  U"<string>"                   },
            {unicode_conversion_error(
                 10, forward_scan_unicode_error_factory::invalid_utf32_code_point(
                     static_cast<char32_t>(0x11'0000), true
                 ), 0
             ), u8"The 11th code unit (0x110000) in the UTF-32 input (\"<"
             u8"string>\", encoded using "
             u8"wchar_t) passed to the function decodes to "
             u8"U+110000. However, U+110000 falls outside the valid Unicode "
             u8"range "
             u8"— "
             u8"valid Unicode scalar values must be inclusively between U+0000 "
             u8"and "
             u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. As "
             u8"U+110000 exceeds the maximum valid codepoint, it cannot "
             u8"represent "
             u8"a "
             u8"valid Unicode scalar value, and the function was terminated.", U"<string>"                   },
    };
    vector<tuple<
        unicode_conversion_error,
        u8string,
        optional<basic_string<CharT>>>>
         rv;
    auto fill_vector_func = [&]<typename T>(T list_arg)
    {
        for (auto& [error, output_str, opt_original_str] : list_arg)
        {
            if (opt_original_str.has_value())
            {
                rv.push_back(make_tuple(
                    error,
                    output_str,
                    make_optional(spl_convert<CharT>(opt_original_str.value()))
                ));
            }
            else
            {
                rv.push_back(make_tuple(error, output_str, std::nullopt));
            }
        }
    };
    if constexpr (same_as<char8_t, CharT>)
    {
        fill_vector_func(u8_input_output_pairs);
    }
    else if constexpr (same_as<char16_t, CharT>)
    {
        fill_vector_func(u16_input_output_pairs);
    }
    else if constexpr (same_as<char32_t, CharT>)
    {
        fill_vector_func(u32_input_output_pairs);
    }
    else if constexpr (same_as<wchar_t, CharT>)
    {
        if constexpr (sizeof(CharT) == 2)
        {
            fill_vector_func(u16_input_output_pairs);
        }
        else if constexpr (sizeof(CharT) == 4)
        {
            fill_vector_func(u32_input_output_pairs);
        }
    }
    return rv;
}
} // namespace unicode_bridge_test_cases

TEST_CASE(
    "unicode_conversion error message tests",
    "[unicode_conversion]"
)
{
    using namespace unicode_bridge;
    using namespace unicode_bridge::internal;
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    auto ki = get_unicode_conversion_error_message<char8_t>();
    test_errors(ki);
    auto k2 = get_unicode_conversion_error_message<char16_t>();
    test_errors(k2);
    auto k3 = get_unicode_conversion_error_message<char32_t>();
    test_errors(k3);
    auto k4 = get_unicode_conversion_error_message<wchar_t>();
    test_errors(k4);
}

TEST_CASE(
    "ascii_to_unicode_error error message tests",
    "[ascii_to_unicode_error]"
)
{
    using namespace unicode_bridge;
    using namespace unicode_bridge::internal;
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    vector<tuple<ascii_to_unicode_error, u8string, std::optional<std::string>>>
        ki = {
            {ascii_to_unicode_error(0, '\x80'),
             u8"The 1st character (0x80) in the ASCII input (\"The world...\") "
             u8"passed to the function was found to be invalid ASCII. "
             u8"This function can only convert ASCII to Unicode — that is, "
             u8"character values inclusively between 0x00 and 0x7F. As 0x80 "
             u8"falls outside this "
             u8"range, it cannot be converted to Unicode, and the function "
             u8"was terminated.", "The world is not enough"},
            {ascii_to_unicode_error(0, '\x80'),
             u8"The 1st character (0x80) in the ASCII input (\"\\x80The "
             u8"worl...\") "
             u8"passed to the function was found to be invalid ASCII. "
             u8"This function can only convert ASCII to Unicode — that is, "
             u8"character values inclusively between 0x00 and 0x7F. As 0x80 "
             u8"falls outside this "
             u8"range, it cannot be converted to Unicode, and the function "
             u8"was terminated.", string(1, '\x80') + "The world is not enough"},
            {ascii_to_unicode_error(0, '\x80'),
             u8"The 1st character (0x80) in the ASCII input "
             u8"passed to the function was found to be invalid ASCII. "
             u8"This function can only convert ASCII to Unicode — that is, "
             u8"character values inclusively between 0x00 and 0x7F. As 0x80 "
             u8"falls outside this "
             u8"range, it cannot be converted to Unicode, and the function "
             u8"was terminated.", std::nullopt}
    };
    test_errors(ki);
}

namespace unicode_bridge_test_cases
{
template <typename CharT>
std::vector<std::tuple<
    unicode_bridge::unicode_to_ascii_error,
    std::u8string,
    std::optional<std::basic_string<CharT>>>>
    get_unicode_to_ascii_error_message()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge::internal;
    using namespace unicode_bridge_testing;
    vector<tuple<unicode_to_ascii_error, u8string, optional<u8string>>>
        u8_input_output_pairs = {
            {unicode_to_ascii_error_factory::invalid_unicode_character(
                 15, forward_scan_unicode_error_factory::invalid_continuation_byte(
                     {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x20),
                      static_cast<char8_t>(0x21),
                      static_cast<char8_t>(0x22)},
             forward_scan_unicode_error::
                         invalid_continuation_byte_sub_error::
                             size_4_invalid_indexes_1_2_3
                 ), 0
             ), u8"The 16th to 19th code units ([0xF0, 0x20, 0x21, "
             u8"0x22]) in the UTF-8 input (\"...Welcome to the large...\") "
             u8"passed to the "
             u8"function form the start of a four-byte sequence. The "
             u8"second, "
             u8"third and fourth code units (0x20, 0x21 and 0x22) were "
             u8"expected to "
             u8"be continuation bytes, but were not — a valid "
             u8"continuation byte must be inclusively between 0x80 and "
             u8"0xBF. "
             u8"As "
             u8"all three are "
             u8"outside "
             u8"this range, the sequence cannot represent a valid Unicode "
             u8"scalar "
             u8"value, and the function was terminated.", u8"       Welcome to the largest    "},
            {unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf8(
                     U'𝄞', {static_cast<char8_t>(0xF0),
                      static_cast<char8_t>(0x9D),
                      static_cast<char8_t>(0x84),
                      static_cast<char8_t>(0x9E)},
             16, 4,
             0
                 ), u8"The 17th to 20th code units ([0xF0, 0x9D, 0x84, "
             u8"0x9E]) "
             u8"in the UTF-8 input (\"...Welcome to the large...\") passed "
             u8"to "
             u8"the "
             u8"function encode the Unicode scalar "
             u8"value "
             u8"U+1D11E ('𝄞'). However, this function converts Unicode to "
             u8"ASCII — "
             u8"valid ASCII values are inclusively between U+0000 and "
             u8"U+007F. "
             u8"As "
             u8"U+1D11E falls outside this range, it cannot be represented "
             u8"as "
             u8"ASCII, "
             u8"and the function was terminated.", u8"        Welcome to the largest    "},
            {unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf8(
                     U'€', {static_cast<char8_t>(0xE2),
                      static_cast<char8_t>(0x82),
                      static_cast<char8_t>(0xAC),
                      static_cast<char8_t>(0x00)},
             16, 3,
             0
                 ), u8"The 17th to 19th code units ([0xE2, 0x82, 0xAC]) "
             u8"in the UTF-8 input (\"...Welcome to the larg...\") passed "
             u8"to "
             u8"the "
             u8"function encode the Unicode scalar "
             u8"value "
             u8"U+20AC ('€'). However, this function converts Unicode to "
             u8"ASCII — "
             u8"valid ASCII values are inclusively between U+0000 and "
             u8"U+007F. "
             u8"As "
             u8"U+20AC falls outside this range, it cannot be represented "
             u8"as "
             u8"ASCII, "
             u8"and the function was terminated.", u8"        Welcome to the largest    "},
            {unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf8(
                     U'é', {static_cast<char8_t>(0xC3),
                      static_cast<char8_t>(0xA9),
                      static_cast<char8_t>(0x00),
                      static_cast<char8_t>(0x00)},
             16, 2,
             0
                 ), u8"The 17th and 18th code units ([0xC3, 0xA9]) "
             u8"in the UTF-8 input (\"...Welcome to the lar...\") passed "
             u8"to "
             u8"the "
             u8"function encode the Unicode scalar "
             u8"value "
             u8"U+00E9 ('é'). However, this function converts Unicode to "
             u8"ASCII — "
             u8"valid ASCII values are inclusively between U+0000 and "
             u8"U+007F. "
             u8"As "
             u8"U+00E9 falls outside this range, it cannot be represented "
             u8"as "
             u8"ASCII, "
             u8"and the function was terminated.", u8"        Welcome to the largest    "},
            {unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf8(
                     static_cast<char32_t>(0xFFFD),
             {static_cast<char8_t>(0xEF),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0xBD),
                      static_cast<char8_t>(0x00)},
             16, 3,
             0
                 ), u8"The 17th to 19th code units ([0xEF, 0xBF, 0xBD]) "
             u8"in the UTF-8 input (\"...Welcome to the larg...\") passed "
             u8"to "
             u8"the "
             u8"function encode the Unicode scalar "
             u8"value "
             u8"U+FFFD. However, this function converts Unicode to "
             u8"ASCII — "
             u8"valid ASCII values are inclusively between U+0000 and "
             u8"U+007F. "
             u8"As "
             u8"U+FFFD falls outside this range, it cannot be represented "
             u8"as "
             u8"ASCII, "
             u8"and the function was terminated.", u8"        Welcome to the largest    "},
    };
    vector<tuple<unicode_to_ascii_error, u8string, optional<u16string>>>
        u16_input_output_pairs = {
            {unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf16(
                     U'é',                          {static_cast<char16_t>(0x00E9), static_cast<char16_t>(0x0)
                     },                                16,
             1,                                                                                      false,
             0
                 ),                   u8"The 17th code unit (0x00E9) "
             u8"in the UTF-16 input (\"...Welcome to the la...\") passed "
             u8"to "
             u8"the "
             u8"function encodes the Unicode scalar "
             u8"value "
             u8"U+00E9 ('é'). However, this function converts Unicode to "
             u8"ASCII — "
             u8"valid ASCII values are inclusively between U+0000 and "
             u8"U+007F. "
             u8"As "
             u8"U+00E9 falls outside this range, it cannot be represented "
             u8"as "
             u8"ASCII, "
             u8"and the function was terminated.",                   u"        Welcome to the largest    "},
            {unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf16(
                     static_cast<char32_t>(0x00A0),
             {static_cast<char16_t>(0x00A0), static_cast<char16_t>(0x0)
                     },                                                                       16,
             1,                                                                                      false,
             0
                 ),                   u8"The 17th code unit (0x00A0) "
             u8"in the UTF-16 input (\"...Welcome to the la...\") passed "
             u8"to "
             u8"the "
             u8"function encodes the Unicode scalar "
             u8"value "
             u8"U+00A0. However, this function converts Unicode to "
             u8"ASCII — "
             u8"valid ASCII values are inclusively between U+0000 and "
             u8"U+007F. "
             u8"As "
             u8"U+00A0 falls outside this range, it cannot be represented "
             u8"as "
             u8"ASCII, "
             u8"and the function was terminated.",                   u"        Welcome to the largest    "},
            {unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf16(
                     U'😂',                         {static_cast<char16_t>(0xD83D),
                      static_cast<char16_t>(0xDE02)},
             16,                                                                                  2,
             true,                                                                                          0
                 ), u8"The 17th and 18th code units ([0xD83D, 0xDE02]) "
             u8"in the UTF-16 input (encoded using wchar_t) passed to "
             u8"the "
             u8"function encode the Unicode scalar "
             u8"value "
             u8"U+1F602 ('😂'). However, this function converts Unicode to "
             u8"ASCII — "
             u8"valid ASCII values are inclusively between U+0000 and "
             u8"U+007F. "
             u8"As "
             u8"U+1F602 falls outside this range, it cannot be represented "
             u8"as "
             u8"ASCII, "
             u8"and the function was terminated.", std::nullopt                         },
            {unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf16(
                     U'😂',                         {static_cast<char16_t>(0xD83D),
                      static_cast<char16_t>(0xDE02)},
             16,                                                                                  2,
             true,                                                                                          0
                 ), u8"The 17th and 18th code units ([0xD83D, 0xDE02]) "
             u8"in the UTF-16 input (\"... to the largest   ...\", encoded "
             u8"using wchar_t) passed to "
             u8"the "
             u8"function encode the Unicode scalar "
             u8"value "
             u8"U+1F602 ('😂'). However, this function converts Unicode to "
             u8"ASCII — "
             u8"valid ASCII values are inclusively between U+0000 and "
             u8"U+007F. "
             u8"As "
             u8"U+1F602 falls outside this range, it cannot be represented "
             u8"as "
             u8"ASCII, "
             u8"and the function was terminated.", u" Welcome to the largest    "       },
    };
    vector<tuple<unicode_to_ascii_error, u8string, optional<u32string>>>
        u32_input_output_pairs = {
            {unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf32(U'é',  16, false, 0),
             u8"The 17th code unit (0x000000E9) "
             u8"in the UTF-32 input (\"...Welcome to the la...\") passed "
             u8"to "
             u8"the "
             u8"function encodes the Unicode scalar "
             u8"value "
             u8"U+00E9 ('é'). However, this function converts Unicode to "
             u8"ASCII — "
             u8"valid ASCII values are inclusively between U+0000 and "
             u8"U+007F. "
             u8"As "
             u8"U+00E9 falls outside this range, it cannot be represented "
             u8"as "
             u8"ASCII, "
             u8"and the function was terminated.",                                                                                                        U"        Welcome to the largest    "},
            {unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf32(
                     static_cast<char32_t>(0x00A0),          16, false, 0
                 ), u8"The 17th code unit (0x000000A0) "
             u8"in the UTF-32 input (\"...Welcome to the la...\") "
             u8"passed to "
             u8"the "
             u8"function encodes the Unicode scalar "
             u8"value "
             u8"U+00A0. However, this function converts Unicode to "
             u8"ASCII — "
             u8"valid ASCII values are inclusively between U+0000 and "
             u8"U+007F. "
             u8"As "
             u8"U+00A0 falls outside this range, it cannot be "
             u8"represented as "
             u8"ASCII, "
             u8"and the function was terminated.", U"        Welcome to the largest    "},
            {unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf32(U'😂', 16, true,  0),
             u8"The 17th code unit (0x0001F602) "
             u8"in the UTF-32 input (encoded using wchar_t) passed to "
             u8"the "
             u8"function encodes the Unicode scalar "
             u8"value "
             u8"U+1F602 ('😂'). However, this function converts Unicode to "
             u8"ASCII — "
             u8"valid ASCII values are inclusively between U+0000 and "
             u8"U+007F. "
             u8"As "
             u8"U+1F602 falls outside this range, it cannot be represented "
             u8"as "
             u8"ASCII, "
             u8"and the function was terminated.",                                                                                                        std::nullopt                         },
            {unicode_to_ascii_error_factory::
                 non_ascii_character_found_from_utf32(U'😂', 16, true,  0),
             u8"The 17th code unit (0x0001F602) "
             u8"in the UTF-32 input (\"...Welcome to the la...\", encoded "
             u8"using wchar_t) passed to "
             u8"the "
             u8"function encodes the Unicode scalar "
             u8"value "
             u8"U+1F602 ('😂'). However, this function converts Unicode to "
             u8"ASCII — "
             u8"valid ASCII values are inclusively between U+0000 and "
             u8"U+007F. "
             u8"As "
             u8"U+1F602 falls outside this range, it cannot be represented "
             u8"as "
             u8"ASCII, "
             u8"and the function was terminated.",                                                                                                        U"        Welcome to the largest    "},
    };
    vector<
        tuple<unicode_to_ascii_error, u8string, optional<basic_string<CharT>>>>
         rv;
    auto fill_vector_func = [&]<typename T>(T list_arg)
    {
        for (auto& [error, output_str, opt_original_str] : list_arg)
        {
            if (opt_original_str.has_value())
            {
                rv.push_back(make_tuple(
                    error,
                    output_str,
                    make_optional(spl_convert<CharT>(opt_original_str.value()))
                ));
            }
            else
            {
                rv.push_back(make_tuple(error, output_str, std::nullopt));
            }
        }
    };
    if constexpr (same_as<char8_t, CharT>)
    {
        fill_vector_func(u8_input_output_pairs);
    }
    else if constexpr (same_as<char16_t, CharT>)
    {
        fill_vector_func(u16_input_output_pairs);
    }
    else if constexpr (same_as<char32_t, CharT>)
    {
        fill_vector_func(u32_input_output_pairs);
    }
    else if constexpr (same_as<wchar_t, CharT>)
    {
        if constexpr (sizeof(CharT) == 2)
        {
            fill_vector_func(u16_input_output_pairs);
        }
        else if constexpr (sizeof(CharT) == 4)
        {
            fill_vector_func(u32_input_output_pairs);
        }
    }
    return rv;
}
} // namespace unicode_bridge_test_cases

TEST_CASE(
    "unicode_to_ascii error message tests",
    "[unicode_to_ascii]"
)
{
    using namespace unicode_bridge;
    using namespace unicode_bridge::internal;
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    auto ki = get_unicode_to_ascii_error_message<char8_t>();
    test_errors(ki);
    auto k2 = get_unicode_to_ascii_error_message<char16_t>();
    test_errors(k2);
    auto k3 = get_unicode_to_ascii_error_message<char32_t>();
    test_errors(k3);
    auto k4 = get_unicode_to_ascii_error_message<wchar_t>();
    test_errors(k4);
}

namespace unicode_bridge_test_cases
{
template <typename CharT>
std::vector<std::tuple<
    unicode_bridge::next_char32_error,
    std::u8string,
    std::optional<std::basic_string<CharT>>>>
    get_next_char32_error_message()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge::internal;
    using namespace unicode_bridge_testing;
    vector<tuple<next_char32_error, u8string, optional<u8string>>>
        u8_input_output_pairs = {
            {next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_utf32_code_point(
                     static_cast<char32_t>(0x11'0000), true
                 )
             ), u8"The code unit at the current iterator position (0x110000) "
             u8"of "
             u8"the UTF-32 input (\"<"
             u8"string>\", encoded using "
             u8"wchar_t) passed to the function decodes to "
             u8"U+110000. However, U+110000 falls outside the valid "
             u8"Unicode "
             u8"range "
             u8"— "
             u8"valid Unicode scalar values must be inclusively between "
             u8"U+0000 "
             u8"and "
             u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. "
             u8"As "
             u8"U+110000 exceeds the maximum valid codepoint, it cannot "
             u8"represent "
             u8"a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"<string>"},
            {next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_utf32_code_point(
                     static_cast<char32_t>(0x11'0000), true
                 )
             ), u8"The code unit at the current iterator position (0x110000) "
             u8"of "
             u8"the UTF-32 input (encoded "
             u8"using "
             u8"wchar_t) passed to the function decodes to "
             u8"U+110000. However, U+110000 falls outside the valid "
             u8"Unicode "
             u8"range "
             u8"— "
             u8"valid Unicode scalar values must be inclusively between "
             u8"U+0000 "
             u8"and "
             u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. "
             u8"As "
             u8"U+110000 exceeds the maximum valid codepoint, it cannot "
             u8"represent "
             u8"a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", std::nullopt},
            {next_char32_error_factory::iterator_exhausted(),
             u8"The current iterator passed to the function was equal to "
             u8"the "
             u8"end iterator — "
             u8"signifying that there were no more code units to read, and "
             u8"the "
             u8"function was terminated.", std::nullopt},
            {next_char32_error_factory::iterator_exhausted(),
             u8"The current iterator passed to the function was equal to "
             u8"the "
             u8"end iterator — "
             u8"signifying that there were no more code units to read, and "
             u8"the "
             u8"function was terminated.", u8"strings!"},
    };
    vector<tuple<next_char32_error, u8string, optional<u16string>>>
        u16_input_output_pairs = {
            {next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_utf32_code_point(
                     static_cast<char32_t>(0x11'0000), true
                 )
             ), u8"The code unit at the current iterator position (0x110000) "
             u8"of "
             u8"the UTF-32 input (\"<"
             u8"string>\", encoded using "
             u8"wchar_t) passed to the function decodes to "
             u8"U+110000. However, U+110000 falls outside the valid "
             u8"Unicode "
             u8"range "
             u8"— "
             u8"valid Unicode scalar values must be inclusively between "
             u8"U+0000 "
             u8"and "
             u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. "
             u8"As "
             u8"U+110000 exceeds the maximum valid codepoint, it cannot "
             u8"represent "
             u8"a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u"<string>"},
            {next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_utf32_code_point(
                     static_cast<char32_t>(0x11'0000), true
                 )
             ), u8"The code unit at the current iterator position (0x110000) "
             u8"of "
             u8"the UTF-32 input (encoded using "
             u8"wchar_t) passed to the function decodes to "
             u8"U+110000. However, U+110000 falls outside the valid "
             u8"Unicode range — "
             u8"valid Unicode scalar values must be inclusively between "
             u8"U+0000 and "
             u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. "
             u8"As "
             u8"U+110000 exceeds the maximum valid codepoint, it cannot "
             u8"represent "
             u8"a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", std::nullopt},
            {next_char32_error_factory::iterator_exhausted(),
             u8"The current iterator passed to the function was equal to "
             u8"the "
             u8"end iterator — "
             u8"signifying that there were no more code units to read, and "
             u8"the "
             u8"function was terminated.", std::nullopt},
            {next_char32_error_factory::iterator_exhausted(),
             u8"The current iterator passed to the function was equal to "
             u8"the "
             u8"end iterator — "
             u8"signifying that there were no more code units to read, and "
             u8"the "
             u8"function was terminated.", u"strings!"},
    };
    vector<tuple<next_char32_error, u8string, optional<u32string>>>
        u32_input_output_pairs = {
            {next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_utf32_code_point(
                     static_cast<char32_t>(0x11'0000), true
                 )
             ), u8"The code unit at the "
             u8"current iterator position (0x110000) of the UTF-32 input "
             u8"(\"<"
             u8"string>\", encoded using "
             u8"wchar_t) passed to the function decodes to "
             u8"U+110000. However, U+110000 falls outside the valid "
             u8"Unicode "
             u8"range "
             u8"— "
             u8"valid Unicode scalar values must be inclusively between "
             u8"U+0000 "
             u8"and "
             u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. "
             u8"As "
             u8"U+110000 exceeds the maximum valid codepoint, it cannot "
             u8"represent "
             u8"a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", U"<string>"},
            {next_char32_error_factory::mk_forward_error(
                 forward_scan_unicode_error_factory::invalid_utf32_code_point(
                     static_cast<char32_t>(0x11'0000), true
                 )
             ), u8"The code unit at the current iterator position (0x110000) "
             u8"of "
             u8"the UTF-32 input (encoded "
             u8"using "
             u8"wchar_t) passed to the function decodes to "
             u8"U+110000. However, U+110000 falls outside the valid "
             u8"Unicode "
             u8"range "
             u8"— "
             u8"valid Unicode scalar values must be inclusively between "
             u8"U+0000 "
             u8"and "
             u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. "
             u8"As "
             u8"U+110000 exceeds the maximum valid codepoint, it cannot "
             u8"represent "
             u8"a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", std::nullopt},
            {next_char32_error_factory::iterator_exhausted(),
             u8"The current iterator passed to the function was equal to "
             u8"the "
             u8"end iterator — "
             u8"signifying that there were no more code units to read, and "
             u8"the "
             u8"function was terminated.", std::nullopt},
            {next_char32_error_factory::iterator_exhausted(),
             u8"The current iterator passed to the function was equal to "
             u8"the "
             u8"end iterator — "
             u8"signifying that there were no more code units to read, and "
             u8"the "
             u8"function was terminated.", U"strings!"},
    };

    vector<tuple<next_char32_error, u8string, optional<basic_string<CharT>>>>
         rv;
    auto fill_vector_func = [&]<typename T>(T list_arg)
    {
        for (auto& [error, output_str, opt_original_str] : list_arg)
        {
            if (opt_original_str.has_value())
            {
                rv.push_back(make_tuple(
                    error,
                    output_str,
                    make_optional(spl_convert<CharT>(opt_original_str.value()))
                ));
            }
            else
            {
                rv.push_back(make_tuple(error, output_str, std::nullopt));
            }
        }
    };
    if constexpr (same_as<char8_t, CharT>)
    {
        fill_vector_func(u8_input_output_pairs);
    }
    else if constexpr (same_as<char16_t, CharT>)
    {
        fill_vector_func(u16_input_output_pairs);
    }
    else if constexpr (same_as<char32_t, CharT>)
    {
        fill_vector_func(u32_input_output_pairs);
    }
    else if constexpr (same_as<wchar_t, CharT>)
    {
        if constexpr (sizeof(CharT) == 2)
        {
            fill_vector_func(u16_input_output_pairs);
        }
        else if constexpr (sizeof(CharT) == 4)
        {
            fill_vector_func(u32_input_output_pairs);
        }
    }
    return rv;
}
} // namespace unicode_bridge_test_cases

TEST_CASE(
    "next_char32_error error message tests",
    "[next_char32]"
)
{
    using namespace unicode_bridge;
    using namespace unicode_bridge::internal;
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    auto local_test_errors = [](auto& input_output_pairs_arg)
    {
        using namespace std;
        for (auto&& [error, expected_msg, input_msg] : input_output_pairs_arg)
        {
            auto     opt = input_msg;
            u8string result;
            if (opt.has_value())
            {
                auto sv_start = std::begin(opt.value());
                auto sv_end   = std::end(opt.value());
                result        = error.message(sv_start, sv_end);
            }
            else
            {
                result = error.message();
            }
            if (result != expected_msg)
            {
                INFO(
                    "error.message() = \""
                    << convert_unicode_to_string(result)
                    << "\", expected_msg = \""
                    << convert_unicode_to_string(expected_msg) << "\""
                );
                result = error.message();
                REQUIRE(result == expected_msg);
            }
            REQUIRE(result == expected_msg);
        }
    };
    auto ki = get_next_char32_error_message<char8_t>();
    local_test_errors(ki);
    auto k2 = get_next_char32_error_message<char16_t>();
    local_test_errors(k2);
    auto k3 = get_next_char32_error_message<char32_t>();
    local_test_errors(k3);
    auto k4 = get_next_char32_error_message<wchar_t>();
    local_test_errors(k4);
}

namespace unicode_bridge_test_cases
{
template <typename CharT>
std::vector<std::tuple<
    unicode_bridge::prev_char32_error,
    std::u8string,
    std::optional<std::basic_string<CharT>>>>
    get_prev_char32_error_message()
{
    using namespace std;
    using namespace unicode_bridge;
    using namespace unicode_bridge::internal;
    using namespace unicode_bridge_testing;
    vector<tuple<prev_char32_error, u8string, optional<u8string>>>
        u8_input_output_pairs = {
            {prev_char32_error_factory::no_valid_leading_byte_found(
                 {static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0xA9),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0xBF)}
             ), u8"The four code units immediately preceding the current "
             u8"iterator "
             u8"position ([0x80, 0xA9, 0x80, 0xBF], shown in their "
             u8"original "
             u8"left-to-right order) of the UTF-8 input "
             u8"(\"...context     \") passed to the function, "
             u8"were scanned backwards. No valid leading byte was found — "
             u8"a "
             u8"valid "
             u8"UTF-8 leading byte must be in one of the following ranges: "
             u8"0x00 to "
             u8"0x7F (single-byte), 0xC2 to 0xDF (two-byte), 0xE0 to 0xEF "
             u8"(three-byte), or 0xF0 to 0xF7 (four-byte). As none of the "
             u8"four "
             u8"code units fall within any of these ranges, and the "
             u8"maximum "
             u8"number "
             u8"of continuation bytes was scanned, it was determined that "
             u8"these "
             u8"code units cannot represent a valid Unicode scalar value, "
             u8"and "
             u8"the "
             u8"function was terminated.", u8" context     "},
            {prev_char32_error_factory::no_valid_leading_byte_found(
                 {static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0xA9),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0xBF)}
             ), u8"The four code units immediately preceding the current "
             u8"iterator "
             u8"position ([0x80, 0xA9, 0x80, 0xBF], shown in their "
             u8"original "
             u8"left-to-right order) of the UTF-8 input passed to the "
             u8"function, "
             u8"were scanned backwards. No valid leading byte was found — "
             u8"a "
             u8"valid "
             u8"UTF-8 leading byte must be in one of the following ranges: "
             u8"0x00 to "
             u8"0x7F (single-byte), 0xC2 to 0xDF (two-byte), 0xE0 to 0xEF "
             u8"(three-byte), or 0xF0 to 0xF7 (four-byte). As none of the "
             u8"four "
             u8"code units fall within any of these ranges, and the "
             u8"maximum "
             u8"number "
             u8"of continuation bytes was scanned, it was determined that "
             u8"these "
             u8"code units cannot represent a valid Unicode scalar value, "
             u8"and "
             u8"the "
             u8"function was terminated.", std::nullopt},
            {prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0x41),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             1
             ), u8"The two code units immediately preceding the current "
             u8"iterator "
             u8"position ([0x41, 0x80], shown in their original "
             u8"left-to-right "
             u8"order) of the UTF-8 input (\"...          \") "
             u8"passed to the function, "
             u8"form an invalid "
             u8"sequence. The first code unit is a single-byte ASCII "
             u8"character and should not be succeeded by any continuation "
             u8"bytes — however, one continuation "
             u8"byte was found succeeding it. As a single-byte character "
             u8"cannot "
             u8"be part of a multi-byte sequence, these code units cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0x41),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             2
             ), u8"The three code units immediately preceding the current "
             u8"iterator "
             u8"position ([0x41, 0x80, 0x80], shown in their original "
             u8"left-to-right order) of the UTF-8 input (\"...           "
             u8"\") "
             u8"passed to the function, form an invalid "
             u8"sequence. The first code unit is a single-byte ASCII "
             u8"character and should not be succeeded by any continuation "
             u8"bytes — however, two continuation "
             u8"bytes were found succeeding it. As a single-byte character "
             u8"cannot "
             u8"be part of a multi-byte sequence, these code units cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0x41),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             3
             ), u8"The four code units immediately preceding the current "
             u8"iterator "
             u8"position ([0x41, 0x80, 0x80, 0x80], shown in their "
             u8"original "
             u8"left-to-right order) of the UTF-8 input (\"...            "
             u8"\") "
             u8"passed to the function, "
             u8"form an invalid "
             u8"sequence. The first code unit is a single-byte ASCII "
             u8"character and should not be succeeded by any continuation "
             u8"bytes — however, three continuation "
             u8"bytes were found succeeding it. As a single-byte character "
             u8"cannot "
             u8"be part of a multi-byte sequence, these code units cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xC3),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             0
             ), u8"The code unit immediately preceding the current "
             u8"iterator position (0xC3) of the UTF-8 input (\"...         "
             u8"\") "
             u8"passed to the function, "
             u8"is invalid. The code unit is a valid leading byte "
             u8"indicating the start of a two-byte sequence, requiring "
             u8"exactly "
             u8"one continuation byte to succeed the leading byte. "
             u8"However, "
             u8"no continuation bytes were found. "
             u8"Therefore, this code unit cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xC3),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             2
             ), u8"The three code units immediately preceding the current "
             u8"iterator position ([0xC3, 0x80, 0x80], shown in their "
             u8"original "
             u8"left-to-right order) of the UTF-8 input (\"...           "
             u8"\") "
             u8"passed to the function, "
             u8"form an invalid "
             u8"sequence. The first code unit is a valid leading byte "
             u8"indicating the start of a two-byte sequence, requiring "
             u8"exactly "
             u8"one continuation byte to succeed the leading byte. "
             u8"However, "
             u8"two continuation bytes were found. "
             u8"Therefore, these code units cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xC3),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             3
             ), u8"The four code units immediately preceding the current "
             u8"iterator position ([0xC3, 0x80, 0x80, 0x80], shown in "
             u8"their "
             u8"original left-to-right order) of the UTF-8 input (\"...    "
             u8"    "
             u8"    \") "
             u8"passed to the function, "
             u8"form an invalid "
             u8"sequence. The first code unit is a valid leading byte "
             u8"indicating the start of a two-byte sequence, requiring "
             u8"exactly "
             u8"one continuation byte to succeed the leading byte. "
             u8"However, "
             u8"three continuation bytes were found. "
             u8"Therefore, these code units cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             0
             ), u8"The code unit immediately preceding the current "
             u8"iterator position (0xE0) of the UTF-8 input (\"...         "
             u8"\") "
             u8"passed to the function, "
             u8"is invalid. The code unit is a valid leading byte "
             u8"indicating the start of a three-byte sequence, requiring "
             u8"exactly "
             u8"two continuation bytes to succeed the leading byte. "
             u8"However, "
             u8"no continuation bytes were found. "
             u8"Therefore, this code unit cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             1
             ), u8"The two code units immediately preceding the current "
             u8"iterator position ([0xE0, 0x80], shown in their original "
             u8"left-to-right order) of the UTF-8 input (\"...          "
             u8"\") "
             u8"passed to the function, "
             u8"form an invalid "
             u8"sequence. The first code unit is a valid leading byte "
             u8"indicating the start of a three-byte sequence, requiring "
             u8"exactly "
             u8"two continuation bytes to succeed the leading byte. "
             u8"However, "
             u8"only one continuation byte was found. "
             u8"Therefore, these code units cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xE0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80)},
             3
             ), u8"The four code units immediately preceding the current "
             u8"iterator position ([0xE0, 0x80, 0x80, 0x80], shown in "
             u8"their "
             u8"original left-to-right order) of the UTF-8 input (\"...    "
             u8"    "
             u8"    \") "
             u8"passed to the function, "
             u8"form an invalid "
             u8"sequence. The first code unit is a valid leading byte "
             u8"indicating the start of a three-byte sequence, requiring "
             u8"exactly "
             u8"two continuation bytes to succeed the leading byte. "
             u8"However, "
             u8"three continuation bytes were found. "
             u8"Therefore, these code units cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             0
             ), u8"The code unit immediately preceding the current "
             u8"iterator position (0xF0) of the UTF-8 input (\"...         "
             u8"\") "
             u8"passed to the function, "
             u8"is invalid. The code unit is a valid leading byte "
             u8"indicating the start of a four-byte sequence, requiring "
             u8"exactly "
             u8"three continuation bytes to succeed the leading byte. "
             u8"However, "
             u8"no continuation bytes were found. "
             u8"Therefore, this code unit cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             1
             ), u8"The two code units immediately preceding the current "
             u8"iterator position ([0xF0, 0x80], shown in their original "
             u8"left-to-right order) of the UTF-8 input (\"...          "
             u8"\") "
             u8"passed to the function, "
             u8"form an invalid "
             u8"sequence. The first code unit is a valid leading byte "
             u8"indicating the start of a four-byte sequence, requiring "
             u8"exactly "
             u8"three continuation bytes to succeed the leading byte. "
             u8"However, "
             u8"only one continuation byte was found. "
             u8"Therefore, these code units cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::leading_byte_sequence_length_mismatch(
                 {static_cast<char8_t>(0xF0),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             2
             ), u8"The three code units immediately preceding the current "
             u8"iterator position ([0xF0, 0x80, 0x80], shown in their "
             u8"original "
             u8"left-to-right order) of the UTF-8 input (\"...           "
             u8"\") "
             u8"passed to the function, "
             u8"form an invalid "
             u8"sequence. The first code unit is a valid leading byte "
             u8"indicating the start of a four-byte sequence, requiring "
             u8"exactly "
             u8"three continuation bytes to succeed the leading byte. "
             u8"However, "
             u8"only two continuation bytes were found. "
             u8"Therefore, these code units cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::invalid_utf8_byte(
                 {static_cast<char8_t>(0xFF),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             0
             ), u8"The code unit immediately preceding the current iterator "
             u8"position (0xFF) of the UTF-8 input (\"...         \") "
             u8"passed to the function, "
             u8"was found to be invalid — a valid UTF-8 byte must be in "
             u8"one "
             u8"of the following ranges: 0x00 to 0x7F (single-byte leading "
             u8"byte), "
             u8"0x80 to 0xBF (continuation byte), 0xC2 to 0xDF (two-byte "
             u8"leading "
             u8"byte), 0xE0 to 0xEF (three-byte leading byte), or 0xF0 to "
             u8"0xF7 "
             u8"(four-byte leading byte). As 0xFF falls outside all of "
             u8"these "
             u8"ranges, "
             u8"it cannot appear in a valid UTF-8 sequence, and the "
             u8"function "
             u8"was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::invalid_utf8_byte(
                 {static_cast<char8_t>(0xFF),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00)},
             2
             ), u8"The three code units immediately preceding the current "
             u8"iterator position ([0xFF, 0x80, 0x80], shown in their "
             u8"original "
             u8"left-to-right order) of the UTF-8 input (\"...           "
             u8"\") "
             u8"passed to the function, were "
             u8"found to be invalid. After finding two continuation bytes, "
             u8"the "
             u8"next code unit "
             u8"(0xFF) was found to be invalid — a valid UTF-8 byte must "
             u8"be in "
             u8"one "
             u8"of the following ranges: 0x00 to 0x7F (single-byte leading "
             u8"byte), "
             u8"0x80 to 0xBF (continuation byte), 0xC2 to 0xDF (two-byte "
             u8"leading "
             u8"byte), 0xE0 to 0xEF (three-byte leading byte), or 0xF0 to "
             u8"0xF7 "
             u8"(four-byte leading byte). As 0xFF falls outside all of "
             u8"these "
             u8"ranges, "
             u8"it cannot appear in a valid UTF-8 sequence, and the "
             u8"function "
             u8"was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::invalid_utf8_byte(
                 {static_cast<char8_t>(0xFF),
                  static_cast<char8_t>(0x80),
                  static_cast<char8_t>(0x00),
                  static_cast<char8_t>(0x00)},
             1
             ), u8"The two code units immediately preceding the current "
             u8"iterator position ([0xFF, 0x80], shown in their original "
             u8"left-to-right order) of the UTF-8 input (\"...          "
             u8"\") "
             u8"passed to the function, were "
             u8"found to be invalid. After finding one continuation byte, "
             u8"the "
             u8"next code "
             u8"unit "
             u8"(0xFF) was found to be invalid — a valid UTF-8 byte must "
             u8"be "
             u8"in one "
             u8"of the following ranges: 0x00 to 0x7F (single-byte leading "
             u8"byte), "
             u8"0x80 to 0xBF (continuation byte), 0xC2 to 0xDF (two-byte "
             u8"leading "
             u8"byte), 0xE0 to 0xEF (three-byte leading byte), or 0xF0 to "
             u8"0xF7 "
             u8"(four-byte leading byte). As 0xFF falls outside all of "
             u8"these "
             u8"ranges, "
             u8"it cannot appear in a valid UTF-8 sequence, and the "
             u8"function "
             u8"was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::
                 invalid_utf32_code_point_after_utf8_conversion(
                     {static_cast<char8_t>(0xED),
                      static_cast<char8_t>(0xA0),
                      static_cast<char8_t>(0x80),
                      u8'\0'},
             3, U'\xD800'
                 ), u8"The three code units immediately preceding the current "
             u8"iterator position ([0xED, 0xA0, "
             u8"0x80], shown in their original left-to-right order) of the "
             u8"UTF-8 input (\"...           \") passed to the "
             u8"function, form a three-byte sequence encoding U+D800. "
             u8"However, U+D800 falls outside the valid Unicode range — "
             u8"valid "
             u8"Unicode scalar values must be inclusively between U+0000 "
             u8"and "
             u8"U+10FFFF, excluding the surrogate range U+D800 to U+DFFF. "
             u8"As "
             u8"U+D800 falls within the surrogate range, it cannot "
             u8"represent "
             u8"a valid Unicode scalar value, and the function was "
             u8"terminated.", u8"           context              "},
            {prev_char32_error_factory::
                 iterator_begin_reached_before_leading_byte(
                     {static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0x00),
                      static_cast<char8_t>(0x00),
                      static_cast<char8_t>(0x00)},
             1
                 ), u8"The code unit immediately preceding the current iterator "
             u8"position (0x80) of the UTF-8 input passed to the function, "
             u8"was "
             u8"scanned "
             u8"backwards. It is a continuation byte, but the beginning of "
             u8"the "
             u8"input was reached before a valid leading byte was found. A "
             u8"valid "
             u8"UTF-8 leading byte must be in one of the following ranges: "
             u8"0x00 to "
             u8"0x7F (single-byte), 0xC2 to 0xDF (two-byte), 0xE0 to 0xEF "
             u8"(three-byte), or 0xF0 to 0xF7 (four-byte). As the input "
             u8"ends "
             u8"before such a byte is found, this code unit cannot "
             u8"represent a "
             u8"valid Unicode scalar value, and the function was "
             u8"terminated.", std::nullopt},
            {prev_char32_error_factory::
                 iterator_begin_reached_before_leading_byte(
                     {static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0xA0),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0x00)},
             3
                 ), u8"The three code units immediately preceding the current "
             u8"iterator "
             u8"position ([0x80, 0xA0, 0xBF], shown in their original "
             u8"left-to-right order) of the UTF-8 input (\"...e bad "
             u8"bytes\") "
             u8"passed to the function, were scanned backwards. They are "
             u8"continuation bytes, but the beginning of the input was "
             u8"reached "
             u8"before a valid leading byte was found. A valid UTF-8 "
             u8"leading "
             u8"byte "
             u8"must be in one of the following ranges: 0x00 to 0x7F "
             u8"(single-byte), "
             u8"0xC2 to 0xDF (two-byte), 0xE0 to 0xEF (three-byte), or "
             u8"0xF0 to "
             u8"0xF7 "
             u8"(four-byte). As the input ends before such a byte is "
             u8"found, "
             u8"these "
             u8"code units cannot represent a valid Unicode scalar value, "
             u8"and "
             u8"the "
             u8"function was terminated.", u8"context follows after the bad bytes"},
            {prev_char32_error_factory::
                 iterator_begin_reached_before_leading_byte(
                     {static_cast<char8_t>(0x80),
                      static_cast<char8_t>(0xBF),
                      static_cast<char8_t>(0x00),
                      static_cast<char8_t>(0x00)},
             2
                 ), u8"The two code units immediately preceding the current "
             u8"iterator "
             u8"position ([0x80, 0xBF], shown in their original "
             u8"left-to-right "
             u8"order) of the UTF-8 input (\"hi there\") passed "
             u8"to the function, were scanned backwards. They are "
             u8"continuation "
             u8"bytes, but the beginning of the input was reached before a "
             u8"valid "
             u8"leading byte was found. A valid UTF-8 leading byte must be "
             u8"in "
             u8"one "
             u8"of the following ranges: 0x00 to 0x7F (single-byte), 0xC2 "
             u8"to "
             u8"0xDF "
             u8"(two-byte), 0xE0 to 0xEF (three-byte), or 0xF0 to 0xF7 "
             u8"(four-byte). "
             u8"As the input ends before such a byte is found, these code "
             u8"units "
             u8"cannot represent a valid Unicode scalar value, and the "
             u8"function "
             u8"was terminated.", u8"hi there"},
    };

    vector<tuple<prev_char32_error, u8string, optional<u16string>>>
        u16_input_output_pairs = {
            {prev_char32_error_factory::low_surrogate_then_start_of_stream(
                 static_cast<char16_t>(0xDC00), true
             ), u8"The code unit immediately preceding the current iterator "
             u8"position (0xDC00) "
             u8"of "
             u8"the UTF-16 input (\"...t        \", encoded using wchar_t) "
             u8"passed to the function, is a low surrogate. Low "
             u8"surrogates must always be preceded by a high surrogate "
             u8"(inclusively "
             u8"between 0xD800 and 0xDBFF) as the second part of a "
             u8"surrogate "
             u8"pair. "
             u8"However, this low surrogate was found at the start of the "
             u8"stream — "
             u8"no preceding code unit exists. As it cannot be the second "
             u8"part "
             u8"of a "
             u8"surrogate pair, it cannot represent a valid Unicode scalar "
             u8"value, and the function was terminated.", u"context        "},
            {prev_char32_error_factory::
                 low_surrogate_not_preceded_by_high_surrogate(
                     static_cast<char16_t>(0x0041),
             static_cast<char16_t>(0xDC00),
             false
                 ), "The two code units immediately preceding the current "
             "iterator "
             u8"position ([0x0041, 0xDC00], shown in their original "
             u8"left-to-right order) of the UTF-16 input "
             u8"(\"...ABCDEFGHIJ\") "
             u8"passed to the function, form the end of a surrogate pair. "
             u8"The "
             u8"second "
             u8"code unit (0xDC00) is a low surrogate, which must be "
             u8"preceded "
             u8"by a "
             u8"high surrogate inclusively between 0xD800 and 0xDBFF. "
             u8"However, "
             u8"the "
             u8"first code unit (0x0041) falls outside this range, and "
             u8"therefore "
             u8"the "
             u8"two code units cannot represent a valid Unicode scalar "
             u8"value, "
             u8"and "
             u8"the "
             u8"function was terminated.", u"123456789ABCDEFGHIJ"},
            {prev_char32_error_factory::unexpected_high_surrogate(
                 static_cast<char16_t>(0xD800), false
             ), u8"The code unit immediately preceding the current iterator "
             u8"position (0xD800) "
             u8"of the "
             u8"UTF-16 input (\"...BCDEFGHIJ\") passed to the function, "
             u8"is a high "
             u8"surrogate. When scanned forwards, high surrogates must "
             u8"always "
             u8"be followed by a low "
             u8"surrogate (inclusively between 0xDC00 and 0xDFFF). This "
             u8"high "
             u8"surrogate was encountered "
             u8"while scanning backwards, and no low surrogate was "
             u8"previously "
             u8"encountered. Therefore, "
             u8"it is not part of a surrogate pair, and cannot represent a "
             u8"valid Unicode scalar value, and the "
             u8"function "
             u8"was terminated.", u"123456789ABCDEFGHIJ"},
    };
    vector<tuple<prev_char32_error, u8string, optional<u32string>>>
        u32_input_output_pairs = {

        };

    vector<tuple<prev_char32_error, u8string, optional<basic_string<CharT>>>>
         rv;
    auto fill_vector_func = [&]<typename T>(T list_arg)
    {
        for (auto& [error, output_str, opt_original_str] : list_arg)
        {
            if (opt_original_str.has_value())
            {
                rv.push_back(make_tuple(
                    error,
                    output_str,
                    make_optional(spl_convert<CharT>(opt_original_str.value()))
                ));
            }
            else
            {
                rv.push_back(make_tuple(error, output_str, std::nullopt));
            }
        }
    };
    if constexpr (same_as<char8_t, CharT>)
    {
        fill_vector_func(u8_input_output_pairs);
    }
    else if constexpr (same_as<char16_t, CharT>)
    {
        fill_vector_func(u16_input_output_pairs);
    }
    else if constexpr (same_as<char32_t, CharT>)
    {
        fill_vector_func(u32_input_output_pairs);
    }
    else if constexpr (same_as<wchar_t, CharT>)
    {
        if constexpr (sizeof(CharT) == 2)
        {
            fill_vector_func(u16_input_output_pairs);
        }
        else if constexpr (sizeof(CharT) == 4)
        {
            fill_vector_func(u32_input_output_pairs);
        }
    }
    return rv;
}
} // namespace unicode_bridge_test_cases

TEST_CASE(
    "prev_char32_error error message tests",
    "[prev_char32_error]"
)
{
    using namespace unicode_bridge;
    using namespace unicode_bridge::internal;
    using namespace std;
    using namespace unicode_bridge_testing;
    using namespace unicode_bridge_test_cases;
    auto local_test_errors = [](auto& input_output_pairs_arg)
    {
        using namespace std;
        for (auto&& [error, expected_msg, input_msg] : input_output_pairs_arg)
        {
            auto     opt = input_msg;
            u8string result;
            if (opt.has_value())
            {
                auto sv_start = std::begin(opt.value());
                auto sv_end   = std::end(opt.value());
                result        = error.message(sv_end, sv_start);
            }
            else
            {
                result = error.message();
            }
            if (result != expected_msg)
            {
                INFO(
                    "error.message() = \""
                    << convert_unicode_to_string(result)
                    << "\", expected_msg = \""
                    << convert_unicode_to_string(expected_msg) << "\""
                );
                REQUIRE(result == expected_msg);
            }
            REQUIRE(result == expected_msg);
        }
    };
    auto ki = get_prev_char32_error_message<char8_t>();
    local_test_errors(ki);
    auto k2 = get_prev_char32_error_message<char16_t>();
    local_test_errors(k2);
    auto k3 = get_prev_char32_error_message<char32_t>();
    local_test_errors(k3);
    auto k4 = get_prev_char32_error_message<wchar_t>();
    local_test_errors(k4);
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