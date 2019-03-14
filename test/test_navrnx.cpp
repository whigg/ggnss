#include <iostream>
#include "navrnx.hpp"

using ngpt::NavigationRnx;
using ngpt::NavDataFrame;

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr<<"\n[ERROR] Run as: $>testNavRnx [Nav. RINEX]\n";
    return 1;
  }

  NavigationRnx nav(argv[1]);
  NavDataFrame  block;
  int j = 0, block_nr = 0;
  while (!j) {
    j = nav.read_next_record(block);
    block_nr++;
  }
  std::cout<<"\nRead "<<block_nr<<" data blocks";
  std::cout<<"\nLast status was: "<<j;

  std::cout<<"\n";
}
