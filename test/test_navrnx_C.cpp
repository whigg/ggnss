#include <iostream>
#include <cassert>
#include "navrnx.hpp"
#include "ggdatetime/datetime_write.hpp"

using ngpt::NavigationRnx;
using ngpt::NavDataFrame;
using ngpt::SATELLITE_SYSTEM;
using ngpt::seconds;
using ngpt::milliseconds;
using ngpt::gps_week;

int  PRN=1;

int main(int argc, char* argv[])
{
  if (argc!=2 && argc!=3) {
    std::cerr<<"\n[ERROR] Run as: $>testNavRnx <Nav. RINEX> [PRN] ("<<argc<<")\n";
    return 1;
  }
  if (argc==3) PRN = std::atoi(argv[2]);

  // try reading through all of the file
  NavigationRnx nav(argv[1]);
  NavDataFrame  block;
  int j = 0, block_nr = 0;
  while (!j) {
    j = nav.read_next_record(block);
    block_nr++;
  }
  std::cout<<"\n# Read "<<block_nr<<" data blocks";
  std::cout<<"\n# Last status was: "<<j;

  // rewind to end of header;
  // compute ECEF coordinates for every 15 min for GPS/PRN=3
  nav.rewind();
  j = 0;
  NavDataFrame navar[2]; // holds current and next data frame
  // read first two frames of GAL/PRN=x
  int frames_read=0;
  while (frames_read<2) {
    auto system = nav.peak_satsys(j);
    if (system==SATELLITE_SYSTEM::beidou) {
      if (nav.read_next_record(block)) {
        std::cerr<<"\n[ERROR] Failed to resolve Data Frame";
        return 10;
      } else {
        if (block.prn()==PRN) {
          navar[frames_read]=block;
          ++frames_read;
          if (frames_read==2) {
            if (navar[0].toc()==navar[1].toc()) {
              std::swap(navar[0], navar[1]);
              --frames_read;
            }
          }
        }
      }
    } else {
      j = nav.ignore_next_block();
    }
  }
  // set starting time
  ngpt::datetime<seconds> cur_dt = navar[0].toc();
  ngpt::datetime_interval<seconds> intrvl (ngpt::modified_julian_day(0), seconds(1*60L));
  double state[3],dt=0e0;
  // compute x,y,z for one day, every 15 min
  while (cur_dt<=cur_dt.add<seconds>(ngpt::modified_julian_day(1))) {
    if (cur_dt>=navar[0].toc() && cur_dt<navar[1].toc()) {
      // compute ecef with navar[0]
      block.bds_stateNclock(cur_dt, state, dt);
      std::cout<<"\n\""<<ngpt::strftime_ymd_hms<seconds>(cur_dt)<<"\" ";
      std::printf("%+15.6f%+15.6f%+15.6f %15.10f", state[0]*1e-3, state[1]*1e-3, state[2]*1e-3, dt*1e6);
      /*std::cout<<"Time-> "<< ngpt::strftime_ymd_hms<seconds>(navar[0].toc())
        <<" "<<ngpt::strftime_ymd_hms<seconds>(cur_dt)
        <<" "<<ngpt::strftime_ymd_hms<seconds>(navar[1].toc());*/
    } else {
      std::cerr<<"\n[ERROR] Unexpected date error!";
      std::cerr<<"\n        Caused at "<<ngpt::strftime_ymd_hms<seconds>(navar[0].toc())
        <<" "<<ngpt::strftime_ymd_hms<seconds>(cur_dt)
        <<" "<<ngpt::strftime_ymd_hms<seconds>(navar[1].toc());
      return 5;
    }
    cur_dt+=intrvl;
    if (cur_dt>=navar[1].toc()) { // i need the next frame
      //std::cout<<"\n ..... reading next frame ...";
      bool next_frame_found = false;
      while (!next_frame_found) {
        auto system = nav.peak_satsys(j);
        if (j<0) {
          std::cerr<<"\n[ERROR] Cannot find frame after "
            <<ngpt::strftime_ymd_hms<seconds>(navar[1].toc())
            <<" to compute coordinates at "
            <<ngpt::strftime_ymd_hms<seconds>(cur_dt);
          return -1;
        } else if (j>0) {
          std::cerr<<"\n[ERROR] While looking for next frame!";
          return j;
        }
        if (system==SATELLITE_SYSTEM::beidou) {
          if (nav.read_next_record(block)) {
            std::cerr<<"\n[ERROR] Failed to resolve Data Frame";
            return 10;
          } else {
            if (block.prn()==PRN) {
              if (block.toc()>navar[1].toc()) {
                navar[0]=block;
                next_frame_found = true;
                std::swap(navar[0], navar[1]);
              }
            }
          }
        } else {
          j = nav.ignore_next_block();
        }
      } // end finding next frame
    }
  }

  std::cout<<"\n";
}
