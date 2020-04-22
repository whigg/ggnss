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
  
typedef std::pair<std::size_t, double> id_pair;
typedef std::vector<id_pair>           vecof_idpair;
using svdit = std::vector<std::pair<Satellite, std::vector<double>>>::iterator;

int
read_nav_till_sat(NavigationRnx& nav, const Satellite& sat,
  std::vector<NavDataFrame>& sat_nav_vec)
{
  int j;
  NavDataFrame frame;
  do {
    if ((j=nav.read_next_record(frame))) return j;
    if (frame.system()==sat.system() && frame.prn()==sat.prn()) {
      sat_nav_vec.emplace_back(std::move(frame));
      return 0;
    } else {
      Satellite s(frame.system(), frame.prn());
      auto it = std::find_if(sat_nav_vec.begin(), sat_nav_vec.end(), 
        [&s](const NavDataFrame& n){return n.system()==s.system() && n.prn()==s.prn();});
      if (it==sat_nav_vec.end()) {
        sat_nav_vec.push_back(frame);
      } else {
        *it=frame;
      }
    }
  } while(true);

  return 50;
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

  // inilialize vector to hold satellite/observation pairs for every epoch
  std::vector<std::pair<Satellite, std::vector<double>>>
    sat_obs_vec (obsrnx.initialize_epoch_vector(sat_obs_map));

  // go on and collect every epoch ....
  int status;
  int epoch_counter=0, satsnum;
  ngpt::modified_julian_day mjd;
  double secday;
  do {
    status = obsrnx.read_next_epoch(sat_obs_map, sat_obs_vec, satsnum, mjd, secday);
    // for every sat-obs pair
    for (svdit oit=sat_obs_vec.begin(); oit!=sat_obs_vec.end(); ++oit) {
      Satellite cursat(oit->first);
      // find satellite's navigation block or read rinex untill we find one
      auto nit = std::find_if(sat_nav_vec.begin(), sat_nav_vec.end(),
        [&cursat](const NavDataFrame& p)
        {return cursat.system()==p.system() && cursat.prn()==p.prn();});
      if (nit==sat_nav_vec.end()) {// need to collect satellite from Nav Rinex
        if (read_nav_till_sat(navrnx, cursat, sat_nav_vec)) return 150;
        nit=sat_nav_vec.end()-1;
      }
      assert(oit->first.system()==nit->system() && oit->first.prn()==nit->prn());
    }
    ++epoch_counter;
  } while (!status);

  return status;
}
