#include <iostream>
#include <map>
#include "obsrnx.hpp"
#include "ggdatetime/datetime_write.hpp"

using ngpt::ObservationRnx;
using ngpt::ObservationCode;
using ngpt::GnssObservable;
using ngpt::SATELLITE_SYSTEM;
  
typedef std::pair<std::size_t, double> id_pair;
typedef std::vector<id_pair>           vecof_idpair;

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr<<"\n[ERROR] Run as: $>testObsRnx [Obs. RINEX]\n";
    return 1;
  }

  // read header and print info
  ObservationRnx rnx(argv[1]);
  rnx.print_members();

  // let's make a map ob observables that we want to collect
  ObservationCode _gc1c("C1C"); //< GPS
  ObservationCode _gc1w("C1W");
  ObservationCode _gc2w("C2W");
  ObservationCode _gc2l("C2L");
  ObservationCode _gc5q("C5Q");
  ObservationCode _rc1p("C1P"); //< GLONASS
  ObservationCode _rc1c("C1C");
  ObservationCode _rc2p("C2P");
  ObservationCode _rc3q("C3Q");
  ObservationCode _ec1c("C1C"); //< GALILEO
  ObservationCode _ec6c("C6C");
  ObservationCode _ec7q("C7Q");
  ObservationCode _ec8q("C8Q");
  ObservationCode _ec5x("C5X");

  GnssObservable gc5q(ngpt::SATELLITE_SYSTEM::gps, _gc5q, 1e0);
  GnssObservable gc3c(ngpt::SATELLITE_SYSTEM::gps, _gc1c, .5e0);
                 gc3c.add(ngpt::SATELLITE_SYSTEM::gps, _gc2w, .5e0);
  GnssObservable rc1p(ngpt::SATELLITE_SYSTEM::glonass, _rc1p, 1e0);
  GnssObservable rc3c(ngpt::SATELLITE_SYSTEM::glonass, _gc1c, .3e0);
                 rc3c.add(ngpt::SATELLITE_SYSTEM::glonass, _rc2p, .3e0);
                 rc3c.add(ngpt::SATELLITE_SYSTEM::glonass, _rc3q, .3e0);
  GnssObservable ec1c(ngpt::SATELLITE_SYSTEM::galileo, _ec1c, 1e0);
  GnssObservable ec3c(ngpt::SATELLITE_SYSTEM::galileo, _ec6c, .3e0);
                 ec3c.add(ngpt::SATELLITE_SYSTEM::galileo, _ec7q, .3e0);
                 ec3c.add(ngpt::SATELLITE_SYSTEM::galileo, _ec8q, .3e0);
  GnssObservable ec8q(ngpt::SATELLITE_SYSTEM::galileo, _ec8q, 1e0);
  GnssObservable ec5x(ngpt::SATELLITE_SYSTEM::galileo, _ec5x, 1e0);

  std::map<ngpt::SATELLITE_SYSTEM, std::vector<GnssObservable>> map;
  map[ngpt::SATELLITE_SYSTEM::gps] = std::vector<GnssObservable>{gc5q, gc3c};
  map[ngpt::SATELLITE_SYSTEM::glonass] = std::vector<GnssObservable>{rc1p, rc3c};
  map[ngpt::SATELLITE_SYSTEM::galileo] = std::vector<GnssObservable>{ec1c, ec3c, ec8q, ec5x};

  // let's see what we are going to collect
  //std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>
  auto sat_obs_map = rnx.set_read_map(map, true);
  if (sat_obs_map.empty()) {
    std::cerr<<"\nWarning!! empty map!";
  } else {
    for (const auto& m : sat_obs_map) {
      if (!m.second.size()) std::cerr<<"\nWarning empty vector for satellite sys. "<<ngpt::satsys_to_char(m.first);
    }
  }

  // inilialize vector to hold results
  // std::vector<std::pair<ngpt::Satellite, std::vector<double>>>
  auto sat_obs_vec = rnx.initialize_epoch_vector(sat_obs_map);

  // go on and collect every epoch ....
  int status;
  int epoch_counter=0, satsnum;
  ngpt::modified_julian_day mjd;
  double secday;
  do {
    status = rnx.read_next_epoch(sat_obs_map, sat_obs_vec, satsnum, mjd, secday);
    // std::cout<<"\n\t-->collected "<<satsnum<<" satellites";
    /*for (auto it=sat_obs_vec.begin(); it<sat_obs_vec.begin()+satsnum; ++it) {
      std::cout<<"\n\t"<<satsys_to_char(it->first.system())<<it->first.prn()<<" : ";
      std::size_t hm = map[it->first.system()].size();
      for (std::size_t i=0; i<hm; i++) std::cout<<it->second.operator[](i)<<" ";
    }*/
    for (auto it=sat_obs_vec.begin(); it<sat_obs_vec.begin()+satsnum; ++it) {
      if (it->first.system()==SATELLITE_SYSTEM::galileo && it->first.prn()==12) {
        printf("\n%15.5f", secday);
        std::size_t hm = map[it->first.system()].size();
        for (std::size_t i=0; i<hm; i++) printf(" %20.5f",it->second.operator[](i));
      }
    }
    ++epoch_counter;
  } while (!status);
  std::cout<<"\nDone reading; last status: "<<status;
  std::cout<<"\nNumber of epochs read:"<<epoch_counter;

  std::cout<<"\n";
  return 0;
}
