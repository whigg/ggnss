#include <iostream>
#include <vector>
#include "ggdatetime/dtcalendar.hpp"
#include "ggdatetime/datetime_write.hpp"
#include "bern_utils.hpp"

using ngpt::BernSatellit;

int main(int argc, char* argv[])
{
  if (argc!=2) {
    std::cerr<<"\nUsage: test_SATELLIT <SATELLIT file>\n";
    return 1;
  }

  BernSatellit sat (argv[1]);

  std::vector<int> svnvec = {714, 769, 784, 856, 801, 733, 701, 802, 776, 723, 853, 734};

  ngpt::datetime<ngpt::seconds> d1(ngpt::year(2016), ngpt::month(11),
    ngpt::day_of_month(3), ngpt::hours(12), ngpt::minutes(59), ngpt::seconds(3));
  ngpt::datetime<ngpt::seconds> d2(ngpt::year(2020), ngpt::month(1),
    ngpt::day_of_month(20), ngpt::hours(12), ngpt::minutes(59), ngpt::seconds(3));

  int prn, frq, status;
  for (auto i : svnvec) {
    status = sat.get_frequency_channel(i, d1, frq, prn);
    if (status>0) {
      std::cerr<<"\n[ERROR] Error encountered while searching for sat with svn="<<i;
      std::cerr<<"\n[ERROR] Return status is: "<<status;
      return 2;
    } else if (status<0) {
      std::cerr<<"\nSatellite with svn="<<i<<" for epoch "<<ngpt::strftime_ymd_hms(d1)<<" not matched!";
    } else {
      std::cout<<"\n"<<ngpt::strftime_ymd_hms(d1)<<" SVN: "<<i<<" PRN: "<<prn<<" FRQ: "<<frq;
    }
  }
  for (auto i : svnvec) {
    status = sat.get_frequency_channel(i, d2, frq, prn);
    if (status>0) {
      std::cerr<<"\n[ERROR] Error encountered while searching for sat with svn="<<i;
      std::cerr<<"\n[ERROR] Return status is: "<<status;
      return 2;
    } else if (status<0) {
      std::cerr<<"\nSatellite with svn="<<i<<" for epoch "<<ngpt::strftime_ymd_hms(d2)<<" not matched!";
    } else {
      std::cout<<"\n"<<ngpt::strftime_ymd_hms(d2)<<" SVN: "<<i<<" PRN: "<<prn<<" FRQ: "<<frq;
    }
  }

  std::cout<<"\n";
  return 0;
}
