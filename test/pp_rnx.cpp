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

int
update_msg(datetime<milliseconds>& t, NavigationRnx& nav, NavDataFrame& msg)
{
  // std::cout<<"\n$$$ Updating msg, status: ";
  SATELLITE_SYSTEM sys = msg.system();
  int prn = msg.prn();
  NavigationRnx::pos_type curpos;
  NavDataFrame nmsg;
  int j = nav.find_next_valid<milliseconds>(t, curpos, nmsg, sys, prn);
  // std::cout<<j;
  if (!j) msg=nmsg;
  nav.rewind(curpos);
  return j;
}

int
check_nav_msg(NavDataFrame& msg, datetime<milliseconds>& ti)
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
    if (ti>=min_t && ti<max_t) return 0; // all ok
    if (ti>=max_t) return 1;             // get next message
    if (ti<min_t) return 100;            // error
  } else {
    if (ti>=msg.toc<milliseconds>()) {
      auto max_t = msg.toc<milliseconds>();
      max_t.add_seconds(fitsec);
      if (ti<max_t) {
        return 0; // all ok
      } else {
        return 1; // get next message
      }
    } else {
      std::cerr<<"\n*** error at check_nav_msg() ToC:"
        <<ngpt::strftime_ymd_hms<milliseconds>(msg.toc<milliseconds>())
        <<" for SV "<<satsys_to_char(msg.system())<<msg.prn();
      std::cerr<<"\n    epoch is: "<<ngpt::strftime_ymd_hms<milliseconds>(ti);
      return 100; //error
    }
  }

  return 110;
}

std::vector<NavDataFrame>::iterator
get_valid_msg(NavigationRnx& nav, const Satellite& sat, 
  const datetime<milliseconds>& t, std::vector<NavDataFrame>& sat_nav_vec,
  int& status)
{
  status=-100;
  auto nit = std::find_if(sat_nav_vec.begin(), sat_nav_vec.end(),
      [&sat, &t, &status](const NavDataFrame& p)
      {return (sat.system()==p.system() && sat.prn()==p.prn()) && !(status=check_nav_msg(p, t));}
    );

  if (!status)  return nit;  // all ok, msg found!
  if (status>1) return nit;  // error
  if (status==1 || status==-100) { // need to read in next message
    NavDataFrame msg; msg.system()=sat.system(); msg.prn()=sat.prn();
    int j=update_msg(t, nav, msg);
    if (j) {
      status=j;
      return sat_nav_vec.end();
    } else {
      if (status==1) {
        auto nit = std::find_if(sat_nav_vec.begin(), sat_nav_vec.end(),
          [&sat](const NavDataFrame& p)
          {return sat.system()==p.system() && sat.prn()==p.prn();});
        assert(nit!=sat_nav_vec.end());
        *nit = msg;
      } else {
        sat_nav_vec.push_back(msg);
        nit = sat_nav_vec.end()-1;
      }
    status=0;
    return nit;
    }
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
  int status, j, k;
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
      // find satellite's navigation block or read rinex untill we find one
      auto nit = get_valid_msg(navrnx, cursat, epoch, sat_nav_vec, status);
      if (!status) {
        // get position and clock bias for satellite
        assert(oit->first.system()==nit->system() && oit->first.prn()==nit->prn());
        nit->stateNclock(epoch, state, clock);
        } else if (j==-1) { // satellite unhealthy!
          std::cerr<<"\n## Msg denotes unhealthy SV at "
            <<ngpt::strftime_ymd_hms<milliseconds>(nit->toc<milliseconds>())
            <<" for SV "<<satsys_to_char(nit->system())<<nit->prn();
        } else if (j==1) {  // need next message
          k=update_msg(epoch, navrnx, *nit);
          if (k==-1) {
            std::cerr<<"\n## Cannot find newer navigation file than "
              <<ngpt::strftime_ymd_hms<milliseconds>(nit->toc<milliseconds>())
              <<" for SV "<<satsys_to_char(nit->system())<<nit->prn();
          } else if (k) {
            std::cerr<<"\n*** Error while checking Nav. Message!";
            return 200;
          }
        } else {
          std::cerr<<"\n*** Error checking msg! check_nav_msg() returned: "<<j;
          return 210;
        }
      }
      /*
      auto nit = std::find_if(sat_nav_vec.begin(), sat_nav_vec.end(),
          [&cursat](const NavDataFrame& p)
          {return cursat.system()==p.system() && cursat.prn()==p.prn();});
      if (nit==sat_nav_vec.end()) {// need to collect satellite from Nav Rinex
        NavDataFrame nwmsg; nwmsg.system()=cursat.system(); nwmsg.prn()=cursat.prn();
        j=update_msg(epoch, navrnx, nwmsg);
        if (!j) {
          sat_nav_vec.push_back(std::move(nwmsg));
          nit=sat_nav_vec.end()-1;
        } else if (j==-1) {
          std::cout<<"\n### Failed to find valid msg! t="
              <<ngpt::strftime_ymd_hms<milliseconds>(epoch)
              <<" for SV "<<satsys_to_char(cursat.system())<<cursat.prn();
        } else {
          std::cout<<"\n*** Error while searching for valid msg";
        }
      } else j=0;
      */

    }
    ++epoch_counter;
  } while (!status);

  return status;
}
