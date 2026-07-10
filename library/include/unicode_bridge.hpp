#pragma once
#include <algorithm>
#include <array>
#include <expected>
#include <optional>
#include <ostream>
#include <string>
#include <type_traits>

// ---- Macros ----

// Macro definitions for namespaces.
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

// Macro for static assert.
#define UNICODE_BRIDGE_STATIC_ASSERT(type_arg, msg_arg) \
    static_assert(dependent_false<type_arg>, msg_arg);

// Core namespace.
UNICODE_BRIDGE_NS_BEGIN

UNICODE_BRIDGE_INTERNAL_NS_BEGIN
// ---- Internal Concepts ----

// Ideally these should be in the internal namespace much further down, however
// as you cannot forward declare namespaces, and they are required for core
// concepts, they are given here.
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

// ---- Empty structs used for concepts ----
/*!
 * @brief Represents a string type argument.
 */
struct string_arg
{};

/*!
 * @brief Represents a forward iterator argument.
 */
struct forward_iterator_arg
{};

/*!
 * @brief Represents a backward iterator argument.
 */
struct backward_iterator_arg
{};

/*!
 * @brief Concept to ensure that the template is one of the three argument types
 * described above.
 */
template <typename Arg_Type>
concept is_arg_type_c = std::same_as<Arg_Type, string_arg>
                        || std::same_as<Arg_Type, forward_iterator_arg>
                        || std::same_as<Arg_Type, backward_iterator_arg>;
UNICODE_BRIDGE_INTERNAL_NS_END

// ---- Concepts ----

/*!
 * @brief Used for recognising Unicode characters.
 */
template <typename CharT>
concept char_type_is_unicode_c
    = std::same_as<CharT, char8_t> || std::same_as<CharT, char16_t>
      || std::same_as<CharT, char32_t> || std::same_as<wchar_t, CharT>;
/*!
 * @brief Used for recognising character types in C++ - both Unicode and ASCII.
 */
template <typename CharT>
concept is_char_type_c
    = char_type_is_unicode_c<CharT> || std::same_as<CharT, char>;
/*!
 * @brief Alias template for representing the internal character type of some
 * string-like object.
 * @tparam String_Like_Type The string-like object type.
 */
template <typename String_Like_Type>
using char_type_of_t = typename UNICODE_BRIDGE_NAMESPACE_INTERNAL::char_type_of<
    String_Like_Type>::type;
UNICODE_BRIDGE_INTERNAL_NS_BEGIN

// ---- Internal concepts ----
// Defined here as require char_type_is_unicode_c as defined above.
/*!
 * @brief Primary template used to obtain the underlying character type of some
 * generic argument representing a string.
 */
template <typename T>
struct complete_string_arg_char_type;

/*!
 * @brief Partial specialisation for monostate.
 */
template <>
struct complete_string_arg_char_type<std::monostate>
{
    using type = void;
};

/*!
 * @brief Partial specialisation for a pair of iterators.
 */
template <typename X>
requires std::input_or_output_iterator<X>
struct complete_string_arg_char_type<std::pair<X, X>>
{
    using type = typename std::iterator_traits<X>::value_type;
};

/*!
 * @brief Partial specialisation for string_view.
 */
template <typename CharT, typename Traits>
struct complete_string_arg_char_type<std::basic_string_view<CharT, Traits>>
{
    using type = CharT;
};

/*!
 * @brief Concept for a generic Unicode string type that can either be a
 * monostate (no string provided), a pair of iterators or a basic_string_view.
 *
 * The template type of the iterators value_type and the basic_string_view's
 * internal type must be a unicode character.
 */

template <typename T>
concept is_complete_unicode_string_arg_type_c
    = std::same_as<typename complete_string_arg_char_type<T>::type, void>
      || (std::convertible_to<T, std::basic_string_view<char_type_of_t<T>>>
          && char_type_is_unicode_c<

              typename complete_string_arg_char_type<
                  std::basic_string_view<char_type_of_t<T>>>::type>);
/*!
 * @brief Concept for a generic ASCII string type that can either be a
 * monostate (no string provided), a pair of iterators or a basic_string_view.
 *
 * The template type of the iterators value_type and the basic_string_view's
 * internal type must be a unicode character.
 */
template <typename T>
concept is_complete_ascii_string_arg_type_c
    = std::same_as<typename complete_string_arg_char_type<T>::type, void>
      || std::same_as<char, typename complete_string_arg_char_type<T>::type>;
UNICODE_BRIDGE_INTERNAL_NS_END
// ---- Forward declarations ----
struct basic_unicode_error;
struct forward_scan_unicode_error;
struct unicode_conversion_error;
struct ascii_to_unicode_error;
struct prev_char32_error;
struct unicode_to_ascii_error;
struct next_char32_error;
UNICODE_BRIDGE_INTERNAL_NS_BEGIN
// ---- Internal namespace forward declarations ----
struct basic_unicode_error_factory;
struct forward_scan_unicode_error_factory;
struct unicode_to_ascii_error_factory;
struct prev_char32_error_factory;
struct next_char32_error_factory;
UNICODE_BRIDGE_INTERNAL_NS_END
// ---- Type Synonyms ----
/*!
 * @brief Type synonym for the result type from next_char32 and related
 * functions.
 * @tparam T The result type.
 */
template <typename T>
using next_char32_result = std::expected<T, next_char32_error>;
/*!
 * @brief Type synonym for the result type from unicode_to_ascii and related
 * functions.
 * @tparam T The result type.
 */
template <typename T>
using unicode_to_ascii_result = std::expected<T, unicode_to_ascii_error>;
/*!
 * @brief Type synonym for the result type from unicode_to_ascii and related
 * functions.
 * @tparam T The result type.
 */
template <typename T>
using ascii_to_unicode_result = std::expected<T, ascii_to_unicode_error>;
/*!
 * @brief Type synonym for the result type from unicode_conversion and related
 * functions.
 * @tparam T The result type.
 */
template <typename T>
using unicode_conversion_result = std::expected<T, unicode_conversion_error>;
/*!
 * @brief Type synonym for the result type from prev_char32 and related
 * functions.
 * @tparam T The result type.
 */
template <typename T>
using prev_char32_result = std::expected<T, prev_char32_error>;

// ---- Concepts ----
// Requires the above forward declarations.

/*!
 * @brief A concept to ensure that the error type is one of the exposed error
 * types from the public functions in unicode_bridge.
 */
template <typename T>
concept is_error_type_c = std::same_as<T, unicode_conversion_error>
                          || std::same_as<T, ascii_to_unicode_error>
                          || std::same_as<T, prev_char32_error>
                          || std::same_as<T, unicode_to_ascii_error>
                          || std::same_as<T, next_char32_error>;

// ---- Error Types ----

/*
 * DESIGN INFO:
 *
 * Error types in unicode_bridge are designed with a hierarchy in mind.
 * Succintly, basic_unicode_error encodes all the data associated with
 * generic unicode errors. Nearly all other error types include an
 * instance of it, either directly or via forward_scan_unicode_error.
 *
 * forward_scan_unicode_error builds on basic_unicode_error. It includes
 * no additional data types instead, through its factory constructor and
 * message functionality, it allows the building of errors encountered
 * when reading a Unicode stream in a forwards direction.
 *
 * unicode_conversion_error describes an error type produced when
 * processing an entire string of characters. It has an instance of
 * forward_scan_unicode_error as a meber variable. It includes a
 * std::size_t representing a position of where the error occoured.
 *
 * next_char32_error describes error types created when extracting single
 * char32 characters from some unicode iterator moving in a left-to-right
 * direction. It contains an instance of forward_scan_unicode_error, with
 * only a single additional error representing being at an iterator's end.
 *
 * prev_char32_error describes error types created when extracting a
 * single char32_t character from some unicode iterator moving in a
 * right-to-left direction. It includes an instance of
 * basic_unicode_error.
 *
 *
 * unicode_to_ascii_error describes error types found when converting
 * unicode to ascii. It contains an instance of unicode_conversion_error,
 * for representing errors based around invalid unicode, and additional
 * constructs for when the Unicode is valid, but cannot be converted to
 * ASCII.
 *
 * ascii_to_unicode_error describes errors encountered when converting
 * ASCII to unicode. It is unconnected to the other error types.
 *
 * The reader may note there is no direct replacement for
 * next_char32_error's use of forward_scan_unicode_error in
 * prev_char32_error. That way the hierarchy of errors is setup allows for
 * forward_scan_unicode_error to be used by both next_char32_error and
 * unicode_conversion_error, and the errors which include instances of it.
 *
 * It would have been possible to mirror this hierarchy for
 * prev_char32_error, however realistically there is no use. We do not
 * have conversion functions that process Unicode strings backwards and,
 * as for now, we cannot see any advantage in having such functions.
 *
 * Finally, there is a generic exception type built around these types,
 * called unicode_bridge_exception, which is used with the
 * exception-throwing variants of the core functions exported from this
 * library.
 */

/*!
 * @brief The base error type in unicode_bridge. Contains all the data
 * required to store errors found when processing Unicode data, allowing
 * the user to inspect the error, or use the in-built error messages to
 * decipher what went wrong.
 */
struct basic_unicode_error
{
public:
    // ---- Friend types ----
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        basic_unicode_error_factory;
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        unicode_to_ascii_error_factory;
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        forward_scan_unicode_error_factory;
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::next_char32_error_factory;
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::prev_char32_error_factory;
    friend struct prev_char32_error;
    friend struct unicode_conversion_error;
    friend struct unicode_to_ascii_error;
    friend struct forward_scan_unicode_error;
    /*!
     * @brief Internal enum for representing Unicode errors that can be
     * encountered when moving forwards or backwards through a Unicode string.
     */
    enum class basic_unicode_error_code : uint8_t
    {
        // ---- UTF-8 Errors ----
        /*!
         * @brief In UTF-8, it is possible to encode a single Unicode character
         * in several different ways using up to four bytes. For example, using
         * 1 and 2 bytes.
         *
         * 0111111.
         *
         * 11000001 10111111.
         *
         * These encode the same unicode character. The Unicode standard
         * stipulates that the smallest encoding should always be used -
         * anything else is an error. overlong_encoding is used to represent
         * this error. It uses:
         * - _u8_code_points. Contains the original bytes encountered.
         * - _auxillery_data. Denotes the number of _u8_code_points used.
         * - _char32_character. The char32_t character encoded by
         * _u8_code_points.
         *
         */
        overlong_encoding,
        /*!
         * @brief UTF-8 encodes UTF-32 characters using between 1 and 4 bytes.
         * However, not all UTF-32 characters are valid unicode.
         *
         * For example, the unicode character 0000DFFF is invalid unicode,
         * however it can be encoded in UTF8.
         *
         *
         * Uses:
         * - _u8_code_points. Contains the original bytes encountered.
         * - _auxillery_data. Contains the number of _u8_code_points used.
         * - _char32_character. The invalid char32_t character encoded by
         * _u8_code_points.
         *
         */
        invalid_utf32_code_point_after_utf8_conversion,
        // ---- UTF-32 Errors ----
        /*!
         * In UTF-32, unicode characters are single code points.
         *
         * Not all UFF-32 characters are valid unicode characters. This enum
         * represents an invalid character.
         *
         * Uses:
         * - _char32_character to represent the char32_t character.
         */
        invalid_utf32_code_point,
        /*!
         * @brief All errors are encoded in terms of basic_unicode_error. For
         * those erros that are defined by some parent class, this enum is used.
         */
        generic_error
    };
private:
    //! Enum error code.
    basic_unicode_error_code _code;
    //! Stores UTF-8 bytes.
    std::array<char8_t, 4> _u8_code_points;
    //! Stores a single UTF-32 character.
    char32_t _char32_character;
    //! Auxillery data. Used for storing limits on arrays and enum values.
    std::uint8_t _auxillery_data;
    //! Stores two UTF-16 characters.
    std::array<char16_t, 2> _u16_code_points;
    //! Denotes whether the original source was a wchar_t character.
    bool _is_wchar;
    /*!
     * @brief Constructs basic_unicode_error object.
     * @param code_arg The enum value.
     * @param u8_code_points_arg The UTF-8 array value.
     * @param char32_character_arg The UTF-32 character value.
     * @param auxillery_data_arg The auxillery data value.
     * @param u16_code_points_arg The UTF-16 array value.
     * @param is_wchar_arg The wchar_t boolean.
     */
    constexpr basic_unicode_error(
        const basic_unicode_error_code code_arg,
        const std::array<char8_t, 4>&  u8_code_points_arg,
        const char32_t                 char32_character_arg,
        const std::uint8_t             auxillery_data_arg,
        const std::array<char16_t, 2>& u16_code_points_arg,
        const bool                     is_wchar_arg
    ) noexcept;
    /*!
     * @brief Creates a generic string representing the start of an error
     * message.
     *
     * It creates a specific message centered around the specific offsets into
     * the string described by the input parameters.
     *
     * @tparam String_Type The string type. Must adhere to
     * is_complete_string_arg_type_c.
     * @tparam Arg_Type The argument type. Tells the function what order the
     * iterator is in (if the string argument is an iterator).
     * @param character_index_arg The first index of the byte of interest.
     * @param chars_as_hex_arg A text representation of the bytes of interest.
     * @param n_code_units_arg The number of bytes of interest.
     * @param utf_standard_arg A const char* representing the UTF standard.
     * @param string_type_arg The string of interest.
     * @return A u8string representing the beginning of the error message.
     */
    template <typename Arg_Type, typename String_Type>
    requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
                 is_complete_unicode_string_arg_type_c<String_Type>
             && UNICODE_BRIDGE_NAMESPACE_INTERNAL::is_arg_type_c<Arg_Type>
    std::u8string
        generic_begin_str(
            const std::size_t    character_index_arg,
            const std::u8string& chars_as_hex_arg,
            const size_t         n_code_units_arg,
            const char8_t*       utf_standard_arg,
            const String_Type&   string_type_arg
        ) const noexcept;
    /*!
     * @brief Given a character index and a string object, creates the error
     * message for the object.
     *
     * This function specifically creates the messages for the enums in
     * basic_unicode_error_code.
     *
     * The string object can either be a std::monstate (no string), a pair of
     * iterators, or a single string_view.
     *
     * Note that this will only work correctly on error enums explicitly defined
     * by the basic_unicode_error_code, except generic_error_code.
     * @tparam String_Type The string type. Must adhere to
     * is_complete_string_arg_type_c.
     * @tparam Arg_Type The argument type. Tells the function what order the
     * iterator is in (if the string argument is an iterator).
     * @param character_index_arg The first index of the byte of interest.
     * @param string_arg The string of interest.
     * @return The error message representing the object.
     */
    template <typename Arg_Type, typename String_Type>
    requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
                 is_complete_unicode_string_arg_type_c<String_Type>
             && UNICODE_BRIDGE_NAMESPACE_INTERNAL::is_arg_type_c<Arg_Type>
    constexpr std::u8string
        internal_message(
            const std::size_t  character_index_arg,
            const String_Type& string_arg
        ) const noexcept;
public:
    /*!
     * @brief Gets the object's code variable.
     * @return The object's code variable.
     */
    constexpr const basic_unicode_error_code
        code() const noexcept;
    /*!
     * @brief Gets the object's u8_code_points variable.
     * @return The object's u8_code_points variable.
     */
    constexpr const std::array<char8_t, 4>&
        u8_code_points() const noexcept;
    /*!
     * @brief Gets the object's char32_character variable.
     * @return The object's char32_character variable.
     */
    constexpr const char32_t
        char32_character() const noexcept;
    /*!
     * @brief Gets the object's auxillery_data variable.
     *
     * Note that, depending on what type of error this is, this data variable
     * may represnt an enum. See truncated_sequence_sub_error and
     * invalid_continuation_byte_sub_error.
     *
     * @return The object's auxillery_data variable.
     */
    constexpr const std::uint8_t
        auxillery_data() const noexcept;
    /*!
     * @brief Gets the object's u16_code_points variable.
     * @return The object's u16_code_points variable.
     */
    constexpr const std::array<char16_t, 2>&
        u16_code_points() const noexcept;
    /*!
     * @brief Gets the object's is_wchar variable.
     * @return The object's is_wchar variable.
     */
    constexpr const bool
        is_wchar() const noexcept;
};

/*!
 * @brief The error type used for errors encountered when scanning some stream
 of characters in a left-to-right direction. It builds heavily on
 basic_unicode_error. The only public functions are those which allow the user
 to extract internal data.
 */
struct forward_scan_unicode_error
{
public:
    // ---- Friend types ----
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        unicode_to_ascii_error_factory;
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        forward_scan_unicode_error_factory;
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::next_char32_error_factory;
    friend struct unicode_to_ascii_error;
    friend struct next_char32_error;
    friend struct unicode_conversion_error;
    /*!
     * @brief Enum type describing the various types of encoding erros that can
     * be found when reading a Unicode string from left-to-right. The comments
     * show which data members of basic_unicode_error are used for each specific
     * error.
     *
     * _code (the instance in this class) are used for all errors.
     */
    enum class forward_scan_unicode_error_code : uint8_t
    {
        /*!
         * Enum used to represent an error defined within basic_unicode_error's
         * enum basic_unicode_error_code.
         */
        basic_unicode_error,
        // ---- UTF-8 Errors ----
        /*!
         * UTF8 strings are in groups of 1-4 bytes. If a 1 byte code point, in
         * binary its in the form 0xxxxxxx. If in 2-4 bytes, the first byte is
         * in the form 110xxxxx, 1110xxxx or 11110xxx. If that first byte is not
         * in any of these forms, it is an invalid_leading_byte.
         *
         * Uses:
         * - _u8_code_points (only index 0)
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
         *
         * Uses:
         * - _u8_code_points (either index 0, indexes {0,1} or indexes {0,1,2}
         * - _u8_sub_error_code. Contains the error code (casted to a uint8_t).
         * See truncated_sequence_sub_error.
         */
        truncated_sequence,
        /*!
         * @brief In a 2-4 byte code point, the format of the 2nd to
         * (potentially) 4th bytes is in the form 10xxxxxx.
         *
         *
         * If any of these bytes (called continuation bytes) are not in this
         * format, this error is thrown.
         *
         * Uses:
         * - _u8_code_points (anywhere from indexes {0,1} to {0,1,2,3} used)
         * - _u8_sub_error_code. Contains the erorr code. See
         * invalid_continuation_byte_sub_error.
         *
         */
        invalid_continuation_byte,
        // ---- UTF-16 Errors ----
        /*!
         * In UTF-16, Unicode characters are either single code points, or pairs
         * of code points. The pairs are high surrogates followed by low
         * surrogats.
         *
         * If a high surrogate is followed by the end of the stream, then
         * high_surrogate_then_end_of_stream is used.
         *
         * Uses:
         * - _u16_code_points (only first element).
         * - _is_whcar. If the original encoding used wchar_t.
         */
        high_surrogate_then_end_of_stream,
        /*!
         * In UTF-16, Unicode characters are either single code points, or pairs
         * of code points. The pairs are high surrogates followed by low
         * surrogats.
         *
         * If a high surrogate is not followed by a low surrogate, then
         * high_surrogate_not_followed_by_low_surrogate is used.
         *
         * Uses:
         * - _u16_code_points (both elements).
         * - _is_whcar. If the original encoding used wchar_t.
         */
        high_surrogate_not_followed_by_low_surrogate,
        /*!
         * In UTF-16, Unicode characters are either single code points, or pairs
         * of code points. The pairs are high surrogates followed by low
         * surrogats.
         *
         * If a low surrogate is encountered unexpectadly (e.g. at the start of
         * a string object), then unexpected_low_surrogate is used.
         *
         * Uses:
         * - _u16_code_points (first element).
         * - _is_whcar. If the original encoding used wchar_t.
         */
        unexpected_low_surrogate,
        /*!
         * @brief In some error hierarchies, an instance of this object is used,
         * however the error is defined by the parent object. This enum is used
         * to represent that.
         *
         * An example if unicode_to_ascii_error. Consider a UTF-8 string where
         * the Unicode elements are valid, but they are not ASCII.
         *
         * It contains an error that represents this. So as to reduce error
         * field duplication, it encodes the error inside a
         * forward_scan_unicode_error object, using this enum to signal that the
         * error is handled differently.
         */
        generic_error,
    };
    /*!
     * @brief Enum which encodes the truncated_sequence error. The names of
     * the enums are in the form expected_N_found_M, where N is the number of
     * expected bytes, and M is the number of bytes encountered.
     */
    enum class truncated_sequence_sub_error : uint8_t
    {
        expected_2_found_1,
        expected_3_found_1,
        expected_3_found_2,
        expected_4_found_1,
        expected_4_found_2,
        expected_4_found_3
    };
    /*!
     * @brief Enum which encodes invalid continuation byte errors. Names of
     * enums are in the form size_X_invalid_indexes_Y, where X is the number of
     * bytes expected based on the leading byte, and Y is the set of
     * continuation bytes which are invalid (indexing starts from 0).
     */
    enum class invalid_continuation_byte_sub_error : uint8_t
    {
        size_2_invalid_indexes_1,
        size_3_invalid_indexes_1,
        size_3_invalid_indexes_2,
        size_3_invalid_indexes_1_2,
        size_4_invalid_indexes_1,
        size_4_invalid_indexes_2,
        size_4_invalid_indexes_3,
        size_4_invalid_indexes_1_2,
        size_4_invalid_indexes_1_3,
        size_4_invalid_indexes_2_3,
        size_4_invalid_indexes_1_2_3,
    };
private:
    //! Internal enum which represents the type of error.
    forward_scan_unicode_error_code _code;
    //! Internal basic_unicode_error, which holds all the data about the error.
    basic_unicode_error _basic_unicode_error;
    /*!
     * @brief Constructor.
     * @param code_arg Used to initialise _code.
     * @param basic_unicode_error_arg Used to initialise _basic_unicode_error
     */
    constexpr forward_scan_unicode_error(
        const forward_scan_unicode_error_code code_arg,
        const basic_unicode_error&            basic_unicode_error_arg
    ) noexcept;
    /*!
     * @brief Given a character index and a string object, creates the error
     * message for the object.
     *
     * This function specifically creates the messages for the enums in
     * forward_scan_unicode_error_code.
     *
     * The string object can either be a std::monstate (no string), a pair of
     * iterators, or a single string_view.
     *
     * Note that this will only work correctly on error enums explicitly defined
     * by the basic_unicode_error_code, except generic_error_code.
     * @tparam String_Type The string type. Must adhere to
     * is_complete_string_arg_type_c.
     * @tparam Arg_Type The argument type. Tells the function what order the
     * iterator is in (if the string argument is an iterator).
     * @param character_index_arg The first index of the byte of interest.
     * @param string_arg The string of interest.
     * @return The error message representing the object.
     */
    template <typename Arg_Type, typename String_Type>
    requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
                 is_complete_unicode_string_arg_type_c<String_Type>
             && UNICODE_BRIDGE_NAMESPACE_INTERNAL::is_arg_type_c<Arg_Type>
    constexpr std::u8string
        message(
            const std::size_t  character_index_arg,
            const String_Type& string_arg
        ) const noexcept;
public:
    /*!
     * @brief Gets the object's forward_scan_unicode_error_code variable.
     * @return The object's forward_scan_unicode_error_code variable.
     */
    constexpr const forward_scan_unicode_error_code
        code() const noexcept;
    /*!
     * @brief Gets a reference to the object's basic_unicode_error variable.
     * @return A reference to the object's basic_unicode_error variable.
     */
    constexpr const basic_unicode_error&
        error() const noexcept;
};

/*!
 * @brief Represents a unicode error found when converting a complete string.
 * The error is used when scanning a complete string forwards.
 *
 * It uses an instance of forward_scan_unicode_error internally.
 */
