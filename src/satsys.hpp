#ifndef __SATELLITE_SYSTEM_HPP__
#define __SATELLITE_SYSTEM_HPP__

/// @file     satsys.hpp
///
/// @author   xanthos@mail.ntua.gr <br>
///           danast@mail.ntua.gr
///
/// @date     Sat 19 Dec 2015 02:23:35 PM EET
///
/// @brief    Define the Global Navigation Satellite Systems (GNSS).
///
/// @details  GNSS are declared as enumerations; the template structure
///           ngpt::satellite_system_traits<>, specialized for each of the
///           systems, offers access to the characteristics of each satellite
///           system. Always use this structure to access sat. system traits,
///           e.g. don't automagically use 'G' to refer to GPS; use
///           satellite_system_traits<SATELLITE_SYSTEM::gps>::identifier.
///
/// @warning  Each of the (enumerated) satellite systems, should have
///           a specialized satellite_system_traits<> class. In case a
///           satellite systems is added in the SATELLITE_SYSTEM enumeration
///           class, you should:
///           - add a specilized satellite_system_traits<> class
///           - modify satsys_to_char function
///           - modify char_to_satsys function
///           These are all dead simple modifications; one line is enough.
///
/// @copyright Copyright © 2015 Dionysos Satellite Observatory,
///           National Technical University of Athens.
///           This work is free. You can redistribute it and/or modify it under
///           the terms of the Do What The Fuck You Want To Public License,
///           Version 2, as published by Sam Hocevar. See http://www.wtfpl.net/
///           for more details.

#include <cmath>
#include <cstddef>
#include <map>
#include <stdexcept>

