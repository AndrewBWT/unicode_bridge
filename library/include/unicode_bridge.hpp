#pragma once
#include <algorithm>
#include <array>
#include <expected>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>


#define UNICODE_BRIDGE_NAMESPACE          unicode_bridge
#define UNICODE_BRIDGE_NAMESPACE_INTERNAL internal

#define UNICODE_BRIDGE_NS_BEGIN        \
    namespace UNICODE_BRIDGE_NAMESPACE \
    {
#define UNICODE_BRIDGE_NS_END }

#define UNICODE_BRIDGE_INTERNAL_NS_BEGIN        \
    namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL \
    {
#define UNICODE_BRIDGE_INTERNAL_NS_END }
template <typename>
constexpr bool dependent_false = false; // workaround before CWG2518/P2593R1
#define UNICODE_BRIDGE_STATIC_ASSERT(type_arg, msg_arg) \
    static_assert(dependent_false<type_arg>, msg_arg);

UNICODE_BRIDGE_NS_BEGIN
/*!
 * @brief Describes a character-type that can hold unicode-data. Specifically,
 char8_t, char16_t, char32_t and wchar_t.
 */
template <typename CharT>
concept char_type_is_unicode_c
    = std::same_as<CharT, char8_t> || std::same_as<CharT, char16_t>
      || std::same_as<CharT, char32_t> || std::same_as<wchar_t, CharT>;

/*!
 * @brief Describes a character-type in C++. Specifically, char,
 char8_t, char16_t, char32_t and wchar_t.
 */
template <typename CharT>
concept is_char_type_c
    = char_type_is_unicode_c<CharT> || std::same_as<CharT, char>;

UNICODE_BRIDGE_INTERNAL_NS_BEGIN
/*!
 * @brief Primary template used to obtain the underlying character type of some
 * string-like object.
 */
template <typename CharT>
struct char_type_of;

/*!
 * @brief Partial specialization of char_type_of for a string-like char-star
 * object.
 */
template <typename CharT>
struct char_type_of<CharT*>
{
    using type = CharT;
};

/*!
 * @brief Partial specialization of char_type_of for a string-like const
 * char-star object.
 */
template <typename CharT>
struct char_type_of<const CharT*>
{
    using type = CharT;
};

/*!
 * @brief Partial specialization of char_type_of for a string-like container
 * object, with an associated type T::value_type.
 */
template <typename String_Type>
requires requires { typename String_Type::value_type; }
struct char_type_of<String_Type>
{
    using type = typename String_Type::value_type;
};

UNICODE_BRIDGE_INTERNAL_NS_END
/*!
 * @brief Alias template for obtaining the character type of some string-like
 * object.
 * @tparam T The string-like object type.
 */
template <typename String_Like_Type>
using char_type_of_t = typename UNICODE_BRIDGE_NAMESPACE_INTERNAL::char_type_of<
    String_Like_Type>::type;
// Forward declaration
UNICODE_BRIDGE_INTERNAL_NS_BEGIN
struct unicode_conversion_error_factory;
struct unicode_bridge_exception_factory;
UNICODE_BRIDGE_INTERNAL_NS_END

/*!
 * @brief Holds generic unicode errors, specifically those pertaining to the
 * unicode standard.
 *
 * It is public as other errors include instances of it.
 */
struct unicode_conversion_error
{
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        unicode_conversion_error_factory;
    /*!
     * @brief Enum type describing the various types of encoding erros that can
     * be found in a unicode string.
     */
    enum class unicode_conversion_error_code : uint8_t
    {
        // UTF-8 errors
        /*!
         * UTF8 strings are in groups of 1-4 bytes. If a 1 byte code point, in
         * binary its in the form 0xxxxxxx. If in 2-4 bytes, the first byte is
         * in the form 110xxxxx, 1110xxxx or 11110xxx. If that first byte is not
         * in any of these forms, it is an invalid_leading_byte.
         */
        invalid_leading_byte,
        /*!
         * @brief In a 2-4 byte code point, the format of the first byte denotes
         * how many code points are in that code point.
         *
         * 110xxxxx -> 2
         * 1110xxxx -> 3
         * 11110xxx -> 4
         *
         * If there are insufficient bytes in the string, then it is flagged as
         * a truncated_sequence.
         */
        truncated_sequence,
        /*!
         * @brief In a 2-4 byte code point, the format of the 2nd to
         * (potentially) 4th bytes is in the form 10xxxxxx.
         *
         *
         * If any of these bytes (called continuation bytes) are not in this
         * format, this error is thrown.
         */
        invalid_continuation_byte,
        /*!
         * @brief In UTF8, it is possible to encode a unicode character in
         * several different ways using the 4-bytes. For example, using a single
         * byte.
         *
         * 0111111.
         *
         * And 2 bytes.
         *
         * 11000001 10111111.
         *
         * Encode the same unicode character. The smallest encoding should
         * always be used - anything else is an error. overlong_encoding is used
         * to represent this error.
         */
        overlong_encoding,
        /*!
         * @brief UTF8 encodes UTF32 characters using between 1 and 4 bytes.
         * However, not all UTF32 characters are valid unicode.
         *
         * For example, the unicode character 0000DFFF is invalid unicode,
         * however it can be encoded in UTF8.
         *
         * invalid_utf32_code_point_after_utf8_conversion is used to represent
         * this error.
         *
         */
        invalid_utf32_code_point_after_utf8_conversion,

        // UTF-16 errors
        /*!
         * In UTF16, unicode characters are either single code points, or pairs
         * of code points. The pairs are high surroates followed by low
         * surrogats.
         *
         * If a high surrogate is followed by the end of the stream, then
         * high_surrogate_then_end_of_stream is used.
         */
        high_surrogate_then_end_of_stream,
        /*!
         * In UTF16, unicode characters are either single code points, or pairs
         * of code points. The pairs are high surroates followed by low
         * surrogats.
         *
         * If a high surrogate is not followed by a low surrogate, then
         * high_surrogate_not_followed_by_low_surrogate is used.
         */
        high_surrogate_not_followed_by_low_surrogate,
        /*!
         * In UTF16, unicode characters are either single code points, or pairs
         * of code points. The pairs are high surroates followed by low
         * surrogats.
         *
         * If a low surrogate is encountered unexpectadly (e.g. at the start of
         * a string object), then unexpected_low_surrogate is used.
         */
        unexpected_low_surrogate,

        // UTF-32 errors
        /*!
         * In UTF32, unicode characters are single code points.
         *
         * Not all UFF32 characters are valid unicode characters. If one is
         * encountered, then invalid_utf32_code_point is raised.
         */
        invalid_utf32_code_point,
        /*!
         * @brief Represents no error. Used as placeholder in types which
         * include a unicode_error_code, but don't use it.
         */
        no_error
    };
private:
    unicode_conversion_error_code _code;
    std::size_t                   _character_index;
    // Data members for UTF-8 errors.
    std::array<char8_t, 4> _u8_code_points;
    std::uint8_t           _code_points_encountered;
    // This data point has a different use when dealing with continuation byte
    // errors. In that case, it acts like an enum with the following
    // meaning: 0 -> 2 byte unicode scalar value. 2nd byte is invalid
    // continuation byte. 1 -> 3 byte unicode scalar value. 2nd byte is invalid
    // continuation byte. 2 -> 3 byte unicode scalar value. 3rd byte is invalid
    // continuation byte. 3 -> 3 byte unicode scalar value. 2nd and 3rd byte are
    // invalid continuation bytes.

    // 4 -> 4 byte unicode scalar value. 2nd byte is invalid continuation byte.
    // 5 -> 4 byte unicode scalar value. 3rd byte is invalid continuation byte.
    // 6 -> 4 byte unicode scalar value. 4th byte is invalid continuation byte.

    // 7 -> 4 byte unicode scalar value. 2nd and 3rd byte are invalid
    // continuation bytes.
    // 8 -> 4 byte unicode scalar value. 2nd and 4th byte are invalid
    // continuation bytes.
    // 9 -> 4 byte unicode scalar value. 3rd and 4th byte
    // are invalid continuation bytes.
    // 10 -> 4 byte unicode scalar value. 2nd,
    // 3rd and 4th byte are invalid continuation bytes.
    std::uint8_t            _expected_code_points_size;
    std::array<char16_t, 2> _u16_code_points;
    char32_t                _char32_character;
    bool                    _is_wchar;

    constexpr unicode_conversion_error(
        const unicode_conversion_error_code code_arg,
        const std::size_t                   character_index_arg,
        const std::array<char8_t, 4>&       u8_code_points_arg,
        const std::uint8_t                  code_points_encountered_arg,
        const std::uint8_t                  expected_code_points_size_arg,
        const std::array<char16_t, 2>&      u16_code_points_arg,
        char32_t                            char32_character_arg,
        const bool                          is_wchar_arg
    ) noexcept;
public:
    // All getters for internal data.
    constexpr const unicode_conversion_error_code
        code() const noexcept;
    constexpr const std::size_t
        character_index() const noexcept;

    constexpr const std::array<char8_t, 4>&
        u8_code_points() const noexcept;

    constexpr const std::size_t
        code_points_encountered() const noexcept;

    constexpr const std::size_t
        expected_code_points_size() const noexcept;

    constexpr const std::array<char16_t, 2>&
        u16_code_points() const noexcept;

    constexpr const char32_t
        char32_character() const noexcept;

    constexpr const bool
        is_wchar() const noexcept;
    /*!
     * @brief Gets message for error. Note that as this error is not returned by
     * any function, it has some placeholders which allow it to create generic
     * error messages, which are used by other functions.
     */
    constexpr std::u8string
        message(
            const std::u8string_view                function_name_arg,
            const std::u8string_view                additional_message_arg,
            const std::optional<std::u8string_view> opt_complete_string_arg
        ) const;
};

/*!
 * @brief Generic exception type for the exception throwing functions.
 * @tparam T
 */
template <typename Error_Type>
class unicode_bridge_exception : public std::exception
{
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        unicode_bridge_exception_factory;
public:
    const char*
        what() const noexcept override;

    constexpr const Error_Type&
        error() const noexcept;
private:
    constexpr explicit unicode_bridge_exception(
        const Error_Type&        error_arg,
        const std::u8string_view function_name_arg
    ) noexcept;

    Error_Type          _error;
    std::u8string       _function_name;
    mutable std::string _what_cache;
};

UNICODE_BRIDGE_INTERNAL_NS_BEGIN
struct ascii_to_unicode_error_factory;
UNICODE_BRIDGE_INTERNAL_NS_END

/*!
 * @brief Error for when a unicode_to_ascii function fails.
 */
struct ascii_to_unicode_error
{
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        ascii_to_unicode_error_factory;
private:
    constexpr ascii_to_unicode_error(
        const std::size_t index_arg,
        const char        character_arg
    ) noexcept;
public:
    constexpr char
        get_character() const noexcept;

    constexpr std::size_t
        get_index() const noexcept;

    std::u8string
        message(
            const std::u8string_view                function_name_arg,
            const std::u8string_view                additional_message_arg,
            const std::optional<std::u8string_view> complete_string_arg
        ) const;
private:
    char        _character;
    std::size_t _index;
};

UNICODE_BRIDGE_INTERNAL_NS_BEGIN
struct unicode_to_ascii_error_factory;
UNICODE_BRIDGE_INTERNAL_NS_END

struct unicode_to_ascii_error
{
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        unicode_to_ascii_error_factory;

    enum unicode_to_ascii_error_code
    {
        non_ascii_character_found,
        invalid_unicode_character
    };
private:
    constexpr unicode_to_ascii_error(
        const unicode_to_ascii_error_code enum_arg,
        const unicode_conversion_error&   error_arg,
        const char32_t                    character_arg,
        const std::size_t                 index_arg
    ) noexcept;
public:
    constexpr const unicode_to_ascii_error_code
        get_enum() const noexcept;

    constexpr const std::size_t
        index() const noexcept;

    constexpr const char32_t
        character() const noexcept;

    constexpr const unicode_conversion_error&
        error() const noexcept;

    std::u8string
        message(
            const std::u8string_view                function_name_arg,
            const std::u8string_view                additional_message_arg,
            const std::optional<std::u8string_view> complete_string_arg
        ) const;
private:
    unicode_to_ascii_error_code _enum;
    unicode_conversion_error    _error;
    char32_t                    _character;
    std::size_t                 _index;
};

UNICODE_BRIDGE_INTERNAL_NS_BEGIN
struct next_char32_error_factory;
UNICODE_BRIDGE_INTERNAL_NS_END

class next_char32_error
{
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::next_char32_error_factory;
public:
    enum class next_char32_error_code
    {
        unicode_error,
        iterator_end
    };
private:
    constexpr next_char32_error(
        const next_char32_error_code    enum_arg,
        const unicode_conversion_error& error_arg
    ) noexcept;
public:
    constexpr const next_char32_error_code
        get_enum() const noexcept;

    constexpr const unicode_conversion_error&
        error() const noexcept;

    std::u8string
        message(
            const std::u8string_view                function_name_arg,
            const std::u8string_view                additional_message_arg,
            const std::optional<std::u8string_view> complete_string_arg
        ) const;
private:
    next_char32_error_code   _enum;
    unicode_conversion_error _error;
};

template <typename T>
using next_char32_result = std::expected<T, next_char32_error>;

/*!
 * @brief Type synonym for the result type from unicode_to_ascii..
 * @tparam T
 */
template <typename T>
using unicode_to_ascii_result = std::expected<T, unicode_to_ascii_error>;
template <typename T>
using ascii_to_unicode_result = std::expected<T, ascii_to_unicode_error>;
template <typename T>
using unicode_conversion_result = std::expected<T, unicode_conversion_error>;
/*!
 * @brief This function converts a unicode string to an ASCII string.
 *
 * If any of the characters are outside the ASCII range, then the function
 * returns an appropriate error message. Otherwise, it returns the
 * constructed std::string.
 *
 * @tparam ArgType The parameter of the argument type. It should be some
 * string-like object containing unicode characters. The concept is designed
 * to take any type convertable to a std::basic_string_view.
 * @param str_arg The input argument.
 * @return Either the output string, or an appropriate failure message.
 */
template <typename ArgType>
requires char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr unicode_to_ascii_result<std::string>
    convert_unicode_to_ascii(ArgType str_arg) noexcept;
template <typename InputChar>
requires char_type_is_unicode_c<InputChar>
constexpr unicode_to_ascii_result<std::string>
    convert_unicode_to_ascii(const InputChar char_arg) noexcept;

/*!
 * @brief This function converts a unicode string to an ASCII string.
 *
 * If any of the characters are outside the ASCII range, then the function
 * returns an appropriate error message. Otherwise, it returns the
 * constructed std::string. Same as previous function, except this takes a
 * string_view as the argument.
 *
 * @tparam T The character type of the input string.
 * @param str_arg The input string.
 * @return Either the output string, or an appropriate failure message.
 */
template <typename ArgType>
requires char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr std::string
    convert_unicode_to_ascii_with_exception(ArgType str_arg);
template <typename InputChar>
requires char_type_is_unicode_c<InputChar>
constexpr std::string
    convert_unicode_to_ascii_with_exception(const InputChar char_arg);

/*!
 * @brief Converts an ASCII string to unicode.
 *
 * If any of the characters from the input are outside the basic ASCII
 * range, this function will return an appropriate error message.
 *
 * @tparam T The type of character type to use in the output.
 * @param str_arg The input std::string object.
 * @return Either a std::basic_string<T> representing str_arg, or an
 * appropriate error message.
 */

template <typename T, typename ArgType>
requires char_type_is_unicode_c<T>
         && std::same_as<char, char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr ascii_to_unicode_result<std::basic_string<T>>
    convert_ascii_to_unicode(ArgType str_arg) noexcept;

template <typename OutputChar>
requires char_type_is_unicode_c<OutputChar>
constexpr ascii_to_unicode_result<std::basic_string<OutputChar>>
    convert_ascii_to_unicode(const char char_arg) noexcept;

template <typename T, typename ArgType>
requires char_type_is_unicode_c<T>
         && std::same_as<char, char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr std::basic_string<T>
    convert_ascii_to_unicode_with_exception(ArgType str_arg);

template <typename OutputChar>
requires char_type_is_unicode_c<OutputChar>
constexpr std::basic_string<OutputChar>
    convert_ascii_to_unicode_with_exception(const char char_arg);

/*!
 * @brief Converts a unicode string from one format to another.
 *
 * If the input unicode string is invalid, then this function returns
 * expected value with an error message in it. Otherwise, it returns the
 * converted string.
 *
 * @tparam T The internal character type of the output argument.
 * @tparam U The internal character type of the input argument.
 * @param str_arg The input argument.
 * @return An expected value containing either the converted output, or an
 * error message.
 */

template <typename OutputChar, typename ArgType>
requires char_type_is_unicode_c<OutputChar>
         && char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr unicode_conversion_result<std::basic_string<OutputChar>>
    unicode_conversion(ArgType str_arg) noexcept;
template <typename OutputChar, typename InputChar>
requires char_type_is_unicode_c<OutputChar> && char_type_is_unicode_c<InputChar>
constexpr unicode_conversion_result<std::basic_string<OutputChar>>
    unicode_conversion(const InputChar char_arg) noexcept;

template <typename OutputChar, typename ArgType>
requires char_type_is_unicode_c<OutputChar>
         && char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr std::basic_string<OutputChar>
    unicode_conversion_with_exception(ArgType str_arg);
template <typename OutputChar, typename InputChar>
requires char_type_is_unicode_c<OutputChar> && char_type_is_unicode_c<InputChar>
constexpr std::basic_string<OutputChar>
    unicode_conversion_with_exception(const InputChar char_arg);
/*!
 * @brief Checks if a unicode string is valid.
 *
 * @param str_arg The string to check.
 * @return True if str_arg is valid, false otherwise.
 */
template <typename ArgType>
requires char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr bool
    is_valid_unicode(ArgType str_arg) noexcept;
/*!
 * @brief Checks if a character is valid unicode on its own.
 *
 * That is to say, if its a char8, checks that it is in the ascii range. If
 * char16_t, that it is not a surrogate character, and a char32_t that it is
 * valid.
 *
 * @param str_arg The character to check
 * @return True if char_arg is valid, false otherwise.
 */
template <typename InputChar>
requires char_type_is_unicode_c<InputChar>
constexpr bool
    is_valid_unicode(const InputChar char_arg) noexcept;
/*!
 * @brief Checks if a character is valid ascii.
 *
 * @param char_arg The character to check.
 * @return True if char_arg is valid, false otherwise.
 */
template <typename ArgType>
requires is_char_type_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr bool
    is_valid_ascii(ArgType str_arg) noexcept;
template <typename InputChar>
requires is_char_type_c<InputChar>
constexpr bool
    is_valid_ascii(const InputChar char_arg) noexcept;

/*!
 * @brief Gets the next character from a stream of characters.
 *
 * This function assumes iterator_arg and itt_end_arg are part of the same
 * iterator.
 *
 * @tparam T The type of the iterators iterator_arg and itt_end_arg.
 * @tparam Return_Reason A boolean parameter which controls the return type;
 * if its true, then the return type is an std::expected of pair of char32_t
 * and size. If false an optional of pair of char32_t and size. The char32_t
 * is the next char32_t character from the stream, and the size is the
 * number of elements of the iterator iterator_arg which were read to create
 * this character.
 * @param iterator_arg The current iterator.
 * @param itt_end_arg The end iterator.
 * @return See above.
 */
template <bool Return_Reason, typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::conditional_t<
    Return_Reason,
    next_char32_result<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    next_char32(
        const ItteratorType iterator_arg,
        const ItteratorType itt_end_arg
    ) noexcept;
/*!
 * @brief Gets the next character from a stream of characters.
 *
 * This function assumes iterator_arg and itt_end_arg are part of the same
 * iterator.
 *
 * @tparam T The type of the iterators iterator_arg and itt_end_arg.
 * @tparam Return_Reason A boolean parameter which controls the return type;
 * if its true, then the return type is an std::expected of char32_t. If
 * false an optional of char32_t. The char32_t is the next char32_t
 * character from the stream.
 * @param iterator_arg The current iterator.
 * @param itt_end_arg The end iterator.
 * @return See above.
 */
template <bool Return_Reason, typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::conditional_t<
    Return_Reason,
    next_char32_result<char32_t>,
    std::optional<char32_t>>
    next_char32_and_increment_iterator(
        ItteratorType&      iterator_arg,
        const ItteratorType itt_end_arg
    ) noexcept;

template <typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::pair<char32_t, std::size_t>
    next_char32_with_exception(
        const ItteratorType iterator_arg,
        const ItteratorType itt_end_arg
    );

template <typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr char32_t
    next_char32_and_increment_iterator_with_exception(
        ItteratorType&      iterator_arg,
        const ItteratorType itt_end_arg
    );

/*!
 * @brief Converts an arbitrary unicode string to a u8string.
 *
 * This function assumes that the input string is a valid unicode string.
 *
 * @tparam CharT The character type of the input string.
 * @param str_arg The string to convert.
 * @return The converted string.
 */
template <typename OutputChar, typename ArgType>
requires is_char_type_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
         && is_char_type_c<OutputChar>
constexpr std::basic_string<OutputChar>
    to_formatted_unicode_string(ArgType str_arg) noexcept;
/*!
 * @brief Converts an arbitrary unicode character to a u8string.
 *
 * This function assumes that the character is a valid unicode string.
 *
 * @tparam CharT The character type of the input character.
 * @param str_arg The character to convert.
 * @return The converted character in string form.
 */
template <typename OutputChar, typename InputChar>
requires is_char_type_c<OutputChar> && is_char_type_c<InputChar>
constexpr std::basic_string<OutputChar>
    to_formatted_unicode_string(const InputChar char_arg) noexcept;
// Internal namespace definitions
UNICODE_BRIDGE_INTERNAL_NS_BEGIN

constexpr bool wchar_is_16_bit = (sizeof(wchar_t) == 2);
constexpr bool wchar_is_32_bit = (sizeof(wchar_t) == 4);
/*!
 * @brief This concept identifies a wchar_t type which is 4 bytes long - 32
 * bits.
 */
template <typename T>
concept is_wchar_and_32_bit_c = std::same_as<T, wchar_t> && wchar_is_32_bit;
/*!
 * @brief This concept identifies a wchar_t type which is 2 bytes long - 16
 * bits.
 */
template <typename T>
concept is_wchar_and_16_bit_c = std::same_as<T, wchar_t> && wchar_is_16_bit;

template <typename T>
struct char_underlying_type_object_t
{
    using type = void;
};

template <>
struct char_underlying_type_object_t<char>
{
    using type = char8_t;
};

template <>
struct char_underlying_type_object_t<char8_t>
{
    using type = uint8_t;
};

template <>
struct char_underlying_type_object_t<char16_t>
{
    using type = uint16_t;
};

template <>
struct char_underlying_type_object_t<char32_t>
{
    using type = uint32_t;
};

template <typename T>
requires is_wchar_and_16_bit_c<T>
struct char_underlying_type_object_t<T>
{
    using type = uint16_t;
};

template <typename T>
requires is_wchar_and_32_bit_c<T>
struct char_underlying_type_object_t<T>
{
    using type = uint32_t;
};

template <typename T>
using char_underlying_type_t =
    typename char_underlying_type_object_t<std::remove_cvref_t<T>>::type;

// Error types;
struct unicode_conversion_error_factory
{
    static constexpr unicode_conversion_error
        empty_error() noexcept;

    static constexpr unicode_conversion_error
        invalid_leading_byte(
            const std::size_t character_index_arg,
            const char8_t     code_points_arg
        ) noexcept;

    static constexpr unicode_conversion_error
        truncated_sequence(
            const std::size_t             character_index_arg,
            const std::array<char8_t, 4>& code_points_arg,
            const std::size_t             code_points_encountered_arg,
            const std::size_t             expected_code_points_size_arg
        ) noexcept;

    static constexpr unicode_conversion_error
        invalid_continuation_byte(
            const std::size_t             character_index_arg,
            const std::array<char8_t, 4>& u8_code_points_arg,
            const std::size_t             code_points_expected_arg,
            const std::size_t             sub_error_enum_arg
        ) noexcept;

    static constexpr unicode_conversion_error
        overlong_encoding(
            const std::size_t             character_index_arg,
            const std::array<char8_t, 4>& u8_code_points_arg,
            const std::size_t             code_points_encountered_arg,
            const char32_t                char32_character_arg
        ) noexcept;

    static constexpr unicode_conversion_error
        invalid_utf32_code_point_after_utf8_conversion(
            const std::size_t             character_index_arg,
            const std::array<char8_t, 4>& u8_code_points_arg,
            const std::size_t             code_points_encountered_arg,
            const char32_t                char32_character_arg
        ) noexcept;

    static constexpr unicode_conversion_error
        high_surrogate_then_end_of_stream(
            const std::size_t character_index_arg,
            const char16_t    char16_character_arg,
            const bool        is_wchar_arg
        ) noexcept;

    static constexpr unicode_conversion_error
        high_surrogate_not_followed_by_low_surrogate(
            const std::size_t character_index_arg,
            const char16_t    char16_first_char_arg,
            const char16_t    char16_second_char_arg,
            const bool        is_wchar_arg
        ) noexcept;

    static constexpr unicode_conversion_error
        unexpected_low_surrogate(
            const std::size_t character_index_arg,
            const char16_t    char16_character_arg,
            const bool        is_wchar_arg
        ) noexcept;

    static constexpr unicode_conversion_error
        invalid_utf32_code_point(
            const std::size_t character_index_arg,
            const char32_t    char32_character_arg,
            const bool        is_wchar_arg
        ) noexcept;
};

struct unicode_bridge_exception_factory
{
    template <typename T>
    static unicode_bridge_exception<T>
        make(const T& error_arg, const std::u8string_view function_name_arg)
            noexcept;
};

struct ascii_to_unicode_error_factory
{
    static constexpr ascii_to_unicode_error
        make(const std::size_t index_arg, const char character_arg) noexcept;
};

struct unicode_to_ascii_error_factory
{
    static constexpr unicode_to_ascii_error
        non_ascii_character_found(
            const char32_t    character_arg,
            const std::size_t index_arg
        ) noexcept;

    static constexpr unicode_to_ascii_error
        invalid_unicode_character(
            const unicode_conversion_error& unicode_error_arg
        ) noexcept;
};

struct next_char32_error_factory
{
    static constexpr next_char32_error
        make_unicode_error(const unicode_conversion_error& error_arg) noexcept;

    static constexpr next_char32_error
        iterator_exhausted() noexcept;
};

template <typename T>
concept unicode_bridge_error_c = requires (
    const T&                          error,
    std::u8string_view                fn_name,
    std::u8string_view                additional,
    std::optional<std::u8string_view> str
) {
    { error.message(fn_name, additional, str) } -> std::same_as<std::u8string>;
};

template <typename ResultT, typename ErrorT>
requires unicode_bridge_error_c<ErrorT>
ResultT
    throw_if_error(
        std::expected<ResultT, ErrorT> result,
        const std::u8string_view       function_name_arg
    );
/*!
 * @brief Gets the specified constant.
 *
 * Equal to 0x7F.
 *
 * @tparam T The return type paramter.
 * @return The encoded constant.
 */
template <typename T>
requires is_char_type_c<T> && (sizeof(T) >= 1)
constexpr T ascii_limit() noexcept;
/*!
 * @brief Gets the specified constant.
 *
 * Equal to 0xFFFF.
 *
 * @return The encoded constant.
 */

template <typename T>
requires char_type_is_unicode_c<T> && (sizeof(T) >= 2)
constexpr T single_char16_limit_and_three_char8_limit() noexcept;
/*!
 * @brief Gets the specified constant.
 *
 * Equal to 0x7FF.
 *
 * @tparam T The return type paramter.
 * @return The encoded constant.
 */
template <typename T>
requires char_type_is_unicode_c<T> && (sizeof(T) >= 2)
constexpr T two_char8_limit() noexcept;
/*!
 * @brief Gets the specified constant.
 *
 * Equal to 0x10'FFFF.
 *
 * @tparam T The return type paramter.
 * @return The encoded constant.
 */
template <typename T>
requires char_type_is_unicode_c<T> && (sizeof(T) >= 4)
constexpr T char32_limit() noexcept;
/*!
 * @brief Gets the specified constant.
 *
 * Equal to 0xD800.
 *
 * @tparam T The return type paramter.
 * @return The encoded constant.
 */
template <typename T>
requires char_type_is_unicode_c<T> && (sizeof(T) >= 2)
constexpr T high_surrogate_lower_value() noexcept;
/*!
 * @brief Gets the specified constant.
 *
 * Equal to 0xDFFF.
 *
 * @tparam T The return type paramter.
 * @return The encoded constant.
 */
template <typename T>
requires char_type_is_unicode_c<T> && (sizeof(T) >= 2)
constexpr T low_surrogate_upper_value() noexcept;

inline std::u8string
    to_u8string(const std::string_view str_arg);

template <typename T>
requires std::unsigned_integral<T>
std::u8string
    positive_integer_to_placement(const T number_arg) noexcept;
template <typename WCharT = wchar_t>
constexpr auto
    cast_wstring_to_unicode_string(const std::wstring_view str_arg) noexcept;
template <typename T, bool Use_Capitals, bool Variable_Size_Prefix>
requires is_char_type_c<T>
constexpr std::u8string
    make_hex_from_char_with_prefix(
        const T                  char_arg,
        const std::u8string_view prefix_arg
    ) noexcept;

template <typename T>
requires is_char_type_c<T>
constexpr std::u8string
    represent_char_as_hex_for_output(const T char_arg) noexcept;
template <typename T>
using basic_unicode_result_t = std::expected<T, unicode_conversion_error>;
constexpr std::optional<std::pair<char32_t, std::size_t>>
    if_invalid_u32string_return_char_and_position(
        const std::u32string_view str_arg
    ) noexcept;

template <bool Return_u32string>
constexpr basic_unicode_result_t<
    std::conditional_t<Return_u32string, std::u32string, std::monostate>>
    validate_u8string_and_convert_to_u32string(const std::u8string_view str_arg
    ) noexcept;

template <bool Return_u32string, typename Original_Type>
requires char_type_is_unicode_c<Original_Type>
constexpr basic_unicode_result_t<
    std::conditional_t<Return_u32string, std::u32string, std::monostate>>
    validate_u16string_and_convert_to_u32string(
        const std::u16string_view str_arg
    ) noexcept;
template <typename T>
requires is_wchar_and_32_bit_c<T> || std::same_as<T, char32_t>
constexpr bool
    is_invalid_char32(const T char_arg) noexcept;
template <typename CharT, typename CharU>
constexpr void
    add_char_to_unicode_string(
        const CharT                                          char_arg,
        std::back_insert_iterator<std::basic_string<CharU>>& inserter_arg
    ) noexcept;
template <bool Return_Reason, typename T, typename Original_Value_Type>
requires char_type_is_unicode_c<typename std::iterator_traits<T>::value_type>
         && char_type_is_unicode_c<Original_Value_Type>
constexpr std::conditional_t<
    Return_Reason,
    next_char32_result<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    next_char32_internal_with_iterator_checking(
        const T iterator_begin_arg,
        const T iterator_arg,
        const T itt_end_arg
    ) noexcept;
template <bool Return_Reason, typename T, typename Original_Value_Type>
requires char_type_is_unicode_c<typename std::iterator_traits<T>::value_type>
         && char_type_is_unicode_c<Original_Value_Type>
constexpr std::conditional_t<
    Return_Reason,
    basic_unicode_result_t<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    next_char32_internal(
        const T iterator_begin_arg,
        const T iterator_arg,
        const T itt_end_arg
    ) noexcept;

template <bool Return_Reason, typename T, typename Original_Type>
requires char_type_is_unicode_c<typename std::iterator_traits<T>::value_type>
         && char_type_is_unicode_c<Original_Type>
constexpr std::conditional_t<
    Return_Reason,
    next_char32_result<char32_t>,
    std::optional<char32_t>>
    next_char32_and_increment_iterator(
        const T iterator_begin_arg,
        T&      iterator_arg,
        const T itt_end_arg
    ) noexcept;
template <typename T>
requires char_type_is_unicode_c<T> && (sizeof(T) >= 4)
constexpr T char16_offset_for_char32_conversion() noexcept;

template <typename T>
requires char_type_is_unicode_c<T> && (sizeof(T) >= 2)
constexpr T high_surrogate_upper_value() noexcept;

template <typename T>
requires char_type_is_unicode_c<T> && (sizeof(T) >= 2)
constexpr T low_surrogate_lower_value() noexcept;
template <typename T>
requires is_char_type_c<T>
constexpr T
    zero() noexcept;
template <typename T>
requires char_type_is_unicode_c<T> && (sizeof(T) >= 2)
constexpr bool is_surrogate(const T char_arg) noexcept;
template <typename T>
requires char_type_is_unicode_c<T> && (sizeof(T) >= 2)
constexpr bool is_high_surrogate(const T char_arg) noexcept;
template <typename T>
requires char_type_is_unicode_c<T> && (sizeof(T) >= 2)
constexpr bool is_low_surrogate(const T char_arg) noexcept;
template <typename CharT, typename CharU>
constexpr std::optional<std::basic_string<CharT>>
    special_char_as_string(const CharU char_arg) noexcept;

template <typename T>
requires is_char_type_c<T>
constexpr std::u8string
    represent_char_as_hex_for_printing(const T char_arg) noexcept;

template <typename T>
inline std::wstring
    cast_unicode_string_to_wstring(const T str_arg_view);

UNICODE_BRIDGE_INTERNAL_NS_END

UNICODE_BRIDGE_NS_END

UNICODE_BRIDGE_NS_BEGIN
constexpr unicode_conversion_error::unicode_conversion_error(
    const unicode_conversion_error_code code_arg,
    const std::size_t                   character_index_arg,
    const std::array<char8_t, 4>&       u8_code_points_arg,
    const std::uint8_t                  code_points_encountered_arg,
    const std::uint8_t                  expected_code_points_size_arg,
    const std::array<char16_t, 2>&      u16_code_points_arg,
    char32_t                            char32_character_arg,
    const bool                          is_wchar_arg
) noexcept
    : _code(code_arg)
    , _character_index(character_index_arg)
    , _u8_code_points(u8_code_points_arg)
    , _code_points_encountered(code_points_encountered_arg)
    , _expected_code_points_size(expected_code_points_size_arg)
    , _u16_code_points(u16_code_points_arg)
    , _char32_character(char32_character_arg)
    , _is_wchar(is_wchar_arg)
{}

constexpr const unicode_conversion_error::unicode_conversion_error_code
    unicode_conversion_error::code() const noexcept
{
    return _code;
}

constexpr const std::size_t
    unicode_conversion_error::character_index() const noexcept
{
    return _character_index;
}

constexpr const std::array<char8_t, 4>&
    unicode_conversion_error::u8_code_points() const noexcept
{
    return _u8_code_points;
}

constexpr const std::size_t
    unicode_conversion_error::code_points_encountered() const noexcept
{
    return _code_points_encountered;
}

constexpr const std::size_t
    unicode_conversion_error::expected_code_points_size() const noexcept
{
    return _expected_code_points_size;
}

constexpr const std::array<char16_t, 2>&
    unicode_conversion_error::u16_code_points() const noexcept
{
    return _u16_code_points;
}

constexpr const char32_t
    unicode_conversion_error::char32_character() const noexcept
{
    return _char32_character;
}

constexpr const bool
    unicode_conversion_error::is_wchar() const noexcept
{
    return _is_wchar;
}

constexpr std::u8string
    unicode_conversion_error::message(
        const std::u8string_view                function_name_arg,
        const std::u8string_view                additional_message_arg,
        const std::optional<std::u8string_view> opt_complete_string_arg
    ) const

{
    using enum unicode_conversion_error_code;
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    u8string msg;
    auto     chars_to_hex
        = []<typename T>(T char_array_arg, const std::size_t chars_size_arg)
    {
        u8string chars_as_hex = u8"(";
        if (chars_size_arg == 1)
        {
            chars_as_hex.append(
                represent_char_as_hex_for_output(char_array_arg[0])
            );
        }
        else
        {
            chars_as_hex.append(u8"[");
            for (size_t idx{0}; idx < chars_size_arg; ++idx)
            {
                if (idx > 0)
                {
                    chars_as_hex.append(u8", ");
                }
                chars_as_hex.append(
                    represent_char_as_hex_for_output(char_array_arg[idx])
                );
            }
            chars_as_hex.append(u8"]");
        }
        chars_as_hex.append(u8")");
        return chars_as_hex;
    };
    auto number_as_string = [](const std::size_t number_arg)
    {
        switch (number_arg)
        {
        case 1:
            return u8"one";
        case 2:
            return u8"two";
        case 3:
            return u8"three";
        case 4:
            return u8"four";
        default:
            return u8"zero";
        }
    };
    auto placement_as_string = [](const std::size_t number_arg)
    {
        switch (number_arg)
        {
        case 1:
            return u8"first";
        case 2:
            return u8"second";
        case 3:
            return u8"third";
        case 4:
            return u8"fourth";
        default:
            return u8"zeroth";
        }
    };
    auto integer_list
        = [](const std::size_t begin_int_arg, const std::size_t n_ints_arg)
    {
        u8string str;
        if (n_ints_arg == 1)
        {
            str.append(positive_integer_to_placement(begin_int_arg));
            str.append(u8" code unit ");
        }
        else
        {
            for (size_t idx = 0; idx < n_ints_arg; ++idx)
            {
                if (idx + 1 == n_ints_arg)
                {
                    str.append(u8" and ");
                }
                else if (idx > 0)
                {
                    str.append(u8", ");
                }
                str.append(positive_integer_to_placement(idx + begin_int_arg));
            }
            str.append(u8" code units ");
        }
        return str;
    };
    auto append_complete_str = [&]()
    {
        if (opt_complete_string_arg.has_value())
        {
            msg.append(u8" (\"");
            msg.append(to_formatted_unicode_string<char8_t>(
                opt_complete_string_arg.value()
            ));
            msg.append(u8"\")");
        }
    };
    auto utf32_invalid = [&](const std::u8string_view codepoint_as_hex_arg)
    {
        msg.append(codepoint_as_hex_arg);
        msg.append(
            u8" falls outside the valid Unicode range — valid Unicode scalar "
            u8"values must be inclusively between U+0000 and U+10FFFF, "
            u8"excluding the surrogate range U+D800 to U+DFFF. As "
        );
        msg.append(codepoint_as_hex_arg);
        msg.append(
            is_surrogate(_char32_character)
                ? u8" falls within the surrogate range"
                : u8" exceeds the maximum valid codepoint"
        );
        msg.append(u8", it cannot represent a valid Unicode scalar value");
    };
    auto generic_begin_str = [&](const u8string& chars_as_hex,
                                 const size_t    n_code_units,
                                 const char8_t*  utf_standard)
    {
        msg.append(u8"The ");
        msg.append(integer_list(_character_index + 1, n_code_units));
        msg.append(chars_as_hex);
        msg.append(u8" in the UTF-");
        msg.append(utf_standard);
        msg.append(u8" input ");
        if (_is_wchar)
        {
            msg.append(u8"(encoded using wchar_t) ");
        }
        msg.append(u8"passed to ");
        msg.append(function_name_arg);
        append_complete_str();
    };
    auto utf8_begin_str
        = [&](const u8string& chars_as_hex, const size_t n_code_units)
    {
        generic_begin_str(chars_as_hex, n_code_units, u8"8");
    };
    auto utf16_begin_str
        = [&](const u8string& chars_as_hex, const size_t n_code_units)
    {
        generic_begin_str(chars_as_hex, n_code_units, u8"16");
    };
    switch (_code)
    {
    case invalid_leading_byte:
    {
        utf8_begin_str(chars_to_hex(_u8_code_points, 1), 1);
        msg.append(
            u8" was found to be an invalid leading byte. A valid leading "
            u8"byte must be inclusively within one of the following ranges: "
            u8"0x00 to 0x7F (single-byte sequence), 0xC0 to 0xDF (two-byte "
            u8"sequence), 0xE0 to 0xEF (three-byte sequence), or 0xF0 to 0xF7 "
            u8"(four-byte sequence). As "
        );
        msg.append(represent_char_as_hex_for_output(_u8_code_points[0]));
        msg.append(u8" falls outside all of these ranges, it cannot begin a "
                   u8"sequence representing a valid Unicode scalar value");
    }
    break;
    case truncated_sequence:
    {
        utf8_begin_str(
            chars_to_hex(_u8_code_points, _code_points_encountered),
            _code_points_encountered
        );
        const size_t missing
            = _expected_code_points_size - _code_points_encountered;
        msg.append(
            _code_points_encountered == 1
                ? u8" was found to be a valid leading byte, indicating "
                  u8"the start of a "
                : u8" form the start of a "
        );
        msg.append(number_as_string(_expected_code_points_size));
        msg.append(u8"-byte sequence. However, the input ended after the ");
        msg.append(placement_as_string(_code_points_encountered));
        msg.append(u8" code unit — ");
        msg.append(to_u8string(std::to_string(missing)));
        if (_code_points_encountered > 1)
        {
            msg.append(u8" further");
        }
        [[assume(missing > 0)]];
        msg.append(
            missing == 1
                ? u8" continuation byte was expected but was not present."
                : u8" continuation bytes were expected but none were present."
        );
        msg.append(u8" As the sequence is incomplete, it cannot represent a "
                   u8"valid Unicode scalar value");
    }
    break;
    case invalid_continuation_byte:
    {
        static constexpr tuple<size_t, const char8_t*, array<size_t, 3>, size_t>
            continuation_byte_error_table[] = {
                {2, u8"second",                   {1, 0, 0}, 1}, // case 0
                {3, u8"second",                   {1, 0, 0}, 1}, // case 1
                {3, u8"third",                    {2, 0, 0}, 1}, // case 2
                {3, u8"second and third",         {1, 2, 0}, 2}, // case 3
                {4, u8"second",                   {1, 0, 0}, 1}, // case 4
                {4, u8"third",                    {2, 0, 0}, 1}, // case 5
                {4, u8"fourth",                   {3, 0, 0}, 1}, // case 6
                {4, u8"second and third",         {1, 2, 0}, 2}, // case 7
                {4, u8"second and fourth",        {1, 3, 0}, 2}, // case 8
                {4, u8"third and fourth",         {2, 3, 0}, 2}, // case 9
                {4, u8"second, third and fourth", {1, 2, 3}, 3}, // case 10
        };

        const auto& [expected_code_point_size, names_of_invalid_continuation_bytes, invalid_code_point_indexes, numb_invalid_code_points]
            = continuation_byte_error_table[_expected_code_points_size];
        u8string invalid_code_points;
        for (size_t idx{0}; idx < numb_invalid_code_points; ++idx)
        {
            const auto& invalid_code_point_index{invalid_code_point_indexes[idx]
            };
            if (idx > 0)
            {
                const bool is_last = (idx + 1 == numb_invalid_code_points);
                invalid_code_points.append(is_last ? u8" and " : u8", ");
            }
            invalid_code_points.append(represent_char_as_hex_for_output(
                _u8_code_points[invalid_code_point_index]
            ));
        }
        utf8_begin_str(
            chars_to_hex(_u8_code_points, _code_points_encountered),
            _code_points_encountered
        );
        msg.append(u8" form the start of a ");
        msg.append(number_as_string(expected_code_point_size));
        msg.append(u8"-byte sequence. The ");
        msg.append(names_of_invalid_continuation_bytes);
        msg.append(
            numb_invalid_code_points > 1 ? u8" code units (" : u8" code unit ("
        );
        msg.append(invalid_code_points);
        msg.append(u8") ");
        msg.append(
            numb_invalid_code_points > 1
                ? u8"were expected to be continuation bytes, but were not"
                : u8"was expected to be a continuation byte, but was not"
        );
        msg.append(u8" — a valid continuation byte must be inclusively between "
                   u8"0x80 and 0xBF. ");
        switch (numb_invalid_code_points)
        {
        case 1:
            msg.append(u8"As ");
            msg.append(invalid_code_points);
            msg.append(u8" falls outside this range, the sequence cannot "
                       u8"represent a valid Unicode scalar value");
            break;
        case 2:
            msg.append(u8"As both are outside this range, the sequence cannot "
                       u8"represent a valid Unicode scalar value");
            break;
        case 3:
            msg.append(u8"As all three are outside this range, the sequence "
                       u8"cannot represent a valid Unicode scalar value");
            break;
        }
    }
    break;
    case overlong_encoding:
    {
        const u8string codepoint_as_hex
            = make_hex_from_char_with_prefix<char32_t, true, true>(
                _char32_character, u8"U+"
            );
        auto char_as_u8
            = unicode_conversion_with_exception<char8_t>(_char32_character);
        utf8_begin_str(
            chars_to_hex(_u8_code_points, _code_points_encountered),
            _code_points_encountered
        );
        msg.append(u8" form a ");
        msg.append(number_as_string(_code_points_encountered));
        msg.append(u8"-byte sequence encoding ");
        msg.append(codepoint_as_hex);
        msg.append(u8". This is an overlong encoding — ");
        msg.append(codepoint_as_hex);
        msg.append(u8" can be represented using ");
        if (char_as_u8.size() == 1)
            msg.append(u8"a single byte ");
        else
        {
            msg.append(number_as_string(char_as_u8.size()));
            msg.append(u8" bytes ");
        }
        msg.append(chars_to_hex(char_as_u8, char_as_u8.size()));
        msg.append(u8", which is the shortest valid UTF-8 representation. The "
                   u8"UTF-8 standard requires that code points are always "
                   u8"encoded using the shortest possible sequence. As this "
                   u8"requirement is not met, the sequence does not represent "
                   u8"a valid Unicode scalar value");
    }
    break;
    case invalid_utf32_code_point_after_utf8_conversion:
    {
        const u8string codepoint_as_hex
            = make_hex_from_char_with_prefix<char32_t, true, true>(
                _char32_character, u8"U+"
            );
        utf8_begin_str(
            chars_to_hex(_u8_code_points, _code_points_encountered),
            _code_points_encountered
        );
        msg.append(u8" form a ");
        msg.append(number_as_string(_code_points_encountered));
        msg.append(u8"-byte sequence encoding ");
        msg.append(codepoint_as_hex);
        msg.append(u8". However, ");
        utf32_invalid(codepoint_as_hex);
    }
    break;
    case high_surrogate_then_end_of_stream:
    {
        utf16_begin_str(chars_to_hex(_u16_code_points, 1), 1);
        msg.append(
            u8" is a high surrogate, indicating the start of a surrogate "
            u8"pair. However, the input ended after this code unit — a low "
            u8"surrogate was expected to follow but was not present. As the "
            u8"surrogate pair is incomplete, it cannot represent a valid "
            u8"Unicode scalar value"
        );
    }
    break;
    case high_surrogate_not_followed_by_low_surrogate:
    {
        utf16_begin_str(chars_to_hex(_u16_code_points, 2), 2);
        msg.append(
            u8" form the start of a surrogate pair. The first code unit ("
        );
        msg.append(represent_char_as_hex_for_output(_u16_code_points[0]));
        msg.append(u8") is a high surrogate, which must be followed by a low "
                   u8"surrogate inclusively between 0xDC00 and 0xDFFF. "
                   u8"However, the second code unit (");
        msg.append(represent_char_as_hex_for_output(_u16_code_points[1]));
        msg.append(u8") falls outside this range, and therefore the two code "
                   u8"units cannot represent a valid Unicode scalar value");
    }
    break;
    case unexpected_low_surrogate:
    {
        utf16_begin_str(chars_to_hex(_u16_code_points, 1), 1);
        msg.append(
            u8" is a low surrogate. Low surrogates must always be preceded by "
            u8"a high surrogate (inclusively between 0xD800 and 0xDBFF) as the "
            u8"second part of a surrogate pair. As this low surrogate appears "
            u8"without a preceding high surrogate, it cannot represent a valid "
            u8"Unicode scalar value"
        );
    }
    break;
    case invalid_utf32_code_point:
    {
        const u8string codepoint_as_hex
            = make_hex_from_char_with_prefix<char32_t, true, true>(
                _char32_character, u8"U+"
            );
        u8string chars_as_hex = u8"(";
        chars_as_hex.append(
            make_hex_from_char_with_prefix<char32_t, true, true>(
                _char32_character, u8"0x"
            )
        );
        chars_as_hex.append(u8")");
        generic_begin_str(chars_as_hex, 1, u8"32");
        msg.append(u8" decodes to ");
        msg.append(codepoint_as_hex);
        msg.append(u8". However, ");
        utf32_invalid(codepoint_as_hex);
        break;
    }
    case no_error:
        msg.append(u8"No error");
        break;
    }
    msg.append(additional_message_arg);
    msg.append(u8".");
    return msg;
}

template <typename Error_Type>
constexpr unicode_bridge_exception<Error_Type>::unicode_bridge_exception(
    const Error_Type&        error_arg,
    const std::u8string_view function_name_arg
) noexcept
    : _error(error_arg), _function_name(function_name_arg)
{}

template <typename Error_Type>
const char*
    unicode_bridge_exception<Error_Type>::what() const noexcept
{
    if (_what_cache.empty())
    {
        const std::u8string u8msg = _error.message(
            _function_name,
            u8", and the conversion was aborted",
            std::optional<std::u8string_view>{}
        );
        _what_cache = std::string(
            reinterpret_cast<const char*>(u8msg.data()), u8msg.size()
        );
    }
    return _what_cache.c_str();
}

template <typename Error_Type>
constexpr const Error_Type&
    unicode_bridge_exception<Error_Type>::error() const noexcept
{
    return _error;
}

constexpr ascii_to_unicode_error::ascii_to_unicode_error(
    const std::size_t index_arg,
    const char        character_arg
) noexcept
    : _index(index_arg), _character(character_arg)
{}

constexpr char
    ascii_to_unicode_error::get_character() const noexcept
{
    return _character;
}

constexpr std::size_t
    ascii_to_unicode_error::get_index() const noexcept
{
    return _index;
}

std::u8string
    ascii_to_unicode_error::message(
        const std::u8string_view                function_name_arg,
        const std::u8string_view                additional_message_arg,
        const std::optional<std::u8string_view> complete_string_arg
    ) const
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    u8string msg;
    msg.append(u8"The ");
    msg.append(positive_integer_to_placement(_index));
    msg.append(u8" code unit (");
    msg.append(represent_char_as_hex_for_output(_character));
    msg.append(u8") in the ASCII input passed to ");
    msg.append(function_name_arg);
    if (complete_string_arg.has_value())
    {
        msg.append(u8" (\"");
        msg.append(complete_string_arg.value());
        msg.append(u8"\")");
    }
    msg.append(u8" lies outside the ASCII range. The ASCII range is "
               u8"inclusively between ");
    msg.append(represent_char_as_hex_for_output(zero<char>()));
    msg.append(u8" and ");
    msg.append(represent_char_as_hex_for_output(ascii_limit<char>()));
    msg.append(u8", therefore it was not possible to encode this character "
               u8"in unicode");
    msg.append(additional_message_arg);
    msg.append(u8".");
    return msg;
}

constexpr unicode_to_ascii_error::unicode_to_ascii_error(
    const unicode_to_ascii_error_code enum_arg,
    const unicode_conversion_error&   error_arg,
    const char32_t                    character_arg,
    const std::size_t                 index_arg
) noexcept
    : _enum(enum_arg)
    , _error(error_arg)
    , _character(character_arg)
    , _index(index_arg)
{}

constexpr const unicode_to_ascii_error::unicode_to_ascii_error_code
    unicode_to_ascii_error::get_enum() const noexcept
{
    return _enum;
}

constexpr const std::size_t
    unicode_to_ascii_error::index() const noexcept
{
    return _index;
}

constexpr const char32_t
    unicode_to_ascii_error::character() const noexcept
{
    return _character;
}

constexpr const unicode_conversion_error&
    unicode_to_ascii_error::error() const noexcept
{
    return _error;
}

std::u8string
    unicode_to_ascii_error::message(
        const std::u8string_view                function_name_arg,
        const std::u8string_view                additional_message_arg,
        const std::optional<std::u8string_view> complete_string_arg
    ) const
{
    using enum unicode_to_ascii_error_code;
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    u8string msg;
    switch (_enum)
    {
    case non_ascii_character_found:
        msg.append(u8"The ");
        msg.append(positive_integer_to_placement(_index + 1));
        msg.append(u8" code unit (");
        msg.append(represent_char_as_hex_for_output(_character));
        msg.append(u8") in the unicode input passed to ");
        msg.append(function_name_arg);
        if (complete_string_arg.has_value())
        {
            msg.append(u8" (\"");
            msg.append(complete_string_arg.value());
            msg.append(u8"\")");
        }
        msg.append(u8" was found to be outside the ASCII range. The ASCII "
                   u8"range is inclusively between ");
        msg.append(represent_char_as_hex_for_output(zero<char32_t>()));
        msg.append(u8" and ");
        msg.append(represent_char_as_hex_for_output(ascii_limit<char32_t>()));
        msg.append(u8", therefore it cannot be represented as an ASCII "
                   u8"character");
        msg.append(additional_message_arg);
        msg.append(u8".");
        break;
    case invalid_unicode_character:
        msg.append(_error.message(
            function_name_arg, additional_message_arg, complete_string_arg
        ));
        break;
    }
    return msg;
}

constexpr next_char32_error::next_char32_error(
    const next_char32_error_code    enum_arg,
    const unicode_conversion_error& error_arg
) noexcept
    : _enum(enum_arg), _error(error_arg)
{}

constexpr const next_char32_error::next_char32_error_code
    next_char32_error::get_enum() const noexcept
{
    return _enum;
}

constexpr const unicode_conversion_error&
    next_char32_error::error() const noexcept
{
    return _error;
}

std::u8string
    next_char32_error::message(
        const std::u8string_view                function_name_arg,
        const std::u8string_view                additional_message_arg,
        const std::optional<std::u8string_view> complete_string_arg
    ) const
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using enum next_char32_error_code;
    u8string msg;
    switch (_enum)
    {
    case unicode_error:
        msg.append(_error.message(
            function_name_arg, additional_message_arg, complete_string_arg
        ));
        break;
    case iterator_end:
        msg.append(u8"The iterator passed to ");
        msg.append(function_name_arg);
        msg.append(u8" was exhausted — no more code units could be read");
        msg.append(additional_message_arg);
        msg.append(u8".");
        break;
    }
    return msg;
}