struct unicode_conversion_error
{
public:
    // ---- Friend types ----
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        unicode_to_ascii_error_factory;
    friend struct unicode_to_ascii_error;
private:
    //! Internal forward_scan_unicode_error object containing error data.
    forward_scan_unicode_error _forward_scan_unicode_error;
    //! Contains position of where error occoured.
    std::size_t _character_index;
    /*!
     * @brief Given a string object, creates the error
     * message for the object.
     *
     * The string object can either be a std::monstate (no string), a pair of
     * iterators, or a single string_view.
     *
     * @tparam String_Type The string type. Must adhere to
     * is_complete_unicode_string_arg_type_c.
     * @tparam Arg_Type The argument type. Tells the function what order the
     * iterator is in (if the string argument is an iterator).
     * @param str_arg The string of interest.
     * @return The error message representing the object.
     */
    template <typename Arg_Type, typename String_Type>
    requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
                 is_complete_unicode_string_arg_type_c<String_Type>
             && UNICODE_BRIDGE_NAMESPACE_INTERNAL::is_arg_type_c<Arg_Type>
    constexpr std::u8string
        message(const String_Type& str_arg) const noexcept;
public:
    /*!
     * @brief Constructor.
     * @param character_index_arg Sets _character_index.
     * @param basic_unicode_error_arg Sets _basic_unicode_error.
     */
    constexpr unicode_conversion_error(
        const std::size_t                 character_index_arg,
        const forward_scan_unicode_error& basic_unicode_error_arg
    ) noexcept;
    /*!
     * @brief Gets the object's forward_scan_unicode_error.
     * @return The object's forward_scan_unicode_error.
     */
    constexpr const forward_scan_unicode_error&
        error() const noexcept;
    /*!
     * @brief Gets the object's std::size_t representing where the error is.
     * @return Represents where the error is in the input string.
     */
    constexpr const std::size_t
        character_index() const noexcept;
    /*!
     * @brief Creates error message.
     *
     * This variant is used when the original input string is not known.
     *
     * @return A std::u8string representing the error.
     */
    constexpr std::u8string
        message() const noexcept;
    /*!
     * @brief Creates error message.
     *
     * This variant is used when the original input string is known.
     *
     * @tparam CharT The type of the input string.
     * @param string_arg The original input string.
     * @return A std::u8string representing the error.
     */
    template <typename CharT>
    requires char_type_is_unicode_c<CharT>
    constexpr std::u8string
        message(const std::basic_string_view<CharT>& str_arg) const noexcept;
    template <typename CharT>
        requires char_type_is_unicode_c<CharT>
    constexpr std::u8string
        message(const CharT char_arg) const noexcept;
};

/*!
 * @brief Error for when a unicode_to_ascii function fails.
 */
struct ascii_to_unicode_error
{
private:
    //! Character found that is invalid.
    char _character;
    //! The index of the character.
    std::size_t _index;
    /*!
     * @brief Given an ASCII string object, creates the error
     * message for the object.
     *
     * The string object can either be a std::monstate (no string), a pair of
     * iterators, or a single string_view.
     *
     * @tparam String_Type The string type. Must adhere to
     * is_complete_ascii_string_arg_type_c.
     * @tparam Arg_Type The argument type. Tells the function what order the
     * iterator is in (if the string argument is an iterator).
     * @param str_arg The string of interest.
     * @return The error message representing the object.
     */
    template <typename Arg_Type, typename String_Type>
    requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
                 is_complete_ascii_string_arg_type_c<String_Type>
             && UNICODE_BRIDGE_NAMESPACE_INTERNAL::is_arg_type_c<Arg_Type>
    constexpr std::u8string
        message(const String_Type& str_arg) const;
public:
    /*!
     * @brief Constructor
     * @param index_arg Internal index of error.
     * @param character_arg Internal invalid character.
     */
    constexpr ascii_to_unicode_error(
        const std::size_t index_arg,
        const char        character_arg
    ) noexcept;
    /*!
     * @brief Returns the invalid vharacter _character.
     * @return The invalid char _character.
     */
    constexpr char
        get_character() const noexcept;
    /*!
     * @brief Returns the invalid vharacter _character.
     * @return The invalid char _character.
     */
    constexpr std::size_t
        get_index() const noexcept;
    /*!
     * @brief Creates and returns the error message for this error.
     *
     * @param str_arg The string that originally created the error. Used to
     * create the output message.
     * @return A string representing the error.
     */
    constexpr std::u8string
        message(const std::string_view complete_string_arg) const;
    constexpr std::u8string
        message(const char char_arg) const;
    /*!
     * @brief Creates and returns the error message for this error.
     *
     * @return A string representing the error.
     */
    constexpr std::u8string
        message() const;
};

/*!
 * @brief Error type for when a conversion from Unicode to ASCII fails.
 *
 * Uses an instance of unicode_conversion_error internally.
 */
struct unicode_to_ascii_error
{
public:
    // ---- Friend types ----
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        unicode_to_ascii_error_factory;

    /*!
     * @brief Enum representing the different types of error this class can
     * represent.
     *
     * There is an enum for each error representative of a conversion from
     * UTF-8, 16 and 32.
     */
    enum unicode_to_ascii_error_code
    {
        /*!
         * @brief Represents an error found when converting from UTF-8 to ASCII.
         */
        non_ascii_character_found_from_utf8,
        /*!
         * @brief Represents an error found when converting from UTF-16 to
         * ASCII.
         */
        non_ascii_character_found_from_utf16,
        /*!
         * @brief Represents an error found when converting from UTF-32 to
         * ASCII.
         */
        non_ascii_character_found_from_utf32,
        /*!
         * @brief Represents an error representing invalid Unicode, using the
         * internal unicode_to_ascii_error_code variable to describe it.
         */
        invalid_unicode_character
    };
private:
    //! Local enum representing the error.
    unicode_to_ascii_error_code _code;
    //! Local variable containing all the error data.
    unicode_conversion_error _error;
    /*!
     * @brief Constructor
     * @param code_arg Value to set _code to
     * @param error_arg Value to set _error to.
     */
    constexpr unicode_to_ascii_error(
        const unicode_to_ascii_error_code code_arg,
        const unicode_conversion_error&   error_arg
    ) noexcept;
    /*!
     * @brief Given a Unicode string object, creates the error
     * message for the object.
     *
     * The string object can either be a std::monstate (no string), a pair of
     * iterators, or a single string_view.
     *
     * Note, the name change compared to many other structs in this library is
     * due to there being a name conflict.
     *
     * @tparam String_Type The string type. Must adhere to
     * is_complete_unicode_string_arg_type_c.
     * @tparam Arg_Type The argument type. Tells the function what order the
     * iterator is in (if the string argument is an iterator).
     * @param str_arg The string of interest.
     * @return The error message representing the object.
     */
    template <typename String_Type>
    requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        is_complete_unicode_string_arg_type_c<String_Type>
        constexpr std::u8string
        internal_message(String_Type str_arg) const;
public:
    /*!
     * @brief Gets the internal _code variable.
     * @return The object's _code variable.
     */
    constexpr const unicode_to_ascii_error_code
        get_enum() const noexcept;
    /*!
     * @brief Gets a const reference to the object's _error variable.
     * @return A cref to the object's _error variable.
     */
    constexpr const unicode_conversion_error&
        error() const noexcept;
    /*!
     * @brief Creates error message for the object.
     *
     * This variant is used when the original input string is not known.
     *
     * @return A std::u8string representing the error.
     */
    constexpr std::u8string
        message() const;
    /*!
     * @brief Creates error message for the object.
     *
     * This variant is used when the original input string is known.
     *
     * @return A std::u8string representing the error.
     */
    template <typename ArgType>
    requires char_type_is_unicode_c<char_type_of_t<ArgType>>
             && std::convertible_to<
                 ArgType,
                 std::basic_string_view<char_type_of_t<ArgType>>>
    constexpr std::u8string
        message(ArgType str_arg) const;
    template <typename CharT>
    requires char_type_is_unicode_c<CharT>
    constexpr std::u8string
        message(const CharT char_arg) const;
};

/*!
 * @brief This error type describes a Unicdoe error encountered when moving
 * through an iterator describing some Unicode string.
 *
 * Uses an instance of forward_scan_unicode_error internally.
 */
class next_char32_error
{
public:
    // ---- Friend types ----
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::next_char32_error_factory;
    /*!
     * @brief Enum describing the different types of error next_char32_error can
     * hold.
     */
    enum class next_char32_error_code
    {
        /*!
         * @brief Signifies that the error is completely encoded in the _error
         * variable.
         */
        unicode_error,
        /*!
         * @brief Signifies that the error corresponds to the iterator being
         * exhausted - no more values can be extracted from it.
         */
        iterator_end
    };
private:
    //! Local enum representing the type of error.
    next_char32_error_code _code;
    //! Local variable encoding certain types of error.
    forward_scan_unicode_error _error;
    /*!
     * @brief Constructor.
     * @param code_arg Value to set _code to.
     * @param error_arg Value to set _error to.
     */
    constexpr next_char32_error(
        const next_char32_error_code      code_arg,
        const forward_scan_unicode_error& error_arg
    ) noexcept;
    /*!
     * @brief Given a Unicode string object, creates the error
     * message for the object.
     *
     * The string object can either be a std::monstate (no string), a pair of
     * iterators, or a single string_view.
     *
     * @tparam String_Type The string type. Must adhere to
     * is_complete_unicode_string_arg_type_c.
     * @tparam Arg_Type The argument type. Tells the function what order the
     * iterator is in (if the string argument is an iterator).
     * @param str_arg The string of interest.
     * @return The error message representing the object.
     */
    template <typename Arg_Type, typename String_Type>
    requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        is_complete_unicode_string_arg_type_c<String_Type>
        constexpr std::u8string
        message(const String_Type& str_arg) const;
public:
    /*!
     * @brief Gets the local variable _code.
     * @return The local next_char32_error_code variable _code.
     */
    constexpr const next_char32_error_code
        get_enum() const noexcept;
    /*!
     * @brief Gets a const reference to the local variable _error.
     * @return The local forward_scan_unicode_error variable _error.
     */
    constexpr const forward_scan_unicode_error&
        error() const noexcept;
    /*!
     * @brief Creates error message for the object.
     *
     * This variant is used when the original input string is known.
     *
     * @tparam ItteratorType The type of the string iterator passed to the
     * function.
     * @param itt_begin_arg The beginning iterator.
     * @param itt_end_arg The end iterator.
     * @return A string representative of the error.
     */
    template <typename ItteratorType>
    requires char_type_is_unicode_c<
        typename std::iterator_traits<ItteratorType>::value_type>
    constexpr std::u8string
        message(ItteratorType itt_begin_arg, ItteratorType itt_end_arg) const;
    /*!
     * @brief Creates error message for the object.
     *
     * This variant is used when the original input string is not known.
     *
     * @return A string representative of the error.
     */
    constexpr std::u8string
        message() const;
};

/*!
 * @brief Error type representing errors found when scanning an iterator
 * containing characters backwards.
 *
 * It contains an instance of basic_unicode_error, which contains all the actual
 * data regarding the error.
 */
struct prev_char32_error
{
public:
    // ---- Friend types ----
    friend struct UNICODE_BRIDGE_NAMESPACE_INTERNAL::prev_char32_error_factory;
    /*!
     * @brief Enum representing the different types of error that can be
     * encountered when when reading a Unicode iterator pair backwards.
     */
    enum class prev_char32_error_code
    {
        /*!
         * @brief Some errors can be encoded by a basic_unicode_error. This enum
         * signifies the use of one of basic_unicode_error's own error codes.
         */
        base_unicode_error,
        /*!
         * @brief Error indicates that the current iterator is at the beginning
         * of the current iterator, and there are no more characters to extract.
         */
        iterator_end,
        // ---- UTF-8 Errors ----
        /*!
         * @brief Indicates that a set of continuation bytes have been
         * encountered, but no leading byte found associted with them. This
         * error is triggered after the fourth continuation byte is found.
         */
        no_valid_leading_byte_found,
        /*!
         * @brief Indicates that the number of continuation bytes found does not
         * match the number the leading byte found indicated.
         */
        leading_byte_sequence_length_mismatch,
        /*!
         * @brief Indicates an invalid UTF-8 character has been encountered. In
         * next_char32_error, this is either invalid_leading_byte or
         * invalid_continuation_byte. As reading backwards ensures that we are
         * looking for either of these, there is a single error code for this
         * error.
         */
        invalid_utf8_byte,
        /*!
         * @brief Indicates a set of continuation bytes have been found, but the
         * iterator reaches the beginning before the leading byte can be found.
         */
        iterator_begin_reached_before_leading_byte,
        // ---- UTF-16 Errors ----
        /*!
         * @brief In UTF-16, indicates a low surrogate is encoutnered, then the
         * beginning of the stream.
         */
        low_surrogate_then_start_of_stream,
        /*!
         * @brief Indicates a low surrogate is encountered, then a UTF-16
         * character that is not a high-surrogate.
         */
        low_surrogate_not_preceded_by_high_surrogate,
        /*!
         * @brief Indicates a high surrogate is encountered, without some
         * preceeding low surrogate.
         */
        unexpected_high_surrogate
    };
private:
    //! Encodes all the error data.
    basic_unicode_error _basic_unicode_error;
    //! Encodes the error itself.
    prev_char32_error_code _code;
    /*!
     * @brief Constructor
     * @param code_arg Value to set _code to
     * @param basic_unicode_error_arg Value to set _basic_unicode_error to.
     */
    constexpr prev_char32_error(
        const prev_char32_error_code code_arg,
        const basic_unicode_error&   basic_unicode_error_arg
    ) noexcept;
    /*!
     * @brief Given a Unicode string object, creates the error
     * message for the object.
     *
     * The string object can either be a std::monstate (no string), a pair of
     * iterators, or a single string_view.
     *
     * @tparam String_Type The string type. Must adhere to
     * is_complete_unicode_string_arg_type_c.
     * @tparam Arg_Type The argument type. Tells the function what order the
     * iterator is in (if the string argument is an iterator).
     * @param str_arg The string of interest.
     * @return The error message representing the object.
     */
    template <typename String_Type>
    requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
        is_complete_unicode_string_arg_type_c<String_Type>
        constexpr std::u8string
        message(const String_Type& iterator_args) const;
public:
    /*!
     * @brief Gets the _code local variable.
     * @return The _code local variable.
     */
    constexpr const prev_char32_error_code
        get_enum() const noexcept;
    /*!
     * @brief Gets a cref to the internal _basic_unicode_error variable.
     * @return A cref to the local _basic_unicode_error variable.
     */
    constexpr const basic_unicode_error&
        error() const noexcept;
    /*!
     * @brief Given two iterators representing a position and begining iterators
     * of a string, creates the error message representative of the object.
     * @tparam ItteratorType The iterator type. Must contain a Unicode
     * character.
     * @param itt_end_arg The end iterator. This is the current position of the
     * iterator.
     * @param itt_begin_arg The beginning of the iterator. Moving backwards,
     * itt_end_arg should be moving towards this.
     * @return The error message representative of this object.
     */
    template <typename ItteratorType>
    requires char_type_is_unicode_c<
        typename std::iterator_traits<ItteratorType>::value_type>
    constexpr std::u8string
        message(ItteratorType itt_end_arg, ItteratorType itt_begin_arg) const;
    /*!
     * @brief Creates the error message representative of the object.
     * @return The error message representative of this object.
     */
    constexpr std::u8string
        message() const;
};

/*!
 * @brief Exception type for the errors expoed by unicode_bridge. The concept
 * ensures that this object can only hold the error types exposed by the
 * library.
 * @tparam Error_Type The internal type of the exception.
 */
template <typename Error_Type>
requires is_error_type_c<Error_Type>
class unicode_bridge_exception : public std::exception
{
private:
    //! The internal error.
    Error_Type _error;
    //! A mutable string representing the error message. Mutablee to ensure
    //! that, after it has been constructed, it is not re-constructed.
    mutable std::string _what_cache;
public:
    /*!
     * @brief Constructor.
     * @param error_arg The value to set _error to.
     */
    constexpr explicit unicode_bridge_exception(const Error_Type& error_arg
    ) noexcept;
    /*!
     * @brief Returns a char* containing the error message.
     *
     * Note the error message will be encoded as a char*, but is internally a
     * char8_t*, and should be processed as such a value.
     * @return const char* string representing an explanation of the error. Note
     * it is a char8_t* internally, and should be processed as such.
     */
    const char*
        what() const noexcept override;
    /*!
     * @brief Gets the internal _error local variable.
     * @return The _error local variable.
     */
    constexpr const Error_Type&
        error() const noexcept;
};

struct string_sink
{
    std::string& _str;

    void
        put(
            char char_arg
        ) noexcept
    {
        _str.push_back(char_arg);
    }

    void
        write(
            const char* char_star_arg,
            std::size_t n_chars_to_append_arg
        ) noexcept
    {
        _str.append(char_star_arg, n_chars_to_append_arg);
    }
};

struct ostream_sink
{
    std::ostream& _stream;

    void
        put(
            char char_arg
        ) noexcept
    {
        _stream.put(char_arg);
    }

    void
        write(
            const char* char_star_arg,
            std::size_t n_chars_to_append_arg
        ) noexcept
    {
        _stream.write(char_star_arg, n_chars_to_append_arg);
    }
};
template <typename CharT>
requires char_type_is_unicode_c<CharT>
struct unicode_print;

template <typename CharT>
requires char_type_is_unicode_c<CharT>
std::ostream&
    operator<<(std::ostream& os, const unicode_print<CharT>& w);

// ---- unicode_print object
/*!
 * @brief Object used to make printing Unicode characters using std::cout
 * easier.
 *
 * std::cout only works with std::strings. By casting a std::u8string to a
 * std::string, a std::u8string's bytes can be sent to the console. Assuming the
 * console understands Unicode, the Unicode strings can then be printed.
 *
 * Typically this code looks something like this
 *
 * std::cout << reinterpret_cast<const char*>(str.data()) << std::endl;
 *
 * This object streamlines this, while also providing internal lossless
 * conversions for std::u16strings, std::u32strings and std::wstrings.
 *
 * To be clear about "lossess conversions", if a UTF-16 code point is
 * invalid (e.g. single surrogate value on its own), it is encoded as a UTF-8
 * byte sequence and just printed - it will not be rejected. This is the same
 * for UTF-32 strings.
 *
 * The only case when this isn't true is when a Unicode scalar value is outside
 * the Unicode range - so it cannot be encoded as a UTF-8 string. In those
 * instances, it is replaced with the Unicode scalar value U+FFFD.
 *
 * str() will allow you to use the actual std::string created as a result.
 *
 * Be careful with unicode_print's use, as it assumes it does not own the
 * argument given. If the original std::basic_string used to initialise it goes
 * out of scope, it then has undefined behaviour.
 *
 * @tparam CharT Internal Unicode character type.
 */
template <typename CharT>
requires char_type_is_unicode_c<CharT>
struct unicode_print
{
private:
    std::basic_string_view<CharT> _str;
    template <typename Sink>
    requires (std::same_as<Sink, ostream_sink> || std::same_as<Sink, string_sink>)
    constexpr void
        stream_impl(Sink& sink_arg) const;
public:
    constexpr explicit unicode_print(const std::basic_string_view<CharT> str_arg
    ) noexcept;
    constexpr std::string
        str() const;
    friend std::ostream&
        operator<< <CharT>(std::ostream& os, const unicode_print& w);
};

unicode_print(std::u8string_view) -> unicode_print<char8_t>;
unicode_print(std::u16string_view) -> unicode_print<char16_t>;
unicode_print(std::u32string_view) -> unicode_print<char32_t>;
unicode_print(std::wstring_view) -> unicode_print<wchar_t>;

// ---- Exported functions ----
// ---- convert_unicode_to_ascii and related functions
/*!
 * @brief Converts a Unicode string to an ASCII string.
 *
 * The input parameter must be convertable to a std::basic_string<CharT>, where
 * CharT is some Unicode character type.
 *
 * If any of the Unicode characters are outside the ASCII range, then the
 * function returns an error. Otherwise it returns the created std::string
 * object.
 *
 * @tparam ArgType. The parameter of the argument type. It should be convertable
 * to std::basic_string<CharT>, where CharT is some Unicode character type.
 * @param str_arg The input Unicode string object.
 * @return An std::expected object, containing either the created std::string,
 * or an unicode_to_ascii_error object.
 */
template <typename ArgType>
requires char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr unicode_to_ascii_result<std::string>
    convert_unicode_to_ascii(ArgType str_arg) noexcept;
/*!
 * @brief Converts a single Unicode character to an ASCII string.
 *
 * The input parameter must be some Unicode character type.
 *
 * If the Unicode characters is outside the ASCII range, then the
 * function returns an error. Otherwise it returns the created std::string
 * object from that character.
 *
 * The reader may question why the return type is not a single character. It is
 * in this form so as to mirror the other functions in unicode_bridge. However,
 * it is true that if the function returns a std::string, then it contains a
 * single character.
 *
 * @tparam InputChar. The parameter of the argument type. It should be some
 * Unicode character type.
 * @param char_arg The input Unicode char.
 * @return An std::expected object, containing either the created std::string,
 * or an unicode_to_ascii_error object.
 */
template <typename InputChar>
requires char_type_is_unicode_c<InputChar>
constexpr unicode_to_ascii_result<std::string>
    convert_unicode_to_ascii(const InputChar char_arg) noexcept;
/*!
 * @brief Converts a Unicode string to an ASCII string.
 *
 * The input parameter must be convertable to a std::basic_string<CharT>, where
 * CharT is some Unicode character type.
 *
 * If any of the Unicode characters are outside the ASCII range, then the
 * function throws an exception of type
 * unicode_bridge_exception<unicode_to_ascii_error>. Otherwise it returns the
 * created std::string object.
 *
 * @tparam ArgType. The parameter of the argument type. It should be convertable
 * to std::basic_string<CharT>, where CharT is some Unicode character type.
 * @param str_arg The input Unicode string object.
 * @return The converted string.
 * @throws unicode_bridge_exception<unicode_to_ascii_error> if the conversion
 * fails.
 */
template <typename ArgType>
requires char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr std::string
    convert_unicode_to_ascii_with_exception(ArgType str_arg);
/*!
 * @brief Converts a single Unicode character to an ASCII string.
 *
 * The input parameter must be some Unicode character type.
 *
 * If the Unicode characters is outside the ASCII range, then the function
 * throws an exception of type unicode_bridge_exception<unicode_to_ascii_error>.
 * Otherwise it returns the created std::string object.
 *
 * The reader may question why the return type is not a single character. It is
 * in this form so as to mirror the other functions in unicode_bridge. However,
 * it is true that if the function is successful, then the returned string
 * contains a single character.
 *
 * @tparam InputChar. The parameter of the argument type. It should be some
 * Unicode character type.
 * @param char_arg The input Unicode char.
 * @return An std::expected object, containing either the created std::string,
 * or an unicode_to_ascii_error object.
 * @throws unicode_bridge_exception<unicode_to_ascii_error> if the conversion
 * fails.
 */
template <typename InputChar>
requires char_type_is_unicode_c<InputChar>
constexpr std::string
    convert_unicode_to_ascii_with_exception(const InputChar char_arg);
// ---- convert_ascii_to_unicode and related functions
/*!
 * @brief Converts an ASCII string to a Unicode string.
 *
 * The input parameter must be convertable to a std::string_view.
 *
 * If any characters in the input string are outside the ASCII range (that is,
 * non-basic ASCII), then the function returns an error. Otherwise it returns
 * the created std::basic_string object.
 *
 * @tparam OutputChar. The character type of the output string. It must be a
 * Unicode character type.
 * @tparam ArgType. The parameter of the argument type. It should be convertable
 * to std::basic_string_view.
 * @param str_arg The input ASCII string object.
 * @return An std::expected object, containing either the created
 * std::basic_string<OutputChar>, or an ascii_to_unicode_error object.
 */
template <typename OutputChar, typename ArgType>
requires char_type_is_unicode_c<OutputChar>
         && std::convertible_to<ArgType, std::string_view>
constexpr ascii_to_unicode_result<std::basic_string<OutputChar>>
    convert_ascii_to_unicode(ArgType str_arg) noexcept;
