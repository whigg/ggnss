#include <iostream>
#include <cstdlib>
#include <vector>

int
str2int(const char* str, bool& status) noexcept
{
  char* end;
  status = true;
  int i = std::strtol(str, &end, 10);
  if (errno || str==end) status = false;
  return i;
}

int main() 
{
  std::vector<const char*> vec {
    "123456",
    " 123456",
    "  123456  ",
    "a123456",
    "123456a",
    "12a456",
    "1a23456",
    ".123456",
    "12.3456",
    "12,3456"
  };

  bool status;
  for (const auto& str : vec) {
    std::cout<<"\nString: \""<<str<<"\"";
    auto i = str2int(str, status);
    if (!status) {
      std::cout<<" --> Conversion failed!";
    } else {
      std::cout<<" --> "<<i;
    }
  }

  std::cout<<"\n";
  return 0;
}
