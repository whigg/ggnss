#include <iostream>
#include "navrnx.hpp"

using ngpt::NavigationRnx;

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr<<"\n[ERROR] Run as: $>testNavRnx [Nav. RINEX]\n";
    return 1;
  }

  NavigationRnx nav(argv[1]);
  std::cout<<"\n";
}
