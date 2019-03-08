#include <iostream>
#include "antex.hpp"

using ngpt::Antex;
using ngpt::ReceiverAntenna;
using ngpt::Satellite;
using ngpt::AntennaPcoList;
using ngpt::seconds;

int main(int argc, char* argv[])
{
  if (argc != 2) {
    std::cerr<<"\n[ERROR] Run as: $>testAntex [antex]\n";
    return 1;
  }

  Antex atx (argv[1]);
  ReceiverAntenna ant;
  ReceiverAntenna an1("TRM41249.00");
  ReceiverAntenna an2("TRM41249.00");
  an2.set_serial_nr("12379133");
  AntennaPcoList pco;
  int j0;

  std::cout<<"\nTryig to get PCO values for antenna \""<<an1.__underlying_char__()<<"\"";

  j0 = atx.get_antenna_pco(an1, pco);
  std::cout<<"\n\tfunction status: "<<j0<<" (serial disregarded)";
  if (!j0) {
    for (const auto p : pco.__vecref__()) { 
      std::cout<<"\n\t";
      p.dummy_print(std::cout);
    }
  }
  j0 = atx.get_antenna_pco(an1, pco, true); // must match serial
  std::cout<<"\n\tfunction status: "<<j0<<" (must match serial)";
  if (!j0) {
    for (const auto p : pco.__vecref__()) { 
      std::cout<<"\n\t";
      p.dummy_print(std::cout);
    }
  }
  
  std::cout<<"\nTryig to get PCO values for antenna \""<<an2.__underlying_char__()<<"\"";
  j0 = atx.get_antenna_pco(an2, pco);
  std::cout<<"\nfunction status: "<<j0<<" (serial disregarded)";
  if (!j0) {
    for (const auto p : pco.__vecref__()) { 
      std::cout<<"\n\t";
      p.dummy_print(std::cout);
    }
  }
  j0 = atx.get_antenna_pco(an2, pco, true); // must match serial
  std::cout<<"\nfunction status: "<<j0<<" (must match serial)";
  if (!j0) {
    for (const auto p : pco.__vecref__()) { 
      std::cout<<"\n\t";
      p.dummy_print(std::cout);
    }
  }

  /*
  Satellite sat (ngpt::SATELLITE_SYSTEM::galileo);
  sat.prn() = 12;
  auto dt = ngpt::datetime<seconds>(ngpt::year(2017),
      ngpt::month(10),
      ngpt::day_of_month(4), seconds(0));

  //std::ifstream::pos_type pos;
  j0 = atx.get_antenna_pco(sat.prn(), sat.system(), dt, pco);
  std::cout<<"\nfind_satellite_antenna returned: "<<j0;
  if (!j0) {
    for (const auto p : pco.__vecref__()) { 
      std::cout<<"\n\t";
      p.dummy_print(std::cout);
    }
  }
  j0 = atx.get_antenna_pco(1204, sat.system(), dt, pco);
  std::cout<<"\nfind_satellite_antenna returned: "<<j0;
  if (!j0) {
    for (const auto p : pco.__vecref__()) { 
      std::cout<<"\n\t";
      p.dummy_print(std::cout);
    }
  }
*/
  std::cout << "\n";
  return 0;
}
