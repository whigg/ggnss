#include <iostream>
#include "navrnx.hpp"
#include "ggdatetime/datetime_write.hpp"

using ngpt::NavigationRnx;
using ngpt::NavDataFrame;
using ngpt::SATELLITE_SYSTEM;

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

  // rewind to end of header; read only BDS and Galileo
  /*
  nav.rewind();
  j = 0;
  while (!j) {
    auto system = nav.peak_satsys(j);
    if (!j) {
      if (system == SATELLITE_SYSTEM::galileo || system == SATELLITE_SYSTEM::beidou) {
        j=nav.read_next_record(block);
      } else {
        j=nav.ignore_next_block();
      }
    }
  }
  std::cout<<"\nLast status was: "<<j;
  */
  
  // rewind to end of header; read only GPS
  /*
  nav.rewind();
  j = 0;
  while (!j) {
    auto system = nav.peak_satsys(j);
    if (!j) {
      if (system == SATELLITE_SYSTEM::gps) {
        j=nav.read_next_record(block);
        std::cout<<"\n> IODE: "<<block.data(3)
          <<", ToE: "<<block.data(11)
          <<", GWk: "<<block.data(21)
          <<", IODC:"<<block.data(26)
          <<", ToM: "<<block.data(27) << "\n";
      } else {
        j=nav.ignore_next_block();
      }
    }
  }
  */
  
  // rewind to end of header; read only Glonass
  nav.rewind();
  j = 0;
  while (!j) {
    auto system = nav.peak_satsys(j);
    if (!j) {
      if (system == SATELLITE_SYSTEM::glonass) {
        j=nav.read_next_record(block);
        if (block.prn() == 3) {
          std::cout<<"\nSatellite "<<block.prn()<<", Time: "<<ngpt::strftime_ymd_hms(block.toc());
          std::cout<<"\n\tMessage frame time: "<<block.data(2)<<", freq. number: "<<block.data(10);
        }
      } else {
        j=nav.ignore_next_block();
      }
    }
  }
  std::cout<<"\nLast status was: "<<j;

  std::cout<<"\n";
}