template <typename ArgType>
requires char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr unicode_to_ascii_result<std::string>
    convert_unicode_to_ascii(
        ArgType str_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using CharT = char_type_of_t<ArgType>;
    basic_string_view<CharT> sv(str_arg);
    using itt              = typename basic_string_view<CharT>::const_iterator;
    itt    string_iterator = sv.begin();
    itt    string_iterator_end = sv.end();
    string output_string;
    auto   output_string_inserter(back_inserter(output_string));
    for (size_t idx{0}; string_iterator != string_iterator_end; ++idx)
    {
        const auto character{*string_iterator};
        // Do this as quicker than finding error. Error handling runs through
        // slower next_char32_t_and_increment_iterator.
        if (is_valid_ascii(character))
        {
            output_string_inserter = character;
            ++string_iterator;
        }
        else
        {
            // Gets the char32_t. There is some redundancy in using this
            // function (there are checks for ascii in next_char32_t) however as
            // this is not the "hot path" we don't think it really matters.
            const basic_unicode_result_t<pair<char32_t, size_t>> character_res{
                next_char32_internal<true, itt, CharT>(
                    std::begin(sv), string_iterator, string_iterator_end
                )
            };
            if (character_res.has_value())
            {
                return unexpected(
                    unicode_to_ascii_error_factory::non_ascii_character_found(
                        character_res.value().first, idx
                    )
                );
            }
            else
            {
                return unexpected(
                    unicode_to_ascii_error_factory::invalid_unicode_character(
                        character_res.error()
                    )
                );
            }
        }
    }
    return output_string;
}

