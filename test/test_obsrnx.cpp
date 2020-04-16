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

void
print_map(std::map<SATELLITE_SYSTEM, std::vector<GnssObservable>>& map, 
  std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>& result)
{
  if (result.empty()) {
    std::cout<<"\nEmpty result map!";
    return;
  }
  for (auto s : {SATELLITE_SYSTEM::gps, SATELLITE_SYSTEM::glonass, SATELLITE_SYSTEM::galileo}) {
    std::cout<<"\nFor sat-sys: "<<ngpt::satsys_to_char(s)
      <<", we will collect the following:";
    int i=0;
    for (const auto& it_i : result[s]) { //
      GnssObservable t = map[s].operator[](i);
      std::cout<<"\n *size = "<<it_i.size()<<" GnssObservable is: "<<t.to_string();
      for (const auto& it_j : it_i) {
        std::cout<<"\n\t"<<it_j.first<<", "<<it_j.second;
      }
      ++i;
    }
  }
}

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

  std::map<ngpt::SATELLITE_SYSTEM, std::vector<GnssObservable>> map;
  map[ngpt::SATELLITE_SYSTEM::gps] = std::vector<GnssObservable>{gc5q, gc3c};
  map[ngpt::SATELLITE_SYSTEM::glonass] = std::vector<GnssObservable>{rc1p, rc3c};
  map[ngpt::SATELLITE_SYSTEM::galileo] = std::vector<GnssObservable>{ec1c, ec3c, ec8q};

  // let's see what we are going to collect
  //std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>
  auto result = rnx.set_read_map(map);
  print_map(map, result);

  // let's make some erronous examples, eg an empty map
  std::map<ngpt::SATELLITE_SYSTEM, std::vector<GnssObservable>> map_er1;
  std::cout<<"\n\nTrying with an empty map:";
  result = rnx.set_read_map(map_er1);
  print_map(map_er1, result);
  std::cout<<"\nThe above should have returned an empty map";

  // last observable is galileo!
  map_er1[ngpt::SATELLITE_SYSTEM::gps] = std::vector<GnssObservable>{gc5q, gc3c, ec8q};
  std::cout<<"\n\nTrying with an observable of different sat. sys.:";
  result = rnx.set_read_map(map_er1);
  print_map(map_er1, result);
  std::cout<<"\nThe above should have returned an empty map and produced an error";
  
  // GnssObservable with missing RINEX obs types
  GnssObservable gmis(ngpt::SATELLITE_SYSTEM::gps, ObservationCode("C1Y"), .5e0);
                 gmis.add(ngpt::SATELLITE_SYSTEM::gps, ObservationCode("C2M"), .5e0);
  map_er1[ngpt::SATELLITE_SYSTEM::gps] = std::vector<GnssObservable>{gc5q, gc3c, gmis};
  std::cout<<"\n\nTrying with Observation Codes that are not there:";
  result = rnx.set_read_map(map_er1);
  print_map(map_er1, result);
  std::cout<<"\nThe above should have returned an empty map and produced an error";
  std::cout<<"\nHowever, we can treat missing as only warnings!";
  result = rnx.set_read_map(map_er1, true);
  print_map(map_er1, result);
  std::cout<<"\nThe above should have returned a non-empty map and produced a warning";

  std::cout<<"\n";
  return 0;
}
