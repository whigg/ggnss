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

  GnssObservable gc5q(ngpt::SATELLITE_SYSTEM::gps, _gc5q, 1e0);
  GnssObservable gc3c(ngpt::SATELLITE_SYSTEM::gps, _gc1c, .5e0);
                 gc3c.add(ngpt::SATELLITE_SYSTEM::gps, _gc2w, .5e0);

  std::map<ngpt::SATELLITE_SYSTEM, std::vector<GnssObservable>> map;
  map[ngpt::SATELLITE_SYSTEM::gps] = std::vector<GnssObservable>{gc5q, gc3c};

  // let's see what we are going to collect
  //std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>
  auto result = rnx.set_read_map(map[ngpt::SATELLITE_SYSTEM::gps]);
  std::cout<<"\nFor satsys=GPS, we will collect the following:";
  for (const auto& it_i : result[ngpt::SATELLITE_SYSTEM::gps]) { // 
    for (const auto& it_j : it_i) {
      std::cout<<"\n\t"<<it_j.first<<", "<<it_j.second;
    }
  }


  std::cout<<"\n";
  return 0;
}
