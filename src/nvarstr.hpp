#ifndef __NTUA_VARIOUS_STR_FUNCTIONS_HPP__
#define __NTUA_VARIOUS_STR_FUNCTIONS_HPP__

#include <string>

namespace ngpt {

namespace str_algorithm_details {

struct StrSearchPolicyEnd {
  /// @brief Righ-trim whitespaces from given string
  /// The function will search for the first whitespace character in the given
  /// string, STARTING FROM THE END of the passed-in string; the position of
  /// this character is stored at end; then it will return the std::string from
  /// position 0 (aka str) untill the marked position (aka the string from str
  /// to str+end).
  /// @param[in]  str  The (C-) string to rtrim
  /// @apram[out] end  Index of the first whitespace character
  /// @param[in]  stop If specified (aka not equal to size_t(-1)) then the
  ///                  inpit string is considered only up to stop index, aka
  ///                  input string spans [0, stop)
  /// @return An std::string starting at index 0 and ending at position end
  /// (exclusive)
  /// Examples:
  static std::string rtrim(const char *str, std::size_t &end,
                           std::size_t stop) noexcept;
}; // StrSearchPolicyEnd

struct StrSearchPolicyStart {
  /// @brief Righ-trim whitespaces from given string
  /// The function will search for the first whitespace character in the given
  /// string; the position of this character is stored at end; then it will
  /// return the std::string from position 0 (aka str) untill the marked
  /// position (aka the string from str to str+end)
  /// @param[in]  str  The (C-) string to rtrim
  /// @apram[out] end  Index of the first whitespace character
  /// @param[in]  stop If specified (aka not equal to size_t(-1)) then the
  ///                  inpit string is considered only up to stop index, aka
  ///                  input string spans [0, stop)
  /// @return An std::string starting at index 0 and ending at position end
  /// (exclusive)
  /// Examples:
  static std::string rtrim(const char *str, std::size_t &end,
                           std::size_t stop) noexcept;
}; // StrSearchPolicyStart

} // namespace str_algorithm_details

/// @brief Righ-trim whitespaces from given string
/// @param[in]  str  The (C-) string to rtrim
/// @apram[out] end  Index of the first whitespace character
/// @param[in]  stop If specified (aka not equal to size_t(-1)) then the
///                  inpit string is considered only up to stop index, aka
///                  input string spans [0, stop)
/// @return An std::string starting at index 0 and ending at position end
template <typename T = str_algorithm_details::StrSearchPolicyStart>
std::string rtrim(const char *str, std::size_t &pos,
                  std::size_t stop = -1) noexcept {
  return T::rtrim(str, pos, stop);
}

/// @brief Left-trim whitespaces from given string
/// The function will search for the first non-whitespace character in the given
/// string; the position of this character is stored at start; then it will
/// return the std::string from position start (aka str[start]) untill the end
/// of the given string.
/// @param[in] str    The (C-) string to rtrim
/// @apram[out] start Index of the first non-whitespace character
/// @return An std::string starting at index start and ending at the end of the
///         input string str
/// (exclusive)
/// Examples:
std::string ltrim(const char *str, std::size_t &start,
                  std::size_t stop = -1) noexcept;

/// @details Resolve a string of N doubles, written with M digits (i.e. in the
///          format N*DM.x as in RINEX 3.x) and asigne them to data[0,N).
/// @param[in]  line  A c-string containing N doubles written with M digits
/// @param[out] data  An array of (at least) N-1 elements; the resolved doubles
///                   will be written in data[0]...data[N-1]
/// @return  True if all numbers were resolved and assigned; false otherwise
/// @warning The function will check 'errno'. Be sure that it is 0 at input. If
///          an error occurs in the function call, it may be set to !=0, so be
///          sure to clear it (aka 'errno') after exit.
template <int N, int M>
inline bool __char2double__(const char *line, double *data) noexcept {
  char *end;
  for (int i = 0; i < N; i++) {
    data[i] = std::strtod(line, &end);
    if (line == end)
      return false;
    line += M;
  }
  return errno ? false : true;
}

/// @details Resolve a string of N doubles, written with M digits (i.e. in the
///          format N*D19.x as in RINEX 3.x) and asigne them to data[0,N).
/// @param[in]  line  A c-string containing N doubles written with M digits
/// @param[out] data  An array of (at least) N-1 elements; the resolved doubles
///                   will be written in data[0]...data[N-1]
/// @return  True if all numbers were resolved and assigned; false otherwise
/// @warning The function will check 'errno'. Be sure that it is 0 at input. If
///          an error occurs in the function call, it may be set to !=0, so be
///          sure to clear it (aka 'errno') after exit.
/// @note exactly the same as the template version, only we don't know how many
/// doubles we are resolving at compile time.
template <int M>
inline bool __char2double__(const char *line, double *data, int N) noexcept {
  char *end;
  for (int i = 0; i < N; i++) {
    data[i] = std::strtod(line, &end);
    if (line == end)
      return false;
    line += M;
  }
  return errno ? false : true;
}

/// @brief Replace all occurancies of 'D' or 'd' with 'E' in given c-string.
/// @param[in] line A c-string; the replacement will happen 'in-place' so at
///                 exit the string may be different.
void __for2cpp__(char *line) noexcept;

/// @brief Check if given string is empty (aka all whitespaces)
/// @param[in]  str  The input string
/// @param[in]  stop If specified (aka not equal to size_t(-1)) then the
///                  inpit string is considered only up to stop index, aka
///                  input string spans [0, stop)
bool string_is_empty(const char *str, std::size_t stop = -1) noexcept;

} // namespace ngpt

#endif