template <typename InputChar>
requires char_type_is_unicode_c<InputChar>
constexpr unicode_to_ascii_result<std::string>
    convert_unicode_to_ascii(
        const InputChar char_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    const InputChar                    char_arr[2] = {char_arg, InputChar{}};
    const basic_string_view<InputChar> sv(char_arr, 1);
    return convert_unicode_to_ascii(sv);
}

template <typename ArgType>
requires char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr std::string
    convert_unicode_to_ascii_with_exception(
        ArgType str_arg
    )
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return throw_if_error<string, unicode_to_ascii_error>(
        convert_unicode_to_ascii(str_arg),
        u8"convert_unicode_to_ascii_with_exception"
    );
}

template <typename InputChar>
requires char_type_is_unicode_c<InputChar>
constexpr std::string
    convert_unicode_to_ascii_with_exception(
        const InputChar char_arg
    )
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return throw_if_error<string, unicode_to_ascii_error>(
        convert_unicode_to_ascii(char_arg),
        u8"convert_unicode_to_ascii_with_exception"
    );
}

template <typename T, typename ArgType>
requires char_type_is_unicode_c<T>
         && std::same_as<char, char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr ascii_to_unicode_result<std::basic_string<T>>
    convert_ascii_to_unicode(
        ArgType str_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using ResultType               = std::basic_string<T>;
    using itt                      = typename string_view::const_iterator;
    using U                        = char_type_of_t<ArgType>;
    auto       sv                  = basic_string_view<U>(str_arg);
    itt        string_iterator     = sv.begin();
    itt        string_iterator_end = sv.end();
    ResultType output_string;
    auto       output_string_inserter(back_inserter(output_string));
    for (size_t idx{0}; string_iterator != string_iterator_end; ++idx)
    {
        const char character{*string_iterator};
        if (not is_valid_ascii(character))
        {
            return unexpected(
                ascii_to_unicode_error_factory::make(idx, character)
            );
        }
        else
        {
            output_string_inserter = static_cast<T>(character);
            ++string_iterator;
        }
    }
    return output_string;
}

