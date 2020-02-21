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
  frame.data(2) = 11700e0;
  frame.data(3) = 7003.008789;
  frame.data(4) = 0.7835417;
  frame.data(5) = 0e0;
  frame.data(7) = -12206.626953;
  frame.data(8) = 2.8042530;
  frame.data(9) = 1.7e-9;
  frame.data(11) = 21280.765625;
  frame.data(12) = 1.3525150;
  frame.data(13) = -5.41e-9;
  frame.toc() = ngpt::datetime<seconds>(ngpt::year(2012), ngpt::month(9),
    ngpt::day_of_month(7), seconds(11700L));

  double x,y,z;
  ngpt::datetime<seconds> ti = ngpt::datetime<seconds>(ngpt::year(2012), ngpt::month(9),
    ngpt::day_of_month(7), seconds(12300L));
  frame.glo_ecef<seconds>(ti, x, y, z);
  printf("\nx=%+20.5f y=%+20.5f z=%+20.5f", x,y,z);
  std::cout<<"\n";
  return 0;
}