/*!
 * @brief Converts an ASCII character to a Unicode string.
 *
 * If the character input is outside the ASCII range (that is,
 * non-basic ASCII), then the function returns an error. Otherwise it returns
 * the created std::basic_string object.
 *
 * @tparam OutputChar. The character type of the output string. It must be a
 * Unicode character type.
 * @param char_arg The input ASCII character.
 * @return An std::expected object, containing either the created
 * std::basic_string<OutputChar>, or an ascii_to_unicode_error object.
 */
template <typename OutputChar>
requires char_type_is_unicode_c<OutputChar>
constexpr ascii_to_unicode_result<std::basic_string<OutputChar>>
    convert_ascii_to_unicode(const char char_arg) noexcept;
/*!
 * @brief Converts an ASCII string to a Unicode string.
 *
 * The input parameter must be convertable to a std::string_view.
 *
 * If any characters in the input string are outside the ASCII range (that is,
 * non-basic ASCII), then the function throws an
 * unicode_bridge_exception<ascii_to_unicode_error> exception. Otherwise it
 * returns the created std::basic_string object.
 *
 * @tparam OutputChar. The character type of the output string. It must be a
 * Unicode character type.
 * @tparam ArgType. The parameter of the argument type. It should be convertable
 * to std::basic_string_view.
 * @param str_arg The input ASCII string object.
 * @return An std::basic_string<OutputChar> object, containing the converted
 * string.
 * @throws unicode_bridge_exception<ascii_to_unicode_error> if the conversion
 * fails.
 */
template <typename OutputChar, typename ArgType>
requires char_type_is_unicode_c<OutputChar>
         && std::convertible_to<ArgType, std::string_view>
constexpr std::basic_string<OutputChar>
    convert_ascii_to_unicode_with_exception(ArgType str_arg);
/*!
 * @brief Converts an ASCII character to a Unicode string.
 *
 * If the character input is outside the ASCII range (that is,
 * non-basic ASCII), then the function throws an
 * unicode_bridge_exception<ascii_to_unicode_error> exception. Otherwise it
 * returns the created std::basic_string object.
 *
 * @tparam OutputChar. The character type of the output string. It must be a
 * Unicode character type.
 * @param char_arg The input ASCII character.
 * @return An std::basic_string<OutputChar> object, containing the converted
 * string.
 * @throws unicode_bridge_exception<ascii_to_unicode_error> if the conversion
 * fails.
 */
template <typename OutputChar>
requires char_type_is_unicode_c<OutputChar>
constexpr std::basic_string<OutputChar>
    convert_ascii_to_unicode_with_exception(const char char_arg);
// ---- unicode_conversion and related functions
/*!
 * @brief Converts a Unicode string from one Unicode format to another.
 *
 * The input parameter must be convertable to a std::basic_string_view, where
 * the string's internal type is some Unicode character type.
 *
 * If the input string is invalid Unicode, then the function returns an error.
 * Otherwise it returns the created std::basic_string object.
 *
 * @tparam OutputChar. The character type of the output string. It must be a
 * Unicode character type.
 * @tparam ArgType. The parameter of the argument type. It should be convertable
 * to std::string_view, whose internal character type is a Unicode character
 * type.
 * @param str_arg The input Unicode string object.
 * @return An std::expected object, containing either the created
 * std::basic_string<OutputChar>, or an unicode_conversion_error object.
 */
template <typename OutputChar, typename ArgType>
requires char_type_is_unicode_c<OutputChar>
         && char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr unicode_conversion_result<std::basic_string<OutputChar>>
    unicode_conversion(ArgType str_arg) noexcept;
/*!
 * @brief Converts a Unicode character into a Unicode string.
 *
 * The input character must be a Unicode character type.
 *
 * If the input string is an invalid Unicode character on its own, then the
 * function returns an error. Otherwise it returns the created std::basic_string
 * object.
 *
 * It is particularly common for UTF-8 and UTF-16 characters on their own to be
 * invalid Unicode.
 *
 * @tparam OutputChar. The character type of the output string. It must be a
 * Unicode character type.
 * @tparam InputChar. The Unicode character type of the input parameter.
 * @param str_arg The input Unicode character.
 * @return An std::expected object, containing either the created
 * std::basic_string<OutputChar>, or an unicode_conversion_error object.
 */
template <typename OutputChar, typename InputChar>
requires char_type_is_unicode_c<OutputChar> && char_type_is_unicode_c<InputChar>
constexpr unicode_conversion_result<std::basic_string<OutputChar>>
    unicode_conversion(const InputChar char_arg) noexcept;
/*!
 * @brief Converts a Unicode string from one Unicode format to another.
 *
 * The input parameter must be convertable to a std::basic_string_view, where
 * the string's internal type is some Unicode character type.
 *
 * If the input string is invalid Unicode, then the function throws an
 * unicode_bridge_exception<unicode_conversion_error> exception. Otherwise it
 * returns the created std::basic_string object.
 *
 * @tparam OutputChar. The character type of the output string. It must be a
 * Unicode character type.
 * @tparam ArgType. The parameter of the argument type. It should be convertable
 * to std::string_view, whose internal character type is a Unicode character
 * type.
 * @param str_arg The input Unicode string object.
 * @return The created std::basic_string<OutputChar> object.
 * @throws unicode_bridge_exception<unicode_conversion_error> if the conversion
 * fails.
 */
template <typename OutputChar, typename ArgType>
requires char_type_is_unicode_c<OutputChar>
         && char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr std::basic_string<OutputChar>
    unicode_conversion_with_exception(ArgType str_arg);
/*!
 * @brief Converts a Unicode character into a Unicode string.
 *
 * The input character must be a Unicode character type.
 *
 * If the input string is an invalid Unicode character on its own, then the
 * function hrows an
 * unicode_bridge_exception<unicode_conversion_error> exception. Otherwise it
 * returns the created std::basic_string object.
 *
 * It is particularly common for UTF-8 and UTF-16 characters on their own to be
 * invalid Unicode.
 *
 * @tparam OutputChar. The character type of the output string. It must be a
 * Unicode character type.
 * @tparam InputChar. The Unicode character type of the input parameter.
 * @param str_arg The input Unicode character.
 * @return The created std::basic_string<OutputChar> object.
 * @throws unicode_bridge_exception<unicode_conversion_error> if the conversion
 * fails.
 */
template <typename OutputChar, typename InputChar>
requires char_type_is_unicode_c<OutputChar> && char_type_is_unicode_c<InputChar>
constexpr std::basic_string<OutputChar>
    unicode_conversion_with_exception(const InputChar char_arg);
// ---- is_valid_unicode functions ----
/*!
 * Checks if a string is valid Unicode.
 *
 * @tparam ArgType. The parameter of the argument type. It should be convertable
 * to std::string_view, whose internal character type is a Unicode character
 * type.
 * @param str_arg The string to check.
 * @return True if string is valid Unicode, false otherwise.
 */
template <typename ArgType>
requires char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr bool
    is_valid_unicode(ArgType str_arg) noexcept;
/*!
 * Checks if a single character is valid Unicode.
 *
 * @tparam InputChar. The character type of the input, should be a Unicode
 * character type.
 * @param char_arg The character to check.
 * @return True if character is valid Unicode on its own, false otherwise.
 */
template <typename InputChar>
requires char_type_is_unicode_c<InputChar>
constexpr bool
    is_valid_unicode(const InputChar char_arg) noexcept;
// ---- is_valid_ascii functions ----
/*!
 * Checks if a string is valid ASCII.
 *
 * @tparam ArgType. The parameter of the argument type. It should be convertable
 * to std::string_view, whose internal character type is can be any character
 * type in C++ - Unicode or ASCII.
 * @param str_arg The string to check.
 * @return True if string is valid ASCII, false otherwise.
 */
template <typename ArgType>
requires is_char_type_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr bool
    is_valid_ascii(ArgType str_arg) noexcept;
/*!
 * Checks if a single character is valid ASCII.
 *
 * @tparam InputChar. The character type of the input, can be any character type
 * in C++ - Unicode or ASCII.
 * @param char_arg The character to check.
 * @return True if string is valid ASCII, false otherwise.
 */
template <typename InputChar>
requires is_char_type_c<InputChar>
constexpr bool
    is_valid_ascii(const InputChar char_arg) noexcept;
// ---- next_char32 and related functions ----
/*!
 * @brief Given two iterators representing a stream of Unicode characters,
 * extracts the next Unicode character from them, and how many elemnets of the
 * iterator were required to build that Unicode character.
 *
 * It is assumed that the iterators represent some stream of Unicode characters,
 * and that the first argument is the current position in the iterator, and the
 * second represents the end of the iterator.
 *
 * The basic return type of this function is a pair; the first element is the
 * Unicode character (represented as a char32_t type), and the second is a
 * std::size_t, representing the number of elements of the iterator required to
 * create the Unicode character.
 *
 * The template parameter Return_Reason changes the return type of this
 * function.
 *
 * If it is true, then an std::expected is returned, either containing the basic
 * return type, or a next_char32_error.
 *
 * If it is false, then an std::optional is returned, containing the basic
 * return type if the function was successful, or an empty std::optional if no
 * character could be extracted.
 *
 * @tparam Return_Reason The bool template parameter that controls the return
 * type. See above.
 * @ItteratorType The type of the input arguments. The iterator's value type's
 * must be a Unicode character type.
 * @param iterator_arg The current iterator.
 * @param itt_end_arg The end iterator. The function does not go past this
 * value.
 * @return If the function successfully extracts a character, then the character
 * and a size_t representing how many elemnets of the iterator were required to
 * create it. For the return type when the function fails, see above.
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
 * @brief Given two iterators representing a stream of Unicode characters,
 * extracts the next Unicode character from them.
 *
 * It is assumed that the iterators represent some stream of Unicode characters,
 * and that the first argument is the current position in the iterator, and the
 * second represents the end of the iterator.
 *
 * This function will update the first iterator to a new position, moving it to
 * the next boundary for extracting the next Unicode character.
 *
 * The basic return type of this function is a single
 * Unicode character (represented as a char32_t type).
 *
 * The template parameter Return_Reason changes the return type of this
 * function.
 *
 * If it is true, then an std::expected is returned, either containing the basic
 * return type, or a next_char32_error.
 *
 * If it is false, then an std::optional is returned, containing the basic
 * return type if the function was successful, or an empty std::optional if no
 * character could be extracted.
 *
 * @tparam Return_Reason The bool template parameter that controls the return
 * type. See above.
 * @ItteratorType The type of the input arguments. The iterator's value type's
 * must be a Unicode character type.
 * @param iterator_arg The current iterator. If the function is successful, then
 * this value will be updated.
 * @param itt_end_arg The end iterator. The function does not go past this
 * value.
 * @return If the function successfully extracts a character, then that
 * character. For the return type when the function fails, see above.
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
/*!
 * @brief Given two iterators representing a stream of Unicode characters,
 * extracts the next Unicode character from them, and how many elemnets of the
 * iterator were required to build that Unicode character.
 *
 * It is assumed that the iterators represent some stream of Unicode characters,
 * and that the first argument is the current position in the iterator, and the
 * second represents the end of the iterator.
 *
 * The basic return type of this function is a pair; the first element is the
 * Unicode character (represented as a char32_t type), and the second is a
 * std::size_t, representing the number of elements of the iterator required to
 * create the Unicode character.
 *
 * If the function is unable to extract a next character, then a
 * unicode_bridge_exception<next_char32_error> exception is thrown.
 *
 * @ItteratorType The type of the input arguments. The iterator's value type's
 * must be a Unicode character type.
 * @param iterator_arg The current iterator.
 * @param itt_end_arg The end iterator. The function does not go past this
 * value.
 * @return A character
 * and a size_t representing how many elemnets of the iterator were required to
 * create it.
 * @throws unicode_bridge_exception<next_char32_error> if the function
 * fails.
 */
template <typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::pair<char32_t, std::size_t>
    next_char32_with_exception(
        const ItteratorType iterator_arg,
        const ItteratorType itt_end_arg
    );
/*!
 * @brief Given two iterators representing a stream of Unicode characters,
 * extracts the next Unicode character from them.
 *
 * It is assumed that the iterators represent some stream of Unicode characters,
 * and that the first argument is the current position in the iterator, and the
 * second represents the end of the iterator.
 *
 * This function will update the first iterator to a new position, moving it to
 * the next boundary for extracting the next Unicode character.
 *
 * The basic return type of this function is a single
 * Unicode character (represented as a char32_t type).
 *
 * If the function is unable to extract a next character, then a
 * unicode_bridge_exception<next_char32_error> exception is thrown.
 *
 * @ItteratorType The type of the input arguments. The iterator's value type's
 * must be a Unicode character type.
 * @param iterator_arg The current iterator. If the function is successful, then
 * this value will be updated.
 * @param itt_end_arg The end iterator. The function does not go past this
 * value.
 * @return The next Unicode character in the string represented by the
 * iterators.
 * @throws unicode_bridge_exception<next_char32_error> if the function
 * fails.
 */
template <typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr char32_t
    next_char32_and_increment_iterator_with_exception(
        ItteratorType&      iterator_arg,
        const ItteratorType itt_end_arg
    );
// ---- prev_char32 and related functions ----
/*!
 * @brief Given two iterators representing a stream of Unicode characters,
 * extracts the previous Unicode character from them, and how many elemnets of
 * the iterator were required to build that Unicode character.
 *
 * It is assumed that the iterators represent some stream of Unicode characters,
 * and that the first argument is the current position in the iterator, and the
 * second represents the beginning of the iterator.
 *
 * The current iterator is one past the first element to be looked at. So if we
 * were extracting the last element from a string, we would give std::end and
 * std::begin as the first and second arguments.
 *
 * The basic return type of this function is a pair; the first element is the
 * Unicode character (represented as a char32_t type), and the second is a
 * std::size_t, representing the number of elements of the iterator required to
 * create the Unicode character.
 *
 * The template parameter Return_Reason changes the return type of this
 * function.
 *
 * If it is true, then an std::expected is returned, either containing the basic
 * return type, or a prev_char32_error.
 *
 * If it is false, then an std::optional is returned, containing the basic
 * return type if the function was successful, or an empty std::optional if no
 * character could be extracted.
 *
 * @tparam Return_Reason The bool template parameter that controls the return
 * type. See above.
 * @ItteratorType The type of the input arguments. The iterator's value type's
 * must be a Unicode character type.
 * @param iterator_arg The current iterator.
 * @param itt_start_arg The beginning iterator. The function does not go past
 * this value.
 * @return If the function successfully extracts a character, then the character
 * and a size_t representing how many elemnets of the iterator were required to
 * create it. For the return type when the function fails, see above.
 */
template <bool Return_Reason, typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::conditional_t<
    Return_Reason,
    prev_char32_result<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    prev_char32(
        const ItteratorType iterator_arg,
        const ItteratorType itt_start_arg
    ) noexcept;
/*!
 * @brief Given two iterators representing a stream of Unicode characters,
 * extracts the previous Unicode character from them.
 *
 * It is assumed that the iterators represent some stream of Unicode characters,
 * and that the first argument is the current position in the iterator, and the
 * second represents the beginning of the iterator.
 *
 * The current iterator is one past the first element to be looked at. So if we
 * were extracting the last element from a string, we would give std::end and
 * std::begin as the first and second arguments.
 *
 * This function will update the first iterator to a new position, moving it to
 * the previous boundary for extracting the previous Unicode character.
 *
 * The basic return type of this function is a single
 * Unicode character (represented as a char32_t type).
 *
 * The template parameter Return_Reason changes the return type of this
 * function.
 *
 * If it is true, then an std::expected is returned, either containing the basic
 * return type, or a prev_char32_error.
 *
 * If it is false, then an std::optional is returned, containing the basic
 * return type if the function was successful, or an empty std::optional if no
 * character could be extracted.
 *
 * @tparam Return_Reason The bool template parameter that controls the return
 * type. See above.
 * @ItteratorType The type of the input arguments. The iterator's value type's
 * must be a Unicode character type.
 * @param iterator_arg The current iterator. If the function is successful, then
 * this value will be updated.
 * @param itt_start_arg The beginning iterator. The function does not go past
 * this value.
 * @return If the function successfully extracts a character, then that
 * character. For the return type when the function fails, see above.
 */
template <bool Return_Reason, typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::conditional_t<
    Return_Reason,
    prev_char32_result<char32_t>,
    std::optional<char32_t>>
    prev_char32_and_decrement_iterator(
        ItteratorType&      iterator_arg,
        const ItteratorType itt_start_arg
    ) noexcept;
/*!
 * @brief Given two iterators representing a stream of Unicode characters,
 * extracts the previous Unicode character from them, and how many elemnets of
 * the iterator were required to build that Unicode character.
 *
 * It is assumed that the iterators represent some stream of Unicode characters,
 * and that the first argument is the current position in the iterator, and the
 * second represents the beginning of the iterator.
 *
 * The current iterator is one past the first element to be looked at. So if we
 * were extracting the last element from a string, we would give std::end and
 * std::begin as the first and second arguments.
 *
 *
 * @ItteratorType The type of the input arguments. The iterator's value type's
 * must be a Unicode character type.
 * @param iterator_arg The current iterator.
 * @param itt_start_arg The beginning iterator. The function does not go past
 * this value.
 * @return If the function successfully extracts a character, then the character
 * and a size_t representing how many elemnets of the iterator were required to
 * create it.
 * @throws unicode_bridge_exception<prev_char32_error> if the function
 * fails.
 */
template <typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::pair<char32_t, std::size_t>
    prev_char32_with_exception(
        const ItteratorType iterator_arg,
        const ItteratorType itt_start_arg
    );
/*!
 * @brief Given two iterators representing a stream of Unicode characters,
 * extracts the previous Unicode character from them.
 *
 * It is assumed that the iterators represent some stream of Unicode characters,
 * and that the first argument is the current position in the iterator, and the
 * second represents the beginning of the iterator.
 *
 * The current iterator is one past the first element to be looked at. So if we
 * were extracting the last element from a string, we would give std::end and
 * std::begin as the first and second arguments.
 *
 * This function will update the first iterator to a new position, moving it to
 * the previous boundary for extracting the previous Unicode character.
 *
 * @ItteratorType The type of the input arguments. The iterator's value type's
 * must be a Unicode character type.
 * @param iterator_arg The current iterator. If the function is successful, then
 * this value will be updated.
 * @param itt_start_arg The beginning iterator. The function does not go past
 * this value.
 * @return If the function successfully extracts a character, then that
 * character.
 * @throws unicode_bridge_exception<prev_char32_error> if the function
 * fails.
 */
template <typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr char32_t
    prev_char32_and_decrement_iterator_with_exception(
        ItteratorType&      iterator_arg,
        const ItteratorType itt_start_arg
    );
// ---- to_formatted_unicode_string and related functions ----
/*!
 * @brief Creates a formatted Unicode string from some arbitrary string.
 *
 * It specifically has the following features:
 * - The following ASCII characters are printed as follows:
 * -- 0x00 -> \0
 * -- 0x07 -> \a
 * -- 0x0A -> \n
 * -- 0x0B -> \v
 * -- 0x0C -> \f
 * -- 0x0D -> \r
 * -- 0x22 -> \"
 * -- 0x27 -> \'
 * -- 0x5C -> \\
 *
 * Any character below 0x20, or above 0x7F and below 0x9F is printed as straight
 hex.
 *
 * If the input characters are ASCII, then anything abouve 7F is printed as hex.
 *
 * If the input is Unicode, the following hex values are printed as hex. This is
 * because in general output, it may be hard to discern the Unicode from other
 * types of character.
 * 0x00A0, 0x1680, 0x180E, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005,
 0x2006
 * 0x2007, 0x2008, 0x2009, 0x200A, 0x200B, 0x200C, 0x200D, 0x2028, 0x2029,
 0x202F,
 * 0x205F, 0x2060, 0x2061, 0x2062, 0x2063, 0x2064, 0x2066, 0x2067, 0x2068,
 0x2069,
 * 0x3000, 0xFEFF, 0xFFF9, 0xFFFA, 0xFFFB, 0xFFFD.
 *
 * All other characters are printed as-is.
 *
 * @tparam OutputChar The character type of the output character. It must be a
 Unicode character type.
 * @tparam ArgType. The parameter of the argument type. It should be convertable
 * to std::string_view, whose internal character type is a Unicode character
 * type.
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
 * @brief Creates a formatted Unicode string from a single arbitrary character.
 *
 * It specifically has the following features:
 * - The following ASCII characters are printed as follows:
 * -- 0x00 -> \0
 * -- 0x07 -> \a
 * -- 0x0A -> \n
 * -- 0x0B -> \v
 * -- 0x0C -> \f
 * -- 0x0D -> \r
 * -- 0x22 -> \"
 * -- 0x27 -> \'
 * -- 0x5C -> \\
 *
 * Any character below 0x20, or above 0x7F and below 0x9F is printed as straight
 hex.
 *
 * If the input characters are ASCII, then anything abouve 7F is printed as hex.
 *
 * If the input is Unicode, the following hex values are printed as hex. This is
 * because in general output, it may be hard to discern the Unicode from other
 * types of character.
 * 0x00A0, 0x1680, 0x180E, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004, 0x2005,
 0x2006
 * 0x2007, 0x2008, 0x2009, 0x200A, 0x200B, 0x200C, 0x200D, 0x2028, 0x2029,
 0x202F,
 * 0x205F, 0x2060, 0x2061, 0x2062, 0x2063, 0x2064, 0x2066, 0x2067, 0x2068,
 0x2069,
 * 0x3000, 0xFEFF, 0xFFF9, 0xFFFA, 0xFFFB, 0xFFFD.
 *
 * All other characters are printed as-is.
 *
 * @tparam
 * @tparam InputChar The character type of the input character. It can be any
 character type.
 * @param char_arg The character to convert.
 * @return The character converted to Unicode.
 */
template <typename OutputChar, typename InputChar>
requires is_char_type_c<OutputChar> && is_char_type_c<InputChar>
constexpr std::basic_string<OutputChar>
    to_formatted_unicode_string(const InputChar char_arg) noexcept;
UNICODE_BRIDGE_INTERNAL_NS_BEGIN
// ---- Internal namespace definitions
// ---- Concepts and related definitions
/*!
 * constexpr expression used for a false parameter; used for static asserts.
 */
template <typename>
constexpr bool dependent_false = false; // workaround before CWG2518/P2593R1
/*!
 * @brief Used to denote when a wchar_t is 16 bits.
 */
constexpr bool wchar_is_16_bit = (sizeof(wchar_t) == 2);
/*!
 * @brief Used to denote when a wchar_t is 32 bits.
 */
constexpr bool wchar_is_32_bit = (sizeof(wchar_t) == 4);
/*!
 * @brief This concept identifies a wchar_t type which is 2 bytes long - 16
 * bits.
 */
template <typename CharT>
concept is_wchar_and_16_bit_c = std::same_as<CharT, wchar_t> && wchar_is_16_bit;
/*!
 * @brief This concept identifies a wchar_t type which is 4 bytes long - 32
 * bits.
 */
template <typename CharT>
concept is_wchar_and_32_bit_c = std::same_as<CharT, wchar_t> && wchar_is_32_bit;

/*!
 * @brief Templated arbitrary type for identifying some ineteger type that a
 * character type can be converted to. Relies on specialisations.
 * @tparam T The templated type.
 */
template <typename T>
struct char_underlying_type_object_t
{
    using type = void;
};

/*!
 * @brief Specalisation of type for identifying some ineteger type that a
 * character type can be converted to.
 *
 * This is a specialisation for char.
 *
 */
template <>
struct char_underlying_type_object_t<char>
{
    using type = char8_t;
};

/*!
 * @brief Specalisation of type for identifying some ineteger type that a
 * character type can be converted to.
 *
 * This is a specialisation for char8_t.
 *
 */
template <>
struct char_underlying_type_object_t<char8_t>
{
    using type = uint8_t;
};

/*!
 * @brief Specalisation of type for identifying some ineteger type that a
 * character type can be converted to.
 *
 * This is a specialisation for char16_t.
 *
 */
template <>
struct char_underlying_type_object_t<char16_t>
{
    using type = uint16_t;
};

