#ifndef __ANTENNA_PCV_HPP__
#define __ANTENNA_PCV_HPP__

#include <vector>
#include "satsys.hpp"
#include "gnssobs.hpp"

/// @file      antenna_pcv.hpp
///
/// @version   0.10
///
/// @author    xanthos@mail.ntua.gr <br>
///            danast@mail.ntua.gr
///
/// @date      Mon 11 Feb 2019 01:08:33 PM EET 
///
/// @brief     Antenna Phase Centre Offset
/// 
/// @see       
///
/// @copyright Copyright © 2019 Dionysos Satellite Observatory, <br>
///            National Technical University of Athens. <br>
///            This work is free. You can redistribute it and/or modify it under
///            the terms of the Do What The Fuck You Want To Public License,
///            Version 2, as published by Sam Hocevar. See http://www.wtfpl.net/
///            for more details.

namespace ngpt
{

/// @class AntennaPco
///
/// A class to hold eccentricities of the mean antenna phase centre; either
/// for a ReceiverAntenna or a SatelliteAntenna.
/// For receiver antennae, this is relative to the antenna reference point 
/// (ARP). North, east and up component (in millimeters).
/// For satellite antenna, relative to the center of mass of the satellite in 
/// X-, Y- and Z-direction (in millimeters).
/// AntennaPco holds phase centre offset values for a given antenna and a
/// single GNSS/ObservationCode pair. That means, that when using an antenna
/// we would normally want a collection of AntennaPco values, at least for
/// the main ObservationCodes of a single (or more) satellite systems. For
/// that, users can use ngpt::AntennaPcoList class.
/// @see ftp://igs.org/pub/station/general/antex14.txt
class AntennaPco
{
public:

  /// @brief Constructor
  AntennaPco(const ObservationCode& obs, SATELLITE_SYSTEM sys, 
             double n=0e0, double e=0e0, double u=0e0)
    noexcept
    : __otype(obs)
    , __ssys(sys)
    , __dn(n)
    , __de(e)
    , __du(u)
    {}

#ifdef DEBUG
  void
  dummy_print(std::ostream&) const;
#endif

private:
  ObservationCode   __otype; ///< ObservationCode for PCVs
  SATELLITE_SYSTEM  __ssys;  ///< Satellite system (of obs code)
  double __dn,               ///< or dx in mm
         __de,               ///< or dy in mm
         __du;               ///< or dz in mm
}; // AntennaPco

/// @class AntennaPcoList
/// A class to hold eccentricities of the mean antenna phase centre; either
/// for a ReceiverAntenna or a SatelliteAntenna.
/// For receiver antennae, this is relative to the antenna reference point 
/// (ARP). North, east and up component (in millimeters).
/// For satellite antenna, relative to the center of mass of the satellite in 
/// X-, Y- and Z-direction (in millimeters).
/// This class holds a list of individual AntennaPco instances for a collection
/// of SATELLITE_SYSTEM/ObservationCode pairs.
class AntennaPcoList
{
public:

  /// @brief Initialize to nothing (empty list)
  AntennaPcoList() noexcept {};

  /// @brief Construct list using one AntennaPco
  /// @param[in] pco  An initial AntennaPco to add to the current list
  explicit
  AntennaPcoList(const AntennaPco& pco) noexcept
    : __pco{pco} {}

  std::vector<AntennaPco>&
  __vecref__() noexcept
  {return __pco;}

private:
  std::vector<AntennaPco> __pco; ///< A vector of AntennaPco instances, each
                                 ///< each one representing a SATELLITE_SYSTEM
                                 ///< +ObservationCode pair
}; // AntennaPcoList

}//namespace ngpt

#endif
