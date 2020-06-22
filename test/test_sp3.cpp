#include <iostream>
#include <map>
#include "sp3c.hpp"
#include "ggdatetime/datetime_write.hpp"

using ngpt::Sp3c;

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr<<"\n[ERROR] Run as: $>testSp3 [Sp3c|d]\n";
    return 1;
  }

  // read header and print info
  Sp3c sp3(argv[1]);
  sp3.print_members();

  std::cout<<"\n";
  return 0;
}