template <typename OutputChar>
requires char_type_is_unicode_c<OutputChar>
constexpr ascii_to_unicode_result<std::basic_string<OutputChar>>
    convert_ascii_to_unicode(
        const char char_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    const char                    char_arr[2] = {char_arg, char{}};
    const basic_string_view<char> sv(char_arr, 1);
    return convert_ascii_to_unicode<OutputChar>(sv);
}

template <typename OutputChar, typename ArgType>
requires char_type_is_unicode_c<OutputChar>
         && std::same_as<char, char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr std::basic_string<OutputChar>
    convert_ascii_to_unicode_with_exception(
        ArgType str_arg
    )
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return throw_if_error<basic_string<OutputChar>, ascii_to_unicode_error>(
        convert_ascii_to_unicode<OutputChar>(str_arg),
        u8"convert_ascii_to_unicode_with_exception"
    );
}

template <typename OutputChar>
requires char_type_is_unicode_c<OutputChar>
constexpr std::basic_string<OutputChar>
    convert_ascii_to_unicode_with_exception(
        const char char_arg
    )
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return throw_if_error<basic_string<OutputChar>, ascii_to_unicode_error>(
        convert_ascii_to_unicode<OutputChar>(char_arg),
        u8"convert_ascii_to_unicode_with_exception"
    );
}

