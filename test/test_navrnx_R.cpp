#include <iostream>
#include "navrnx.hpp"
#include "ggdatetime/datetime_write.hpp"

using ngpt::NavigationRnx;
using ngpt::NavDataFrame;
using ngpt::SATELLITE_SYSTEM;
using ngpt::seconds;
using ngpt::gps_week;

int
read_next_glo_frame(NavigationRnx& nav, int prn, NavDataFrame& block)
{
  int frame_read=0,j;
  while (!frame_read) {
    auto system = nav.peak_satsys(j);
    if (system==SATELLITE_SYSTEM::glonass) {
      if (nav.read_next_record(block)) {
        std::cerr<<"\n[ERROR] Failed to resolve Data Frame";
        return 10;
      } else {
        if (block.prn()==prn) {
          // navar=block;
          frame_read=true;
        }
      }
    } else {
      j = nav.ignore_next_block();
      if (j) {
        std::cerr<<"\n[ERROR] Failed to resolve Data Frame";
        return j;
      }
    }
  }
  return 0;
}

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
  j=0;
  nav.rewind();
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
  int it = 0;
  // compute x,y,z for one day, every 15 min
  while (cur_dt_utc<=cur_dt_utc.add<seconds>(ngpt::modified_julian_day(1)) && ++it<1500) {
    auto tb = navar.glo_tb2date(false); // tb in UTC
    double sec_diff = cur_dt_utc.sec().to_fractional_seconds() 
             - tb.sec().to_fractional_seconds();
    std::cout<<"\nti: "<<ngpt::strftime_ymd_hms<seconds>(cur_dt_utc)
      <<", tb: "<<ngpt::strftime_ymd_hms<seconds>(tb)
      <<" delta_t="<<sec_diff<<"sec";
    if (std::abs(sec_diff)<15*60e0) {
      std::cout<<"\n .... computing values";
      std::cout<<" block.glo_ecef("<<ngpt::strftime_ymd_hms<seconds>(cur_dt_utc)<<")";
      if (block.glo_ecef(cur_dt_utc, x, y, z)) {
        std::cerr<<"\n[ERROR] Failed to compute orbit for "<<ngpt::strftime_ymd_hms<seconds>(cur_dt_utc);
        return 10;
      }
      //std::cout<<"\n\""<<ngpt::strftime_ymd_hms<seconds>(cur_dt_utc)<<"\" ";
      //std::printf("%+20.6f%+20.6f%+20.6f", x*1e-3, y*1e-3, z*1e-3);
      cur_dt_utc+=intrvl;
    } else if (sec_diff<=-15*60e0) {
      std::cout<<"\n .... cannot compute SV crd; adding interval";
      cur_dt_utc+=intrvl;
    } else if (sec_diff>=15*60e0) {
      std::cout<<"\n .... reading next frame";
      if (read_next_glo_frame(nav, PRN, navar)) return 5;
    } else {
      std::cerr<<"\n[ERROR] Unexpected date error!";
      std::cerr<<"\n        Caused at "<<ngpt::strftime_ymd_hms<seconds>(navar.toc())
        <<" "<<ngpt::strftime_ymd_hms<seconds>(cur_dt_utc);
      return 5;
    }
  }

  std::cout<<"\n";
}
