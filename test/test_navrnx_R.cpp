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
  // compute ECEF coordinates for every 15 min for GLO/PRN=3
  nav.rewind();
  j = 0;
  NavDataFrame navar; // holds current data frame
  int PRN=3;
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
  std::cout<<"\nFrame read for PRN="<<PRN<<"; going on ...";
  // set starting time
  ngpt::datetime<seconds> cur_dt_utc = navar.toc();
  ngpt::datetime_interval<seconds> intrvl (ngpt::modified_julian_day(0), seconds(60L));
  double x,y,z;
  // compute x,y,z for one day, every 15 min
  while (cur_dt_utc<=cur_dt_utc.add<seconds>(ngpt::modified_julian_day(1))) {
    auto toe = navar.glo_tb2date(true);
    auto cur_dt_mt = cur_dt_utc; cur_dt_mt.add_seconds(ngpt::seconds(10800L));
    seconds sec_diff (std::abs(ngpt::delta_sec(toe, cur_dt_mt).as_underlying_type()));
    // std::cout<<"\nCurrent date:"<<ngpt::strftime_ymd_hms<seconds>(cur_dt_mt)
    //  <<" toe date: "<<ngpt::strftime_ymd_hms<seconds>(toe)<<" MT";
    if (sec_diff<seconds(15*60L)) {
      // compute ecef with navar[0]
      block.glo_ecef(cur_dt_mt, x, y, z);
      std::cout<<"\n\""<<ngpt::strftime_ymd_hms<seconds>(cur_dt_utc)<<"\" ";
      std::printf("%+20.6f%+20.6f%+20.6f", x*1e-3, y*1e-3, z*1e-3);
      // std::cout<<" (frame date in MT: "<<ngpt::strftime_ymd_hms<seconds>(toe)<<")";
    } else if (sec_diff>=seconds(15*60L)) {
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
        <<" "<<ngpt::strftime_ymd_hms<seconds>(cur_dt_utc);
      return 5;
    }
    cur_dt_utc+=intrvl;
  }

  std::cout<<"\n";
}