template <typename OutputChar, typename ArgType>
requires char_type_is_unicode_c<OutputChar>
         && char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr unicode_conversion_result<std::basic_string<OutputChar>>
    unicode_conversion(
        ArgType str_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using InputChar = char_type_of_t<ArgType>;
    auto sv         = basic_string_view<InputChar>(str_arg);
    // Function converts an already valid u32string to a u16string.
    auto convert_u32string_to_u16string_function
        = [](const u32string_view u32_arg) -> u16string
    {
        u16string return_value;
        auto      return_value_inserter{std::back_inserter(return_value)};
        auto      str_end_iterator{std::end(u32_arg)};
        for (auto str_iterator{std::begin(u32_arg)};
             str_iterator != str_end_iterator;
             ++str_iterator)
        {
            const char32_t character{*str_iterator};
            // Encode as UTF-16
            if (character
                <= single_char16_limit_and_three_char8_limit<char32_t>())
            {
                // BMP (Basic Multilingual Plane)
                return_value_inserter = static_cast<char16_t>(character);
            }
            else if (character <= char32_limit<char32_t>())
            {
                // Supplementary Plane → surrogate pair
                // Moves the character from the range 0x10000 to 0x10FFFF to
                // 0... 0xFFFFF.
                const char32_t character_cpy{
                    character - char16_offset_for_char32_conversion<char32_t>()
                };
                return_value_inserter = static_cast<char16_t>(
                    (character_cpy >> 10)
                    + high_surrogate_lower_value<char32_t>()
                );
                return_value_inserter = static_cast<char16_t>(
                    (character_cpy & 0b0011'1111'1111)
                    + low_surrogate_lower_value<char32_t>()
                );
            }
            else
            {
                std::unreachable();
            }
        }
        return return_value;
    };
    // Function converts an already valid u32string to a valid u8string.
    auto convert_u32string_to_u8string_function
        = [](const u32string_view u32_arg)
    {
        u8string return_value;
        auto     return_value_inserter{std::back_inserter(return_value)};
        for (const char32_t character : u32_arg)
        {
            add_char_to_unicode_string(character, return_value_inserter);
        }
        return return_value;
    };
    // Runs the code for converting a UTF16 string to any other unicode
    // string type.
    auto char16_convert_function
        = [&](const std::basic_string_view<char16_t> u16_arg
          ) -> basic_unicode_result_t<std::basic_string<OutputChar>>
    {
        // If the output is UTF16, then only validates the input. Otherwise,
        // either converts and validates a UTF16 to UTF32, or reports the
        // error.
        auto str_conversion_result{validate_u16string_and_convert_to_u32string<
            not (same_as<char16_t, OutputChar>),
            InputChar>(u16_arg)};
        if constexpr (same_as<char8_t, OutputChar>)
        {
            // Converts UTF32 to UTF8
            return str_conversion_result.transform(
                [&](const auto u32_arg)
                {
                    return convert_u32string_to_u8string_function(u32_arg);
                }
            );
        }
        else if constexpr (same_as<char16_t, OutputChar>)
        {
            // Either reports invalid UTF16 string, or returns the original
            // string if its valid.
            return str_conversion_result.transform(
                [&](const std::monostate monostate_arg) -> std::u16string
                {
                    return u16string(u16_arg);
                }
            );
        }
        // Just return the result if UTF32.
        else if constexpr (same_as<char32_t, OutputChar>)
        {
            return str_conversion_result;
        }
        else if constexpr (same_as<wchar_t, OutputChar>)
        {
            if constexpr (wchar_is_16_bit)
            {
                if (str_conversion_result.has_value())
                {
                    return cast_unicode_string_to_wstring(u16_arg);
                }
                else
                {
                    return unexpected(str_conversion_result.error());
                }
            }
            else if constexpr (wchar_is_32_bit)
            {
                return str_conversion_result.transform(
                    [](const auto unicode_str)
                    {
                        return cast_unicode_string_to_wstring(unicode_str);
                    }
                );
            }
            else
            {
                UNICODE_BRIDGE_STATIC_ASSERT(
                    OutputChar,
                    "unicode_conversion valid for wchar_t, "
                    "however Invalid for this wchar_t size."
                );
            }
        }
        else
        {
            UNICODE_BRIDGE_STATIC_ASSERT(
                OutputChar,
                "unicode_conversion invalid for this character "
                "type"
            );
        }
    };
    // Contains the logic for converting UTF32 into all other unicode types.
    auto char32_convert_function
        = [&](const std::basic_string_view<char32_t> u32_str_arg
          ) -> basic_unicode_result_t<std::basic_string<OutputChar>>
    {
        // Checks the input is valid.
        const auto conversion_result{
            if_invalid_u32string_return_char_and_position(u32_str_arg)
        };
        // If invalid, return the reason its invalid.
        if (conversion_result.has_value())
        {
            return unexpected(unicode_conversion_error_factory::invalid_utf32_code_point(
                    conversion_result.value().second,
                    conversion_result.value().first, same_as<InputChar, wchar_t>
                ));
        }
        // Otherwise convert the already validated unicode string.
        if constexpr (same_as<char8_t, OutputChar>)
        {
            return convert_u32string_to_u8string_function(u32_str_arg);
        }
        else if constexpr (same_as<char16_t, OutputChar>)
        {
            return convert_u32string_to_u16string_function(u32_str_arg);
        }
        else if constexpr (same_as<char32_t, OutputChar>)
        {
            return u32string(u32_str_arg);
        }
        else if constexpr (same_as<wchar_t, OutputChar>)
        {
            if constexpr (wchar_is_16_bit)
            {
                return cast_unicode_string_to_wstring(
                    convert_u32string_to_u16string_function(u32_str_arg)
                );
            }
            else if constexpr (wchar_is_32_bit)
            {
                return cast_unicode_string_to_wstring(u32_str_arg);
            }
            else
            {
                UNICODE_BRIDGE_STATIC_ASSERT(
                    OutputChar,
                    "unicode_conversion valid for wchar_t, "
                    "however Invalid for this wchar_t size."
                );
            }
        }
        else
        {
            UNICODE_BRIDGE_STATIC_ASSERT(
                OutputChar,
                "unicode_conversion invalid for this character "
                "type"
            );
        }
    };
    if constexpr (same_as<char8_t, InputChar>)
    {
        // Converts UTF8 to all other character types.
        auto conversion_result{validate_u8string_and_convert_to_u32string<
            not (same_as<char8_t, OutputChar>)>(sv)};
        // If invalid then report why, otherwise just returns original
        // string.
        if constexpr (same_as<char8_t, OutputChar>)
        {
            return conversion_result.transform(
                [&](auto monostate_arg)
                {
                    return basic_string<OutputChar>(sv);
                }
            );
        }
        // Converts UTF32 string to U16.
        else if constexpr (same_as<char16_t, OutputChar>)
        {
            return conversion_result.transform(
                [&](auto u32_arg)
                {
                    return convert_u32string_to_u16string_function(u32_arg);
                }
            );
        }
        else if constexpr (same_as<char32_t, OutputChar>)
        {
            return conversion_result;
        }
        else if constexpr (same_as<wchar_t, OutputChar>)
        {
            if constexpr (wchar_is_16_bit)
            {
                return conversion_result
                    .transform(
                        [&](auto u32_arg)
                        {
                            return convert_u32string_to_u16string_function(
                                u32_arg
                            );
                        }
                    )
                    .transform(
                        [](auto u16_arg)
                        {
                            return cast_unicode_string_to_wstring(u16_arg);
                        }
                    );
            }
            else if constexpr (wchar_is_32_bit)
            {
                return conversion_result.transform(
                    [](auto u32_arg)
                    {
                        return cast_unicode_string_to_wstring(u32_arg);
                    }
                );
            }
            else
            {
                UNICODE_BRIDGE_STATIC_ASSERT(
                    OutputChar,
                    "unicode_conversion valid for wchar_t, "
                    "however "
                    "Invalid "
                    "for "
                    "this "
                    "wchar_t size."
                );
            }
        }
        else
        {
            UNICODE_BRIDGE_STATIC_ASSERT(
                OutputChar,
                "unicode_conversion invalid for this character "
                "type"
            );
        }
    }
    else if constexpr (same_as<char16_t, InputChar>)
    {
        return char16_convert_function(sv);
    }
    else if constexpr (same_as<char32_t, InputChar>)
    {
        return char32_convert_function(sv);
    }
    else if constexpr (same_as<wchar_t, InputChar>)
    {
        if constexpr (wchar_is_16_bit)
        {
            return char16_convert_function(cast_wstring_to_unicode_string(sv));
        }
        else if constexpr (wchar_is_32_bit)
        {
            return char32_convert_function(cast_wstring_to_unicode_string(sv));
        }
        else
        {
            UNICODE_BRIDGE_STATIC_ASSERT(
                OutputChar,
                "unicode_conversion valid for wchar_t, however "
                "Invalid for "
                "this "
                "wchar_t size."
            );
        }
    }
    else
    {
        UNICODE_BRIDGE_STATIC_ASSERT(
            OutputChar, "unicode_conversion invalid for this character type"
        );
    }
}

template <typename OutputChar, typename InputChar>
requires char_type_is_unicode_c<OutputChar> && char_type_is_unicode_c<InputChar>
constexpr unicode_conversion_result<std::basic_string<OutputChar>>
    unicode_conversion(
        const InputChar char_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    const InputChar                    char_arr[2] = {char_arg, InputChar{}};
    const basic_string_view<InputChar> sv(char_arr, 1);
    return unicode_conversion<OutputChar>(sv);
}

template <typename OutputChar, typename ArgType>
requires char_type_is_unicode_c<OutputChar>
         && char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr std::basic_string<OutputChar>
    unicode_conversion_with_exception(
        ArgType str_arg
    )
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return throw_if_error<basic_string<OutputChar>, unicode_conversion_error>(
        unicode_conversion<OutputChar>(str_arg),
        u8"unicode_conversion_with_exception"
    );
}

template <typename OutputChar, typename InputChar>
requires char_type_is_unicode_c<OutputChar> && char_type_is_unicode_c<InputChar>
constexpr std::basic_string<OutputChar>
    unicode_conversion_with_exception(
        const InputChar char_arg
    )
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return throw_if_error<basic_string<OutputChar>, unicode_conversion_error>(
        unicode_conversion<OutputChar>(char_arg),
        u8"unicode_conversion_with_exception"
    );
}

template <typename ArgType>
requires char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr bool
    is_valid_unicode(
        ArgType str_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using InputChar = char_type_of_t<ArgType>;
    auto sv         = basic_string_view<InputChar>(str_arg);
    if constexpr (same_as<InputChar, wchar_t>)
    {
        return is_valid_unicode(cast_wstring_to_unicode_string(sv));
    }
    else if constexpr (same_as<InputChar, char32_t>)
    {
        return not if_invalid_u32string_return_char_and_position(sv).has_value(
        );
    }
    else if constexpr (same_as<InputChar, char16_t>)
    {
        return validate_u16string_and_convert_to_u32string<false, InputChar>(sv)
            .has_value();
    }
    else if constexpr (same_as<InputChar, char8_t>)
    {
        return validate_u8string_and_convert_to_u32string<false>(sv).has_value(
        );
    }
    else
    {
        UNICODE_BRIDGE_STATIC_ASSERT(
            InputChar, "is_valid_unicode invalid for this character type"
        );
    }
}

template <typename InputChar>
requires char_type_is_unicode_c<InputChar>
constexpr bool
    is_valid_unicode(
        const InputChar char_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    if constexpr (same_as<InputChar, char8_t>)
    {
        return is_valid_ascii(char_arg);
    }
    else if constexpr (same_as<InputChar, char16_t>
                       || is_wchar_and_16_bit_c<InputChar>)
    {
        return not (is_surrogate(char_arg));
    }
    else if constexpr (same_as<InputChar, char32_t>
                       || is_wchar_and_32_bit_c<InputChar>)
    {
        return not is_invalid_char32(char_arg);
    }
    else
    {
        UNICODE_BRIDGE_STATIC_ASSERT(
            InputChar, "is_valid_unicode invalid for this character type"
        );
    }
}

template <typename ArgType>
requires is_char_type_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr bool
    is_valid_ascii(
        ArgType str_arg
    ) noexcept
{
    using namespace std;
    using InputChar = char_type_of_t<ArgType>;
    const auto sv   = basic_string_view<InputChar>(str_arg);
    for (const auto& character : sv)
    {
        if (not is_valid_ascii(character))
        {
            return false;
        }
    }
    return true;
}

