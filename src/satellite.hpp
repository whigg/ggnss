#ifndef __GNSS_SATELLITE_HPP__
#define __GNSS_SATELLITE_HPP__

/// @file     satellite.hpp
///
/// @version  0.10
///
/// @author   xanthos@mail.ntua.gr
///           danast@mail.ntua.gr
///
/// @date     Mon 11 Feb 2019 01:08:33 PM EET 
///
/// @brief    Definitions of a GNSS Satellite
/// 
/// @see      
///
/// @copyright Copyright © 2019 Dionysos Satellite Observatory,
///        National Technical University of Athens.
///        This work is free. You can redistribute it and/or modify it under
///        the terms of the Do What The Fuck You Want To Public License,
///        Version 2, as published by Sam Hocevar. See http://www.wtfpl.net/
///        for more details.

#include "satsys.hpp"
#include "antenna.hpp"

namespace ngpt
{

  int              __prn;     ///< PRN or slot number (GLONASS) or the
                              ///< SVID number (Galileo),
  int              __svn;     ///< SVN number (GPS), GLONASS number,
                              ///< GSAT number (Galileo) or SVN number
                              ///< (QZSS); blank (Compass, SBAS)
/// @class Satellite
/// This class is used to represent a GNSS satellite belonging to any GNSSystem
/// Different GNSS have/use different identifiers for their constellations, so
/// e.g. GPS uses PRN whereas GLONASS uses slot numbers. In this class, we
/// use two identifying integers per satellite (following RINEX v3.x), which
/// represent the following:
/// ----------+-------------+----------------+
/// GNSSystem | __prn       | __svn          |
/// ----------+-------------+----------------+
/// GPS       | PRN         | SVN            |
/// GLONASS   | slot number | GLONASS number |
/// GALILEO   | SVID        | GSAT number    |
/// QZSS      | PRN         | SVN            |
/// BeiDou    | PRN         | blank          |
/// SBAS      | PRN         | blank          |
/// ----------+-------------+----------------+
class Satellite
{
public:

  /// @brief Default constructor
  explicit
  Satellite(SATELLITE_SYSTEM s=SATELLITE_SYSTEM::mixed) noexcept
    : __system(s)
    , __prn(-1)
    , __svn(-1)
    , __antenna()
    {};

  /// @brief   Get the antenna type (non-const)
  /// @return  The satellite's antenna model as SatelliteAntenna
  SatelliteAntenna&
  antenna() noexcept
  {return __antenna;}

  /// @brief   Get the antenna type (const)
  /// @return  The satellite's antenna model as SatelliteAntenna
  SatelliteAntenna
  antenna() const noexcept
  {return __antenna;}

  /// @brief  Get the satellite system (non-const)
  /// @return The satellite's satellite system as SATELLITE_SYSTEM
  SATELLITE_SYSTEM&
  system() noexcept
  {return __system;}

  /// @brief  Get the satellite system (const)
  /// @return The satellite's satellite system as SATELLITE_SYSTEM
  SATELLITE_SYSTEM
  system() const noexcept
  {return __system;}

  /// @brief  Get the PRN number of the satellite (const)
  /// @return The satellite's PRN number
  /// @note PRN can mean different things per satellite system; see the
  ///       table at the class documentation
  int
  prn() const noexcept
  { return __prn;}

  /// @brief  Get the PRN number of the satellite (non-const)
  /// @return The satellite's PRN number
  /// @note PRN can mean different things per satellite system; see the
  ///       table at the class documentation
  int&
  prn() noexcept
  { return __prn;}

  /// @brief  Get the satellite's SVN number (const)
  /// @return The satellite's SVN number
  /// @note SVN can mean different things per satellite system; see the
  ///       table at the class documentation
  int
  svn() const noexcept
  { return __svn;}

  /// @brief  Get the satellite's SVN number (non-const)
  /// @return The satellite's SVN number
  /// @note SVN can mean different things per satellite system; see the
  ///       table at the class documentation
  int&
  svn() noexcept
  { return __svn;}

private:
  SATELLITE_SYSTEM __system;  ///< the satellite system
  int              __prn;     ///< PRN or slot number (GLONASS) or the
                              ///< SVID number (Galileo),
  int              __svn;     ///< SVN number (GPS), GLONASS number,
                              ///< GSAT number (Galileo) or SVN number
                              ///< (QZSS); blank (Compass, SBAS)
  SatelliteAntenna __antenna; ///< antenna type
}; // Satellite

} // namespace ngpt

#endif
