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
  nav.rewind();
  j = 0;
  while (!j) {
    auto system = nav.peak_satsys(j); // peak next block's system
    if (!j) {
      if (system == SATELLITE_SYSTEM::gps) { // if GPS, read the block
        j=nav.read_next_record(block);
        if (block.prn()==3) { // if GPS and PRN=3, print details
          int gw = static_cast<int>(block.data(21));
          int sw = static_cast<int>(block.data(11));
          ngpt::datetime<seconds> toe {gps_week(gw), seconds(sw)};
          std::cout<<"\n> IODE: "<<block.data(3)
            <<", IODC:"<<block.data(26)
            <<", ToM: "<<block.data(27)
            <<", ToE: "<<block.data(11)
            <<", GWk: "<<block.data(21);
          std::cout<<"\n\taka TOE is: "<<ngpt::strftime_ymd_hms<seconds>(toe);
          std::cout<<"\n\taka TOC is: "<<ngpt::strftime_ymd_hms<seconds>(block.toc());
        }
      } else {
        j=nav.ignore_next_block();
      }
    }
  }
  // rewind to end of header;
  // compute ECEF coordinates for every 15 min for GPS/PRN=3
  nav.rewind();
  j = 0;
  NavDataFrame navar[2]; // holds current and next data frame
  int PRN=2;
  // read first two frames of GPS/PRN=3
  int frames_read=0;
  while (frames_read<2) {
    auto system = nav.peak_satsys(j);
    if (system==SATELLITE_SYSTEM::gps) {
      if (nav.read_next_record(block)) {
        std::cerr<<"\n[ERROR] Failed to resolve Data Frame";
        return 10;
      } else {
        if (block.prn()==PRN) {
          navar[frames_read]=block;
          ++frames_read;
        }
      }
    } else {
      j = nav.ignore_next_block();
    }
  }
  // set starting time
  ngpt::datetime<seconds> cur_dt = navar[0].toc();
  // int lp = ngpt::dat<seconds>(cur_dt);
  ngpt::datetime_interval<seconds> intrvl (ngpt::modified_julian_day(0), seconds(15*60L));
  double x,y,z,dt;
  // compute x,y,z for one day, every 15 min
  while (cur_dt<=cur_dt.add<seconds>(ngpt::modified_julian_day(1))) {
    if (cur_dt>=navar[0].toc() && cur_dt<navar[1].toc()) {
      // compute ecef with navar[0]
      block.gps_ecef(cur_dt, x, y, z);
      block.gps_dtsv(cur_dt, dt);
      /*
      std::cout<<"\n\t"<<ngpt::strftime_ymd_hms<seconds>(cur_dt)<<" "<<x<<", "<<y<<", "<<z
        <<" computed from data frame at "<<ngpt::strftime_ymd_hms<seconds>(navar[0].toc());
      */
      std::cout<<"\n\""<<ngpt::strftime_ymd_hms<seconds>(cur_dt)<<"\" ";
      std::printf("%+15.6f%+15.6f%+15.6f %15.10f", x*1e-3, y*1e-3, z*1e-3, dt*1e6);
      //std::cout<<" (\""<<ngpt::strftime_ymd_hms<seconds>(eph)<<"\") ";
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
        if (system==SATELLITE_SYSTEM::gps) {
          if (nav.read_next_record(block)) {
            std::cerr<<"\n[ERROR] Failed to resolve Data Frame";
            return 10;
          } else {
            if (block.prn()==PRN) {
              navar[0]=block;
              next_frame_found = true;
              std::swap(navar[0], navar[1]);
            }
          }
        } else {
          j = nav.ignore_next_block();
        }
      } // end finding next frame
    }
  }

  // rewind to end of header; read only Glonass
  /*
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
  */

  std::cout<<"\n";
}