/*!
 * @brief Specalisation of type for identifying some ineteger type that a
 * character type can be converted to.
 *
 * This is a specialisation for char32_t.
 *
 */
template <>
struct char_underlying_type_object_t<char32_t>
{
    using type = uint32_t;
};

/*!
 * @brief Specalisation of type for identifying some ineteger type that a
 * character type can be converted to.
 *
 * This is a specialisation for wchar_t on systems where wchar_t is 16 bits..
 *
 * @tparam CharT The input parameter type.
 *
 */
template <typename CharT>
requires is_wchar_and_16_bit_c<CharT>
struct char_underlying_type_object_t<CharT>
{
    using type = uint16_t;
};

/*!
 * @brief Specalisation of type for identifying some ineteger type that a
 * character type can be converted to.
 *
 * This is a specialisation for wchar_t on systems where wchar_t is 32 bits..
 *
 * @tparam CharT The input parameter type.
 *
 */
template <typename T>
requires is_wchar_and_32_bit_c<T>
struct char_underlying_type_object_t<T>
{
    using type = uint32_t;
};

/*!
 * @brief Type for referencing the underlying integer type that can represent a
 * character.
 * @tparam CharT The character type.
 */
template <typename CharT>
using char_underlying_type_t =
    typename char_underlying_type_object_t<std::remove_cvref_t<CharT>>::type;

// ---- Internal error factory definitions
/*!
 * @brief Type used to create basic_unicode_error errors. Used instead of having
 * basic_unicode_error have a public constructor, as it is easy to make mistakes
 * initailising variables.
 */
struct basic_unicode_error_factory
{
public:
    /*!
     * @brief Constructor representing a UTF-8 overlong encoding.
     * @param u8_code_points_arg The UTF-8 characters in question.
     * @param code_points_encountered_arg The number of pertinant UTF-8
     * characters in u8_code_points_arg.
     * @param char32_character_arg The Unicode character the characters in
     * u8_code_points_arg represents.
     * @return basic_unicode_error representation of the error.
     */
    static constexpr basic_unicode_error
        overlong_encoding(
            const std::array<char8_t, 4>& u8_code_points_arg,
            const std::uint8_t            code_points_encountered_arg,
            const char32_t                char32_character_arg
        ) noexcept;
    /*!
     * @brief Constructor representing a set of UTF-8 bytes converted to a
     * Unicode scalar value that is invalid..
     * @param u8_code_points_arg The UTF-8 characters in question.
     * @param code_points_encountered_arg The number of pertinant UTF-8
     * characters in u8_code_points_arg.
     * @param char32_character_arg The Unicode character the characters in
     * u8_code_points_arg represents.
     * @return basic_unicode_error representation of the error.
     */
    static constexpr basic_unicode_error
        invalid_utf32_code_point_after_utf8_conversion(
            const std::array<char8_t, 4>& u8_code_points_arg,
            const std::uint8_t            code_points_encountered_arg,
            const char32_t                char32_character_arg
        ) noexcept;
    /*!
     * @brief Constructor representing a UTF-32 code point that is invalid.
     * @param char32_character_arg The Unicode character in question.
     * @param is_wchar_arg Whether the original encoding was a wchar_t or not.
     * @return basic_unicode_error representation of the error.
     */
    static constexpr basic_unicode_error
        invalid_utf32_code_point(
            const char32_t char32_character_arg,
            const bool     is_wchar_arg
        ) noexcept;
};

struct forward_scan_unicode_error_factory
{
    static constexpr forward_scan_unicode_error
        invalid_leading_byte(const char8_t code_points_arg) noexcept;

    static constexpr forward_scan_unicode_error
        truncated_sequence(
            const std::array<char8_t, 4>& code_points_arg,
            const forward_scan_unicode_error::truncated_sequence_sub_error
                truncated_sequence_error_enum_arg
        ) noexcept;

    static constexpr forward_scan_unicode_error
        invalid_continuation_byte(
            const std::array<char8_t, 4>& u8_code_points_arg,
            const forward_scan_unicode_error::
                invalid_continuation_byte_sub_error sub_error_enum_arg
        ) noexcept;

    static constexpr forward_scan_unicode_error
        overlong_encoding(
            const std::array<char8_t, 4>& u8_code_points_arg,
            const std::uint8_t            code_points_encountered_arg,
            const char32_t                char32_character_arg
        ) noexcept;

    static constexpr forward_scan_unicode_error
        invalid_utf32_code_point_after_utf8_conversion(
            const std::array<char8_t, 4>& u8_code_points_arg,
            const std::size_t             code_points_encountered_arg,
            const char32_t                char32_character_arg
        ) noexcept;

    static constexpr forward_scan_unicode_error
        high_surrogate_then_end_of_stream(
            const char16_t char16_character_arg,
            const bool     is_wchar_arg
        ) noexcept;

    static constexpr forward_scan_unicode_error
        high_surrogate_not_followed_by_low_surrogate(
            const char16_t char16_first_char_arg,
            const char16_t char16_second_char_arg,
            const bool     is_wchar_arg
        ) noexcept;

    static constexpr forward_scan_unicode_error
        unexpected_low_surrogate(
            const char16_t char16_character_arg,
            const bool     is_wchar_arg
        ) noexcept;
    static constexpr forward_scan_unicode_error
        unexpected_high_surrogate(
            const char16_t char16_character_arg,
            const bool     is_wchar_arg
        ) noexcept;
    static constexpr forward_scan_unicode_error
        invalid_utf32_code_point(
            const char32_t char32_character_arg,
            const bool     is_wchar_arg
        ) noexcept;
};

struct unicode_to_ascii_error_factory
{
    static constexpr unicode_to_ascii_error
        non_ascii_character_found_from_utf8(
            const char32_t                character_arg,
            const std::array<char8_t, 4>& utf8_chars_arg,
            const std::size_t             index_arg,
            const std::uint8_t            length_of_chars_arg
        ) noexcept;
    static constexpr unicode_to_ascii_error
        non_ascii_character_found_from_utf16(
            const char32_t                 character_arg,
            const std::array<char16_t, 2>& utf16_chars_arg,
            const std::size_t              index_arg,
            const std::uint8_t             length_of_chars_arg,
            const bool                     is_wchar_arg
        ) noexcept;
    static constexpr unicode_to_ascii_error
        non_ascii_character_found_from_utf32(
            const char32_t    character_arg,
            const std::size_t index_arg,
            const bool        is_wchar_arg
        ) noexcept;
    static constexpr unicode_to_ascii_error
        invalid_unicode_character(
            const std::size_t                 index_arg,
            const forward_scan_unicode_error& unicode_error_arg
        ) noexcept;
};

struct next_char32_error_factory
{
    static constexpr next_char32_error
        mk_forward_error(const forward_scan_unicode_error& error_arg) noexcept;

    static constexpr next_char32_error
        iterator_exhausted() noexcept;
};

struct prev_char32_error_factory
{
    static constexpr prev_char32_error
        iterator_exhausted() noexcept
    {
        return prev_char32_error(
            prev_char32_error::prev_char32_error_code::iterator_end,
            basic_unicode_error(
                basic_unicode_error::basic_unicode_error_code::generic_error,
                {u8'\0', u8'\0', u8'\0', u8'\0'},
                U'\0',
                0,
                {u'\0', u'\0'},
                false
            )
        );
    }

    static constexpr prev_char32_error
        invalid_utf32_code_point_after_utf8_conversion(
            const std::array<char8_t, 4>& u8_code_points_arg,
            const std::size_t             code_points_encountered_arg,
            const char32_t                char32_character_arg
        ) noexcept
    {
        return prev_char32_error(
            prev_char32_error::prev_char32_error_code::base_unicode_error,
            basic_unicode_error(
                basic_unicode_error::basic_unicode_error_code::
                    invalid_utf32_code_point_after_utf8_conversion,
                u8_code_points_arg,
                char32_character_arg,
                code_points_encountered_arg,
                {u'\0', u'\0'},
                false
            )
        );
    }

    static constexpr prev_char32_error
        overlong_encoding(
            const std::array<char8_t, 4>& u8_code_points_arg,
            const std::uint8_t            code_points_encountered_arg,
            const char32_t                char32_character_arg
        ) noexcept
    {
        return prev_char32_error(
            prev_char32_error::prev_char32_error_code::base_unicode_error,
            basic_unicode_error(
                basic_unicode_error::basic_unicode_error_code::
                    overlong_encoding,
                u8_code_points_arg,
                char32_character_arg,
                code_points_encountered_arg,
                {u'\0', u'\0'},
                false
            )
        );
    }

    static constexpr prev_char32_error
        low_surrogate_then_start_of_stream(
            char16_t   last_element_arg,
            const bool is_whcar_arg
        ) noexcept
    {
        return prev_char32_error(
            prev_char32_error::prev_char32_error_code::
                low_surrogate_then_start_of_stream,
            basic_unicode_error(
                basic_unicode_error::basic_unicode_error_code::generic_error,
                {u8'\0', u8'\0', u8'\0', u8'\0'},
                U'\0',
                0,
                {last_element_arg, u'\0'},
                is_whcar_arg
            )
        );
    }

    static constexpr prev_char32_error
        low_surrogate_not_preceded_by_high_surrogate(
            char16_t   first_element_arg,
            char16_t   last_element_arg,
            const bool is_whcar_arg
        ) noexcept
    {
        return prev_char32_error(
            prev_char32_error::prev_char32_error_code::
                low_surrogate_not_preceded_by_high_surrogate,
            basic_unicode_error(
                basic_unicode_error::basic_unicode_error_code::generic_error,
                {u8'\0', u8'\0', u8'\0', u8'\0'},
                U'\0',
                0,
                {first_element_arg, last_element_arg},
                is_whcar_arg
            )
        );
    }

    static constexpr prev_char32_error
        unexpected_high_surrogate(
            char16_t   last_element_arg,
            const bool is_whcar_arg
        ) noexcept
    {
        return prev_char32_error(
            prev_char32_error::prev_char32_error_code::
                unexpected_high_surrogate,
            basic_unicode_error(
                basic_unicode_error::basic_unicode_error_code::generic_error,
                {u8'\0', u8'\0', u8'\0', u8'\0'},
                U'\0',
                0,
                {last_element_arg, u'\0'},
                is_whcar_arg
            )
        );
    }

    static constexpr prev_char32_error
        no_valid_leading_byte_found(
            const std::array<char8_t, 4>& array_arg
        ) noexcept
    {
        return prev_char32_error(
            prev_char32_error::prev_char32_error_code::
                no_valid_leading_byte_found,
            basic_unicode_error(
                basic_unicode_error::basic_unicode_error_code::generic_error,
                array_arg,
                U'\0',
                0,
                {u'\0', u'\0'},
                false
            )
        );
    }

    static constexpr prev_char32_error
        leading_byte_sequence_length_mismatch(
            const std::array<char8_t, 4>& char8_arr_arg,
            const std::uint8_t            n_bytes_arg
        ) noexcept
    {
        return prev_char32_error(
            prev_char32_error::prev_char32_error_code::
                leading_byte_sequence_length_mismatch,
            basic_unicode_error(
                basic_unicode_error::basic_unicode_error_code::generic_error,
                char8_arr_arg,
                U'\0',
                n_bytes_arg,
                {u'\0', u'\0'},
                false
            )
        );
    }

    static constexpr prev_char32_error
        invalid_utf8_byte(
            const std::array<char8_t, 4>& char8_arr_arg,
            const std::size_t             n_bytes_arg
        ) noexcept
    {
        return prev_char32_error(
            prev_char32_error::prev_char32_error_code::invalid_utf8_byte,
            basic_unicode_error(
                basic_unicode_error::basic_unicode_error_code::generic_error,
                char8_arr_arg,
                U'\0',
                n_bytes_arg,
                {u'\0', u'\0'},
                false
            )
        );
    }

    static constexpr prev_char32_error
        iterator_begin_reached_before_leading_byte(
            const std::array<char8_t, 4>& char8_arr_arg,
            const std::size_t             n_bytes_arg
        ) noexcept
    {
        return prev_char32_error(
            prev_char32_error::prev_char32_error_code::
                iterator_begin_reached_before_leading_byte,
            basic_unicode_error(
                basic_unicode_error::basic_unicode_error_code::generic_error,
                char8_arr_arg,
                U'\0',
                n_bytes_arg,
                {u'\0', u'\0'},
                false
            )
        );
    }

    static constexpr prev_char32_error
        invalid_utf32_code_point(
            const char32_t char32_character_arg,
            const bool     is_wchar_arg
        ) noexcept
    {
        return prev_char32_error(
            prev_char32_error::prev_char32_error_code::base_unicode_error,
            basic_unicode_error(
                basic_unicode_error::basic_unicode_error_code::
                    invalid_utf32_code_point,
                {u8'\0', u8'\0', u8'\0', u8'\0'},
                char32_character_arg,
                std::numeric_limits<uint8_t>::max(),
                {u'\0', u'\0'},
                false
            )
        );
    }
};

template <typename T>
concept unicode_bridge_error_c = requires (const T& error) {
    { error.message() } -> std::same_as<std::u8string>;
};

template <typename ResultT, typename ErrorT>
requires unicode_bridge_error_c<ErrorT>
ResultT
    throw_if_error(std::expected<ResultT, ErrorT> result);
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

constexpr std::u8string
    to_u8string(const std::string_view str_arg);

template <typename T>
requires std::unsigned_integral<T>
constexpr std::u8string
    positive_integer_to_placement(const T number_arg) noexcept;
template <typename WCharT = wchar_t>
constexpr auto
    cast_wstring_to_unicode_string(const std::wstring_view str_arg) noexcept;

