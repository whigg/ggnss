#include <iostream>
#include "navrnx.hpp"
#include "ggdatetime/datetime_write.hpp"
#include "ggeodesy/geodesy.hpp"

using ngpt::NavigationRnx;
using ngpt::NavDataFrame;
using ngpt::SATELLITE_SYSTEM;
using ngpt::seconds;
using ngpt::gps_week;

bool utc2gps = true;
bool pz902wgs = false;
int  PRN=1;

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
  if (argc!=2 && argc!=3) {
    std::cerr<<"\n[ERROR] Run as: $>testNavRnx <Nav. RINEX> [PRN] ("<<argc<<")\n";
    return 1;
  }
  if (argc==3) PRN = std::atoi(argv[2]);

  NavigationRnx nav(argv[1]);
  NavDataFrame  block;
  int j = 0, block_nr = 0, block_prn_nr=0;
  while (!j) {
    j = nav.read_next_record(block);
    block_nr++;
    if (block.prn()==PRN && block.satsys()==SATELLITE_SYSTEM::glonass) ++block_prn_nr;
  }
  std::cout<<"\n# Read "<<block_nr<<" data blocks";
  std::cout<<"\n# Last status was: "<<j;
  std::cout<<"\n# Navigation frames for PRN"<<PRN<<": "<<block_prn_nr;

  // rewind to end of header; read only GLONASS
  // compute ECEF coordinates for every 15 min for GLO/PRN=3
  j=0;
  nav.rewind();
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
          frame_read=true;
        }
      }
    } else {
      j = nav.ignore_next_block();
    }
  }
  if (!frame_read) {
    std::cerr<<"\n[ERROR] Failed to find nav block for PRN="<<PRN;
    return 6;
  }
  std::cout<<"\n# First frame read for PRN="<<PRN<<"; going on ...";
  // set starting time
  ngpt::datetime<seconds> cur_dt_utc = block.toc();
  cur_dt_utc.remove_seconds(seconds(10800L));
  ngpt::datetime<seconds> utc_limit = block.toc();
  utc_limit.add_seconds(seconds(86400));
  ngpt::datetime_interval<seconds> intrvl (ngpt::modified_julian_day(0), seconds(1*60L));
  double state[6];
  double dt;
  int it = 0;
  // compute x,y,z for one day, every 15 min
  while (cur_dt_utc<=utc_limit && ++it<1500) {
    auto tb = block.glo_tb2date(false); // tb in UTC
    auto sec_diff = delta_sec(tb, cur_dt_utc);
    double delta_sec = sec_diff.to_fractional_seconds(); // tb - ti
    if (std::abs(delta_sec)<15*60e0) {
      if ((j=block.glo_stateNclock(cur_dt_utc, state, dt))) {
        std::cerr<<"\n[ERROR] Failed to compute orbit for "<<ngpt::strftime_ymd_hms<seconds>(cur_dt_utc)<<" UTC";
        std::cerr<<"\n        tb date is                  "<<ngpt::strftime_ymd_hms<seconds>(tb)<<" UTC";
        std::cerr<<"\n        Time difference (in sec)    "<<delta_sec<<", error_code: "<<j<<"\n";
        return 10;
      }
      if (utc2gps) {
        int leap = ngpt::dat(cur_dt_utc);
        auto cur_dt_gps = cur_dt_utc;
        cur_dt_gps.add_seconds(ngpt::seconds(leap-19L));
        std::cout<<"\n\""<<ngpt::strftime_ymd_hms<seconds>(cur_dt_gps)<<"\" ";
      } else {
        std::cout<<"\n\""<<ngpt::strftime_ymd_hms<seconds>(cur_dt_utc)<<"\" ";
      }
      if (pz902wgs) {
        double xwgs[3];
        ngpt::pz90_to_wgs84(state, xwgs, 1, 0);
        std::copy(xwgs, xwgs+3, state);
      }
      std::printf("%+20.6f%+20.6f%+20.6f%+20.8f", state[0]*1e-3, state[1]*1e-3, state[2]*1e-3, dt*1e6);
      cur_dt_utc+=intrvl;
    } else if (delta_sec<=-15*60e0) { // ti > tb && ti - tb > 15min
      if (read_next_glo_frame(nav, PRN, block)) return 5;
    } else if (delta_sec>=15*60e0) {  // ti < tb && ti - tb < 15min
      cur_dt_utc+=intrvl;
    } else {
      std::cerr<<"\n[ERROR] Unexpected date error!";
      std::cerr<<"\n        Caused at "<<ngpt::strftime_ymd_hms<seconds>(block.toc())
        <<" "<<ngpt::strftime_ymd_hms<seconds>(cur_dt_utc);
      return 5;
    }
  }

  std::cout<<"\n";
}