namespace ngpt {

/// @enum  SATELLITE_SYSTEM
/// @brief Enumeration for known Satellite systems. This is extracted from
/// @see   RINEX v3.x
///
/// @note Each of the enumerations recorded here, should have a specialized
///       ngpt::satellite_system_traits<> template.
/// @warning Any change here, will reflect to:
///          * function ngpt::satsys_to_char
///          * function ngpt::char_to_satsys
///          * specialization template for satellite_system_triats<...>
enum class SATELLITE_SYSTEM : char {
  gps,     ///< GPS 'G'
  glonass, ///< GLONASS 'R'
  sbas,    ///< SBAS 'S'
  galileo, ///< GALILEO 'E'
  beidou,  ///< BeiDou (BDS/Compass) 'C'
  qzss,    ///< Quasi Zenith Satellite SYstem 'J'
  irnss,   ///< 'I'
  mixed    ///< 'M' Can denote any satellite system
};

/// Given a satellite system, return its identifier (i.e. the identifying
/// char).
char satsys_to_char(SATELLITE_SYSTEM) noexcept;

/// Given a char (i.e. the identifying char), return the corresponding
/// satellite system.
SATELLITE_SYSTEM
char_to_satsys(char);

/// Traits for Satellite Systems. A collection of satellite system - specific
/// "static" information for each system in ngpt::SATELLITE_SYSTEM. To be
/// specialized for each SATELLITE_SYSTEM
template <SATELLITE_SYSTEM S> struct satellite_system_traits {};

/// Specialize traits for Satellite System Gps
template <> struct satellite_system_traits<SATELLITE_SYSTEM::gps> {
  /// Identifier
  static constexpr char identifier{'G'};

  /// Dictionary holding pairs of <frequency band, freq. value>.
  static const std::map<int, double> frequency_map;

  /// Dictionary holding pairs of <frequency band, std::string>. The
  /// string is a seqeuence of (the **only**) valid attributes for
  /// each frequency band.
  static const std::map<int, std::string> valid_atributes;

  static double band2frequency(int band) { return frequency_map.at(band); }

  /// WGS 84 value of the earth's gravitational constant for GPS user μ
  static constexpr double mi() { return 3.986005e14; }

  /// WGS 84 value of the earth's rotation rate
  static constexpr double omegae_dot() { return 7.2921151467e-5; }

  /// Constant F for SV Clock Correction in seconds/sqrt(meters)
  static constexpr double f_clock() { return -4.442807633e-10; }
};

/// Specialize traits for Satellite System Glonass
template <> struct satellite_system_traits<SATELLITE_SYSTEM::glonass> {
  /// Identifier
  static constexpr char identifier{'R'};

  /// Dictionary holding pairs of <frequency band, freq. value>.
  static const std::map<int, double> frequency_map;

  /// Dictionary holding pairs of <frequency band, std::string>. The
  /// string is a seqeuence of (the **only**) valid attributes for
  /// each frequency band.
  static const std::map<int, std::string> valid_atributes;
};

/// Specialize traits for Satellite System Galileo
template <> struct satellite_system_traits<SATELLITE_SYSTEM::galileo> {
  /// Identifier
  static constexpr char identifier{'E'};

  /// Dictionary holding pairs of <frequency band, freq. value>.
  static const std::map<int, double> frequency_map;

  /// Dictionary holding pairs of <frequency band, std::string>. The
  /// string is a seqeuence of (the **only**) valid attributes for
  /// each frequency band.
  static const std::map<int, std::string> valid_atributes;

  static double band2frequency(int band) { return frequency_map.at(band); }

  /// Geocentric gravitational constant
  static constexpr double mi() { return 3.986004418e14; }

  /// Mean angular velocity of the Earth
  static constexpr double omegae_dot() { return 7.2921151467e-5; }

  /// Constant F for SV Clock Correction in seconds/sqrt(meters)
  static constexpr double f_clock() { return -4.442807309e-10; }
};

/// Specialize traits for Satellite System SBAS
template <> struct satellite_system_traits<SATELLITE_SYSTEM::sbas> {
  /// Identifier
  static constexpr char identifier{'S'};

  /// Dictionary holding pairs of <frequency band, freq. value>.
  static const std::map<int, double> frequency_map;

  /// Dictionary holding pairs of <frequency band, std::string>. The
  /// string is a seqeuence of (the **only**) valid attributes for
  /// each frequency band.
  static const std::map<int, std::string> valid_atributes;

  static double band2frequency(int band) { return frequency_map.at(band); }
};

/// Specialize traits for Satellite System QZSS
template <> struct satellite_system_traits<SATELLITE_SYSTEM::qzss> {
  /// Identifier
  static constexpr char identifier{'J'};

  /// Dictionary holding pairs of <frequency band, freq. value>.
  static const std::map<int, double> frequency_map;

  /// Dictionary holding pairs of <frequency band, std::string>. The
  /// string is a seqeuence of (the **only**) valid attributes for
  /// each frequency band.
  static const std::map<int, std::string> valid_atributes;

  static double band2frequency(int band) { return frequency_map.at(band); }
};

/// Specialize traits for Satellite System BDS
template <> struct satellite_system_traits<SATELLITE_SYSTEM::beidou> {
  /// Identifier
  static constexpr char identifier{'C'};

  /// Dictionary holding pairs of <frequency band, freq. value>.
  static const std::map<int, double> frequency_map;

  /// Dictionary holding pairs of <frequency band, std::string>. The
  /// string is a seqeuence of (the **only**) valid attributes for
  /// each frequency band.
  static const std::map<int, std::string> valid_atributes;

  static double band2frequency(int band) { return frequency_map.at(band); }

  /// Geocentric gravitational constant
  static constexpr double mi() { return 3.986004418e14; }

  /// Mean angular velocity of the Earth
  static constexpr double omegae_dot() { return 7.2921150e-5; }

  /// Constant F for SV Clock Correction in seconds/sqrt(meters)
  /// Note that std::sqrt is not constexpr (but it is for gcc)
  static
#if defined(__GNUC__)
      constexpr
#endif
      double
      f_clock() {
    constexpr double par = 2.99792458e8 * 2.99792458e8;
    return -2e0 * std::sqrt(3.986004418e14) / par;
  }
};

/// Specialize traits for Satellite System IRNSS
template <> struct satellite_system_traits<SATELLITE_SYSTEM::irnss> {
  /// Identifier
  static constexpr char identifier{'I'};

  /// Dictionary holding pairs of <frequency band, freq. value>.
  static const std::map<int, double> frequency_map;

  /// Dictionary holding pairs of <frequency band, std::string>. The
  /// string is a seqeuence of (the **only**) valid attributes for
  /// each frequency band.
  static const std::map<int, std::string> valid_atributes;

  static double band2frequency(int band) { return frequency_map.at(band); }
};

/// Specialize traits for Satellite System MIXED
template <> struct satellite_system_traits<SATELLITE_SYSTEM::mixed> {
  /// Identifier
  static constexpr char identifier{'M'};

  /// Number of frequency bands.
  static const std::size_t num_of_bands{0};

  /// Dictionary holding pairs of <frequency band, std::string>. The
  /// string is a seqeuence of (the **only**) valid attributes for
  /// each frequency band.
  static const std::map<int, std::string> valid_atributes;
};
} // namespace ngpt

#endif
