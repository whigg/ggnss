#include <stdexcept>
#include <algorithm>
#include "gnssobsrv.hpp"
 
double
ngpt::__ObsPart::frequency() const
{
  using ngpt::SATELLITE_SYSTEM;
  int band = __type.band();

  switch (__type.satsys())
  {
    case SATELLITE_SYSTEM::gps :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::gps>
        ::band2frequency(band) * __coef;
    case SATELLITE_SYSTEM::glonass :
      /*return ngpt::satellite_system_traits<SATELLITE_SYSTEM::glonass>
        ::band2frequency(band)*__coef;*/
      return 0;
    case SATELLITE_SYSTEM::sbas :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::sbas>
        ::band2frequency(band)*__coef;
    case SATELLITE_SYSTEM::galileo :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::galileo>
        ::band2frequency(band)*__coef;
    case SATELLITE_SYSTEM::beidou :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::beidou>
        ::band2frequency(band)*__coef;
    case SATELLITE_SYSTEM::qzss :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::qzss>
        ::band2frequency(band)*__coef;
    case SATELLITE_SYSTEM::irnss :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::irnss>
        ::band2frequency(band)*__coef;
    case SATELLITE_SYSTEM::mixed :
      /*return ngpt::satellite_system_traits<SATELLITE_SYSTEM::mixed>
        ::band2frequency(band)*__coef;*/
      return 0;
  }
  // shoud never reach here!
  return 0e0;
}
  
std::string
ngpt::GnssRawObservable::to_string() const noexcept
{
  std::string str(1, ngpt::satsys_to_char(__sys));
  return str + "::" + __code.to_string();
}

std::string
ngpt::__ObsPart::to_string() const noexcept
{
  return __type.to_string() + "*" + std::to_string(__coef);
}

std::string
ngpt::GnssObservable::to_string() const noexcept
{
  std::string str = __vec[0].to_string();
  auto len = __vec.size();
  for (std::size_t i=1; i<len; i++) str += ("+" + __vec[i].to_string());
  return str;
}
  
bool
ngpt::GnssObservable::operator==(const ngpt::GnssObservable& o) const noexcept
{
  if (o.__vec.size()!=__vec.size()) return false;
  for (const auto& i : __vec)
    if (auto it=std::find_if(o.__vec.begin(), o.__vec.end(), 
        [&ic=std::as_const(i)](const __ObsPart& p){return p==ic;}); it==o.__vec.end())
      return false;
  return true;
}
