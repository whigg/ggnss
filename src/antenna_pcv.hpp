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
/// @brief     
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
/// A class to hold eccentricities of the mean antenna phase centre.
/// For receiver antennae, this is relative to the antenna reference point 
/// (ARP). North, east and up component (in millimeters).
/// For satellite antenna, relative to the center of mass of the satellite in 
/// X-, Y- and Z-direction (in millimeters).
/// @see ftp://igs.org/pub/station/general/antex14.txt
class AntennaPco
{
public:
    AntennaPco(const ObservationCode& obs, satellite_system sys, 
        double n=0e0, double e=0e0, double u=0e0)
    noexcept
    : __otype(obs),
      __ssys(sys),
      __dn(n),
      __de(e),
      __du(u)
    {}

#ifdef DEBUG
    void
    dummy_print(std::ostream&) const;
#endif

private:
    ObservationCode   __otype; ///< ObservationCode for PCVs
    satellite_system  __ssys;  ///< Satellite system (of obs code)
    double __dn,               ///< or dx in mm
           __de,               ///< or dy in mm
           __du;               ///< or dz in mm
}; // AntennaPco

class AntennaPcoList
{
public:
    AntennaPcoList() noexcept {};

    explicit
    AntennaPcoList(const AntennaPco& pco)
    noexcept
    : __pco{pco}
    {}

    std::vector<AntennaPco>&
    __vecref__() noexcept
    {return __pco;}

private:
    std::vector<AntennaPco> __pco;
}; // AntennaPcoList

}//namespace ngpt

#endif
