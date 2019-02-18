#ifndef __GNSS_SATELLITE_HPP__
#define __GNSS_SATELLITE_HPP__

#include "satsys.hpp"
#include "antenna.hpp"

namespace ngpt
{

class Satellite
{
public:
    explicit
    Satellite(satellite_system s=satellite_system::mixed)
    : __system(s),
      __prn(-1),
      __svn(-1),
      __antenna("                    \0")
    {};

    SatelliteAntenna&
    antenna() noexcept
    {return __antenna;}

    SatelliteAntenna
    antenna() const noexcept
    {return __antenna;}

    satellite_system&
    system() noexcept
    {return __system;}
    
    satellite_system
    system() const noexcept
    {return __system;}

    int
    prn() const noexcept
    { return __prn;}
    
    int&
    prn() noexcept
    { return __prn;}
    
    int
    svn() const noexcept
    { return __svn;}
    
    int&
    svn() noexcept
    { return __svn;}
      
private:
    satellite_system __system; ///< the satellite system
    int              __prn;    ///< prn or slot number (GLONASS) or the
                               ///< SVID number (Galileo),
    int              __svn;    ///< SVN number (GPS), GLONASS number,
                               ///< GSAT number (Galileo) or SVN number
                               ///< (QZSS); blank (Compass, SBAS)
    SatelliteAntenna __antenna; ///< antenna type
}; // Satellite

} // namespace ngpt

#endif
