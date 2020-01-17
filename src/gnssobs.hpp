#ifndef __GNSS_OBS_HPP__
#define __GNSS_OBS_HPP__

/// @file      gnssobs.hpp
///
/// @version   0.10
///
/// @author    xanthos@mail.ntua.gr <br>
///            danast@mail.ntua.gr
///
/// @date      Mon 11 Feb 2019 01:08:33 PM EET 
///
/// @brief    Define the basics of GNSS observation types 
/// 
/// @see       RINEX v3.x
///
/// @copyright Copyright © 2019 Dionysos Satellite Observatory, <br>
///            National Technical University of Athens. <br>
///            This work is free. You can redistribute it and/or modify it under
///            the terms of the Do What The Fuck You Want To Public License,
///            Version 2, as published by Sam Hocevar. See http://www.wtfpl.net/
///            for more details.

namespace ngpt
{

/// @enum observable_type
/// @see ftp://ftp.igs.org/pub/data/format/rinex304.pdf
/// @warning Any change here should always be reflected in the functions
///          * ngpt::char_to_observabletype
///          * ngpt::observabletype_to_char
enum class OBSERVABLE_TYPE
: char
{
  pseudorange,
  carrier_phase,
  doppler,
  signal_strength,
  ionosphere_phase_delay,  ///< rnx304, sec. 5.12
  receiver_channel_number, ///< rnx304, sec. 5.13
  any                      ///< match anything
}; // OBSERVABLE_TYPE

/// TODO: what char should represent 'any'? 'any' is translated to '?' but
/// should also work the other way around, or use e.g. ' '?
/// @brief Translate a character to an OBSERVABLE_TYPE
OBSERVABLE_TYPE
char_to_observabletype(char);

/// @brief Translate an OBSERVABLE_TYPE to a character
char
observabletype_to_char(OBSERVABLE_TYPE) noexcept;

/// @class Attribute
///
/// @note The character '?', denotes any attribute
///
/// @see ftp://ftp.igs.org/pub/data/format/rinex304.pdf
class ObservationAttribute
{
public:
  /// @brief Default constructor; defaults to '?' aka any attribute
  explicit
  ObservationAttribute(char c='?') noexcept
  : __c(c)
  {}

  /// @brief Cast to char  
  char
  as_char() const noexcept
  {return __c;}

private:
  char __c; ///< tracking mode or channel
}; // Attribute

/// @class ObservationCode
///
/// The new signal structures for GPS, Galileo and BDS make it possible to 
/// generate code and phase observations based on one or a combination of 
/// several channels: Two-channel signals are composed of I and Q components, 
/// three-channel signals of A, B, and C components. Moreover, a wideband 
/// tracking of a combined E5a + E5b Galileo frequency is possible. In order 
/// to keep the observation codes short but still allow for a detailed 
/// characterization of the actual signal generation, the length of the codes 
/// is increased from two (Version 1 and 2) to three by adding a signal 
/// generation attribute. The observation code 'tna' consists of three parts:
///  +-------------------+-------------+----------+---------+----------+
///  | t: observation    | C           | L        | D       | S        |
///  |    type           | pseudorange | carrier  | doppler | signal   |
///  |                   |             | phase    |         | strength |
///  +-------------------+-------------+----------+---------+----------+
///  | n: band/frequency | 1, 2, ....,9                                |
///  +-------------------+---------------------------------------------+
///  | a: attribute      | tracking mode or channel, e.g. I, Q, etc    |
///  +-------------------+---------------------------------------------+
/// 
/// Examples:
/// - L1C: C/A code-derived L1 carrier phase (GPS, GLONASS) Carrier phase on 
///        E2-L1-E1 derived from C channel (Galileo)
/// - C2L: L2C pseudorange derived from the L channel (GPS)
/// - C2X: L2C pseudorange derived from the mixed (M+L) codes (GPS)
///
/// @warning Unknown tracking modes are not supported in RINEX 3.02 and 3.03. 
///          Only the complete specification of all signals is allowed i.e. 
///          all three fields must be defined as specified
///
/// @see ftp://ftp.igs.org/pub/data/format/rinex304.pdf
class ObservationCode
{
public:
  /// @brief Default constructor
  explicit
  ObservationCode(OBSERVABLE_TYPE otype=OBSERVABLE_TYPE::any, int band=0,
      ObservationAttribute att=ObservationAttribute())
    noexcept
    : __type(otype)
    , __band(band)
    , __attr(att)
    {};

  /// @brief Constructor from a c-string
  explicit
  ObservationCode(const char* str);

  /// @brief Get the instance's band (non-const)
  int&
  band() noexcept
  {return __band;}

  /// @brief Get the instance's band (const)
  int
  band() const noexcept
  {return __band;}

  /// @brief Cast to std::string
  std::string
  to_string() const;

private:
    OBSERVABLE_TYPE      __type;
    int                  __band;
    ObservationAttribute __attr;
}; // ObservationCode

} // ngpt

#endif