template <typename InputChar>
requires is_char_type_c<InputChar>
constexpr bool
    is_valid_ascii(
        const InputChar char_arg
    ) noexcept
{
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    if constexpr (std::signed_integral<InputChar>)
    {
        return char_arg >= 0 && char_arg <= ascii_limit<InputChar>();
    }
    else
    {
        return char_arg <= ascii_limit<InputChar>();
    }
}

template <bool Return_Reason, typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::conditional_t<
    Return_Reason,
    next_char32_result<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    next_char32(
        const ItteratorType iterator_arg,
        const ItteratorType itt_end_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using CharT = ItteratorType::value_type;
    return next_char32_internal_with_iterator_checking<
        Return_Reason,
        ItteratorType,
        CharT>(iterator_arg, iterator_arg, itt_end_arg);
}

template <bool Return_Reason, typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::conditional_t<
    Return_Reason,
    next_char32_result<char32_t>,
    std::optional<char32_t>>
    next_char32_and_increment_iterator(
        ItteratorType&      iterator_arg,
        const ItteratorType itt_end_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using CharT = std::iterator_traits<ItteratorType>::value_type;
    return next_char32_and_increment_iterator<
        Return_Reason,
        ItteratorType,
        CharT>(iterator_arg, iterator_arg, itt_end_arg);
}

template <typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::pair<char32_t, std::size_t>
    next_char32_with_exception(
        const ItteratorType iterator_arg,
        const ItteratorType itt_end_arg
    )
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return throw_if_error<pair<char32_t, size_t>, next_char32_error>(
        next_char32<true>(iterator_arg, itt_end_arg),
        u8"next_char32_with_exception"
    );
}

template <typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr char32_t
    next_char32_and_increment_iterator_with_exception(
        ItteratorType&      iterator_arg,
        const ItteratorType itt_end_arg
    )
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return throw_if_error<char32_t, next_char32_error>(
        next_char32_and_increment_iterator<true>(iterator_arg, itt_end_arg),
        u8"next_char32_and_increment_iterator_with_exception"
    );
}

template <typename OutputChar, typename ArgType>
requires is_char_type_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
         && is_char_type_c<OutputChar>
constexpr std::basic_string<OutputChar>
    to_formatted_unicode_string(
        ArgType str_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using InputChar = char_type_of_t<ArgType>;
    basic_string<OutputChar> return_value{};
    auto return_value_inserter{std::back_inserter(return_value)};
    auto sv = basic_string_view<InputChar>(str_arg);
    auto string_iterator{std::begin(sv)};
    auto string_iterator_end{std::end(sv)};
    auto micro_conversion_func
        = [](const u8string_view sv_arg) -> basic_string<OutputChar>
    {
        return basic_string<OutputChar>(sv_arg.begin(), sv_arg.end());
    };
    if constexpr (same_as<char, InputChar>)
    {
        for (; string_iterator != string_iterator_end; ++string_iterator)
        {
            const char character{*string_iterator};
            if (auto special_char_str{special_char_as_string<char8_t>(character)
                };
                special_char_str.has_value())
            {
                return_value.append(
                    micro_conversion_func(special_char_str.value())
                );
            }
            else if (character > ascii_limit<char>() || character < 0)
            {
                return_value.append(micro_conversion_func(
                    represent_char_as_hex_for_printing(character)
                ));
            }
            else
            {
                return_value.push_back(static_cast<char8_t>(character));
            }
        }
    }
    else
    {
        while (string_iterator != string_iterator_end)
        {
            const optional<char32_t> character_opt{
                next_char32_and_increment_iterator<
                    false,
                    decltype(string_iterator)>(
                    string_iterator, string_iterator_end
                )
            };
            if (character_opt.has_value())
            {
                if (auto special_char_str{special_char_as_string<OutputChar>(
                        character_opt.value()
                    )};
                    special_char_str.has_value())
                {
                    return_value.append(special_char_str.value());
                }
                else
                {
                    add_char_to_unicode_string(
                        character_opt.value(), return_value_inserter
                    );
                }
            }
            else
            {
                if constexpr (std::same_as<char32_t, InputChar>)
                {
                    if (is_surrogate<InputChar>(*string_iterator))
                    {
                        return_value.append(micro_conversion_func(
                            represent_char_as_hex_for_printing(
                                static_cast<char16_t>(*string_iterator)
                            )
                        ));
                    }
                    else
                    {
                        return_value.append(micro_conversion_func(
                            represent_char_as_hex_for_printing(*string_iterator)
                        ));
                    }
                }
                else
                {
                    return_value.append(micro_conversion_func(
                        represent_char_as_hex_for_printing(*string_iterator)
                    ));
                }
                ++string_iterator;
            }
        }
    }
    return return_value;
}

template <typename OutputChar, typename InputChar>
requires is_char_type_c<OutputChar> && is_char_type_c<InputChar>
constexpr std::basic_string<OutputChar>
    to_formatted_unicode_string(
        const InputChar char_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    const InputChar                    char_arr[2] = {char_arg, InputChar{}};
    const basic_string_view<InputChar> sv(char_arr, 1);
    return to_formatted_unicode_string<OutputChar>(sv);
}

UNICODE_BRIDGE_INTERNAL_NS_BEGIN

constexpr unicode_conversion_error
    unicode_conversion_error_factory::empty_error() noexcept
{
    return unicode_conversion_error(
        unicode_conversion_error::unicode_conversion_error_code::no_error,
        std::numeric_limits<std::size_t>::max(),
        {u8'\0', u8'\0', u8'\0', u8'\0'},
        std::numeric_limits<std::size_t>::max(),
        std::numeric_limits<std::size_t>::max(),
        {u'\0', u'\0'},
        U'\0',
        false
    );
}

constexpr unicode_conversion_error
    unicode_conversion_error_factory::invalid_leading_byte(
        const std::size_t character_index_arg,
        const char8_t     code_points_arg
    ) noexcept
{
    return unicode_conversion_error(
        unicode_conversion_error::unicode_conversion_error_code::
            invalid_leading_byte,
        character_index_arg,
        {code_points_arg, u8'\0', u8'\0', u8'\0'},
        std::numeric_limits<std::size_t>::max(),
        std::numeric_limits<std::size_t>::max(),
        {u'\0', u'\0'},
        U'\0',
        false
    );
}

constexpr unicode_conversion_error
    unicode_conversion_error_factory::truncated_sequence(
        const std::size_t             character_index_arg,
        const std::array<char8_t, 4>& code_points_arg,
        const std::size_t             code_points_encountered_arg,
        const std::size_t             expected_code_points_size_arg
    ) noexcept
{
    return unicode_conversion_error(
        unicode_conversion_error::unicode_conversion_error_code::
            truncated_sequence,
        character_index_arg,
        code_points_arg,
        code_points_encountered_arg,
        expected_code_points_size_arg,
        {u'\0', u'\0'},
        U'\0',
        false
    );
}

constexpr unicode_conversion_error
    unicode_conversion_error_factory::invalid_continuation_byte(
        const std::size_t             character_index_arg,
        const std::array<char8_t, 4>& u8_code_points_arg,
        const std::size_t             code_points_expected_arg,
        const std::size_t             sub_error_enum_arg
    ) noexcept
{
    return unicode_conversion_error(
        unicode_conversion_error::unicode_conversion_error_code::
            invalid_continuation_byte,
        character_index_arg,
        u8_code_points_arg,
        code_points_expected_arg,
        sub_error_enum_arg,
        {u'\0', u'\0'},
        U'\0',
        false
    );
}

constexpr unicode_conversion_error
    unicode_conversion_error_factory::overlong_encoding(
        const std::size_t             character_index_arg,
        const std::array<char8_t, 4>& u8_code_points_arg,
        const std::size_t             code_points_encountered_arg,
        const char32_t                char32_character_arg
    ) noexcept
{
    return unicode_conversion_error(
        unicode_conversion_error::unicode_conversion_error_code::
            overlong_encoding,
        character_index_arg,
        u8_code_points_arg,
        code_points_encountered_arg,
        std::numeric_limits<std::size_t>::max(),
        {u'\0', u'\0'},
        char32_character_arg,
        false
    );
}

constexpr unicode_conversion_error
    unicode_conversion_error_factory::
        invalid_utf32_code_point_after_utf8_conversion(
            const std::size_t             character_index_arg,
            const std::array<char8_t, 4>& u8_code_points_arg,
            const std::size_t             code_points_encountered_arg,
            const char32_t                char32_character_arg
        ) noexcept
{
    return unicode_conversion_error(
        unicode_conversion_error::unicode_conversion_error_code::
            invalid_utf32_code_point_after_utf8_conversion,
        character_index_arg,
        u8_code_points_arg,
        code_points_encountered_arg,
        std::numeric_limits<std::size_t>::max(),
        {u'\0', u'\0'},
        char32_character_arg,
        false
    );
}

constexpr unicode_conversion_error
    unicode_conversion_error_factory::high_surrogate_then_end_of_stream(
        const std::size_t character_index_arg,
        const char16_t    char16_character_arg,
        const bool        is_wchar_arg
    ) noexcept
{
    return unicode_conversion_error(
        unicode_conversion_error::unicode_conversion_error_code::
            high_surrogate_then_end_of_stream,
        character_index_arg,
        {u8'\0', u8'\0', u8'\0', u8'\0'},
        std::numeric_limits<std::size_t>::max(),
        std::numeric_limits<std::size_t>::max(),
        {char16_character_arg, u'\0'},
        U'\0',
        is_wchar_arg
    );
}

constexpr unicode_conversion_error
    unicode_conversion_error_factory::
        high_surrogate_not_followed_by_low_surrogate(
            const std::size_t character_index_arg,
            const char16_t    char16_first_char_arg,
            const char16_t    char16_second_char_arg,
            const bool        is_wchar_arg
        ) noexcept
{
    return unicode_conversion_error(
        unicode_conversion_error::unicode_conversion_error_code::
            high_surrogate_not_followed_by_low_surrogate,
        character_index_arg,
        {u8'\0', u8'\0', u8'\0', u8'\0'},
        std::numeric_limits<std::size_t>::max(),
        std::numeric_limits<std::size_t>::max(),
        {char16_first_char_arg, char16_second_char_arg},
        U'\0',
        is_wchar_arg
    );
}

constexpr unicode_conversion_error
    unicode_conversion_error_factory::unexpected_low_surrogate(
        const std::size_t character_index_arg,
        const char16_t    char16_character_arg,
        const bool        is_wchar_arg
    ) noexcept
{
    return unicode_conversion_error(
        unicode_conversion_error::unicode_conversion_error_code::
            unexpected_low_surrogate,
        character_index_arg,
        {u8'\0', u8'\0', u8'\0', u8'\0'},
        std::numeric_limits<std::size_t>::max(),
        std::numeric_limits<std::size_t>::max(),
        {char16_character_arg, u'\0'},
        U'\0',
        is_wchar_arg
    );
}

constexpr unicode_conversion_error
    unicode_conversion_error_factory::invalid_utf32_code_point(
        const std::size_t character_index_arg,
        const char32_t    char32_character_arg,
        const bool        is_wchar_arg
    ) noexcept
{
    return unicode_conversion_error(
        unicode_conversion_error::unicode_conversion_error_code::
            invalid_utf32_code_point,
        character_index_arg,
        {u8'\0', u8'\0', u8'\0', u8'\0'},
        std::numeric_limits<std::size_t>::max(),
        std::numeric_limits<std::size_t>::max(),
        {u'\0', u'\0'},
        char32_character_arg,
        is_wchar_arg
    );
}

template <typename T>
unicode_bridge_exception<T>
    unicode_bridge_exception_factory::make(
        const T&                 error_arg,
        const std::u8string_view function_name_arg
    ) noexcept
{
    return unicode_bridge_exception<T>(error_arg, function_name_arg);
}

constexpr ascii_to_unicode_error
    ascii_to_unicode_error_factory::make(
        const std::size_t index_arg,
        const char        character_arg
    ) noexcept
{
    return ascii_to_unicode_error(index_arg, character_arg);
}

constexpr unicode_to_ascii_error
    unicode_to_ascii_error_factory::non_ascii_character_found(
        const char32_t    character_arg,
        const std::size_t index_arg
    ) noexcept
{
    return unicode_to_ascii_error(
        unicode_to_ascii_error::unicode_to_ascii_error_code::
            non_ascii_character_found,
        unicode_conversion_error_factory::empty_error(),
        character_arg,
        index_arg
    );
}

constexpr unicode_to_ascii_error
    unicode_to_ascii_error_factory::invalid_unicode_character(
        const unicode_conversion_error& unicode_error_arg
    ) noexcept
{
    return unicode_to_ascii_error(
        unicode_to_ascii_error::unicode_to_ascii_error_code::
            invalid_unicode_character,
        unicode_error_arg,
        U'\0',
        std::numeric_limits<std::size_t>::max()
    );
}

constexpr next_char32_error
    next_char32_error_factory::make_unicode_error(
        const unicode_conversion_error& error_arg
    ) noexcept
{
    return next_char32_error(
        next_char32_error::next_char32_error_code::unicode_error, error_arg
    );
}

constexpr next_char32_error
    next_char32_error_factory::iterator_exhausted() noexcept
{
    return next_char32_error(
        next_char32_error::next_char32_error_code::iterator_end,
        unicode_conversion_error_factory::empty_error()
    );
}

template <typename ResultT, typename ErrorT>
requires unicode_bridge_error_c<ErrorT>
ResultT
    throw_if_error(
        std::expected<ResultT, ErrorT> result,
        const std::u8string_view       function_name_arg
    )
{
    if (result.has_value())
    {
        return std::move(result.value());
    }
    else
    {
        throw unicode_bridge_exception_factory::make(
            result.error(), function_name_arg
        );
    }
}

template <typename T>
requires is_char_type_c<T>
         && (
             sizeof(T) >= 1
         )
constexpr T ascii_limit() noexcept
{
    return T{0x7F};
}

template <typename T>
requires char_type_is_unicode_c<T>
         && (
             sizeof(T) >= 2
         )
constexpr T single_char16_limit_and_three_char8_limit() noexcept
{
    return 0xFFFF;
}

template <typename T>
requires char_type_is_unicode_c<T>
         && (
             sizeof(T) >= 2
         )
constexpr T two_char8_limit() noexcept
{
    return 0x7FF;
}

template <typename T>
requires char_type_is_unicode_c<T>
         && (
             sizeof(T) >= 4
         )
