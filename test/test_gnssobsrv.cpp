#include <iostream>
#include <vector>
#include <cassert>
#include "gnssobs.hpp"
#include "gnssobsrv.hpp"

using ngpt::SATELLITE_SYSTEM;
using ngpt::ObservationCode;
using ngpt::GnssRawObservable;

int main(/*int argc, char* argv[]*/)
{
  int EXIT_STATUS = 0;
  
  // First of all, let's check Structured Bindings for the class 
  // GnssRawObservable
  GnssRawObservable robs(SATELLITE_SYSTEM::glonass, ObservationCode("L5Q"));
  
  auto [s1, c1] = robs;
  assert(s1==SATELLITE_SYSTEM::glonass);
  assert(c1==ObservationCode("L5Q"));

  auto& [s2, c2] = robs;
  s2 = SATELLITE_SYSTEM::beidou;
  c2 = ObservationCode("C6I" );
  assert(robs==GnssRawObservable(SATELLITE_SYSTEM::beidou, ObservationCode("C6I")));
  
  // let's check Structured Bindings for the class __ObsPart
  ngpt::__ObsPart op(robs, 3.14e0);
  auto [t1, f1] = op;
  assert(t1==GnssRawObservable(SATELLITE_SYSTEM::beidou, ObservationCode("C6I")));
  assert(f1==3.14e0);
  
  auto& [t2, f2] = op;
  auto t3 = GnssRawObservable(SATELLITE_SYSTEM::glonass, ObservationCode("L5Q"));
  auto t4 = std::move(GnssRawObservable(SATELLITE_SYSTEM::glonass, ObservationCode("L5Q")));
  GnssRawObservable foo1(SATELLITE_SYSTEM::glonass, ObservationCode("L5Q"));
  auto foo2(foo1);
  auto t5 = std::move(foo1);
  t2 = foo2;
  assert(t3==t4 && t4==t5);
  f2 = 1e0;
  assert(op==ngpt::__ObsPart(GnssRawObservable(SATELLITE_SYSTEM::glonass, ObservationCode("L5Q")),
    1e0));
  
  assert(!EXIT_STATUS);
  std::cout << "\n";
  return 0;
}
