#include <iostream>
#include "navrnx.hpp"
#include "ggdatetime/datetime_write.hpp"

using ngpt::NavigationRnx;
using ngpt::NavDataFrame;
using ngpt::SATELLITE_SYSTEM;
using ngpt::seconds;
using ngpt::gps_week;

int main()
{
  NavDataFrame frame;
  frame.data(2) = 11700e0 + 86400e0*5;
  frame.data(3) = 7003.008789e3;
  frame.data(4) = 0.7835417e3;
  frame.data(5) = 0e3;
  frame.data(7) = -12206.626953e3;
  frame.data(8) = 2.8042530e3;
  frame.data(9) = 1.7e-6;
  frame.data(11) = 21280.765625e3;
  frame.data(12) = 1.3525150e3;
  frame.data(13) = -5.41e-6;
  frame.set_toc(ngpt::datetime<seconds>(ngpt::year(2012), ngpt::month(9),
    ngpt::day_of_month(7), seconds(11700L)));

  double x,y,z;
  double vel[3];
  ngpt::datetime<seconds> ti = ngpt::datetime<seconds>(ngpt::year(2012), ngpt::month(9),
    ngpt::day_of_month(7), seconds(12300L));
  frame.glo_ecef<seconds>(ti, x, y, z, vel);
  printf("\n x=%+20.5f  y=%+20.5f  z=%+20.5f meters", x,y,z);
  printf("\nVx=%+20.5f Vy=%+20.5f Vz=%+20.5f meters/sec", vel[0], vel[1], vel[2]);
  printf("\nDx=%20.5f  Dy=%20.5f  Dz=%20.5f  meters", std::abs(x-7523174.853), 
    std::abs(y+10506962.176), std::abs(z-21999239.866));
  std::cout<<"\n";
  return 0;
}
