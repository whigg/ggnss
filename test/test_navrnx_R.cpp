#include <iostream>
#include "navrnx.hpp"
#include "ggdatetime/datetime_write.hpp"

using ngpt::NavigationRnx;
using ngpt::NavDataFrame;
using ngpt::SATELLITE_SYSTEM;
using ngpt::seconds;
using ngpt::gps_week;

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

  // rewind to end of header; read only GLONASS
  nav.rewind();
  j = 0;
  // rewind to end of header;
  // compute ECEF coordinates for every 15 min for GLO/PRN=3
  nav.rewind();
  j = 0;
  NavDataFrame navar; // holds current data frame
  int PRN=2;
  // read first frame of GLO/PRN=3
  int frame_read=0;
  while (!frame_read) {
    auto system = nav.peak_satsys(j);
    if (system==SATELLITE_SYSTEM::glonass) {
      if (nav.read_next_record(block)) {
        std::cerr<<"\n[ERROR] Failed to resolve Data Frame";
        return 10;
      } else {
        if (block.prn()==PRN) {
          navar=block;
          frame_read=true;
        }
      }
    } else {
      j = nav.ignore_next_block();
    }
  }
  // set starting time
  ngpt::datetime<seconds> cur_dt = navar.toc();
  ngpt::datetime_interval<seconds> intrvl (ngpt::modified_julian_day(0), seconds(15*60L));
  double x,y,z;
  // compute x,y,z for one day, every 15 min
  while (cur_dt<=cur_dt.add<seconds>(ngpt::modified_julian_day(1))) {
    auto toe = navar.glo_tb2date();
    seconds sec_diff (std::abs(ngpt::delta_sec(toe, cur_dt).as_underlying_type()));
    if (sec_diff<=seconds(15*60L)) {
      // compute ecef with navar[0]
      block.glo_ecef(cur_dt, x, y, z);
      // block.gps_dtsv(cur_dt, dt);
      std::cout<<"\n\""<<ngpt::strftime_ymd_hms<seconds>(cur_dt)<<"\" ";
      std::printf("%+20.6f%+20.6f%+20.6f", x, y, z);
    } else if (sec_diff>seconds(15*60L)) {
      frame_read = false;
      while (!frame_read) {
        auto system = nav.peak_satsys(j);
        if (j) return 5;
        if (system==SATELLITE_SYSTEM::glonass) {
          if (nav.read_next_record(block)) {
            std::cerr<<"\n[ERROR] Failed to resolve Data Frame";
            return 10;
          } else {
            if (block.prn()==PRN) {
              navar=block;
              frame_read=true;
            }
          }
        } else {
          j = nav.ignore_next_block();
        }
      }
    } else {
      std::cerr<<"\n[ERROR] Unexpected date error!";
      std::cerr<<"\n        Caused at "<<ngpt::strftime_ymd_hms<seconds>(navar.toc())
        <<" "<<ngpt::strftime_ymd_hms<seconds>(cur_dt);
      return 5;
    }
    cur_dt+=intrvl;
  }

  // rewind to end of header; read only Glonass
  /*
  nav.rewind();
  int num_msg=0;
  j = 0;
  while (!j) {
    auto system = nav.peak_satsys(j);
    if (!j) {
      if (system == SATELLITE_SYSTEM::glonass) {
        j=nav.read_next_record(block);
        if (block.prn() == 3) {
          ++num_msg;
          int day_in_week = static_cast<int>(block.data(2))/86400;
          int sec_in_day = static_cast<int>(block.data(2))%86400;
          std::cout<<"\n#"<<num_msg<<" Satellite "<<block.prn()<<", Time: "
            <<ngpt::strftime_ymd_hms(block.toc())
            <<"\n\tMessage frame time: "<<block.data(2)<<" (DOW: "<<day_in_week
            <<", SEC: "<<sec_in_day<<"), freq. number: "<<block.data(10);
        }
      } else {
        j=nav.ignore_next_block();
      }
    }
  }
  std::cout<<"\nLast status was: "<<j;
  */

  std::cout<<"\n";
}
