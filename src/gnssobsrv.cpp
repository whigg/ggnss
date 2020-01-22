#include <stdexcept>
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