constexpr T char32_limit() noexcept
{
    return T{0x10'FFFF};
}

template <typename T>
requires char_type_is_unicode_c<T>
         && (
             sizeof(T) >= 2
         )
constexpr T high_surrogate_lower_value() noexcept
{
    return T{0xD800};
}

template <typename T>
requires char_type_is_unicode_c<T>
         && (
             sizeof(T) >= 2
         )
constexpr T low_surrogate_upper_value() noexcept
{
    return T{0xDFFF};
}

inline std::u8string
    to_u8string(
        const std::string_view str_arg
    )
{
    return std::u8string(
        reinterpret_cast<const char8_t*>(str_arg.data()), str_arg.size()
    );
}

template <typename T>
requires std::unsigned_integral<T>
inline std::u8string
    positive_integer_to_placement(
        const T number_arg
    ) noexcept
{
    using namespace std;
    auto suffix_function = [&]() -> u8string_view
    {
        const T normalised_str{static_cast<T>(number_arg % T(100))};
        if (normalised_str > 10 && normalised_str < 14)
        {
            return u8"th";
        }
        else
        {
            switch (static_cast<T>(number_arg % T(10)))
            {
            case 1:
                return u8"st";
            case 2:
                return u8"nd";
            case 3:
                return u8"rd";
            default:
                return u8"th";
            }
        }
    };
    u8string msg;
    msg.append(to_u8string(std::to_string(number_arg)));
    msg.append(suffix_function());
    return msg;
}

template <typename WCharT>
constexpr auto
    cast_wstring_to_unicode_string(
        const std::wstring_view str_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    if constexpr (wchar_is_16_bit)
    {
        return u16string(str_arg.begin(), str_arg.end());
    }
    else if constexpr (wchar_is_32_bit)
    {
        return u32string(str_arg.begin(), str_arg.end());
    }
    else
    {
        UNICODE_BRIDGE_STATIC_ASSERT(
            WCharT,
            "cast_wstring_to_unicode_string not defined for wchar_t of "
            "this "
            "size"
        );
    }
}

template <typename T, bool Use_Capitals, bool Variable_Size_Prefix>
requires is_char_type_c<T>
constexpr std::u8string
    make_hex_from_char_with_prefix(
        const T                  char_arg,
        const std::u8string_view prefix_arg
    ) noexcept
{
    using namespace std;
    using Type_To_Cast_To = char_underlying_type_t<T>;
    u8string        character_as_hex;
    Type_To_Cast_To character_as_casted_type{
        static_cast<Type_To_Cast_To>(char_arg)
    };
    if constexpr (Use_Capitals)
    {
        const char8_t digits[] = u8"0123456789ABCDEF";
        std::u8string buf;
        if (character_as_casted_type == 0)
        {
            buf = u8"0";
        }
        while (character_as_casted_type)
        {
            buf                       += digits[character_as_casted_type & 0xF];
            character_as_casted_type >>= 4;
        }
        // digits accumulated in reverse
        std::reverse(buf.begin(), buf.end());
        character_as_hex = buf;
    }
    else
    {
        const char8_t digits[] = u8"0123456789abcdef";
        std::u8string buf;
        if (character_as_casted_type == 0)
        {
            buf = u8"0";
        }
        while (character_as_casted_type)
        {
            buf                       += digits[character_as_casted_type & 0xF];
            character_as_casted_type >>= 4;
        }
        // digits accumulated in reverse
        std::reverse(buf.begin(), buf.end());
        character_as_hex = buf;
    }
    u8string return_value{prefix_arg};
    if constexpr (Variable_Size_Prefix)
    {
        if (character_as_hex.size() <= 4)
        {
            return_value.append(4 - character_as_hex.size(), u8'0');
        }
        else if (character_as_hex.size() <= 6)
        {
            return_value.append(6 - character_as_hex.size(), u8'0');
        }
        else
        {
            return_value.append(8 - character_as_hex.size(), u8'0');
        }
    }
    else
    {
        return_value.append(u8string(
            (sizeof(Type_To_Cast_To) * 2) - character_as_hex.size(), u8'0'
        ));
    }
    return_value.append(character_as_hex);
    return return_value;
}

template <typename T>
requires is_char_type_c<T>
constexpr std::u8string
    represent_char_as_hex_for_output(
        const T char_arg
    ) noexcept
{
    return UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        make_hex_from_char_with_prefix<T, true, false>(char_arg, u8"0x");
}

constexpr std::optional<std::pair<char32_t, std::size_t>>
    if_invalid_u32string_return_char_and_position(
        const std::u32string_view str_arg
    ) noexcept
{
    using namespace std;
    for (size_t idx{0}; const char32_t character : str_arg)
    {
        if (is_invalid_char32(character))
        {
            return make_optional(make_pair(character, idx));
        }
        ++idx;
    }
    return nullopt;
}

template <bool Return_u32string>
constexpr basic_unicode_result_t<
    std::conditional_t<Return_u32string, std::u32string, std::monostate>>
    validate_u8string_and_convert_to_u32string(
        const std::u8string_view str_arg
    ) noexcept
{
    using namespace std;
    conditional_t<Return_u32string, u32string, monostate> return_value;
    auto str_iterator_end{std::end(str_arg)};
    for (auto str_iterator{std::begin(str_arg)};
         str_iterator != str_iterator_end;)
    {
        auto next_char_result{
            next_char32_internal<true, decltype(str_iterator), char8_t>(
                std::begin(str_arg), str_iterator, str_iterator_end
            )
        };
        if (next_char_result.has_value())
        {
            auto& [character, iterator_offset]{next_char_result.value()};
            std::advance(str_iterator, iterator_offset);
            if constexpr (Return_u32string)
            {
                return_value.push_back(character);
            }
        }
        else
        {
            return unexpected(next_char_result.error());
        }
    }
    return return_value;
}

template <bool Return_u32string, typename Original_Type>
requires char_type_is_unicode_c<Original_Type>
constexpr basic_unicode_result_t<
    std::conditional_t<Return_u32string, std::u32string, std::monostate>>
    validate_u16string_and_convert_to_u32string(
        const std::u16string_view str_arg
    ) noexcept
{
    using namespace std;
    conditional_t<Return_u32string, u32string, monostate> return_value;
    auto str_iterator_end{std::end(str_arg)};
    for (auto str_iterator{std::begin(str_arg)};
         str_iterator != str_iterator_end;)
    {
        auto next_char_result{
            next_char32_internal<true, decltype(str_iterator), Original_Type>(
                std::begin(str_arg), str_iterator, str_iterator_end
            )
        };
        if (next_char_result.has_value())
        {
            auto& [character, iterator_offset]{next_char_result.value()};
            std::advance(str_iterator, iterator_offset);
            if constexpr (Return_u32string)
            {
                return_value.push_back(character);
            }
        }
        else
        {
            return unexpected(next_char_result.error());
        }
    }
    return return_value;
}

template <typename T>
requires is_wchar_and_32_bit_c<T> || std::same_as<T, char32_t>
constexpr bool
    is_invalid_char32(
        const T char_arg
    ) noexcept
{
    return is_surrogate<T>(char_arg) || char_arg > char32_limit<T>();
}

template <typename CharT, typename CharU>
constexpr void
    add_char_to_unicode_string(
        const CharT                                          char_arg,
        std::back_insert_iterator<std::basic_string<CharU>>& inserter_arg
    ) noexcept
{
    using namespace std;
    if constexpr (same_as<CharT, char32_t> && same_as<CharU, char8_t>)
    {
        constexpr char8_t final_six_bits_set{0b0011'1111};
        constexpr char8_t first_four_bits_set{0b1111'0000};
        constexpr char8_t first_three_bits_set{0b1110'0000};
        constexpr char8_t first_bit_set{0b1000'0000};
        if (is_valid_ascii(char_arg))
        {
            // 1-byte UTF-8
            inserter_arg = static_cast<char8_t>(char_arg);
        }
        else if (char_arg <= two_char8_limit<char32_t>())
        {
            // 2-byte UTF-8.
            // Isolate first 6 bits by rshift, then use binary or to set the
            // other bits.
            inserter_arg = static_cast<char8_t>(0b1100'0000 | (char_arg >> 6));
            // Isolate first 6 bits by binary and, then use binary or to set the
            // other bit.
            inserter_arg = static_cast<char8_t>(
                first_bit_set | (char_arg & final_six_bits_set)
            );
        }
        else if (char_arg
                 <= single_char16_limit_and_three_char8_limit<char32_t>())
        {
            // 3-byte UTF-8
            inserter_arg
                = static_cast<char8_t>(first_three_bits_set | (char_arg >> 12));
            inserter_arg = static_cast<char8_t>(
                first_bit_set | ((char_arg >> 6) & final_six_bits_set)
            );
            inserter_arg = static_cast<char8_t>(
                first_bit_set | (char_arg & final_six_bits_set)
            );
        }
        else if (char_arg <= char32_limit<char32_t>())
        {
            // 4-byte UTF-8
            inserter_arg
                = static_cast<char8_t>(first_four_bits_set | (char_arg >> 18));
            inserter_arg = static_cast<char8_t>(
                first_bit_set | ((char_arg >> 12) & final_six_bits_set)
            );
            inserter_arg = static_cast<char8_t>(
                first_bit_set | ((char_arg >> 6) & final_six_bits_set)
            );
            inserter_arg = static_cast<char8_t>(
                first_bit_set | (char_arg & final_six_bits_set)
            );
        }
        else
        {
            std::unreachable();
        }
    }
    else
    {
        auto res = unicode_conversion_with_exception<CharU>(
            basic_string<CharT>(1, char_arg)
        );
        for (auto&& character : res)
        {
            inserter_arg = character;
        }
    }
}

template <bool Return_Reason, typename T, typename Original_Value_Type>
requires char_type_is_unicode_c<typename std::iterator_traits<T>::value_type>
         && char_type_is_unicode_c<Original_Value_Type>
constexpr std::conditional_t<
    Return_Reason,
    next_char32_result<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    next_char32_internal_with_iterator_checking(
        const T iterator_begin_arg,
        const T iterator_arg,
        const T itt_end_arg
    ) noexcept
{
    using namespace std;
    if (iterator_arg == itt_end_arg)
    {
        if constexpr (Return_Reason)
        {
            return unexpected(next_char32_error_factory::iterator_exhausted());
        }
        else
        {
            return nullopt;
        }
    }
    else
    {
        auto res{next_char32_internal<Return_Reason, T, Original_Value_Type>(
            iterator_begin_arg, iterator_arg, itt_end_arg
        )};
        if (res.has_value())
        {
            return res.value();
        }
        else
        {
            if constexpr (Return_Reason)
            {
                return unexpected(
                    next_char32_error_factory::make_unicode_error(res.error())
                );
            }
            else
            {
                return nullopt;
            }
        }
    }
}

template <bool Return_Reason, typename T, typename Original_Value_Type>
requires char_type_is_unicode_c<typename std::iterator_traits<T>::value_type>
         && char_type_is_unicode_c<Original_Value_Type>
constexpr std::conditional_t<
    Return_Reason,
    basic_unicode_result_t<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    next_char32_internal(
        const T iterator_begin_arg,
        const T iterator_arg,
        const T itt_end_arg
    ) noexcept
{
    using namespace std;
    using CharT = std::iterator_traits<T>::value_type;
    using rv_t  = conditional_t<
         Return_Reason,
         basic_unicode_result_t<pair<char32_t, size_t>>,
         optional<pair<char32_t, size_t>>>;
    auto next_from_u8string_function = [&](const T iterator_begin_arg,
                                           const T iterator_arg,
                                           const T itt_end_arg) -> rv_t
    {
        auto       local_iterator{iterator_arg};
        const auto byte_1{*local_iterator};
        if (not (is_valid_ascii(byte_1)))
        {
            constexpr array<char32_t, 3> and_array{
                static_cast<char32_t>(0b0001'1111),
                static_cast<char32_t>(0b0000'1111),
                static_cast<char32_t>(0b0000'0111)
            };
            constexpr array<pair<char8_t, char8_t>, 3> byte_1_and_equal_values{
                {{static_cast<char8_t>(0b1110'0000),
                  static_cast<char8_t>(0b1100'0000)},
                 {static_cast<char8_t>(0b1111'0000),
                  static_cast<char8_t>(0b1110'0000)},
                 {static_cast<char8_t>(0b1111'1000),
                  static_cast<char8_t>(0b1111'0000)}}
            };
            size_t code_point_size{byte_1_and_equal_values.size()};
            for (size_t idx{0}; idx < byte_1_and_equal_values.size(); ++idx)
            {
                const auto& [and_val, equal_val]{byte_1_and_equal_values[idx]};
                if ((byte_1 & and_val) == equal_val)
                {
                    code_point_size = idx;
                    break;
                }
            }
            if (code_point_size == byte_1_and_equal_values.size())
            {
                if constexpr (Return_Reason)
                {
                    return unexpected(
                        unicode_conversion_error_factory::invalid_leading_byte(
                            std::distance(iterator_begin_arg, local_iterator),
                            byte_1
                        )
                    );
                }
                else
                {
                    return nullopt;
                }
            }
            char32_t code_point{
                static_cast<char32_t>(byte_1) & (and_array[code_point_size])
            };
            const size_t code_units_processed_remaining{
                static_cast<size_t>(std::distance(local_iterator, itt_end_arg))
            };
            if (code_units_processed_remaining <= code_point_size + 1)
            {
                if constexpr (Return_Reason)
                {
                    std::array<char8_t, 4> code_units
                        = {static_cast<char8_t>(byte_1),
                           code_units_processed_remaining >= 2
                               ? static_cast<char8_t>(*(local_iterator + 1))
                               : u8'\0',
                           code_units_processed_remaining >= 3
                               ? static_cast<char8_t>(*(local_iterator + 2))
                               : u8'\0',
                           u8'\0'};
                    return unexpected(
                        unicode_conversion_error_factory::truncated_sequence(
                            std::distance(iterator_begin_arg, local_iterator),
                            code_units,
                            code_units_processed_remaining,
                            code_point_size + 2
                        )
                    );
                }
                else
                {
                    return nullopt;
                }
            }
            for (size_t idx{0}; idx <= code_point_size; ++idx)
            {
                ++local_iterator;
                const auto byte_n{*local_iterator};
                if ((byte_n & 0b1100'0000) != 0b1000'0000)
                {
                    if constexpr (Return_Reason)
                    {
                        std::array<char8_t, 4> code_units{
                            static_cast<char8_t>(*iterator_arg),
                            static_cast<char8_t>(*(iterator_arg + 1)),
                            (code_point_size > 0)
                                ? static_cast<char8_t>(*(iterator_arg + 2))
                                : u8'\0',
                            (code_point_size > 1)
                                ? static_cast<char8_t>(*(iterator_arg + 3))
                                : u8'\0',
                        };
                        return unexpected(unicode_conversion_error_factory::
                                              invalid_continuation_byte(
                                                  std::distance(
                                                      iterator_begin_arg,
                                                      local_iterator
                                                  ) - idx
                                                      - 1,
                                                  code_units,
                                                  code_point_size + 2,
                                                  idx + 2
                                              ));
                    }
                    else
                    {
                        return nullopt;
                    }
                }
                // Shifts the bit into the code point. It shifts by
                // 6 due to each byte in a UTF8 string only using
                // the last 6 bits. The logical and isolates these 6
                // bits.
                code_point = (code_point << 6) | (byte_n & 0b0011'1111);
            }
            constexpr array<char32_t, 3> code_point_limits{
                0b1000'0000, 0b1000'0000'0000, 0b0001'0000'0000'0000'0000
            };
            if (code_point < code_point_limits[code_point_size])
            {
                if constexpr (Return_Reason)
                {
                    std::array<char8_t, 4> code_units{
                        static_cast<char8_t>(*iterator_arg),
                        static_cast<char8_t>(*(iterator_arg + 1)),
                        (code_point_size > 0)
                            ? static_cast<char8_t>(*(iterator_arg + 2))
                            : u8'\0',
                        (code_point_size > 1)
                            ? static_cast<char8_t>(*(iterator_arg + 3))
                            : u8'\0',
                    };
                    return unexpected(
                        unicode_conversion_error_factory::overlong_encoding(
                            std::distance(iterator_begin_arg, local_iterator)
                                - ( code_point_size ) -1,
                            code_units,
                            code_point_size + 2,
                            code_point
                        )
                    );
                }
                else
                {
                    return nullopt;
                }
            }
            if (is_invalid_char32(code_point))
            {
                if constexpr (Return_Reason)
                {
                    std::array<char8_t, 4> code_units{
                        static_cast<char8_t>(*iterator_arg),
                        static_cast<char8_t>(*(iterator_arg + 1)),
                        (code_point_size > 0)
                            ? static_cast<char8_t>(*(iterator_arg + 2))
                            : u8'\0',
                        (code_point_size > 1)
                            ? static_cast<char8_t>(*(iterator_arg + 3))
                            : u8'\0',
                    };
                    return unexpected(
                        unicode_conversion_error_factory::
                            invalid_utf32_code_point_after_utf8_conversion(
                                std::distance(
                                    iterator_begin_arg, local_iterator
                                ) - ( code_point_size ) -1,
                                code_units,
                                code_point_size + 2,
                                code_point
                            )
                    );
                }
                else
                {
                    return nullopt;
                }
            }
            else
            {
                return make_pair(code_point, code_point_size + 2);
            }
        }
        else
        {
            return rv_t(make_pair(static_cast<char32_t>(byte_1), 1));
        }
    };
    auto next_from_u16_string = [&](const T iterator_begin_arg,
                                    const T iterator_arg,
                                    const T itt_end_arg) -> rv_t
    {
        using CharT = std::iterator_traits<T>::value_type;
        auto           local_iterator{iterator_arg};
        const char16_t first_element{static_cast<char16_t>(*local_iterator)};
        if (is_high_surrogate(first_element))
        {
            if (std::distance(local_iterator, itt_end_arg) <= 1)
            {
                if constexpr (Return_Reason)
                {
                    return unexpected(
                        unicode_conversion_error_factory::
                            high_surrogate_then_end_of_stream(std::distance(iterator_begin_arg, local_iterator), first_element, same_as<Original_Value_Type, wchar_t>)
                    );
                }
                else
                {
                    return nullopt;
                }
            }
            ++local_iterator;
            const char16_t second_element{static_cast<char16_t>(*local_iterator)
            };
            if (not is_low_surrogate(second_element))
            {
                if constexpr (Return_Reason)
                {
                    return unexpected(
                        unicode_conversion_error_factory::
                            high_surrogate_not_followed_by_low_surrogate(std::distance(iterator_begin_arg, local_iterator) - 1, first_element, second_element, std::same_as<Original_Value_Type, wchar_t>)
                    );
                }
                else
                {
                    return nullopt;
                }
            }
            // Decode surrogate pair to code point
            char32_t character
                = char16_offset_for_char32_conversion<char32_t>();
            character
                += ((static_cast<char32_t>(first_element)
                     - high_surrogate_lower_value<char32_t>())
                    << 10);
            character
                += (static_cast<char32_t>(second_element)
                    - low_surrogate_lower_value<char32_t>());
            return rv_t(make_pair(character, 2));
        }
        else if (is_low_surrogate(first_element))
        {
            // Unpaired low surrogate
            if constexpr (Return_Reason)
            {
                return unexpected(
                    unicode_conversion_error_factory::
                        unexpected_low_surrogate(std::distance(iterator_begin_arg, local_iterator), first_element, same_as<Original_Value_Type, wchar_t>)
                );
            }
            else
            {
                return nullopt;
            }
        }
        else
        {
            return rv_t(make_pair(static_cast<char32_t>(first_element), 1));
        }
    };
    if constexpr (same_as<CharT, char8_t>)
    {
        return next_from_u8string_function(
            iterator_begin_arg, iterator_arg, itt_end_arg
        );
    }
    else if constexpr (same_as<CharT, char16_t> || is_wchar_and_16_bit_c<CharT>)
    {
        return next_from_u16_string(
            iterator_begin_arg, iterator_arg, itt_end_arg
        );
    }
    else if constexpr (same_as<CharT, char32_t> || is_wchar_and_32_bit_c<CharT>)
    {
        const CharT character{*iterator_arg};
        if (is_invalid_char32(character))
        {
            if constexpr (Return_Reason)
            {
                return unexpected(
                    unicode_conversion_error_factory::
                        invalid_utf32_code_point(std::distance(iterator_begin_arg, iterator_arg), character, same_as<Original_Value_Type, wchar_t>)
                );
            }
            else
            {
                return nullopt;
            }
        }
        else
        {
            return make_pair(character, 1);
        }
    }
    else
    {
        UNICODE_BRIDGE_STATIC_ASSERT(
            T, "next_char32_t invalid for this character type"
        );
    }
}

template <bool Return_Reason, typename T, typename Original_Type>
requires char_type_is_unicode_c<typename std::iterator_traits<T>::value_type>
         && char_type_is_unicode_c<Original_Type>
constexpr std::conditional_t<
    Return_Reason,
    next_char32_result<char32_t>,
    std::optional<char32_t>>
    next_char32_and_increment_iterator(
        const T iterator_begin_arg,
        T&      iterator_arg,
        const T itt_end_arg
    ) noexcept
{
    using namespace std;
    auto next_char32_t_result{next_char32_internal_with_iterator_checking<
        Return_Reason,
        T,
        Original_Type>(iterator_begin_arg, iterator_arg, itt_end_arg)};
    // If its a valid unicode character.
    if (next_char32_t_result.has_value())
    {
        const auto& [character, char_size]{next_char32_t_result.value()};
        std::advance(iterator_arg, char_size);
        return conditional_t<
            Return_Reason,
            next_char32_result<char32_t>,
            std::optional<char32_t>>(character);
    }
    else
    {
        if constexpr (Return_Reason)
        {
            return unexpected(next_char32_t_result.error());
        }
        else
        {
            return nullopt;
        }
    }
}

template <typename T>
requires char_type_is_unicode_c<T>
         && (
             sizeof(T) >= 4
         )
constexpr T char16_offset_for_char32_conversion() noexcept
{
    return T{0x1'0000};
}

template <typename T>
requires char_type_is_unicode_c<T>
         && (
             sizeof(T) >= 2
         )
constexpr T high_surrogate_upper_value() noexcept
{
    return T{0xDBFF};
}

template <typename T>
requires char_type_is_unicode_c<T>
         && (
             sizeof(T) >= 2
         )
constexpr T low_surrogate_lower_value() noexcept
{
    return T{0xDC00};
}

template <typename T>
requires is_char_type_c<T>
constexpr T
    zero() noexcept
{
    return T(0x0);
}

template <typename T>
requires char_type_is_unicode_c<T>
         && (
             sizeof(T) >= 2
         )
constexpr bool is_surrogate(
    const T char_arg
) noexcept
{
    return char_arg >= high_surrogate_lower_value<T>()
           && char_arg <= low_surrogate_upper_value<T>();
}

template <typename T>
requires char_type_is_unicode_c<T>
         && (
             sizeof(T) >= 2
         )
constexpr bool is_high_surrogate(
    const T char_arg
) noexcept
{
    return char_arg >= high_surrogate_lower_value<T>()
           && char_arg <= high_surrogate_upper_value<T>();
}

template <typename T>
requires char_type_is_unicode_c<T>
         && (
             sizeof(T) >= 2
         )
constexpr bool is_low_surrogate(
    const T char_arg
) noexcept
{
    return char_arg >= low_surrogate_lower_value<T>()
           && char_arg <= low_surrogate_upper_value<T>();
}

template <typename CharT, typename CharU>
constexpr std::optional<std::basic_string<CharT>>
    special_char_as_string(
        const CharU char_arg
    ) noexcept
{
    using namespace std;
    const auto char_as_u32 = static_cast<char32_t>(char_arg);
    auto       micro_conversion_func
        = [](const u8string_view sv) -> basic_string<CharT>
    {
        return basic_string<CharT>(sv.begin(), sv.end());
    };
    switch (static_cast<char32_t>(char_as_u32))
    {
    case 0x00:
        return micro_conversion_func(u8"\\0");
    case 0x07:
        return micro_conversion_func(u8"\\a");
    case 0x08:
        return micro_conversion_func(u8"\\b");
    case 0x09:
        return micro_conversion_func(u8"\\t");
    case 0x0A:
        return micro_conversion_func(u8"\\n");
    case 0x0B:
        return micro_conversion_func(u8"\\v");
    case 0x0C:
        return micro_conversion_func(u8"\\f");
    case 0x0D:
        return micro_conversion_func(u8"\\r");
    case 0x22:
        return micro_conversion_func(u8"\\\"");
    case 0x27:
        return micro_conversion_func(u8"\\'");
    case 0x5C:
        return micro_conversion_func(u8"\\\\");
    default:
        break;
    }
    if (char_as_u32 < 0x20 || (char_as_u32 >= 0x7F && char_as_u32 <= 0x9F))
    {
        if (char_as_u32 <= 0xFFFF)
        {
            return micro_conversion_func(represent_char_as_hex_for_printing(
                static_cast<char16_t>(char_arg)
            ));
        }
        else
        {
            return micro_conversion_func(
                represent_char_as_hex_for_printing(char_arg)
            );
        }
    }
    else
    {
        if constexpr (same_as<char, CharT>)
        {
            return char_arg > ascii_limit<CharT>()
                       ? make_optional(micro_conversion_func(
                             represent_char_as_hex_for_printing(char_arg)
                         ))
                       : nullopt;
        }
        else
        {
            constexpr std::array special_unicode_chars = {
                char32_t{0x00A0}, // non-breaking space
                char32_t{0x1680}, // ogham space mark
                char32_t{0x180E}, // mongolian vowel separator
                char32_t{0x2000}, // en quad
                char32_t{0x2001}, // em quad
                char32_t{0x2002}, // en space
                char32_t{0x2003}, // em space
                char32_t{0x2004}, // three-per-em space
                char32_t{0x2005}, // four-per-em space
                char32_t{0x2006}, // six-per-em space
                char32_t{0x2007}, // figure space
                char32_t{0x2008}, // punctuation space
                char32_t{0x2009}, // thin space
                char32_t{0x200A}, // hair space
                char32_t{0x200B}, // zero width space
                char32_t{0x200C}, // zero width non-joiner
                char32_t{0x200D}, // zero width joiner
                char32_t{0x2028}, // line separator
                char32_t{0x2029}, // paragraph separator
                char32_t{0x202F}, // narrow no-break space
                char32_t{0x205F}, // medium mathematical space
                char32_t{0x2060}, // word joiner
                char32_t{0x2061}, // function application
                char32_t{0x2062}, // invisible times
                char32_t{0x2063}, // invisible separator
                char32_t{0x2064}, // invisible plus
                char32_t{0x2066}, // left-to-right isolate
                char32_t{0x2067}, // right-to-left isolate
                char32_t{0x2068}, // first strong isolate
                char32_t{0x2069}, // pop directional isolate
                char32_t{0x3000}, // ideographic space
                char32_t{0xFEFF}, // byte order mark
                char32_t{0xFFF9}, // interlinear annotation anchor
                char32_t{0xFFFA}, // interlinear annotation separator
                char32_t{0xFFFB}, // interlinear annotation terminator
                char32_t{0xFFFD}, // replacement character
            };
            return std::ranges::binary_search(
                       special_unicode_chars, static_cast<char32_t>(char_arg)
                   )
                       ? std::make_optional(micro_conversion_func(
                             represent_char_as_hex_for_printing(
                                 static_cast<char16_t>(char_arg)
                             )
                         ))
                       : std::nullopt;
        }
    }
}

template <typename T>
requires is_char_type_c<T>
constexpr std::u8string
    represent_char_as_hex_for_printing(
        const T char_arg
    ) noexcept
{
    auto prefix_func = [&]()
    {
        if constexpr (sizeof(T) == 1)
        {
            return u8"\\x";
        }
        else if constexpr (sizeof(T) == 2)
        {
            return u8"\\u";
        }
        else if constexpr (sizeof(T) == 4)
        {
            return u8"\\U";
        }
        else
        {
            UNICODE_BRIDGE_STATIC_ASSERT(
                T, "prefix_func undefined for this size of type"
            );
        }
    };
    return UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        make_hex_from_char_with_prefix<T, true, false>(char_arg, prefix_func());
}

template <typename T>
inline std::wstring
    cast_unicode_string_to_wstring(
        const T str_arg_view
    )
{
    using namespace std;
    if constexpr (sizeof(typename T::value_type) == sizeof(wchar_t))
    {
        return wstring(str_arg_view.begin(), str_arg_view.end());
    }
    else
    {
        UNICODE_BRIDGE_STATIC_ASSERT(
            wstring,
            "cast_unicode_string_to_wstring called with template parameter "
            "T "
            "that is not equal in size to wchar_t"
        );
    }
}

UNICODE_BRIDGE_INTERNAL_NS_END

UNICODE_BRIDGE_NS_END
