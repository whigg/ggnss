#include <algorithm>
#include <cstring>
#include "nvarstr.hpp"


std::string
ngpt::str_algorithm_details::StrSearchPolicyEnd::rtrim(const char* str, 
  std::size_t& end, std::size_t stop)
noexcept
{
  std::size_t len = std::strlen(str);
  if (stop!=(std::size_t)-1 && stop<len) len = stop;
  // quick return
  if (!len) {
    end=0;
    return std::string("");
  }
  int it = len-1;
  while (it>=0 && str[it]==' ') --it;
  end = ++it;
  return std::string(str, end);
}

std::string
ngpt::str_algorithm_details::StrSearchPolicyStart::rtrim(const char* str, 
  std::size_t& end, std::size_t stop) noexcept
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

std::string
ngpt::ltrim(const char* str, std::size_t& start, std::size_t stop) noexcept
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

void
ngpt::__for2cpp__(char* line) noexcept
{
  char* c = line;
  while (*c) {
    if (*c == 'D' || *c == 'd') *c = 'E';
    ++c;
  }
  return;
}

bool
ngpt::string_is_empty(const char* str, std::size_t stop) noexcept
{
  std::size_t len = std::strlen(str);
  if (stop!=(std::size_t)-1 && stop<len) len = stop;
  for (std::size_t i=0; i<len; i++) if (*(str+i)!=' ') return false;
  return true;
}
