#include <iostream>
#include "navrnx.hpp"
#include "ggdatetime/datetime_write.hpp"

using ngpt::NavigationRnx;
using ngpt::NavDataFrame;
using ngpt::SATELLITE_SYSTEM;
using ngpt::milliseconds;
using ngpt::datetime;

typedef std::ifstream::pos_type pos_type;

int
get_next_msg(NavigationRnx& nav, pos_type& streampos, NavDataFrame& msg, 
  SATELLITE_SYSTEM sys, int prn)
{
  int status = nav.find_next(streampos, msg, sys, prn);
  if (status>0) {
    std::cerr<<"\n[ERROR] Failed seeking SV"<<prn<<" in navigation file";
  } else if (status<0) {
    std::cerr<<"\n[DEBUG] EOF in navigation file encountered!";
  }
  return status;
}

int
msg_is_ok(NavDataFrame& msg, datetime<milliseconds>& t)
{
  if (msg.sv_health()) {
    std::cerr<<"\n## Navigation message signals unhealthy SV";
    // get next message and set t=msg.toc
    return -1;
  }

  ngpt::seconds fitsec (msg.fit_interval());
  if (msg.system()==SATELLITE_SYSTEM::glonass) {//TODO is this toc or toe ?
    auto min_t = msg.toc<milliseconds>();
    auto max_t = msg.toc<milliseconds>();
    max_t.add_seconds(fitsec);
    min_t.remove_seconds(fitsec);
    if (t>=min_t && t<max_t) return 0; // all ok
    if (t>=max_t) return 1;            // get next message
    if (t<min_t) return 100;           // error
  } else {
    if (t>=msg.toc<milliseconds>()) {
      auto max_t = msg.toc<milliseconds>();
      max_t.add_seconds(fitsec);
      if (t<max_t) {
        return 0; // all ok
      } else {
        return 1; // get next message
      }
    } else {
      return 100; //error
    }
  }

  return -100;
}

int main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr<<"\n[ERROR] Run as: $>testNavRnx [Nav. RINEX] [SV e.g. G01]\n";
    return 1;
  }

  // resolve satellite
  SATELLITE_SYSTEM sys;
  int prn;
  try {
    sys = ngpt::char_to_satsys(argv[2][0]);
  } catch (std::exception& e) {
    std::cerr<<"\n"<<e.what();
    std::cerr<<"\n[ERROR] Failed to resolve satelite system!";
    return 1;
  }
  char* end;
  prn = std::strtol(argv[2]+1, &end, 10);

  // construct the navigation rinex instance
  NavigationRnx nav(argv[1]);

  // the (first) navigation block for this satellite
  pos_type streampos;
  NavDataFrame msg,msg2;
  if (get_next_msg(nav, streampos, msg, sys, prn)) return 10;

  // start epoch (same as epoch of first block)
  datetime<milliseconds> start = msg.toc<milliseconds>();

  // loop untill next day
  auto stop(start); stop.add_seconds(milliseconds(milliseconds::max_in_day));
  milliseconds dt(60e0*999);

  double state[3], clock;
  int j,status;
  while (start<stop) {
    j = msg_is_ok(msg, start);
    if (j==-1) {
      std::cerr<<"\n## Getting new nav message, and resetting epoch to:";
      status=get_next_msg(nav, streampos, msg, sys, prn);
      if (status<0) {
        std::cerr<<"\n### EOF encountered while searching for next nav message!";
        break;
      } else if (status>0) {
        std::cerr<<"\n**** ERROR while searching for next nav message!";
        break;
      }
      start = msg.toc<milliseconds>();
      std::cerr<<ngpt::strftime_ymd_hms<milliseconds>(start); 
    } else if (j==1) {
      std::cerr<<"\n## Getting new nav message";
      status=get_next_msg(nav, streampos, msg, sys, prn);
      if (status<0) {
        std::cerr<<"\n### EOF encountered while searching for next nav message!";
        break;
      } else if (status>0) {
        std::cerr<<"\n**** ERROR while searching for next nav message!";
        break;
      }
    } else {
      if (msg.stateNclock(start, state, clock)) return 200;
      std::cout<<"\n\""<<ngpt::strftime_ymd_hms<milliseconds>(start)<<"\" ";
      std::printf("%+15.6f%+15.6f%+15.6f %15.10f", state[0]*1e-3, state[1]*1e-3, state[2]*1e-3, clock*1e6);
      start.add_seconds(dt);
    }
  }

  std::cout<<"\n";
}