template <typename T, bool Use_Capitals, bool Variable_Size_Prefix>
requires is_char_type_c<T>
constexpr std::u8string
    make_hex_from_char_with_prefix_2(
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
template <typename T>
using forward_scan_result_t = std::expected<T, forward_scan_unicode_error>;
constexpr std::optional<std::pair<char32_t, std::size_t>>
    if_invalid_u32string_return_char_and_position(
        const std::u32string_view str_arg
    ) noexcept;

template <bool Return_u32string>
constexpr unicode_conversion_result<
    std::conditional_t<Return_u32string, std::u32string, std::monostate>>
    validate_u8string_and_convert_to_u32string(const std::u8string_view str_arg
    ) noexcept;

template <bool Return_u32string, typename Original_Type>
requires char_type_is_unicode_c<Original_Type>
constexpr forward_scan_result_t<
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

constexpr std::size_t
    utf8_sequence_length(
        const char8_t leading_byte_arg
    ) noexcept
{
    if (is_valid_ascii(leading_byte_arg))
    {
        return 1;
    }
    if ((leading_byte_arg & 0b1110'0000) == 0b1100'0000)
    {
        return 2;
    }
    if ((leading_byte_arg & 0b1111'0000) == 0b1110'0000)
    {
        return 3;
    }
    if ((leading_byte_arg & 0b1111'1000) == 0b1111'0000)
    {
        return 4;
    }
    else
    {
        return 0;
    } // invalid leading byte
}

template <
    bool Forward_Moving,
    bool Return_Reason,
    typename Original_Value_Type,
    typename Error_Factory,
    typename Error_Type,
    typename T>
constexpr std::conditional_t<
    Return_Reason,
    std::expected<std::pair<char32_t, std::size_t>, Error_Type>,
    std::optional<std::pair<char32_t, std::size_t>>>
    process_utf32(
        const T& current_iterator_arg
    )
{
    using namespace std;
    using CharT = iterator_traits<T>::value_type;
    const CharT character{*current_iterator_arg};
    if (is_invalid_char32(character))
    {
        if constexpr (Return_Reason)
        {
            return unexpected(
                Error_Factory::
                    invalid_utf32_code_point(character, same_as<Original_Value_Type, wchar_t>)
            );
        }
        else
        {
            return nullopt;
        }
    }
    return make_pair(static_cast<char32_t>(character), 1);
}

template <bool Return_Reason, typename T, typename Original_Value_Type>
requires char_type_is_unicode_c<typename std::iterator_traits<T>::value_type>
         && char_type_is_unicode_c<Original_Value_Type>
constexpr std::conditional_t<
    Return_Reason,
    prev_char32_result<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    prev_char32_internal_with_iterator_checking(
        const T itt_end_arg,
        const T iterator_arg,
        const T iterator_begin_arg
    ) noexcept;
template <bool Return_Reason, typename T, typename Original_Value_Type>
requires char_type_is_unicode_c<typename std::iterator_traits<T>::value_type>
         && char_type_is_unicode_c<Original_Value_Type>
constexpr std::conditional_t<
    Return_Reason,
    forward_scan_result_t<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    forward_scan_for_next_char32(const T iterator_arg, const T itt_end_arg)
        noexcept;
template <bool Return_Reason, typename T, typename Original_Value_Type>
requires char_type_is_unicode_c<typename std::iterator_traits<T>::value_type>
         && char_type_is_unicode_c<Original_Value_Type>
constexpr std::conditional_t<
    Return_Reason,
    basic_unicode_result_t<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    prev_char32_internal(
        const T itt_end_arg,
        const T iterator_arg,
        const T iterator_begin_arg
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
template <bool Return_Reason, typename T, typename Original_Type>
requires char_type_is_unicode_c<typename std::iterator_traits<T>::value_type>
         && char_type_is_unicode_c<Original_Type>
constexpr std::conditional_t<
    Return_Reason,
    prev_char32_result<char32_t>,
    std::optional<char32_t>>
    prev_char32_and_decrement_iterator(
        const T itt_end_arg,
        T&      iterator_arg,
        const T iterator_begin_arg
    ) noexcept;

constexpr char32_t
    leading_byte_data_bits(
        const char8_t     leading_byte_arg,
        const std::size_t sequence_length_arg
    ) noexcept
{
    constexpr std::array<char32_t, 4> leading_masks{
        static_cast<char32_t>(0b0111'1111),
        static_cast<char32_t>(0b0001'1111),
        static_cast<char32_t>(0b0000'1111),
        static_cast<char32_t>(0b0000'0111),
    };
    return static_cast<char32_t>(leading_byte_arg)
           & leading_masks[sequence_length_arg - 1];
}

constexpr void
    accumulate_continuation_byte(
        char32_t&     code_point_arg,
        const char8_t continuation_byte_arg
    ) noexcept
{
    code_point_arg
        = (code_point_arg << 6) | (continuation_byte_arg & 0b0011'1111);
}

template <
    bool Return_Reason,
    typename Original_Value_Type,
    typename Error_Factory,
    typename Error_Type,
    typename T>
constexpr std::conditional_t<
    Return_Reason,
    std::expected<std::pair<char32_t, std::size_t>, Error_Type>,
    std::optional<std::pair<char32_t, std::size_t>>>
    check_decoded_utf8_codepoint(
        const T           sequence_begin_arg,
        const T           iterator_begin_arg,
        const T           local_iterator_arg,
        const std::size_t sequence_length_arg,
        const char32_t    code_point_arg
    ) noexcept
{
    using namespace std;

    constexpr array<char32_t, 5> code_point_limits{
        0b0, 0b0, 0b1000'0000, 0b1000'0000'0000, 0b0001'0000'0000'0000'0000
    };

    std::array<char8_t, 4> code_units{
        static_cast<char8_t>(*sequence_begin_arg),
        (sequence_length_arg > 1)
            ? static_cast<char8_t>(*(sequence_begin_arg + 1))
            : u8'\0',
        (sequence_length_arg > 2)
            ? static_cast<char8_t>(*(sequence_begin_arg + 2))
            : u8'\0',
        (sequence_length_arg > 3)
            ? static_cast<char8_t>(*(sequence_begin_arg + 3))
            : u8'\0',
    };

    if (code_point_arg < code_point_limits[sequence_length_arg])
    {
        if constexpr (Return_Reason)
        {
            return unexpected(Error_Factory::overlong_encoding(
                code_units, sequence_length_arg, code_point_arg
            ));
        }
        else
        {
            return nullopt;
        }
    }

    if (is_invalid_char32(code_point_arg))
    {
        if constexpr (Return_Reason)
        {
            return unexpected(
                Error_Factory::invalid_utf32_code_point_after_utf8_conversion(
                    code_units, sequence_length_arg, code_point_arg
                )
            );
        }
        else
        {
            return nullopt;
        }
    }

    return make_pair(code_point_arg, sequence_length_arg);
}

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
template <typename InputChar, typename OutputChar>
constexpr std::optional<std::basic_string<OutputChar>>
    special_char_as_string(const char32_t char_arg) noexcept;

template <typename T>
requires is_char_type_c<T>
constexpr std::u8string
    represent_char_as_hex_for_printing(const T char_arg) noexcept;

template <typename T>
constexpr std::wstring
    cast_unicode_string_to_wstring(const T str_arg_view);

template <typename T>
constexpr std::tuple<bool, bool, std::basic_string_view<T>>
    make_focused_string(
        const std::basic_string_view<T> string_view_arg,
        const std::size_t               idx_to_focus_on_arg,
        const std::size_t               size_of_idx_to_focus_on,
        const std::size_t               maximum_offset_from_idx_arg = 8
    ) noexcept
{
    using namespace std;
    if (string_view_arg.empty())
    {
        return {false, false, string_view_arg};
    }
    bool         cutoff_left  = true;
    bool         cutoff_right = true;
    const size_t normalised_focused_idx
        = std::min(idx_to_focus_on_arg, string_view_arg.size() - 1);
    const size_t normalised_focused_idx_end = std::min(
        normalised_focused_idx + size_of_idx_to_focus_on,
        string_view_arg.size() - 1
    );
    size_t start_char    = normalised_focused_idx;
    size_t chars_counted = 0;
    while (chars_counted < maximum_offset_from_idx_arg && start_char > 0)
    {
        if constexpr (char_type_is_unicode_c<T>)
        {
            auto res = prev_char32<false>(
                string_view_arg.begin() + start_char, string_view_arg.begin()
            );
            chars_counted++;
            start_char -= (res.has_value() ? res.value().second : 1);
        }
        else
        {
            optional<char> res;
            res = (start_char == 0)
                      ? std::nullopt
                      : make_optional(
                            *(string_view_arg.begin() + start_char - 1)
                        );
            chars_counted++;
            start_char -= (res.has_value() ? 1 : 0);
        }
    }
    if (start_char == 0)
    {
        cutoff_left = false;
    }
    size_t end_char = normalised_focused_idx_end;
    chars_counted   = 0;
    while (chars_counted < maximum_offset_from_idx_arg
           && end_char < string_view_arg.size())
    {
        if constexpr (char_type_is_unicode_c<T>)
        {
            auto res = next_char32<false>(
                string_view_arg.begin() + end_char, string_view_arg.end()
            );
            chars_counted++;
            end_char += (res.has_value() ? res.value().second : 1);
        }
        else
        {
            optional<char> res;
            res = (end_char >= string_view_arg.size())
                      ? std::nullopt
                      : make_optional(*(string_view_arg.begin() + end_char));
            chars_counted++;
            end_char += (res.has_value() ? 1 : 0);
        }
    }
    if (end_char == string_view_arg.size())
    {
        cutoff_right = false;
    }

    return {
        cutoff_left,
        cutoff_right,
        string_view_arg.substr(start_char, end_char - start_char)
    };
}

template <typename T>
constexpr char32_t
    decode_surrogate_pair(
        const T first_char_arg,
        const T second_char_arg
    ) noexcept
{
    char32_t character = char16_offset_for_char32_conversion<char32_t>();
    character
        += ((static_cast<char32_t>(first_char_arg)
             - high_surrogate_lower_value<char32_t>())
            << 10);
    character
        += (static_cast<char32_t>(second_char_arg)
            - low_surrogate_lower_value<char32_t>());
    return character;
}

constexpr std::u8string
    small_number_as_string(
        std::size_t number_arg
    ) noexcept
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
}

constexpr std::u8string
    integer_list(
        std::size_t begin_int_arg,
        std::size_t n_ints_arg
    ) noexcept
{
    using namespace std;
    u8string str;
    if (n_ints_arg == 1)
    {
        str.append(positive_integer_to_placement(begin_int_arg));
        str.append(u8" code unit ");
    }
    else if (n_ints_arg == 2)
    {
        str.append(positive_integer_to_placement(begin_int_arg));
        str.append(u8" and ");
        str.append(positive_integer_to_placement(begin_int_arg + n_ints_arg - 1)
        );
        str.append(u8" code units ");
    }
    else
    {
        str.append(positive_integer_to_placement(begin_int_arg));
        str.append(u8" to ");
        str.append(positive_integer_to_placement(begin_int_arg + n_ints_arg - 1)
        );
        str.append(u8" code units ");
    }
    return str;
}

template <typename T>
constexpr std::u8string
    chars_to_hex(
        const T&    char_array_arg,
        std::size_t chars_size_arg
    ) noexcept
{
    using namespace std;
    if (chars_size_arg == 1)
    {
        return represent_char_as_hex_for_output(char_array_arg[0]);
    }
    else
    {
        u8string chars_as_hex = u8"[";
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
        return chars_as_hex;
    }
}

UNICODE_BRIDGE_INTERNAL_NS_END
UNICODE_BRIDGE_NS_END
// ---- Implementations ---- //
UNICODE_BRIDGE_NS_BEGIN

constexpr basic_unicode_error::basic_unicode_error(
    const basic_unicode_error_code code_arg,
    const std::array<char8_t, 4>&  u8_code_points_arg,
    const char32_t                 char32_character_arg,
    const std::uint8_t             auxillery_data_arg,
    const std::array<char16_t, 2>& u16_code_points_arg,
    const bool                     is_wchar_arg
) noexcept
    : _code(code_arg)
    , _u8_code_points(u8_code_points_arg)
    , _char32_character(char32_character_arg)
    , _auxillery_data(auxillery_data_arg)
    , _u16_code_points(u16_code_points_arg)
    , _is_wchar(is_wchar_arg)
{}

template <typename Arg_Type, typename String_Type>
requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
             is_complete_unicode_string_arg_type_c<String_Type>
         && UNICODE_BRIDGE_NAMESPACE_INTERNAL::is_arg_type_c<Arg_Type>
std::u8string
    basic_unicode_error::generic_begin_str(
        const std::size_t    character_index_arg,
        const std::u8string& chars_as_hex_arg,
        const size_t         n_code_units_arg,
        const char8_t*       utf_standard_arg,
        const String_Type&   string_type_arg
    ) const noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    u8string msg;
    msg.append(u8"The ");
    if constexpr (same_as<Arg_Type, string_arg>)
    {
        msg.append(integer_list(character_index_arg + 1, n_code_units_arg));
        msg.append(u8"(");
        msg.append(chars_as_hex_arg);
        msg.append(u8") in ");
    }
    else if constexpr (same_as<Arg_Type, forward_iterator_arg>)
    {
        if (n_code_units_arg == 1)
        {
            msg.append(u8"code unit ");
        }
        else
        {
            msg.append(small_number_as_string(n_code_units_arg));
            msg.append(u8" code units ");
        }
        msg.append(u8"at the current "
                   u8"iterator position (");
        msg.append(chars_as_hex_arg);
        msg.append(u8") of ");
    }
    else
    {
        if (n_code_units_arg == 1)
        {
            msg.append(u8"code unit ");
        }
        else
        {
            msg.append(small_number_as_string(n_code_units_arg));
            msg.append(u8" code units ");
        }
        msg.append(u8"immediately preceding the current "
                   u8"iterator position (");
        msg.append(chars_as_hex_arg);
        if (n_code_units_arg > 1)
        {
            msg.append(u8", shown in their original left-to-right order");
        }
        msg.append(u8") of ");
    }
    msg.append(u8"the UTF-");
    msg.append(utf_standard_arg);
    msg.append(u8" input ");
    bool brackets_open = false;
    if constexpr (not same_as<String_Type, monostate>)
    {
        using CharT = typename complete_string_arg_char_type<
            std::basic_string_view<char_type_of_t<String_Type>>>::type;
        basic_string_view<CharT> sv;
        size_t                   start_idx = character_index_arg;
        if constexpr (requires { typename String_Type::first_type; })
        {
            if constexpr (same_as<Arg_Type, backward_iterator_arg>)
            {
                sv = basic_string_view<CharT>(
                    string_type_arg.second, string_type_arg.first
                );
                start_idx = sv.size() - n_code_units_arg;
            }
            else
            {
                sv = basic_string_view<CharT>(
                    string_type_arg.first, string_type_arg.second
                );
            }
        }
        else
        {
            sv = string_type_arg;
        }
        brackets_open = true;
        msg.append(u8"(\"");
        auto [cutoff_left, cutoff_right, focused_str_view]
            = make_focused_string(sv, start_idx, n_code_units_arg);
        msg.append(cutoff_left ? u8"..." : u8"");
        msg.append(to_formatted_unicode_string<char8_t>(focused_str_view));
        msg.append(cutoff_right ? u8"..." : u8"");
        msg.append(u8"\"");
    }
    if (is_wchar())
    {
        msg.append(brackets_open ? u8", " : u8"(");
        brackets_open = true;
        msg.append(u8"encoded using wchar_t");
    }
    if (brackets_open)
    {
        msg.append(u8") ");
    }
    msg.append(u8"passed to the function");
    if constexpr (same_as<Arg_Type, backward_iterator_arg>)
    {
        msg.append(u8",");
    }
    return msg;
}

template <typename Arg_Type, typename String_Type>
requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
             is_complete_unicode_string_arg_type_c<String_Type>
         && UNICODE_BRIDGE_NAMESPACE_INTERNAL::is_arg_type_c<Arg_Type>
constexpr std::u8string
    basic_unicode_error::internal_message(
        const std::size_t  character_index_arg,
        const String_Type& string_arg
    ) const noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    u8string msg;
    auto     utf32_invalid = [&](const std::u8string_view codepoint_as_hex_arg)
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
        msg.append(u8", it cannot represent a valid Unicode scalar value, and "
                   u8"the function was terminated.");
    };
    auto utf8_begin_str
        = [&](const u8string& chars_as_hex, const size_t n_code_units)
    {
        msg.append(generic_begin_str<Arg_Type>(
            character_index_arg, chars_as_hex, n_code_units, u8"8", string_arg
        ));
    };
    using enum basic_unicode_error::basic_unicode_error_code;
    switch (_code)
    {
    case invalid_utf32_code_point:
    {
        const u8string codepoint_as_hex
            = make_hex_from_char_with_prefix_2<char32_t, true, true>(
                _char32_character, u8"U+"
            );
        u8string chars_as_hex;
        chars_as_hex.append(
            make_hex_from_char_with_prefix<char32_t, true, true>(
                _char32_character, u8"0x"
            )
        );
        msg.append(generic_begin_str<Arg_Type>(
            character_index_arg, chars_as_hex, 1, u8"32", string_arg
        ));
        msg.append(u8" decodes to ");
        msg.append(codepoint_as_hex);
        msg.append(u8". However, ");
        utf32_invalid(codepoint_as_hex);
        break;
    }
    case overlong_encoding:
    {
        const u8string codepoint_as_hex
            = make_hex_from_char_with_prefix_2<char32_t, true, true>(
                _char32_character, u8"U+"
            );
        auto char_as_u8
            = unicode_conversion_with_exception<char8_t>(_char32_character);
        utf8_begin_str(
            chars_to_hex(_u8_code_points, _auxillery_data), _auxillery_data
        );
        msg.append(u8" form a ");
        msg.append(small_number_as_string(_auxillery_data));
        msg.append(u8"-byte sequence encoding ");
        msg.append(codepoint_as_hex);
        msg.append(u8". This is an overlong encoding — ");
        msg.append(codepoint_as_hex);
        msg.append(u8" can be represented using ");
        if (char_as_u8.size() == 1)
        {
            msg.append(u8"a single byte (");
        }
        else
        {
            msg.append(small_number_as_string(char_as_u8.size()));
            msg.append(u8" bytes (");
        }
        msg.append(chars_to_hex(char_as_u8, char_as_u8.size()));
        msg.append(
            u8"), which is the shortest valid UTF-8 representation. The "
            u8"UTF-8 standard requires that code points are always "
            u8"encoded using the shortest possible sequence. As this "
            u8"requirement is not met, the sequence does not represent "
            u8"a valid Unicode scalar value, and the function was terminated."
        );
    }
    break;
    case invalid_utf32_code_point_after_utf8_conversion:
    {
        const u8string codepoint_as_hex
            = make_hex_from_char_with_prefix<char32_t, true, true>(
                _char32_character, u8"U+"
            );
        utf8_begin_str(
            chars_to_hex(_u8_code_points, _auxillery_data), _auxillery_data
        );
        msg.append(u8" form a ");
        msg.append(small_number_as_string(_auxillery_data));
        msg.append(u8"-byte sequence encoding ");
        msg.append(codepoint_as_hex);
        msg.append(u8". However, ");
        utf32_invalid(codepoint_as_hex);
    }
    break;
    }
    return msg;
}

constexpr const basic_unicode_error::basic_unicode_error_code
    basic_unicode_error::code() const noexcept
{
    return _code;
}

constexpr const std::array<char8_t, 4>&
    basic_unicode_error::u8_code_points() const noexcept
{
    return _u8_code_points;
}

constexpr const char32_t
    basic_unicode_error::char32_character() const noexcept
{
    return _char32_character;
}

constexpr const std::uint8_t
    basic_unicode_error::auxillery_data() const noexcept
{
    return _auxillery_data;
}

constexpr const std::array<char16_t, 2>&
    basic_unicode_error::u16_code_points() const noexcept
{
    return _u16_code_points;
}

constexpr const bool
    basic_unicode_error::is_wchar() const noexcept
{
    return _is_wchar;
}

constexpr forward_scan_unicode_error::forward_scan_unicode_error(
    const forward_scan_unicode_error_code code_arg,
    const basic_unicode_error&            basic_unicode_error_arg
) noexcept
    : _code(code_arg), _basic_unicode_error(basic_unicode_error_arg)
{}

template <typename Arg_Type, typename String_Type>
requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
             is_complete_unicode_string_arg_type_c<String_Type>
         && UNICODE_BRIDGE_NAMESPACE_INTERNAL::is_arg_type_c<Arg_Type>
constexpr std::u8string
    forward_scan_unicode_error::message(
        const std::size_t  character_index_arg,
        const String_Type& string_arg
    ) const noexcept
{
    using enum forward_scan_unicode_error_code;
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    auto     char32_character{_basic_unicode_error.char32_character()};
    u8string msg;
    auto     utf8_begin_str
        = [&](const u8string& chars_as_hex, const size_t n_code_units)
    {
        msg.append(_basic_unicode_error.generic_begin_str<Arg_Type>(
            character_index_arg, chars_as_hex, n_code_units, u8"8", string_arg
        ));
    };
    auto utf16_begin_str
        = [&](const u8string& chars_as_hex, const size_t n_code_units)
    {
        msg.append(_basic_unicode_error.generic_begin_str<Arg_Type>(
            character_index_arg, chars_as_hex, n_code_units, u8"16", string_arg
        ));
    };
    auto u8_code_points{_basic_unicode_error.u8_code_points()};
    switch (_code)
    {
    case invalid_leading_byte:
    {
        utf8_begin_str(chars_to_hex(u8_code_points, 1), 1);
        msg.append(
            u8" was found to be an invalid leading byte. A valid leading "
            u8"byte must be inclusively within one of the following "
            u8"ranges: "
            u8"0x00 to 0x7F (single-byte sequence), 0xC0 to 0xDF (two-byte "
            u8"sequence), 0xE0 to 0xEF (three-byte sequence), or 0xF0 to "
            u8"0xF7 "
            u8"(four-byte sequence). As "
        );
        msg.append(represent_char_as_hex_for_output(u8_code_points[0]));
        msg.append(u8" falls outside all of these ranges, it cannot begin a "
                   u8"sequence representing a valid Unicode scalar value, and "
                   u8"the function was terminated.");
    }
    break;
    case truncated_sequence:
    {
        static constexpr tuple<uint8_t, uint8_t, const char8_t*>
            truncated_sequence_byte_table[] = {
                {2,
                 1, u8" was found to be a valid leading byte, indicating the "
                 u8"start of a two-byte sequence. However, the input ended "
                 u8"after the first code unit — one continuation byte was "
                 u8"expected but was not present. As the sequence is "
                 u8"incomplete, it cannot represent a valid Unicode scalar "
                 u8"value, and the function was terminated."               }, //
                // case 0
                {3,
                 1, u8" was found to be a valid leading byte, indicating "
                 u8"the "
                 u8"start of a three-byte sequence. However, the input "
                 u8"ended "
                 u8"after the first code unit — two continuation bytes "
                 u8"were "
                 u8"expected but none were present. As the sequence is "
                 u8"incomplete, it cannot represent a valid Unicode "
                 u8"scalar "
                 u8"value, and the function was terminated."               }, //   case 1
                {3,
                 2, u8" form the start of a three-byte sequence. However, "
                 u8"the "
                 u8"input ended after the second code unit — one "
                 u8"further "
                 u8"continuation byte was expected but was not present. "
                 u8"As "
                 u8"the sequence is incomplete, it cannot represent a "
                 u8"valid "
                 u8"Unicode scalar value, and the function was "
                 u8"terminated."                                           }, // case 2
                {4,
                 1, u8" was found to be a valid leading byte, indicating the "
                 u8"start of a four-byte sequence. However, the input "
                 u8"ended "
                 u8"after the first code unit — three continuation bytes "
                 u8"were "
                 u8"expected but none were present. As the sequence is "
                 u8"incomplete, it cannot represent a valid Unicode scalar "
                 u8"value, and the function was terminated."               }, //
                // case 3
                {4,
                 2, u8" form the start of a four-byte sequence. However, "
                 u8"the "
                 u8"input ended after the second code unit — two "
                 u8"further "
                 u8"continuation bytes were expected but none were "
                 u8"present. As "
                 u8"the sequence is incomplete, it cannot represent a "
                 u8"valid "
                 u8"Unicode scalar value, and the function was "
                 u8"terminated."                                           }, // case 4
                {4,
                 3, u8" form the start of a four-byte sequence. However, the "
                 u8"input ended after the third code unit — one further "
                 u8"continuation byte was expected but was not present. As "
                 u8"the sequence is incomplete, it cannot represent a "
                 u8"valid "
                 u8"Unicode scalar value, and the function was terminated."
                }, // case 5
        };
        const auto& [expected_code_points, code_points_encountered, error_str]
            = truncated_sequence_byte_table[_basic_unicode_error.auxillery_data(
            )];
        utf8_begin_str(
            chars_to_hex(u8_code_points, code_points_encountered),
            code_points_encountered
        );
        msg.append(error_str);
    }
    break;
    case invalid_continuation_byte:
    {
        static constexpr tuple<
            uint8_t,
            array<size_t, 3>,
            size_t,
            const char8_t*,
            const char8_t*,
            const char8_t*>
            continuation_byte_error_table[] = {
                {2,
                 {1, 0, 0},
                 1, u8" form the start of a two-byte sequence. The second "
                 u8"code "
                 u8"unit (",                  u8") was expected to be a continuation byte, but was not "
                 u8"— a "
                 u8"valid continuation byte must be inclusively between "
                 u8"0x80 "
                 u8"and 0xBF. As ",                  u8" falls outside this range, the sequence cannot "
                 u8"represent a "
                 u8"valid Unicode scalar value, and the function was "
                 u8"terminated."                     }, // case 0
                {3,
                 {1, 0, 0},
                 1, u8" form the start of a three-byte sequence. The second "
                 u8"code "
                 u8"unit (",                  u8") was expected to be a continuation byte, but was not "
                 u8"— a "
                 u8"valid continuation byte must be inclusively between "
                 u8"0x80 "
                 u8"and 0xBF. As ",                  u8" falls outside this range, the sequence cannot "
                 u8"represent a "
                 u8"valid Unicode scalar value, and the function was "
                 u8"terminated."                     }, // case 1
                {3,
                 {2, 0, 0},
                 1, u8" form the start of a three-byte sequence. The third "
                 u8"code "
                 u8"unit (",                  u8") was expected to be a continuation byte, but was not "
                 u8"— a "
                 u8"valid continuation byte must be inclusively between "
                 u8"0x80 "
                 u8"and 0xBF. As ",                  u8" falls outside this range, the sequence cannot "
                 u8"represent a "
                 u8"valid Unicode scalar value, and the function was "
                 u8"terminated."                     }, // case 2
                {3,
                 {1, 2, 0},
                 2, u8" form the start of a three-byte sequence. The second "
                 u8"and "
                 u8"third code units (",      u8") were expected to be continuation bytes, but were not "
                 u8"— a "
                 u8"valid continuation byte must be inclusively between "
                 u8"0x80 "
                 u8"and 0xBF. As ",      u8"both are outside this range, the sequence cannot "
                 u8"represent "
                 u8"a valid Unicode scalar value, and the function was "
                 u8"terminated."         }, // case 3
                {4,
                 {1, 0, 0},
                 1, u8" form the start of a four-byte sequence. The second "
                 u8"code "
                 u8"unit (",                  u8") was expected to be a continuation byte, but was not "
                 u8"— a "
                 u8"valid continuation byte must be inclusively between "
                 u8"0x80 "
                 u8"and 0xBF. As ",                  u8" falls outside this range, the sequence cannot "
                 u8"represent a "
                 u8"valid Unicode scalar value, and the function was "
                 u8"terminated."                     }, // case 4
                {4,
                 {2, 0, 0},
                 1, u8" form the start of a four-byte sequence. The third "
                 u8"code "
                 u8"unit (",                  u8") was expected to be a continuation byte, but was not "
                 u8"— a "
                 u8"valid continuation byte must be inclusively between "
                 u8"0x80 "
                 u8"and 0xBF. As ",                  u8" falls outside this range, the sequence cannot "
                 u8"represent a "
                 u8"valid Unicode scalar value, and the function was "
                 u8"terminated."                     }, // case 5
                {4,
                 {3, 0, 0},
                 1, u8" form the start of a four-byte sequence. The fourth "
                 u8"code "
                 u8"unit (",                  u8") was expected to be a continuation byte, but was not "
                 u8"— a "
                 u8"valid continuation byte must be inclusively between "
                 u8"0x80 "
                 u8"and 0xBF. As ",                  u8" falls outside this range, the sequence cannot "
                 u8"represent a "
                 u8"valid Unicode scalar value, and the function was "
                 u8"terminated."                     }, // case 6
                {4,
                 {1, 2, 0},
                 2, u8" form the start of a four-byte sequence. The second "
                 u8"and "
                 u8"third code units (",      u8") were expected to be continuation bytes, but were not "
                 u8"— a "
                 u8"valid continuation byte must be inclusively between "
                 u8"0x80 "
                 u8"and 0xBF. As ",      u8"both are outside this range, the sequence cannot "
                 u8"represent "
                 u8"a valid Unicode scalar value, and the function was "
                 u8"terminated."         }, // case 7
                {4,
                 {1, 3, 0},
                 2, u8" form the start of a four-byte sequence. The second "
                 u8"and "
                 u8"fourth code units (",     u8") were expected to be continuation bytes, but were not "
                 u8"— a "
                 u8"valid continuation byte must be inclusively between "
                 u8"0x80 "
                 u8"and 0xBF. As ",     u8"both are outside this range, the sequence cannot "
                 u8"represent "
                 u8"a valid Unicode scalar value, and the function was "
                 u8"terminated."        }, // case 8
                {4,
                 {2, 3, 0},
                 2, u8" form the start of a four-byte sequence. The third and "
                 u8"fourth code units (",     u8") were expected to be continuation bytes, but were not "
                 u8"— a "
                 u8"valid continuation byte must be inclusively between "
                 u8"0x80 "
                 u8"and 0xBF. As ",     u8"both are outside this range, the sequence cannot "
                 u8"represent "
                 u8"a valid Unicode scalar value, and the function was "
                 u8"terminated."        }, // case 9
                {4,
                 {1, 2, 3},
                 3, u8" form the start of a four-byte sequence. The second, "
                 u8"third "
                 u8"and fourth code units (", u8") were expected to be continuation bytes, but were not "
                 u8"— a "
                 u8"valid continuation byte must be inclusively between "
                 u8"0x80 "
                 u8"and 0xBF. As ", u8"all three are outside this range, the sequence cannot "
                 u8"represent a valid Unicode scalar value, and the "
                 u8"function "
                 u8"was terminated."}, // case 10
        };

        const auto& [expected_code_point_size, invalid_code_point_indexes, numb_invalid_code_points, str_1, str_2, str_3]
            = continuation_byte_error_table[_basic_unicode_error.auxillery_data(
            )];
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
                u8_code_points[invalid_code_point_index]
            ));
        }
        utf8_begin_str(
            chars_to_hex(u8_code_points, expected_code_point_size),
            expected_code_point_size
        );
        msg.append(str_1);
        msg.append(invalid_code_points);
        msg.append(str_2);
        msg.append(numb_invalid_code_points == 1 ? invalid_code_points : u8"");
        msg.append(str_3);
    }
    break;
    case high_surrogate_then_end_of_stream:
    {
        utf16_begin_str(
            chars_to_hex(_basic_unicode_error.u16_code_points(), 1), 1
        );
        msg.append(
            u8" is a high surrogate, indicating the start of a surrogate "
            u8"pair. However, the input ended after this code unit — a low "
            u8"surrogate was expected to follow but was not present. As "
            u8"the "
            u8"surrogate pair is incomplete, it cannot represent a valid "
            u8"Unicode scalar value, and the function was terminated."
        );
    }
    break;
    case high_surrogate_not_followed_by_low_surrogate:
    {
        utf16_begin_str(
            chars_to_hex(_basic_unicode_error.u16_code_points(), 2), 2
        );
        msg.append(
            u8" form the start of a surrogate pair. The first code unit ("
        );
        msg.append(represent_char_as_hex_for_output(
            _basic_unicode_error.u16_code_points()[0]
        ));
        msg.append(u8") is a high surrogate, which must be followed by a low "
                   u8"surrogate inclusively between 0xDC00 and 0xDFFF. "
                   u8"However, the second code unit (");
        msg.append(represent_char_as_hex_for_output(
            _basic_unicode_error.u16_code_points()[1]
        ));
        msg.append(u8") falls outside this range, and therefore the two code "
                   u8"units cannot represent a valid Unicode scalar value, and "
                   u8"the function was terminated.");
    }
    break;
    case unexpected_low_surrogate:
    {
        utf16_begin_str(
            chars_to_hex(_basic_unicode_error.u16_code_points(), 1), 1
        );
        msg.append(
            u8" is a low surrogate. Low surrogates must always be preceded "
            u8"by "
            u8"a high surrogate (inclusively between 0xD800 and 0xDBFF) as "
            u8"the "
            u8"second part of a surrogate pair. As this low surrogate "
            u8"appears "
            u8"without a preceding high surrogate, it cannot represent a "
            u8"valid "
            u8"Unicode scalar value, and the function was terminated."
        );
    }
    break;
    case basic_unicode_error:
        msg.append(_basic_unicode_error.internal_message<Arg_Type>(
            character_index_arg, string_arg
        ));
        break;
    }
    return msg;
}

constexpr const forward_scan_unicode_error::forward_scan_unicode_error_code
    forward_scan_unicode_error::code() const noexcept
{
    return _code;
}

constexpr const basic_unicode_error&
    forward_scan_unicode_error::error() const noexcept
{
    return _basic_unicode_error;
}

template <typename Arg_Type, typename String_Type>
requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
             is_complete_unicode_string_arg_type_c<String_Type>
         && UNICODE_BRIDGE_NAMESPACE_INTERNAL::is_arg_type_c<Arg_Type>
constexpr std::u8string
    unicode_conversion_error::message(
        const String_Type& str_arg
    ) const noexcept
{
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return _forward_scan_unicode_error.message<string_arg>(
        _character_index, str_arg
    );
}

constexpr unicode_conversion_error::unicode_conversion_error(
    const std::size_t                 character_index_arg,
    const forward_scan_unicode_error& basic_unicode_error_arg
) noexcept
    : _forward_scan_unicode_error(basic_unicode_error_arg)
    , _character_index(character_index_arg)
{}

constexpr const forward_scan_unicode_error&
    unicode_conversion_error::error() const noexcept
{
    return _forward_scan_unicode_error;
}

constexpr const std::size_t
    unicode_conversion_error::character_index() const noexcept
{
    return _character_index;
}

constexpr std::u8string
    unicode_conversion_error::message() const noexcept
{
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return message<string_arg>(std::monostate{});
}

template <typename CharT>
requires char_type_is_unicode_c<CharT>
constexpr std::u8string
    unicode_conversion_error::message(
        const std::basic_string_view<CharT>& str_arg
    ) const noexcept
{
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return message<string_arg>(str_arg);
}

template <typename CharT>
    requires char_type_is_unicode_c<CharT>
constexpr std::u8string
unicode_conversion_error::message(const CharT char_arg) const noexcept
{
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return message<string_arg,std::basic_string_view<CharT>>(std::basic_string(1,char_arg));
}

template <typename Error_Type>
requires is_error_type_c<Error_Type>
constexpr unicode_bridge_exception<Error_Type>::unicode_bridge_exception(
    const Error_Type& error_arg
) noexcept
    : _error(error_arg)
{}

template <typename Error_Type>
requires is_error_type_c<Error_Type>
const char*
    unicode_bridge_exception<Error_Type>::what() const noexcept
{
    using namespace std;
    if (_what_cache.empty())
    {
        const u8string u8msg = _error.message();
        _what_cache
            = string(reinterpret_cast<const char*>(u8msg.data()), u8msg.size());
    }
    return _what_cache.c_str();
}

template <typename Error_Type>
requires is_error_type_c<Error_Type>
constexpr const Error_Type&
    unicode_bridge_exception<Error_Type>::error() const noexcept
{
    return _error;
}

template <typename CharT>
requires char_type_is_unicode_c<CharT>
template <typename Sink>
requires (std::same_as<Sink, ostream_sink> || std::same_as<Sink, string_sink>)
constexpr void
    unicode_print<CharT>::stream_impl(
        Sink& sinkg_arg
    ) const
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    auto char32_to_stream = [&](const char32_t char_arg)
    {
        if (char_arg > 0x10'FFFF)
        {
            sinkg_arg.write(reinterpret_cast<const char*>(u8"\uFFFD"), 3);
        }
        else if (char_arg <= 0x7F)
        {
            sinkg_arg.put(static_cast<char>(char_arg));
        }
        else if (char_arg <= 0x7FF)
        {
            sinkg_arg.put(static_cast<char>(0xC0 | (char_arg >> 6)));
            sinkg_arg.put(static_cast<char>(0x80 | (char_arg & 0x3F)));
        }
        else if (char_arg <= 0xFFFF)
        {
            sinkg_arg.put(static_cast<char>(0xE0 | (char_arg >> 12)));
            sinkg_arg.put(static_cast<char>(0x80 | ((char_arg >> 6) & 0x3F)));
            sinkg_arg.put(static_cast<char>(0x80 | (char_arg & 0x3F)));
        }
        else
        {
            sinkg_arg.put(static_cast<char>(0xF0 | (char_arg >> 18)));
            sinkg_arg.put(static_cast<char>(0x80 | ((char_arg >> 12) & 0x3F)));
            sinkg_arg.put(static_cast<char>(0x80 | ((char_arg >> 6) & 0x3F)));
            sinkg_arg.put(static_cast<char>(0x80 | (char_arg & 0x3F)));
        }
    };
    auto stream_u16 = [&](const auto str_arg)
    {
        auto       it  = str_arg.begin();
        const auto end = str_arg.end();
        while (it != end)
        {
            const char16_t unit = *it++;
            if (unit >= 0xD800 && unit <= 0xDBFF)
            {
                // High surrogate -- look for following low surrogate
                if (it != end)
                {
                    const char16_t next = *it;
                    if (next >= 0xDC00 && next <= 0xDFFF)
                    {
                        // Valid pair -- decode to char32_t and encode as
                        // UTF-8
                        const char32_t cp
                            = 0x1'0000
                              + ((static_cast<char32_t>(unit) - 0xD800) << 10)
                              + (static_cast<char32_t>(next) - 0xDC00);
                        char32_to_stream(cp);
                        ++it;
                        continue;
                    }
                }
                // Lone high surrogate -- encode mechanically
                char32_to_stream(static_cast<char32_t>(unit));
            }
            else
            {
                // BMP character or lone low surrogate -- encode
                // mechanically
                char32_to_stream(static_cast<char32_t>(unit));
            }
        }
    };
    auto stream_u32 = [&](const auto str_arg)
    {
        for (auto& character : str_arg)
        {
            char32_to_stream(character);
        }
    };
    if constexpr (same_as<char16_t, CharT>)
    {
        stream_u16(_str);
    }
    else if constexpr (is_wchar_and_16_bit_c<CharT>)
    {
        auto u16_str = cast_wstring_to_unicode_string(_str);
        stream_u16(u16_str);
    }
    else if constexpr (same_as<char32_t, CharT>)
    {
        stream_u32(_str);
    }
    else if constexpr (is_wchar_and_32_bit_c<CharT>)
    {
        auto u32_str = cast_wstring_to_unicode_string(_str);
        stream_u32(u32_str);
    }
    else if constexpr (same_as<char8_t, CharT>)
    {
        sinkg_arg.write(
            reinterpret_cast<const char*>(_str.data()), _str.size()
        );
    }
}

template <typename CharT>
requires char_type_is_unicode_c<CharT>
constexpr unicode_print<CharT>::unicode_print(
    const std::basic_string_view<CharT> str_arg
) noexcept
    : _str(str_arg)
{}

template <typename CharT>
requires char_type_is_unicode_c<CharT>
constexpr std::string
    unicode_print<CharT>::str() const
{
    std::string rv;
    string_sink sink{rv};
    stream_impl(sink);
    return rv;
}

template <typename CharT>
requires char_type_is_unicode_c<CharT>
inline std::ostream&
    operator<<(
        std::ostream&               os,
        const unicode_print<CharT>& w
    )
{
    ostream_sink sink{os};
    w.stream_impl(sink);
    return os;
}

template <typename Arg_Type, typename String_Type>
requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::is_complete_ascii_string_arg_type_c<
             String_Type>
         && UNICODE_BRIDGE_NAMESPACE_INTERNAL::is_arg_type_c<Arg_Type>
constexpr std::u8string
    ascii_to_unicode_error::message(
        const String_Type& str_arg
    ) const
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    u8string msg;
    msg.append(u8"The ");
    msg.append(positive_integer_to_placement(_index + 1));
    msg.append(u8" character (");
    msg.append(represent_char_as_hex_for_output(_character));
    msg.append(u8") in the ASCII input ");
    if constexpr (not std::same_as<String_Type, std::monostate>)
    {
        msg.append(u8"(\"");
        auto [cutoff_left, cutoff_right, focused_str_view]
            = make_focused_string(str_arg, _index, 1);
        msg.append(cutoff_left ? u8"..." : u8"");
        msg.append(to_formatted_unicode_string<char8_t>(focused_str_view));
        msg.append(cutoff_right ? u8"..." : u8"");
        msg.append(u8"\") ");
    }
    msg.append(u8"passed to the function was found to be invalid ASCII. This "
               u8"function can only convert ASCII to Unicode — that is, "
               u8"character values inclusively between 0x00 and 0x7F. As ");
    msg.append(represent_char_as_hex_for_output(_character));
    msg.append(u8" falls outside this "
               u8"range, it cannot be converted to Unicode, and the function "
               u8"was terminated.");
    return msg;
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

constexpr std::u8string
    ascii_to_unicode_error::message(
        const std::string_view str_arg
    ) const
{
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return message<string_arg>(str_arg);
}

constexpr std::u8string
    ascii_to_unicode_error::message(
        const char char_arg
    ) const
{
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return message<string_arg, std::string_view>(std::string(1, char_arg));
}

constexpr std::u8string
    ascii_to_unicode_error::message() const
{
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return message<forward_iterator_arg>(std::monostate{});
}

constexpr unicode_to_ascii_error::unicode_to_ascii_error(
    const unicode_to_ascii_error_code code_arg,
    const unicode_conversion_error&   error_arg
) noexcept
    : _code(code_arg), _error(error_arg)
{}

template <typename String_Type>
requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
    is_complete_unicode_string_arg_type_c<String_Type>
    constexpr std::u8string
    unicode_to_ascii_error::internal_message(
        String_Type str_arg
    ) const
{
    using enum unicode_to_ascii_error_code;
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    u8string msg;
    auto&    basic_error_data{
        _error._forward_scan_unicode_error._basic_unicode_error
    };
    auto ki = special_char_as_string<char32_t, char32_t>(
        basic_error_data._char32_character
    );
    auto print_func = [&]()
    {
        msg.append(u8" encode");
        msg.append(basic_error_data._auxillery_data == 1 ? u8"s" : u8"");
        msg.append(u8" the Unicode scalar value ");
        msg.append(make_hex_from_char_with_prefix_2<char32_t, true, true>(
            basic_error_data._char32_character, u8"U+"
        ));
        if (ki.has_value() == false || ki.value().size() <= 2)
        {
            msg.append(u8" ('");
            msg.append(to_formatted_unicode_string<char8_t>(
                basic_error_data._char32_character
            ));
            msg.append(u8"')");
        }
        msg.append(
            u8". However, this function converts Unicode to ASCII — valid "
            u8"ASCII values are inclusively between U+0000 and U+007F. As "
        );
        msg.append(make_hex_from_char_with_prefix_2<char32_t, true, true>(
            basic_error_data._char32_character, u8"U+"
        ));
        msg.append(u8" falls outside this range, it cannot be represented as "
                   "ASCII, and the function was terminated.");
    };
    switch (_code)
    {
    case non_ascii_character_found_from_utf8:
        msg.append(basic_error_data.generic_begin_str<string_arg>(
            _error._character_index,
            chars_to_hex(
                basic_error_data._u8_code_points,
                basic_error_data._auxillery_data
            ),
            basic_error_data._auxillery_data,
            u8"8",
            str_arg
        ));
        print_func();
        break;
    case non_ascii_character_found_from_utf16:
        msg.append(basic_error_data.generic_begin_str<string_arg>(
            _error._character_index,
            chars_to_hex(
                basic_error_data._u16_code_points,
                basic_error_data._auxillery_data
            ),
            basic_error_data._auxillery_data,
            u8"16",
            str_arg
        ));
        print_func();
        break;
    case non_ascii_character_found_from_utf32:
        msg.append(basic_error_data.generic_begin_str<string_arg>(
            _error._character_index,
            chars_to_hex(
                std::array<char32_t, 1>{basic_error_data._char32_character},
                basic_error_data._auxillery_data
            ),
            basic_error_data._auxillery_data,
            u8"32",
            str_arg
        ));
        print_func();
        break;
    case invalid_unicode_character:
        msg.append(_error.message<string_arg>(str_arg));
        break;
    }
    return msg;
}

constexpr const unicode_to_ascii_error::unicode_to_ascii_error_code
    unicode_to_ascii_error::get_enum() const noexcept
{
    return _code;
}

constexpr const unicode_conversion_error&
    unicode_to_ascii_error::error() const noexcept
{
    return _error;
}

constexpr std::u8string
    unicode_to_ascii_error::message() const
{
    return internal_message(std::monostate{});
}

template <typename ArgType>
requires char_type_is_unicode_c<char_type_of_t<ArgType>>
         && std::convertible_to<
             ArgType,
             std::basic_string_view<char_type_of_t<ArgType>>>
constexpr std::u8string
    unicode_to_ascii_error::message(
        ArgType str_arg
    ) const
{
    return internal_message(str_arg);
}

template <typename CharT>
requires char_type_is_unicode_c<CharT>
constexpr std::u8string
    unicode_to_ascii_error::message(
        const CharT char_arg
    ) const
{
    return internal_message<std::basic_string_view<CharT>>(
        std::basic_string<CharT>(1, char_arg)
    );
}

constexpr next_char32_error::next_char32_error(
    const next_char32_error_code      code_arg,
    const forward_scan_unicode_error& error_arg
) noexcept
    : _code(code_arg), _error(error_arg)
{}

template <typename Arg_Type, typename String_Type>
requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
    is_complete_unicode_string_arg_type_c<String_Type>
    constexpr std::u8string
    next_char32_error::message(
        const String_Type& str_arg
    ) const
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using enum next_char32_error_code;
    u8string msg;
    switch (_code)
    {
    case unicode_error:
        msg.append(_error.message<Arg_Type>(0, str_arg));
        break;
    case iterator_end:
        msg.append(
            u8"The current iterator passed to the function was equal to the "
            u8"end iterator — "
            u8"signifying that there were no more code units to read, and the "
            u8"function was terminated."
        );
        break;
    }
    return msg;
}

constexpr const next_char32_error::next_char32_error_code
    next_char32_error::get_enum() const noexcept
{
    return _code;
}

constexpr const forward_scan_unicode_error&
    next_char32_error::error() const noexcept
{
    return _error;
}

template <typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::u8string
    next_char32_error::message(
        ItteratorType itt_begin_arg,
        ItteratorType itt_end_arg
    ) const
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return message<forward_iterator_arg>(make_pair(itt_begin_arg, itt_end_arg));
}

constexpr std::u8string
    next_char32_error::message() const
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return message<forward_iterator_arg>(std::monostate{});
}

