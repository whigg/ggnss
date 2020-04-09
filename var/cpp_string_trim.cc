#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cassert>

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
  static std::string
  rtrim(const char* str, std::size_t& end, std::size_t stop) noexcept
  {
    std::size_t len = std::strlen(str);
    if (stop!=(std::size_t)-1 && stop<len) len = stop;
    // quick return
    if (!len) {
      end=0;
      return std::string("");
    }
    auto it = len-1;
    while (it>=0 && str[it]==' ') --it;
    end = ++it;
    return std::string(str, end);
  }
};

struct StrSearchPolicyStart {
  /// @brief Righ-trim whitespaces from given string
  /// The function will search for the first whitespace character in the given
  /// string; the position of this character is stored at end; then it will
  /// return the std::string from position 0 (aka str) untill the marked position
  /// (aka the string from str to str+end)
  /// @param[in]  str  The (C-) string to rtrim
  /// @apram[out] end  Index of the first whitespace character
  /// @param[in]  stop If specified (aka not equal to size_t(-1)) then the
  ///                  inpit string is considered only up to stop index, aka
  ///                  input string spans [0, stop)
  /// @return An std::string starting at index 0 and ending at position end 
  /// (exclusive)
  /// Examples:
  static std::string
  rtrim(const char* str, std::size_t& end, std::size_t stop) noexcept
  {
    std::size_t len = std::strlen(str);
    if (stop!=(std::size_t)-1 && stop<len) len = stop;
    // quick return
    if (!len) {
      end=0;
      return std::string("");
    }

    /* remember, it is const char* */
    auto it = std::find(str, str+len, ' ');
    end = std::distance(str, it);
    return std::string(str, end);
  }
};
}

template<typename T=str_algorithm_details::StrSearchPolicyStart>
  std::string 
  rtrim(const char* str, std::size_t& pos, std::size_t stop=-1) noexcept
{ return T::rtrim(str, pos, stop); }

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
std::string
ltrim(const char* str, std::size_t& start, std::size_t stop=-1) noexcept
{
  std::size_t len = std::strlen(str);
  if (stop!=(std::size_t)-1 && stop<len) len = stop;
  // quick return
  if (!len) {
    start=0;
    return std::string("");
  }
  /* remember, it is const char* */
  auto it = std::find_if_not(str, str+len, [](const char c1){return c1==' ';});
  start = std::distance(str, it);
  return std::string(str+start, len-start);
}

int main()
{
  typedef str_algorithm_details::StrSearchPolicyEnd END;
  typedef str_algorithm_details::StrSearchPolicyStart START;

  std::vector<const char*> vec = {
    "foobar         ",
    "foobar foo     ",
    "foobar      foo",
    "  foobar       ",
    "f foobar       ",
    "         foobar"
  };
  std::vector<const char*> vec2 = {
    "foobar         COMMENT",
    "foobar foo     COMMENT",
    "foobar      fooCOMMENT",
    "  foobar       COMMENT",
    "f foobar       COMMENT",
    "         foobarCOMMENT"
  };
  std::vector<const char*> vec3 = {
    "FOOfoobar         COMMENT",
    "BARfoobar foo     COMMENT",
    "BAZfoobar      fooCOMMENT",
    "FOO  foobar       COMMENT",
    "BARf foobar       COMMENT",
    "BAZ         foobarCOMMENT"
  };
  std::vector<std::string> res1s, res2s, res3s;
  std::vector<std::string> res1e, res2e, res3e;
  std::size_t OFF = 3;

  std::cout<<"\nAll original strings have a size of "<<std::strlen(vec[0])<<" chars.";

  std::size_t val;
  for (const auto str : vec) {
    auto s1 = rtrim<START>(str, val);
    std::cout<<"\nOriginal string: \""<<str<<"\""
      <<"\n\t(Policy = Start) rtrim: \""<<s1<<"\" index at: "<<val<<" aka \'"<<str[val]<<"\'";
    // and without using the std::string, the c-string is:
    std::cout<<" validate: \""<<std::string(str, val)<<"\" size of new string: "<<val;
    assert(s1==std::string(str, val));
    res1s.push_back(s1);
    
    // rtrim from right!
    s1 = rtrim<END>(str, val);
    std::cout<<"\n\t(Policy = End) rtrim:   \""<<s1<<"\" index at: "<<val<<" aka \'"<<str[val]<<"\'";
    // and without using the std::string, the c-string is:
    std::cout<<" validate: \""<<std::string(str, val)<<"\" size of new string: "<<val;
    assert(s1==std::string(str, val));
    res1e.push_back(s1);
  }
  
  for (const auto str : vec2) {
    auto s1 = rtrim<START>(str, val, 15);
    std::cout<<"\nOriginal string: \""<<str<<"\""
      <<"\n\t(Policy = Start) rtrim: \""<<s1<<"\" index at: "<<val<<" aka \'"<<str[val]<<"\'";
    // and without using the std::string, the c-string is:
    std::cout<<" validate: \""<<std::string(str, val)<<"\" size of new string: "<<val;
    assert(s1==std::string(str, val));
    res2s.push_back(s1);
    
    // rtrim from right!
    s1 = rtrim<END>(str, val, 15);
    std::cout<<"\n\t(Policy = End) rtrim: \""<<s1<<"\" index at: "<<val<<" aka \'"<<str[val]<<"\'";
    // and without using the std::string, the c-string is:
    std::cout<<" validate: \""<<std::string(str, val)<<"\" size of new string: "<<val;
    assert(s1==std::string(str, val));
    res2e.push_back(s1);
  }
  
  for (const auto str : vec3) {
    auto s1 = rtrim<START>(str+OFF, val, 15);
    std::cout<<"\nOriginal string: \""<<str<<"\""
      <<"\n\t(Policy = Start) rtrim: \""<<s1<<"\" index at: "<<val+OFF<<" aka \'"<<str[val+OFF]<<"\'";
    // and without using the std::string, the c-string is:
    std::cout<<" validate: \""<<std::string(str+OFF, val)<<"\" size of new string: "<<val;
    assert(s1==std::string(str+OFF, val));
    res3s.push_back(s1);
    
    // rtrim from right!
    s1 = rtrim<END>(str+OFF, val, 15);
    std::cout<<"\n\t(Policy = End) rtrim: \""<<s1<<"\" index at: "<<val<<" aka \'"<<str[val+OFF]<<"\'";
    // and without using the std::string, the c-string is:
    std::cout<<" validate: \""<<std::string(str+OFF, val)<<"\" size of new string: "<<val;
    assert(s1==std::string(str+OFF, val));
    res3e.push_back(s1);
  }

  // validate all results for vec 1,2 and 3 are the same
  for (int i=0; i<vec2.size(); i++) {
    assert(res1s[i] == res2s[i]);
    assert(res2s[i] == res3s[i]);
    assert(res1e[i] == res2e[i]);
    assert(res2e[i] == res3e[i]);
  }

  /*
  for (const auto str : vec) {
    auto s2 = ltrim(str, val);
    std::cout<<"\n\tltrim: \""<<s2<<"\" index at: "<<val<<" aka \'"<<str[val]<<"\'";
    // and without using the std::string, the c-string is:
    std::cout<<" validate: \""<<std::string(str+val, std::strlen(str)-val)<<"\""
      <<" size of new string: "<<std::strlen(str)-val;
  }
  */

  std::cout<<"\n";
  return 0;
}
