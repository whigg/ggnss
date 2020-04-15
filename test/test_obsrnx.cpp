#include <iostream>
#include <map>
#include "obsrnx.hpp"
#include "ggdatetime/datetime_write.hpp"

using ngpt::ObservationRnx;
using ngpt::ObservationCode;
using ngpt::GnssObservable;

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr<<"\n[ERROR] Run as: $>testObsRnx [Obs. RINEX]\n";
    return 1;
  }

  // read header and print info
  ObservationRnx rnx(argv[1]);
  rnx.print_members();

  // read epochs
  int status=0, numepochs=0;
  while (!status) {
    status = rnx.read_next_epoch();
    ++numepochs;
  }
  std::cout<<"\nEnded reading Rinex file; last status: "<<status
    <<" num of epochs: "<<numepochs;

  // go to END OF HEADER
  rnx.rewind();

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
  for (auto s : {ngpt::SATELLITE_SYSTEM::gps, ngpt::SATELLITE_SYSTEM::glonass, ngpt::SATELLITE_SYSTEM::galileo}) {
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

  std::cout<<"\n";
  return 0;
}