constexpr prev_char32_error::prev_char32_error(
    const prev_char32_error_code code_arg,
    const basic_unicode_error&   basic_unicode_error_arg
) noexcept
    : _basic_unicode_error(basic_unicode_error_arg), _code(code_arg)
{}

template <typename String_Type>
requires UNICODE_BRIDGE_NAMESPACE_INTERNAL::
    is_complete_unicode_string_arg_type_c<String_Type>
    constexpr std::u8string
    prev_char32_error::message(
        const String_Type& string_type_arg
    ) const
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using CharT = typename complete_string_arg_char_type<String_Type>::type;
    using enum prev_char32_error_code;
    bool     brackets_open = false;
    u8string msg;
    switch (_code)
    {
    case base_unicode_error:
    {
        msg.append(_basic_unicode_error.internal_message<backward_iterator_arg>(
            0, string_type_arg
        ));
        break;
    }
    case no_valid_leading_byte_found:
        msg.append(
            _basic_unicode_error.generic_begin_str<backward_iterator_arg>(
                0,
                chars_to_hex(_basic_unicode_error.u8_code_points(), 4),
                4,
                u8"8",
                string_type_arg
            )
        );
        msg.append(
            u8" were scanned backwards. No "
            u8"valid leading byte was found — a valid UTF-8 leading byte must "
            u8"be "
            u8"in one of the following ranges: 0x00 to 0x7F (single-byte), "
            u8"0xC2 to 0xDF (two-byte), 0xE0 to 0xEF (three-byte), or 0xF0 to "
            u8"0xF7 (four-byte). As none of the four code units fall within "
            u8"any "
            u8"of these ranges, and the maximum number of continuation bytes "
            u8"was "
            u8"scanned, it was determined that these code units cannot "
            u8"represent "
            u8"a valid Unicode scalar value, and the function was terminated."
        );
        break;
    case leading_byte_sequence_length_mismatch:
    {
        const size_t  n_continuation_found{_basic_unicode_error.auxillery_data()
        };
        const size_t  total_units{n_continuation_found + 1};
        const char8_t leading_byte{_basic_unicode_error.u8_code_points()[0]};
        const size_t expected_sequence_length{utf8_sequence_length(leading_byte)
        };
        const size_t expected_continuation{expected_sequence_length - 1};
        const bool   is_ascii{expected_sequence_length == 1};
        msg.append(
            _basic_unicode_error.generic_begin_str<backward_iterator_arg>(
                0,
                chars_to_hex(
                    _basic_unicode_error.u8_code_points(), total_units
                ),
                total_units,
                u8"8",
                string_type_arg
            )
        );
        msg.append(
            total_units == 1
                ? u8" is invalid. The code unit is "
                : u8" form an invalid sequence. The first code unit is "
        );
        if (is_ascii)
        {
            msg.append(u8"a single-byte ASCII character and should not be "
                       u8"succeeded by any continuation bytes — however, ");
            msg.append(small_number_as_string(n_continuation_found));
            msg.append(
                n_continuation_found == 1
                    ? u8" continuation byte was found succeeding it. As a "
                      u8"single-byte character cannot be part of a multi-byte "
                      u8"sequence, these code units cannot represent a valid "
                      u8"Unicode scalar value, and the function was terminated."
                    : u8" continuation bytes were found succeeding it. As a "
                      u8"single-byte character cannot be part of a multi-byte "
                      u8"sequence, these code units cannot represent a valid "
                      u8"Unicode scalar value, and the function was terminated."
            );
        }
        else
        {
            msg.append(u8"a valid leading byte indicating the start of a ");
            msg.append(small_number_as_string(expected_sequence_length));
            msg.append(u8"-byte sequence, requiring exactly ");
            msg.append(small_number_as_string(expected_continuation));
            msg.append(
                expected_continuation == 1 ? u8" continuation byte to succeed "
                                             u8"the leading byte. However, "
                                           : u8" continuation bytes to succeed "
                                             u8"the leading byte. However, "
            );
            if (n_continuation_found == 0)
            {
                msg.append(u8"no continuation bytes were found. ");
            }
            else
            {
                msg.append(
                    n_continuation_found < expected_continuation ? u8"only "
                                                                 : u8""
                );
                msg.append(small_number_as_string(n_continuation_found));
                msg.append(
                    n_continuation_found == 1
                        ? u8" continuation byte was found. "
                        : u8" continuation bytes were found. "
                );
            }
            msg.append(u8"Therefore, ");
            msg.append(
                n_continuation_found == 0 ? u8"this code unit"
                                          : u8"these code units"
            );
            msg.append(
                u8" cannot represent a valid "
                u8"Unicode scalar value, and the function was terminated."
            );
        }
    }
    break;
    case invalid_utf8_byte:
    {
        const size_t  n_continuation_found{_basic_unicode_error.auxillery_data()
        };
        const size_t  total_units{n_continuation_found + 1};
        const char8_t invalid_byte{_basic_unicode_error.u8_code_points()[0]};
        msg.append(
            _basic_unicode_error.generic_begin_str<backward_iterator_arg>(
                0,
                chars_to_hex(
                    _basic_unicode_error.u8_code_points(), total_units
                ),
                total_units,
                u8"8",
                string_type_arg
            )
        );
        if (total_units == 1)
        {
            msg.append(u8" was found to be invalid — ");
        }
        else
        {
            msg.append(u8" were found to be invalid. "
                       u8"After finding ");
            msg.append(small_number_as_string(n_continuation_found));
            msg.append(
                n_continuation_found == 1
                    ? u8" continuation byte, the next code unit ("
                    : u8" continuation bytes, the next code unit ("
            );
            msg.append(represent_char_as_hex_for_output(invalid_byte));
            msg.append(u8") was found to be invalid — ");
        }
        msg.append(
            u8"a valid UTF-8 byte must be in one of the following ranges: "
            u8"0x00 to 0x7F (single-byte leading byte), 0x80 to 0xBF "
            u8"(continuation byte), 0xC2 to 0xDF (two-byte leading byte), "
            u8"0xE0 to 0xEF (three-byte leading byte), or 0xF0 to 0xF7 "
            u8"(four-byte leading byte). As "
        );
        msg.append(represent_char_as_hex_for_output(invalid_byte));
        msg.append(
            u8" falls outside all of these ranges, it cannot appear in a "
            u8"valid UTF-8 sequence, and the function was terminated."
        );
    }
    break;
    case iterator_begin_reached_before_leading_byte:
    {
        const size_t n_continuation_found{_basic_unicode_error.auxillery_data()
        };
        msg.append(
            _basic_unicode_error.generic_begin_str<backward_iterator_arg>(
                0,
                chars_to_hex(
                    _basic_unicode_error.u8_code_points(), n_continuation_found
                ),
                n_continuation_found,
                u8"8",
                string_type_arg
            )
        );
        msg.append(
            n_continuation_found == 1 ? u8" was scanned backwards. "
                                      : u8" were scanned backwards. "
        );
        msg.append(
            n_continuation_found == 1 ? u8"It is a continuation byte, but "
                                      : u8"They are continuation bytes, but "
        );
        msg.append(
            u8"the beginning of the input was reached before a valid leading "
            u8"byte was found. A valid UTF-8 leading byte must be in one of "
            u8"the "
            u8"following ranges: 0x00 to 0x7F (single-byte), 0xC2 to 0xDF "
            u8"(two-byte), 0xE0 to 0xEF (three-byte), or 0xF0 to 0xF7 "
            u8"(four-byte). As the input ends before such a byte is found, "
        );
        msg.append(
            n_continuation_found == 1 ? u8"this code unit cannot"
                                      : u8"these code units cannot"
        );
        msg.append(
            u8" represent a valid Unicode scalar value, and the function was "
            u8"terminated."
        );
    }
    break;
    case low_surrogate_then_start_of_stream:
    {
        msg.append(
            _basic_unicode_error.generic_begin_str<backward_iterator_arg>(
                0,
                chars_to_hex(_basic_unicode_error.u16_code_points(), 1),
                1,
                u8"16",
                string_type_arg
            )
        );
        msg.append(
            u8" is a low surrogate. Low surrogates "
            u8"must always be preceded "
            u8"by a high surrogate (inclusively between 0xD800 and 0xDBFF) as "
            u8"the second part of a surrogate pair. However, this low "
            u8"surrogate "
            u8"was found at the start of the stream — no preceding code unit "
            u8"exists. As it cannot be the second part of a surrogate pair, it "
            u8"cannot represent a valid Unicode scalar value, and the function "
            u8"was terminated."
        );
    }
    break;
    case low_surrogate_not_preceded_by_high_surrogate:
    {
        msg.append(
            _basic_unicode_error.generic_begin_str<backward_iterator_arg>(
                0,
                chars_to_hex(_basic_unicode_error.u16_code_points(), 2),
                2,
                u8"16",
                string_type_arg
            )
        );
        msg.append(u8" form the end of a surrogate "
                   u8"pair. The second code unit (");
        msg.append(represent_char_as_hex_for_output(
            _basic_unicode_error.u16_code_points()[1]
        ));
        msg.append(u8") is a low surrogate, which must be preceded by a high "
                   u8"surrogate inclusively between 0xD800 and 0xDBFF. "
                   u8"However, the first code unit (");
        msg.append(represent_char_as_hex_for_output(
            _basic_unicode_error.u16_code_points()[0]
        ));
        msg.append(u8") falls outside this range, and therefore the two code "
                   u8"units cannot represent a valid Unicode scalar value, "
                   u8"and the function was terminated.");
        break;
    }
    case unexpected_high_surrogate:
    {
        msg.append(
            _basic_unicode_error.generic_begin_str<backward_iterator_arg>(
                0,
                chars_to_hex(_basic_unicode_error.u16_code_points(), 1),
                1,
                u8"16",
                string_type_arg
            )
        );
        msg.append(u8" is a high surrogate. When "
                   u8"scanned forwards, high "
                   u8"surrogates must always be followed by a low surrogate "
                   u8"(inclusively between "
                   u8"0xDC00 and 0xDFFF). This high surrogate was encountered "
                   u8"while scanning "
                   u8"backwards, and no low surrogate was previously "
                   u8"encountered. Therefore, it is "
                   u8"not part of a surrogate pair, and cannot represent a "
                   u8"valid Unicode scalar "
                   u8"value, and the function was terminated.");
        break;
    }
    }
    return msg;
}

constexpr const prev_char32_error::prev_char32_error_code
    prev_char32_error::get_enum() const noexcept
{
    return _code;
}

constexpr const basic_unicode_error&
    prev_char32_error::error() const noexcept
{
    return _basic_unicode_error;
}

