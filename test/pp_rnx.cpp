#include <iostream>
#include <algorithm>
#include <cassert>
#include "obsrnx.hpp"
#include "navrnx.hpp"
#include "ggdatetime/datetime_write.hpp"

using ngpt::ObservationRnx;
using ngpt::NavigationRnx;
using ngpt::NavDataFrame;
using ngpt::ObservationCode;
using ngpt::GnssObservable;
using ngpt::SATELLITE_SYSTEM;
using ngpt::Satellite;
using ngpt::datetime;
using ngpt::milliseconds;
  
typedef std::pair<std::size_t, double> id_pair;
typedef std::vector<id_pair>           vecof_idpair;
using svdit = std::vector<std::pair<Satellite, std::vector<double>>>::iterator;

/// Search through the Navigation RInex file to find a valid message for
/// the given satellite (resolved from msg) at the given epoch (t).
/// If such a message is found, it is stored in msg and the function returns
/// 0. If -1 is returned, we reached EOF before finding such a message. Else,
/// if we return with >0, an error has occured.
int
update_msg(const datetime<milliseconds>& t, NavigationRnx& nav, 
  NavDataFrame& msg)
{
  SATELLITE_SYSTEM sys = msg.system();
  int prn = msg.prn();
  NavigationRnx::pos_type curpos;
  NavDataFrame nmsg;
  int j = nav.find_next_valid<milliseconds>(t, curpos, nmsg, sys, prn);
  if (!j) msg=nmsg;
  nav.rewind(curpos);
  return j;
}

/// Check if the navigation message is ok to use, for epoch t; aka, t is
/// inside the message's fit interval and the SV health is ok.
/// If everything is ok, 0 is returned.
/// If the satellite is unhealthy, -1 is returned.
/// If 1 is returned, t is outside the message's interval
/// if anything >1 is returned, an error has occured
int
check_nav_msg(const NavDataFrame& msg, const datetime<milliseconds>& ti)
{
  // first check health
  if (msg.sv_health()) return -1;

  // check fit interval
  ngpt::seconds fitsec (msg.fit_interval());
  if (msg.system()==SATELLITE_SYSTEM::glonass) {
    auto min_t = msg.toe<milliseconds>();
    auto max_t = msg.toe<milliseconds>();
    max_t.add_seconds(fitsec);
    min_t.remove_seconds(fitsec);
    if (ti>=min_t && ti<max_t) return 0;
    return 1;
  } else {
    if (ti>=msg.toc<milliseconds>()) {
      auto max_t = msg.toc<milliseconds>();
      max_t.add_seconds(fitsec);
      if (ti<max_t) return 0;
    }
  }

  return 1;
}

std::vector<NavDataFrame>::iterator
get_valid_msg(NavigationRnx& nav, const Satellite& sat, 
  const datetime<milliseconds>& t, std::vector<NavDataFrame>& sat_nav_vec,
  int& status)
{
  status=-200;
  auto nit = std::find_if(sat_nav_vec.begin(), sat_nav_vec.end(),
      [&sat, &t, &status](const NavDataFrame& p)
      {return (sat.system()==p.system() && sat.prn()==p.prn()) 
        && !(status=check_nav_msg(p, t));}
    );

  if (!status)  return nit;  // all ok, msg found!
    
  NavDataFrame msg; msg.system()=sat.system(); msg.prn()=sat.prn();
  int j=update_msg(t, nav, msg);
  if (j) {
    std::cerr<<" failed to valid new valid message for epoch: "
      <<ngpt::strftime_ymd_hms<milliseconds>(t);
    status=j;
    return sat_nav_vec.end();
  } else {
    if (status>=0) {
      nit = std::find_if(sat_nav_vec.begin(), sat_nav_vec.end(),
          [&sat](const NavDataFrame& p)
          {return sat.system()==p.system() && sat.prn()==p.prn();});
      assert(nit!=sat_nav_vec.end());
      *nit = std::move(msg);
    } else {
      sat_nav_vec.push_back(std::move(msg));
      nit = sat_nav_vec.end()-1;
    }
    status=0;
    return nit;
  }

  status=1000;
  return sat_nav_vec.end();
}

int main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr<<"\n[ERROR] Run as: $>pprnx [Obs. RINEX] [Nav. RINEX]\n";
    return 1;
  }

  // open Obs Rnx and printf info
  ObservationRnx obsrnx(argv[1]);
  obsrnx.print_members();
  
  // open Nav Rnx
  NavigationRnx navrnx(argv[2]);
  std::vector<NavDataFrame> sat_nav_vec; sat_nav_vec.reserve(50);
  
  // use GPS C1C
  GnssObservable gc1c(SATELLITE_SYSTEM::gps, ObservationCode("C1C"), 1e0);
  
  // make map to extract GnssObservables for Obs Rnx
  std::map<SATELLITE_SYSTEM, std::vector<GnssObservable>> map;
  map[SATELLITE_SYSTEM::gps] = std::vector<GnssObservable>{gc1c};
  auto sat_obs_map = obsrnx.set_read_map(map);
  if (sat_obs_map.empty()) return 100;
  std::cout<<"\nList of observables to collect per satellite system:";
  for (const auto& m : sat_obs_map) {
    if (!m.second.size()) std::cerr<<"\nWarning empty vector for satellite sys. "<<ngpt::satsys_to_char(m.first);
  }
  for (auto const& m : map) {
    std::cout<<"\n\tSystem: "<<ngpt::satsys_to_char(m.first);
    for (const auto& v : m.second) std::cout<<" "<<v.to_string();
  }

  // inilialize vector to hold satellite/observation pairs for every epoch
  std::vector<std::pair<Satellite, std::vector<double>>>
    sat_obs_vec (obsrnx.initialize_epoch_vector(sat_obs_map));

  // go on and collect every epoch ....
  int status, j;
  int epoch_counter=0, satsnum;
  ngpt::modified_julian_day mjd;
  double secday, clock, state[6];
  do {
    status = obsrnx.read_next_epoch(sat_obs_map, sat_obs_vec, satsnum, mjd, secday);
    ngpt::datetime<milliseconds> epoch 
      (mjd, milliseconds(static_cast<long>(secday*milliseconds::sec_factor<double>())));
    // for every sat-obs pair
    for (int i=0; i<satsnum; i++) {
      svdit oit=sat_obs_vec.begin()+i; // iterator to sat_obs_vec
      Satellite cursat(oit->first);    // current satellite
      assert(cursat.system()==SATELLITE_SYSTEM::gps);
      if (cursat.prn()==4){
      // find satellite's navigation block or read rinex untill we find one
      auto nit = get_valid_msg(navrnx, cursat, epoch, sat_nav_vec, j);
      if (!j) {
        // get position and clock bias for satellite
        assert(nit!=sat_nav_vec.end());
        assert(cursat.system()==nit->system() && cursat.prn()==nit->prn());
        nit->stateNclock(epoch, state, clock);
      } else {
        std::cerr<<"\n*** Cannot find valid message for SV "
          <<satsys_to_char(cursat.system())<<cursat.prn()
          <<" Epoch is "<<ngpt::strftime_ymd_hms<milliseconds>(epoch)
          <<" status= "<<j;
      }
      }
    }
    ++epoch_counter;
  } while (!status);

  return status;
}