template <typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::u8string
    prev_char32_error::message(
        ItteratorType itt_end_arg,
        ItteratorType itt_begin_arg
    ) const
{
    using namespace std;
    return message(make_pair(itt_end_arg, itt_begin_arg));
}

constexpr std::u8string
    prev_char32_error::message() const
{
    using namespace std;
    return message(std::monostate{});
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
            const auto character_res{
                forward_scan_for_next_char32<true, itt, CharT>(
                    string_iterator, string_iterator_end
                )
            };
            if (character_res.has_value())
            {
                if constexpr (same_as<CharT, char8_t>)
                {
                    array<char8_t, 4> u8_chars
                        = {*string_iterator,
                           *(string_iterator + 1),
                           (character_res.value().second > 2)
                               ? *(string_iterator + 2)
                               : u8'\0',
                           (character_res.value().second > 3)
                               ? *(string_iterator + 3)
                               : u8'\0'};
                    return unexpected(unicode_to_ascii_error_factory::
                                          non_ascii_character_found_from_utf8(
                                              character_res.value().first,
                                              u8_chars,
                                              idx,
                                              character_res.value().second
                                          ));
                }
                else if constexpr (same_as<CharT, char16_t>
                                   || wchar_is_16_bit
                                          && same_as<CharT, wchar_t>)
                {
                    array<char16_t, 2> u16_chars
                        = {*string_iterator,
                           (character_res.value().second > 1)
                               ? *(string_iterator + 1)
                               : zero<CharT>()};
                    return unexpected(unicode_to_ascii_error_factory::
                        non_ascii_character_found_from_utf16(
                            character_res.value().first,
                            u16_chars,
                            idx,
                            character_res.value().second,
                            same_as<CharT,wchar_t>
                        ));
                }
                else if constexpr (same_as<CharT, char32_t>
                                   || wchar_is_32_bit
                                          && same_as<CharT, wchar_t>)
                {
                    return unexpected(unicode_to_ascii_error_factory::
                        non_ascii_character_found_from_utf32(
                            character_res.value().first,
                            idx,
                            same_as<CharT, wchar_t>
                        ));
                }
            }
            else
            {
                return unexpected(
                    unicode_to_ascii_error_factory::invalid_unicode_character(
                        idx, character_res.error()
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
        convert_unicode_to_ascii(str_arg)
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
        convert_unicode_to_ascii(char_arg)
    );
}

template <typename OutputChar, typename ArgType>
requires char_type_is_unicode_c<OutputChar>
         && std::convertible_to<ArgType, std::string_view>
constexpr ascii_to_unicode_result<std::basic_string<OutputChar>>
    convert_ascii_to_unicode(
        ArgType str_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using ResultType               = std::basic_string<OutputChar>;
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
            return unexpected(ascii_to_unicode_error(idx, character));
        }
        else
        {
            output_string_inserter = static_cast<OutputChar>(character);
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
         && std::convertible_to<ArgType, std::string_view>
constexpr std::basic_string<OutputChar>
    convert_ascii_to_unicode_with_exception(
        ArgType str_arg
    )
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return throw_if_error<basic_string<OutputChar>, ascii_to_unicode_error>(
        convert_ascii_to_unicode<OutputChar>(str_arg)
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
        convert_ascii_to_unicode<OutputChar>(char_arg)
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
    auto sv2        = basic_string<OutputChar>();
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
                // Supplementary Plane to surrogate pair
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
    auto validate_u32_string
        = [](const u32string_view str_arg) -> optional<unicode_conversion_error>
    {
        // Checks the input is valid.
        const auto conversion_result{
            if_invalid_u32string_return_char_and_position(str_arg)
        };
        // If invalid, return the reason its invalid.
        if (conversion_result.has_value())
        {
            return make_optional(unicode_conversion_error(
                conversion_result.value().second,
                forward_scan_unicode_error_factory::
                    invalid_utf32_code_point(conversion_result.value().first, same_as<InputChar, wchar_t>)
            ));
        }
        else
        {
            return std::nullopt;
        }
    };
    auto validate_unicode = []<bool Return_Str, typename T>(const T str_arg)
        -> std::conditional_t<
            Return_Str,
            unicode_conversion_result<std::u32string>,
            optional<unicode_conversion_error>>
    {
        auto begin_str   = std::begin(str_arg);
        auto end_str     = std::end(str_arg);
        auto current_itt = begin_str;
        std::conditional_t<
            Return_Str,
            unicode_conversion_result<std::u32string>,
            optional<unicode_conversion_error>>
            rv;
        while (current_itt != end_str)
        {
            auto next_char32 = forward_scan_for_next_char32<
                true,
                decltype(current_itt),
                InputChar>(current_itt, end_str);
            if (next_char32.has_value())
            {
                current_itt += next_char32.value().second;
                if constexpr (Return_Str)
                {
                    rv.value().push_back(next_char32.value().first);
                }
            }
            else
            {
                if constexpr (Return_Str)
                {
                    return unexpected(unicode_conversion_error(
                        std::distance(begin_str, current_itt),
                        next_char32.error()
                    ));
                }
                else
                {
                    return make_optional(unicode_conversion_error(
                        std::distance(begin_str, current_itt),
                        next_char32.error()
                    ));
                }
            }
        }
        return rv;
    };

    // If input is same as output, all we gotta do is validate the unicode
    // input.
    if constexpr (std::same_as<InputChar, OutputChar>)
    {
        // Covers 1, 7, 13, 19, 25.
        if (auto result = validate_unicode.operator()<false>(sv);
            result.has_value())
        {
            return unexpected(result.value());
        }
        else
        {
            return basic_string<OutputChar>(sv);
        }
    }
    else if constexpr (( is_wchar_and_16_bit_c<InputChar>
                         && same_as<OutputChar, char16_t> )
                       || ( is_wchar_and_32_bit_c<InputChar>
                            && same_as<OutputChar, char32_t> ) )
    {
        // Covers 9, 15
        if (auto result = validate_unicode.operator()<false>(sv);
            result.has_value())
        {
            return unexpected(result.value());
        }
        else
        {
            return cast_wstring_to_unicode_string(sv);
        }
    }
    else if constexpr (( is_wchar_and_16_bit_c<OutputChar>
                         && same_as<InputChar, char16_t> )
                       || ( is_wchar_and_32_bit_c<OutputChar>
                            && same_as<InputChar, char32_t> ) )
    {
        // Covers 17, 23
        if (auto result = validate_unicode.operator()<false>(sv);
            result.has_value())
        {
            return unexpected(result.value());
        }
        else
        {
            return cast_unicode_string_to_wstring(sv);
        }
    }
    else if constexpr (same_as<InputChar, char8_t>)
    {
        // Covers 6, 11, 16, 21
        // Convert to char32_t as we go.
        unicode_conversion_result<u32string> converted_str
            = validate_unicode.operator()<true>(sv);
        // converted_str is validated, so just a straight conversion now.
        if constexpr (same_as<char16_t, OutputChar>)
        {
            return converted_str.transform(
                [&](auto& str_arg)
                {
                    return convert_u32string_to_u16string_function(str_arg);
                }
            );
        }
        else if constexpr (is_wchar_and_16_bit_c<OutputChar>)
        {
            return converted_str.transform(
                [&](auto& str_arg)
                {
                    return cast_unicode_string_to_wstring(
                        convert_u32string_to_u16string_function(str_arg)
                    );
                }
            );
        }
        else if constexpr (is_wchar_and_32_bit_c<OutputChar>)
        {
            return converted_str.transform(
                [](auto& str_arg)
                {
                    return cast_unicode_string_to_wstring(str_arg);
                }
            );
        }
        else
        {
            return converted_str;
        }
    }
    else if constexpr (same_as<char16_t, InputChar>)
    {
        unicode_conversion_result<u32string> converted_str
            = validate_unicode.operator()<true>(sv);
        if constexpr (same_as<char8_t, OutputChar>)
        {
            return converted_str.transform(
                [&](auto& str_arg)
                {
                    return convert_u32string_to_u8string_function(str_arg);
                }
            );
        }
        else if constexpr (is_wchar_and_32_bit_c<OutputChar>)
        {
            return converted_str.transform(
                [](auto& str_arg)
                {
                    return cast_unicode_string_to_wstring(str_arg);
                }
            );
        }
        else
        {
            return converted_str;
        }
    }
    else if constexpr (same_as<char32_t, InputChar>)
    {
        if (auto result = validate_unicode.operator()<false>(sv);
            result.has_value())
        {
            return unexpected(result.value());
        }
        else
        {
            if constexpr (same_as<char8_t, OutputChar>)
            {
                return convert_u32string_to_u8string_function(sv);
            }
            else
            {
                auto as_u16_str{convert_u32string_to_u16string_function(sv)};
                if constexpr (same_as<char16_t, OutputChar>)
                {
                    return as_u16_str;
                }
                else
                {
                    return cast_unicode_string_to_wstring(as_u16_str);
                }
            }
        }
    }
    else if constexpr (is_wchar_and_16_bit_c<InputChar>)
    {
        if (auto result = validate_unicode.operator()<true>(sv);
            result.has_value())
        {
            if constexpr (same_as<char8_t, OutputChar>)
            {
                return convert_u32string_to_u8string_function(result.value());
            }
            else
            {
                return result.value();
            }
            return unexpected(result.error());
        }
        else
        {
            return unexpected(result.error());
        }
    }
    else if constexpr (is_wchar_and_32_bit_c<InputChar>)
    {
        if (auto result = validate_u32_string(sv); result.has_value())
        {
            return unexpected(result.value());
        }
        else
        {
            if constexpr (same_as<char8_t, OutputChar>)
            {
                return convert_u32string_to_u8string_function(sv);
            }
            else
            {
                return sv;
            }
        }
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
        unicode_conversion<OutputChar>(str_arg)
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
        unicode_conversion<OutputChar>(char_arg)
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
        next_char32<true>(iterator_arg, itt_end_arg)
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
        next_char32_and_increment_iterator<true>(iterator_arg, itt_end_arg)
    );
}

template <bool Return_Reason, typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::conditional_t<
    Return_Reason,
    prev_char32_result<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    prev_char32(
        const ItteratorType iterator_arg,
        const ItteratorType itt_start_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using CharT = ItteratorType::value_type;
    return prev_char32_internal_with_iterator_checking<
        Return_Reason,
        ItteratorType,
        CharT>(iterator_arg, iterator_arg, itt_start_arg);
}

template <bool Return_Reason, typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::conditional_t<
    Return_Reason,
    prev_char32_result<char32_t>,
    std::optional<char32_t>>
    prev_char32_and_decrement_iterator(
        ItteratorType&      iterator_arg,
        const ItteratorType itt_start_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using CharT = std::iterator_traits<ItteratorType>::value_type;
    return prev_char32_and_decrement_iterator<
        Return_Reason,
        ItteratorType,
        CharT>(iterator_arg, iterator_arg, itt_start_arg);
}

template <typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr std::pair<char32_t, std::size_t>
    prev_char32_with_exception(
        const ItteratorType iterator_arg,
        const ItteratorType itt_start_arg
    )
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return throw_if_error<pair<char32_t, size_t>, prev_char32_error>(
        prev_char32<true>(iterator_arg, itt_start_arg)
    );
}

template <typename ItteratorType>
requires char_type_is_unicode_c<
    typename std::iterator_traits<ItteratorType>::value_type>
constexpr char32_t
    prev_char32_and_decrement_iterator_with_exception(
        ItteratorType&      iterator_arg,
        const ItteratorType itt_start_arg
    )
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    return throw_if_error<char32_t, prev_char32_error>(
        prev_char32_and_decrement_iterator<true, ItteratorType>(
            iterator_arg, itt_start_arg
        )
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
            if (auto special_char_str{special_char_as_string<char, char8_t>(
                    static_cast<char32_t>(character)
                )};
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
                if (auto special_char_str{
                        special_char_as_string<InputChar, OutputChar>(
                            character_opt.value()
                        )
                    };
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

constexpr basic_unicode_error
    basic_unicode_error_factory::overlong_encoding(
        const std::array<char8_t, 4>& u8_code_points_arg,
        const std::uint8_t            code_points_encountered_arg,
        const char32_t                char32_character_arg
    ) noexcept
{
    return basic_unicode_error(
        basic_unicode_error::basic_unicode_error_code::overlong_encoding,
        u8_code_points_arg,
        char32_character_arg,
        code_points_encountered_arg,
        {u'\0', u'\0'},
        false
    );
}

constexpr basic_unicode_error
    basic_unicode_error_factory::invalid_utf32_code_point_after_utf8_conversion(
        const std::array<char8_t, 4>& u8_code_points_arg,
        const std::uint8_t            code_points_encountered_arg,
        const char32_t                char32_character_arg
    ) noexcept
{
    return basic_unicode_error(
        basic_unicode_error::basic_unicode_error_code::
            invalid_utf32_code_point_after_utf8_conversion,
        u8_code_points_arg,
        char32_character_arg,
        code_points_encountered_arg,
        {u'\0', u'\0'},
        false
    );
}

constexpr basic_unicode_error
    basic_unicode_error_factory::invalid_utf32_code_point(
        const char32_t char32_character_arg,
        const bool     is_wchar_arg
    ) noexcept
{
    return basic_unicode_error(
        basic_unicode_error::basic_unicode_error_code::invalid_utf32_code_point,
        {u8'\0', u8'\0', u8'\0', u8'\0'},
        char32_character_arg,
        0,
        {u'\0', u'\0'},
        is_wchar_arg
    );
}

constexpr forward_scan_unicode_error
    forward_scan_unicode_error_factory::invalid_leading_byte(
        const char8_t code_points_arg
    ) noexcept
{
    return forward_scan_unicode_error(
        forward_scan_unicode_error::forward_scan_unicode_error_code::
            invalid_leading_byte,
        basic_unicode_error(
            basic_unicode_error::basic_unicode_error_code::generic_error,
            {code_points_arg, u8'\0', u8'\0', u8'\0'},
            U'\0',
            0,
            {u'\0', u'\0'},
            false
        )
    );
}

constexpr forward_scan_unicode_error
    forward_scan_unicode_error_factory::truncated_sequence(
        const std::array<char8_t, 4>& code_points_arg,
        const forward_scan_unicode_error::truncated_sequence_sub_error
            truncated_sequence_error_enum_arg
    ) noexcept
{
    return forward_scan_unicode_error(
        forward_scan_unicode_error::forward_scan_unicode_error_code::
            truncated_sequence,
        basic_unicode_error(
            basic_unicode_error::basic_unicode_error_code::generic_error,
            code_points_arg,
            U'\0',
            std::to_underlying(truncated_sequence_error_enum_arg),
            {u'\0', u'\0'},
            false
        )
    );
}

constexpr forward_scan_unicode_error
    forward_scan_unicode_error_factory::invalid_continuation_byte(
        const std::array<char8_t, 4>& u8_code_points_arg,
        const forward_scan_unicode_error::invalid_continuation_byte_sub_error
            sub_error_enum_arg
    ) noexcept
{
    return forward_scan_unicode_error(
        forward_scan_unicode_error::forward_scan_unicode_error_code::
            invalid_continuation_byte,
        basic_unicode_error(
            basic_unicode_error::basic_unicode_error_code::generic_error,
            u8_code_points_arg,
            U'\0',
            std::to_underlying(sub_error_enum_arg),
            {u'\0', u'\0'},
            false
        )
    );
}

constexpr forward_scan_unicode_error
    forward_scan_unicode_error_factory::overlong_encoding(
        const std::array<char8_t, 4>& u8_code_points_arg,
        const std::uint8_t            code_points_encountered_arg,
        const char32_t                char32_character_arg
    ) noexcept
{
    return forward_scan_unicode_error(
        forward_scan_unicode_error::forward_scan_unicode_error_code::
            basic_unicode_error,
        basic_unicode_error_factory::overlong_encoding(
            u8_code_points_arg,
            code_points_encountered_arg,
            char32_character_arg
        )
    );
}

constexpr forward_scan_unicode_error
    forward_scan_unicode_error_factory::
        invalid_utf32_code_point_after_utf8_conversion(
            const std::array<char8_t, 4>& u8_code_points_arg,
            const std::size_t             code_points_encountered_arg,
            const char32_t                char32_character_arg
        ) noexcept
{
    return forward_scan_unicode_error(
        forward_scan_unicode_error::forward_scan_unicode_error_code::
            basic_unicode_error,
        basic_unicode_error_factory::
            invalid_utf32_code_point_after_utf8_conversion(
                u8_code_points_arg,
                code_points_encountered_arg,
                char32_character_arg
            )
    );
}

constexpr forward_scan_unicode_error
    forward_scan_unicode_error_factory::high_surrogate_then_end_of_stream(
        const char16_t char16_character_arg,
        const bool     is_wchar_arg
    ) noexcept
{
    return forward_scan_unicode_error(
        forward_scan_unicode_error::forward_scan_unicode_error_code::
            high_surrogate_then_end_of_stream,
        basic_unicode_error(
            basic_unicode_error::basic_unicode_error_code::generic_error,
            {u8'\0', u8'\0', u8'\0', u8'\0'},
            U'\0',
            0,
            {char16_character_arg, u'\0'},
            is_wchar_arg
        )
    );
}

constexpr forward_scan_unicode_error
    forward_scan_unicode_error_factory::
        high_surrogate_not_followed_by_low_surrogate(
            const char16_t char16_first_char_arg,
            const char16_t char16_second_char_arg,
            const bool     is_wchar_arg
        ) noexcept
{
    return forward_scan_unicode_error(
        forward_scan_unicode_error::forward_scan_unicode_error_code::
            high_surrogate_not_followed_by_low_surrogate,
        basic_unicode_error(
            basic_unicode_error::basic_unicode_error_code::generic_error,
            {u8'\0', u8'\0', u8'\0', u8'\0'},
            U'\0',
            0,
            {char16_first_char_arg, char16_second_char_arg},
            is_wchar_arg
        )
    );
}

constexpr forward_scan_unicode_error
    forward_scan_unicode_error_factory::unexpected_low_surrogate(
        const char16_t char16_character_arg,
        const bool     is_wchar_arg
    ) noexcept
{
    return forward_scan_unicode_error(
        forward_scan_unicode_error::forward_scan_unicode_error_code::
            unexpected_low_surrogate,
        basic_unicode_error(
            basic_unicode_error::basic_unicode_error_code::generic_error,
            {u8'\0', u8'\0', u8'\0', u8'\0'},
            U'\0',
            0,
            {char16_character_arg, u'\0'},
            is_wchar_arg
        )
    );
}

constexpr forward_scan_unicode_error
    forward_scan_unicode_error_factory::unexpected_high_surrogate(
        const char16_t char16_character_arg,
        const bool     is_wchar_arg
    ) noexcept
{
    return forward_scan_unicode_error(
        forward_scan_unicode_error::forward_scan_unicode_error_code::
            unexpected_low_surrogate,
        basic_unicode_error(
            basic_unicode_error::basic_unicode_error_code::generic_error,
            {u8'\0', u8'\0', u8'\0', u8'\0'},
            U'\0',
            0,
            {u'\0', u'\0'},
            is_wchar_arg
        )
    );
}

constexpr forward_scan_unicode_error
    forward_scan_unicode_error_factory::invalid_utf32_code_point(
        const char32_t char32_character_arg,
        const bool     is_wchar_arg
    ) noexcept
{
    return forward_scan_unicode_error(
        forward_scan_unicode_error::forward_scan_unicode_error_code::
            basic_unicode_error,
        basic_unicode_error(
            basic_unicode_error::basic_unicode_error_code::
                invalid_utf32_code_point,
            {u8'\0', u8'\0', u8'\0', u8'\0'},
            char32_character_arg,
            0,
            {u'\0', u'\0'},
            is_wchar_arg
        )
    );
}

constexpr unicode_to_ascii_error
    unicode_to_ascii_error_factory::non_ascii_character_found_from_utf8(
        const char32_t                character_arg,
        const std::array<char8_t, 4>& utf8_chars_arg,
        const std::size_t             index_arg,
        const std::uint8_t            length_of_chars
    ) noexcept
{
    return unicode_to_ascii_error(
        unicode_to_ascii_error::unicode_to_ascii_error_code::
            non_ascii_character_found_from_utf8,
        unicode_conversion_error(
            index_arg,
            forward_scan_unicode_error(
                forward_scan_unicode_error::forward_scan_unicode_error_code::
                    generic_error,
                basic_unicode_error(
                    basic_unicode_error::basic_unicode_error_code::
                        generic_error,
                    utf8_chars_arg,
                    character_arg,
                    length_of_chars,
                    {u'\0', u'\0'},
                    false
                )
            )
        )
    );
}

constexpr unicode_to_ascii_error
    unicode_to_ascii_error_factory::non_ascii_character_found_from_utf16(
        const char32_t                 character_arg,
        const std::array<char16_t, 2>& utf16_chars_arg,
        const std::size_t              index_arg,
        const std::uint8_t             length_of_chars,
        const bool                     is_wchar_arg
    ) noexcept
{
    return unicode_to_ascii_error(
        unicode_to_ascii_error::unicode_to_ascii_error_code::
            non_ascii_character_found_from_utf16,
        unicode_conversion_error(
            index_arg,
            forward_scan_unicode_error(
                forward_scan_unicode_error::forward_scan_unicode_error_code::
                    generic_error,
                basic_unicode_error(
                    basic_unicode_error::basic_unicode_error_code::
                        generic_error,
                    {u8'\0', u8'\0', u8'\0', u8'\0'},
                    character_arg,
                    length_of_chars,
                    utf16_chars_arg,
                    is_wchar_arg
                )
            )
        )
    );
}

constexpr unicode_to_ascii_error
    unicode_to_ascii_error_factory::non_ascii_character_found_from_utf32(
        const char32_t    character_arg,
        const std::size_t index_arg,
        const bool        is_wchar_arg
    ) noexcept
{
    return unicode_to_ascii_error(
        unicode_to_ascii_error::unicode_to_ascii_error_code::
            non_ascii_character_found_from_utf32,
        unicode_conversion_error(
            index_arg,
            forward_scan_unicode_error(
                forward_scan_unicode_error::forward_scan_unicode_error_code::
                    generic_error,
                basic_unicode_error(
                    basic_unicode_error::basic_unicode_error_code::
                        generic_error,
                    {u8'\0', u8'\0', u8'\0', u8'\0'},
                    character_arg,
                    1,
                    {u'\0', u'\0'},
                    is_wchar_arg
                )
            )
        )
    );
}

constexpr unicode_to_ascii_error
    unicode_to_ascii_error_factory::invalid_unicode_character(
        const std::size_t                 index_arg,
        const forward_scan_unicode_error& unicode_error_arg
    ) noexcept
{
    return unicode_to_ascii_error(
        unicode_to_ascii_error::unicode_to_ascii_error_code::
            invalid_unicode_character,
        unicode_conversion_error(index_arg, unicode_error_arg)
    );
}

constexpr next_char32_error
    next_char32_error_factory::mk_forward_error(
        const forward_scan_unicode_error& error_arg
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
        forward_scan_unicode_error(
            forward_scan_unicode_error::forward_scan_unicode_error_code::
                generic_error,
            basic_unicode_error(
                basic_unicode_error::basic_unicode_error_code::generic_error,
                {u8'\0', u8'\0', u8'\0', u8'\0'},
                U'\0',
                std::numeric_limits<std::uint8_t>::max(),
                {u'\0', u'\0'},
                false
            )
        )
    );
}

template <typename ResultT, typename ErrorT>
requires unicode_bridge_error_c<ErrorT>
ResultT
    throw_if_error(
        std::expected<ResultT, ErrorT> result
    )
{
    if (result.has_value())
    {
        return std::move(result.value());
    }
    else
    {
        throw unicode_bridge_exception(result.error());
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

constexpr std::u8string
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
constexpr std::u8string
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
        if (character_as_hex.size() <= 2)
        {
            return_value.append(2 - character_as_hex.size(), u8'0');
        }
        else if (character_as_hex.size() <= 4)
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
constexpr unicode_conversion_result<
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
            forward_scan_for_next_char32<true, decltype(str_iterator), char8_t>(
                str_iterator, str_iterator_end
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
            return unexpected(unicode_conversion_error(
                std::distance(std::begin(str_arg), str_iterator),
                next_char_result.error()
            ));
        }
    }
    return return_value;
}

template <bool Return_u32string, typename Original_Type>
requires char_type_is_unicode_c<Original_Type>
constexpr forward_scan_result_t<
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
        auto next_char_result{forward_scan_for_next_char32<
            true,
            decltype(str_iterator),
            Original_Type>(str_iterator, str_iterator_end)};
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
        auto res{
            forward_scan_for_next_char32<Return_Reason, T, Original_Value_Type>(
                iterator_arg, itt_end_arg
            )
        };
        if (res.has_value())
        {
            return res.value();
        }
        else
        {
            if constexpr (Return_Reason)
            {
                return unexpected(
                    next_char32_error_factory::mk_forward_error(res.error())
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
    prev_char32_result<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    prev_char32_internal_with_iterator_checking(
        const T itt_end_arg,
        const T iterator_arg,
        const T iterator_begin_arg
    ) noexcept
{
    using namespace std;
    using namespace UNICODE_BRIDGE_NAMESPACE_INTERNAL;
    using CharT               = std::iterator_traits<T>::value_type;
    auto is_continuation_byte = [](const char8_t char_arg)
    {
        return (char_arg & 0b1100'0000) == 0b1000'0000;
    };
    auto is_leading_byte = [](const char8_t char_arg)
    {
        return is_valid_ascii(char_arg)
               || (char_arg & 0b1110'0000) == 0b1100'0000  // 2-byte
               || (char_arg & 0b1111'0000) == 0b1110'0000  // 3-byte
               || (char_arg & 0b1111'1000) == 0b1111'0000; // 4-byte
    };
    if (iterator_arg == iterator_begin_arg)
    {
        if constexpr (Return_Reason)
        {
            return unexpected(prev_char32_error_factory::iterator_exhausted());
        }
        else
        {
            return nullopt;
        }
    }
    auto local_iterator = iterator_arg - 1;
    if constexpr (same_as<CharT, char32_t> || is_wchar_and_32_bit_c<CharT>)
    {
        return process_utf32<
            false,
            Return_Reason,
            Original_Value_Type,
            prev_char32_error_factory,
            prev_char32_error>(local_iterator);
    }
    else if constexpr (same_as<CharT, char16_t> || is_wchar_and_16_bit_c<CharT>)
    {
        const T        prev{iterator_arg - 1};
        const char16_t last{static_cast<char16_t>(*prev)};

        if (is_low_surrogate(last))
        {
            if (prev == iterator_begin_arg)
            {
                if constexpr (Return_Reason)
                {
                    return unexpected(
                        prev_char32_error_factory::
                            low_surrogate_then_start_of_stream(last, same_as<Original_Value_Type, wchar_t>)
                    );
                }
                else
                {
                    return nullopt;
                }
            }
            const T        prev2{prev - 1};
            const char16_t first{static_cast<char16_t>(*prev2)};
            if (not is_high_surrogate(first))
            {
                if constexpr (Return_Reason)
                {
                    return unexpected(
                        prev_char32_error_factory::
                            low_surrogate_not_preceded_by_high_surrogate(first, last, same_as<Original_Value_Type, wchar_t>)
                    );
                }
                else
                {
                    return nullopt;
                }
            }
            return make_pair(decode_surrogate_pair(first, last), 2);
        }
        else if (is_high_surrogate(last))
        {
            if constexpr (Return_Reason)
            {
                return unexpected(
                    prev_char32_error_factory::
                        unexpected_high_surrogate(last, same_as<Original_Value_Type, wchar_t>)
                );
            }
            else
            {
                return nullopt;
            }
        }
        else
        {
            return make_pair(static_cast<char32_t>(last), 1);
        }
    }
    else if constexpr (same_as<CharT, char8_t>)
    {
        T      scan{iterator_arg};
        size_t n_continuation{0};

        while (scan != iterator_begin_arg)
        {
            --scan;
            const char8_t byte{*scan};

            if (is_continuation_byte(byte))
            {
                ++n_continuation;
                if (n_continuation > 3)
                {
                    // Scanned 4 continuation bytes with no leading byte found
                    if constexpr (Return_Reason)
                    {
                        std::array<char8_t, 4> code_units
                            = {static_cast<char8_t>(*(iterator_arg - 4)),
                               static_cast<char8_t>(*(iterator_arg - 3)),
                               static_cast<char8_t>(*(iterator_arg - 2)),
                               static_cast<char8_t>(*(iterator_arg - 1))};
                        return unexpected(
                            prev_char32_error_factory::
                                no_valid_leading_byte_found(code_units)
                        );
                    }
                    else
                    {
                        return nullopt;
                    }
                }
            }
            else if (is_valid_ascii(byte) || is_leading_byte(byte))
            {
                const size_t expected
                    = is_valid_ascii(byte)                  ? 0
                      : (byte & 0b1110'0000) == 0b1100'0000 ? 1
                      : (byte & 0b1111'0000) == 0b1110'0000 ? 2
                                                            : 3;

                if (expected != n_continuation)
                {
                    if constexpr (Return_Reason)
                    {
                        std::array<char8_t, 4> code_units
                            = {static_cast<char8_t>(*scan),
                               (n_continuation > 0)
                                   ? static_cast<char8_t>(*(scan + 1))
                                   : u8'\0',
                               (n_continuation > 1)
                                   ? static_cast<char8_t>(*(scan + 2))
                                   : u8'\0',
                               (n_continuation > 2)
                                   ? static_cast<char8_t>(*(scan + 3))
                                   : u8'\0'};
                        return unexpected(
                            prev_char32_error_factory::
                                leading_byte_sequence_length_mismatch(

                                    code_units,
                                    static_cast<uint8_t>(n_continuation)
                                )
                        );
                    }
                    else
                    {
                        return nullopt;
                    }
                }

                // Valid sequence found — decode it
                const size_t sequence_length = n_continuation + 1;
                char32_t     code_point
                    = leading_byte_data_bits(*scan, sequence_length);
                // Accumulate continuation bytes — validity already checked by
                // caller
                T local{scan};
                for (size_t idx{0}; idx < sequence_length - 1; ++idx)
                {
                    ++local;
                    accumulate_continuation_byte(
                        code_point, static_cast<char8_t>(*local)
                    );
                }
                return check_decoded_utf8_codepoint<
                    Return_Reason,
                    Original_Value_Type,
                    prev_char32_error_factory,
                    prev_char32_error>(
                    scan, iterator_begin_arg, local, sequence_length, code_point
                );
            }
            else
            {
                // Non-continuation, non-leading byte encountered mid-scan
                if constexpr (Return_Reason)
                {
                    std::array<char8_t, 4> code_units = {
                        static_cast<char8_t>(*scan),
                        (n_continuation > 0) ? static_cast<char8_t>(*(scan + 1))
                                             : u8'\0',
                        (n_continuation > 1) ? static_cast<char8_t>(*(scan + 2))
                                             : u8'\0',
                        (n_continuation > 2) ? static_cast<char8_t>(*(scan + 3))
                                             : u8'\0'
                    };
                    return unexpected(
                        prev_char32_error_factory::invalid_utf8_byte(

                            code_units, n_continuation
                        )
                    );
                }
                else
                {
                    return nullopt;
                }
            }
        }

        // Reached iterator_begin_arg without finding a leading byte
        if constexpr (Return_Reason)
        {
            std::array<char8_t, 4> code_units
                = {static_cast<char8_t>(*scan),
                   (n_continuation > 1) ? static_cast<char8_t>(*(scan + 1))
                                        : u8'\0',
                   (n_continuation > 2) ? static_cast<char8_t>(*(scan + 2))
                                        : u8'\0',
                   (n_continuation > 3) ? static_cast<char8_t>(*(scan + 3))
                                        : u8'\0'};
            return unexpected(prev_char32_error_factory::
                                  iterator_begin_reached_before_leading_byte(
                                      code_units, n_continuation
                                  ));
        }
        else
        {
            return nullopt;
        }
    }
    else
    {
        UNICODE_BRIDGE_STATIC_ASSERT(
            CharT,
            "prev_char32_internal_with_iterator_checking invalid for this "
            "character type"
        );
    }
}

template <bool Return_Reason, typename T, typename Original_Value_Type>
requires char_type_is_unicode_c<typename std::iterator_traits<T>::value_type>
         && char_type_is_unicode_c<Original_Value_Type>
constexpr std::conditional_t<
    Return_Reason,
    forward_scan_result_t<std::pair<char32_t, std::size_t>>,
    std::optional<std::pair<char32_t, std::size_t>>>
    forward_scan_for_next_char32(
        const T iterator_arg,
        const T itt_end_arg
    ) noexcept
{
    using namespace std;
    using CharT = std::iterator_traits<T>::value_type;
    using rv_t  = conditional_t<
         Return_Reason,
         forward_scan_result_t<pair<char32_t, size_t>>,
         optional<pair<char32_t, size_t>>>;
    auto next_from_u8string_function
        = [&](const T iterator_arg, const T itt_end_arg) -> rv_t
    {
        auto       local_iterator{iterator_arg};
        const auto byte_1{*local_iterator};
        auto       is_continuation_byte = [](const char8_t char_arg)
        {
            return (char_arg & 0b1100'0000) != 0b1000'0000;
        };
        if (not (is_valid_ascii(byte_1)))
        {
            size_t code_point_size = utf8_sequence_length(byte_1);
            if (code_point_size == 0)
            {
                if constexpr (Return_Reason)
                {
                    return unexpected(forward_scan_unicode_error_factory::
                                          invalid_leading_byte(byte_1));
                }
                else
                {
                    return nullopt;
                }
            }
            char32_t code_point
                = leading_byte_data_bits(byte_1, code_point_size);
            const size_t code_units_processed_remaining{
                static_cast<size_t>(std::distance(local_iterator, itt_end_arg))
            };
            if (code_units_processed_remaining <= code_point_size - 1)
            {
                if constexpr (Return_Reason)
                {
                    static constexpr uint8_t sub_error_offsets[] = {0, 1, 3};
                    const auto               sub_error
                        = static_cast<forward_scan_unicode_error::
                                          truncated_sequence_sub_error>(
                            sub_error_offsets[code_point_size - 2]
                            + (code_units_processed_remaining - 1)
                        );
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
                        forward_scan_unicode_error_factory::truncated_sequence(
                            code_units, sub_error
                        )
                    );
                }
                else
                {
                    return nullopt;
                }
            }
            for (size_t idx{0}; idx < code_point_size - 1; ++idx)
            {
                ++local_iterator;
                const auto byte_n{*local_iterator};
                if (is_continuation_byte(byte_n))
                {
                    if constexpr (Return_Reason)
                    {
                        std::array<char8_t, 4> code_units{
                            static_cast<char8_t>(*iterator_arg),
                            static_cast<char8_t>(*(iterator_arg + 1)),
                            (code_point_size > 2)
                                ? static_cast<char8_t>(*(iterator_arg + 2))
                                : u8'\0',
                            (code_point_size > 3)
                                ? static_cast<char8_t>(*(iterator_arg + 3))
                                : u8'\0',
                        };
                        using enum forward_scan_unicode_error::
                            invalid_continuation_byte_sub_error;
                        forward_scan_unicode_error::
                            invalid_continuation_byte_sub_error sub_error;
                        switch (code_point_size)
                        {
                        case 2:
                            sub_error = size_2_invalid_indexes_1;
                            break;
                        case 3:
                            sub_error = (idx == 0)
                                            ? (is_continuation_byte(
                                                   *(local_iterator + 1)
                                               )
                                                   ? size_3_invalid_indexes_1_2
                                                   : size_3_invalid_indexes_1)
                                            : size_3_invalid_indexes_2;
                            break;
                        case 4:
                            switch (idx)
                            {
                            case 0:
                            {
                                const bool third_invalid
                                    = is_continuation_byte(*(local_iterator + 1)
                                    );
                                const bool fourth_invalid
                                    = is_continuation_byte(*(local_iterator + 2)
                                    );
                                if (third_invalid && fourth_invalid)
                                {
                                    sub_error = size_4_invalid_indexes_1_2_3;
                                }
                                else if (third_invalid)
                                {
                                    sub_error = size_4_invalid_indexes_1_2;
                                }
                                else if (fourth_invalid)
                                {
                                    sub_error = size_4_invalid_indexes_1_3;
                                }
                                else
                                {
                                    sub_error = size_4_invalid_indexes_1;
                                }
                            }
                            break;
                            case 1:
                                sub_error
                                    = is_continuation_byte(*(local_iterator + 1)
                                      )
                                          ? size_4_invalid_indexes_2_3
                                          : size_4_invalid_indexes_2;
                                break;
                            default:
                                sub_error = size_4_invalid_indexes_3;
                            }
                            break;
                        }
                        return unexpected(
                            forward_scan_unicode_error_factory::
                                invalid_continuation_byte(code_units, sub_error)
                        );
                    }
                    else
                    {
                        return nullopt;
                    }
                }
                accumulate_continuation_byte(code_point, byte_n);
            }
            return check_decoded_utf8_codepoint<
                Return_Reason,
                Original_Value_Type,
                forward_scan_unicode_error_factory,
                forward_scan_unicode_error>(
                iterator_arg,
                local_iterator,
                local_iterator,
                code_point_size,
                code_point
            );
        }
        else
        {
            return rv_t(make_pair(static_cast<char32_t>(byte_1), 1));
        }
    };
    auto next_from_u16_string
        = [&](const T iterator_arg, const T itt_end_arg) -> rv_t
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
                        forward_scan_unicode_error_factory::
                            high_surrogate_then_end_of_stream(first_element, same_as<Original_Value_Type, wchar_t>)
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
                        forward_scan_unicode_error_factory::
                            high_surrogate_not_followed_by_low_surrogate(first_element, second_element, std::same_as<Original_Value_Type, wchar_t>)
                    );
                }
                else
                {
                    return nullopt;
                }
            }
            return rv_t(make_pair(
                decode_surrogate_pair(first_element, second_element), 2
            ));
        }
        else if (is_low_surrogate(first_element))
        {
            // Unpaired low surrogate
            if constexpr (Return_Reason)
            {
                return unexpected(
                    forward_scan_unicode_error_factory::
                        unexpected_low_surrogate(first_element, same_as<Original_Value_Type, wchar_t>)
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
        return next_from_u8string_function(iterator_arg, itt_end_arg);
    }
    else if constexpr (same_as<CharT, char16_t> || is_wchar_and_16_bit_c<CharT>)
    {
        return next_from_u16_string(iterator_arg, itt_end_arg);
    }
    else if constexpr (same_as<CharT, char32_t> || is_wchar_and_32_bit_c<CharT>)
    {
        return process_utf32<
            true,
            Return_Reason,
            Original_Value_Type,
            forward_scan_unicode_error_factory,
            forward_scan_unicode_error>(iterator_arg);
        const CharT character{*iterator_arg};
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

template <bool Return_Reason, typename T, typename Original_Type>
requires char_type_is_unicode_c<typename std::iterator_traits<T>::value_type>
         && char_type_is_unicode_c<Original_Type>
constexpr std::conditional_t<
    Return_Reason,
    prev_char32_result<char32_t>,
    std::optional<char32_t>>
    prev_char32_and_decrement_iterator(
        const T itt_end_arg,
        T&      iterator_arg,
        const T iterator_begin_arg
    ) noexcept
{
    using namespace std;
    auto prev_char32_t_result{prev_char32_internal_with_iterator_checking<
        Return_Reason,
        T,
        Original_Type>(itt_end_arg, iterator_arg, iterator_begin_arg)};
    // If its a valid unicode character.
    if (prev_char32_t_result.has_value())
    {
        const auto& [character, char_size]{prev_char32_t_result.value()};
        iterator_arg = std::prev(iterator_arg, char_size);
        return conditional_t<
            Return_Reason,
            prev_char32_result<char32_t>,
            std::optional<char32_t>>(character);
    }
    else
    {
        if constexpr (Return_Reason)
        {
            return unexpected(prev_char32_t_result.error());
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

template <typename InputChar, typename OutputChar>
constexpr std::optional<std::basic_string<OutputChar>>
    special_char_as_string(
        const char32_t char_arg
    ) noexcept
{
    using namespace std;
    auto micro_conversion_func
        = [](const u8string_view sv) -> basic_string<OutputChar>
    {
        return basic_string<OutputChar>(sv.begin(), sv.end());
    };
    switch (char_arg)
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
    if (char_arg < 0x20 || (char_arg >= 0x7F && char_arg <= 0x9F))
    {
        if (char_arg <= 0xFFFF)
        {
            u8string prefix;
            if constexpr (same_as<char8_t, InputChar>
                          || same_as<char, InputChar>)
            {
                prefix = u8"\\x";
            }
            else if constexpr (same_as<char16_t, InputChar>
                               || ( wchar_is_16_bit
                                    && same_as<wchar_t, InputChar> ) )
            {
                if (char_arg <= 0xFF)
                {
                    prefix = u8"\\x";
                }
                else
                {
                    prefix = u8"\\u";
                }
            }
            else if constexpr (same_as<char32_t, InputChar>
                               || ( wchar_is_32_bit
                                    && same_as<wchar_t, InputChar> ) )
            {
                if (char_arg <= 0xFF)
                {
                    prefix = u8"\\x";
                }
                else if (char_arg <= 0xFFFF)
                {
                    prefix = u8"\\u";
                }
                else
                {
                    prefix = u8"\\U";
                }
            }
            else
            {
            }
            return micro_conversion_func(
                UNICODE_BRIDGE_NAMESPACE_INTERNAL::
                    make_hex_from_char_with_prefix<InputChar, true, true>(
                        static_cast<InputChar>(char_arg), prefix
                    )
            );
            // return micro_conversion_func(represent_char_as_hex_for_printing(
            //     static_cast<char16_t>(char_arg)
            // ));
        }
        else
        {
            u8string prefix;
            if constexpr (same_as<char8_t, InputChar>
                          || same_as<char, InputChar>)
            {
                prefix = u8"\\x";
            }
            else if constexpr (same_as<char16_t, InputChar>
                               || ( wchar_is_16_bit
                                    && same_as<wchar_t, InputChar> ) )
            {
                prefix = u8"\\u";
            }
            else if constexpr (same_as<char32_t, InputChar>
                               || ( wchar_is_32_bit
                                    && same_as<wchar_t, InputChar> ) )
            {
                prefix = u8"\\U";
            }
            else
            {
            }
            return micro_conversion_func(
                UNICODE_BRIDGE_NAMESPACE_INTERNAL::
                    make_hex_from_char_with_prefix<char32_t, true, false>(
                        static_cast<InputChar>(char_arg), prefix
                    )
            );
        }
    }
    else
    {
        if constexpr (same_as<char, InputChar>)
        {
            u8string prefix;
            if constexpr (same_as<char8_t, InputChar>
                          || same_as<char, InputChar>)
            {
                prefix = u8"\\x";
            }
            else if constexpr (same_as<char16_t, InputChar>
                               || ( wchar_is_16_bit
                                    && same_as<wchar_t, InputChar> ) )
            {
                prefix = u8"\\u";
            }
            else if constexpr (same_as<char32_t, InputChar>
                               || ( wchar_is_32_bit
                                    && same_as<wchar_t, InputChar> ) )
            {
                prefix = u8"\\U";
            }
            else
            {
            }
            return char_arg > ascii_limit<char32_t>()
                       ? make_optional(micro_conversion_func(
                             UNICODE_BRIDGE_NAMESPACE_INTERNAL::
                                 make_hex_from_char_with_prefix<
                                     InputChar,
                                     true,
                                     false>(
                                     static_cast<InputChar>(char_arg), prefix
                                 )
                         ))
                       : nullopt;
        }
        else
        {
            constexpr std::array<std::pair<char32_t, std::u8string_view>, 36>
                special_unicode_chars = {
                    make_pair(
                        char32_t{0x00A0}, u8"\\xA0"
                    ), // non-breaking space
                    make_pair(
                        char32_t{0x1680}, u8"\\u1680"
                    ), // ogham space mark
                    make_pair(
                        char32_t{0x180E}, u8"\\u180E"
                    ), // mongolian vowel separator
                    make_pair(char32_t{0x2000}, u8"\\u2000"), // en quad
                    make_pair(char32_t{0x2001}, u8"\\u2001"), // em quad
                    make_pair(char32_t{0x2002}, u8"\\u2002"), // en space
                    make_pair(char32_t{0x2003}, u8"\\u2003"), // em space
                    make_pair(
                        char32_t{0x2004}, u8"\\u2004"
                    ), // three-per-em space
                    make_pair(
                        char32_t{0x2005}, u8"\\u2005"
                    ), // four-per-em space
                    make_pair(
                        char32_t{0x2006}, u8"\\u2006"
                    ), // six-per-em space
                    make_pair(char32_t{0x2007}, u8"\\u2007"), // figure space
                    make_pair(
                        char32_t{0x2008}, u8"\\u2008"
                    ), // punctuation space
                    make_pair(char32_t{0x2009}, u8"\\u2009"), // thin space
                    make_pair(char32_t{0x200A}, u8"\\u200A"), // hair space
                    make_pair(
                        char32_t{0x200B}, u8"\\u200B"
                    ), // zero width space
                    make_pair(
                        char32_t{0x200C}, u8"\\u200C"
                    ), // zero width non-joiner
                    make_pair(
                        char32_t{0x200D}, u8"\\u200D"
                    ), // zero width joiner
                    make_pair(char32_t{0x2028}, u8"\\u2028"), // line separator
                    make_pair(
                        char32_t{0x2029}, u8"\\u2029"
                    ), // paragraph separator
                    make_pair(
                        char32_t{0x202F}, u8"\\u202F"
                    ), // narrow no-break space
                    make_pair(
                        char32_t{0x205F}, u8"\\u205F"
                    ), // medium mathematical space
                    make_pair(char32_t{0x2060}, u8"\\u2060"), // word joiner
                    make_pair(
                        char32_t{0x2061}, u8"\\u2061"
                    ), // function application
                    make_pair(char32_t{0x2062}, u8"\\u2062"), // invisible times
                    make_pair(
                        char32_t{0x2063}, u8"\\u2063"
                    ), // invisible separator
                    make_pair(char32_t{0x2064}, u8"\\u2064"), // invisible plus
                    make_pair(
                        char32_t{0x2066}, u8"\\u2066"
                    ), // left-to-right isolate
                    make_pair(
                        char32_t{0x2067}, u8"\\u2067"
                    ), // right-to-left isolate
                    make_pair(
                        char32_t{0x2068}, u8"\\u2068"
                    ), // first strong isolate
                    make_pair(
                        char32_t{0x2069}, u8"\\u2069"
                    ), // pop directional isolate
                    make_pair(
                        char32_t{0x3000}, u8"\\u3000"
                    ), // ideographic space
                    make_pair(char32_t{0xFEFF}, u8"\\uFEFF"), // byte order mark
                    make_pair(
                        char32_t{0xFFF9}, u8"\\uFFF9"
                    ), // interlinear annotation anchor
                    make_pair(
                        char32_t{0xFFFA}, u8"\\uFFFA"
                    ), // interlinear annotation separator
                    make_pair(
                        char32_t{0xFFFB}, u8"\\uFFFB"
                    ), // interlinear annotation terminator
                    make_pair(
                        char32_t{0xFFFD}, u8"\\uFFFD"
                    ), // replacement character
                };
            auto it = std::ranges::lower_bound(
                special_unicode_chars,
                static_cast<char32_t>(char_arg),
                {},
                &std::pair<char32_t, u8string_view>::first
            );

            return (it != special_unicode_chars.end()
                    && it->first == static_cast<char32_t>(char_arg))
                       ? std::make_optional(micro_conversion_func(it->second))
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
constexpr std::wstring
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
            "T that is not equal in size to wchar_t"
        );
    }
}

UNICODE_BRIDGE_INTERNAL_NS_END

UNICODE_BRIDGE_NS_END
