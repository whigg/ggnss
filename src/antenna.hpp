#ifndef __GNSS_ANTENNA_HPP__
#define __GNSS_ANTENNA_HPP__

/// @file     antenna.hpp
///
/// @version  0.10
///
/// @author   xanthos@mail.ntua.gr
///           danast@mail.ntua.gr
///
/// @date     Mon 11 Feb 2019 01:08:33 PM EET 
///
/// @brief    GNSS Receiver and Satellite Antenna Classes.
/// 
/// @see      http://www.epncb.oma.be/ftp/station/general/rcvr_ant.tab
///
/// @copyright Copyright © 2019 Dionysos Satellite Observatory,
///        National Technical University of Athens.
///        This work is free. You can redistribute it and/or modify it under
///        the terms of the Do What The Fuck You Want To Public License,
///        Version 2, as published by Sam Hocevar. See http://www.wtfpl.net/
///        for more details.

namespace ngpt
{

/// Namespace to hide antenna specific details.
namespace antenna_details
{
  /// Maximum number of characters describing a GNSS antenna model 
  /// (no radome) for receiver antennas
  constexpr std::size_t antenna_model_max_chars { 15 };

  /// Maximum number of characters describing a GNSS antenna radome type
  /// for receiver antennas
  constexpr std::size_t antenna_radome_max_chars { 4 };

  /// Maximum number of characters describing a GNSS antenna serial number
  /// receiver antennas
  constexpr std::size_t antenna_serial_max_chars { 20 };

  /// Maximum size to represent all fields, including whitespaces and
  /// the (last) null terminating character for receiver antennas
  constexpr std::size_t antenna_full_max_chars
  {  antenna_model_max_chars  + 1 /* whitespace */
   + antenna_radome_max_chars
   + antenna_serial_max_chars + 1 /* null-reminating char */
  };

  /// Maximum size of a satellite antenna (chars)
  constexpr std::size_t satellite_antenna_max_chars {20};
}

/// @class ReceiverAntenna
///
/// @details A ReceiverAntenna is just a string, combining the
///          * antenna model
///          * radome
///          * serial number (if any)
///          According to rcvr_ant this is how they are defined:
///          Antennas:  15 columns maximum
///           First three characters are manufacturer code (except satellites)
///            Allowed in manufacturer code: - and A-Z and 0-9
///            Allowed in model name: /-_.+ and A-Z and 0-9
///            Model name must start with A-Z or 0-9
///          Radomes:   4 columns; A-Z and 0-9 allowed
///          Antenna+Radome: Combine them with the radome code in columns 
///           17-20. Fill with spaces between the end of the antenna and column
///           17.
///           Example: 'AOAD/M_T        SCIT'
///                     01234567890123456789
///          (**note** that this description, copyied from rcvr_ant, is not
///          zero offset, thus each number should be -1).
///          To represent such an antenna, we are using a contiguous 40+1 char
///          string. The first twenty chars are the model+serial and the
///          following 20 chars are the serial number (if any).
///
/// @see    rcvr_ant
///
/// @example  test_antenna.cpp
///       An example to show how to use the Antenna class.
class ReceiverAntenna
{
public:

  /// @brief Default constructor; model+radome are empty (i.e. filled with
  ///        whitespace characters, serial is nothing.
  ReceiverAntenna() noexcept 
  {initialize();}

  /// @brief Constructor from a c-string (model+radome)
  explicit ReceiverAntenna 
  (const char*) noexcept;

  /// @brief Constructor from a string (model+radome)
  explicit ReceiverAntenna
  (const std::string&) noexcept;

  /// @brief Equality operator (checks antenna type, radome and serial nr).
  bool
  is_same(const ReceiverAntenna&) const noexcept;

  /// @brief Compare antenna's serial number to a c-string
  bool
  compare_serial(const char*) const noexcept;

  /// @brief Check if antenna has serial number
  bool
  has_serial() const noexcept;

  /// @brief Compare model and radome (diregard serials if any)
  bool
  compare_model(const ReceiverAntenna&) const noexcept;

  /// @brief Set antenna's serial number
  void
  set_serial_nr(const char*) noexcept;

  /// @brief Get the underlying c-string
  const char*
  __underlying_char__() const noexcept
  { return __name; }

private:
  /// @brief Initialize antenna 
  void
  initialize() noexcept;

  /// @brief Set radome to 'NONE'
  void
  set_none_radome() noexcept;
  
  /// @brief check if radome is empty (aka only whitespaces)
  bool
  radome_is_empty() const noexcept;

  /// @brief Copy from a c-string
  void
  copy_from_cstr(const char*) noexcept;
  
  /// Combined antenna, radome and serial number (model+dome+serial).
  char __name[antenna_details::antenna_full_max_chars]; 

}; // ReceiverAntenna

/// @class SatelliteAntenna
///
/// @details A SatelliteAntenna is just a string, containing the model
///          information, e.g. 'BEIDOU-2G' or 'GLONASS-K2', which can be at
///          maximium 20 characters long.
///
/// @see     rcvr_ant
///
/// @example  test_antenna.cpp
///       An example to show how to use the Antenna class.
///
class SatelliteAntenna
{
public:
  /// @brief Constructor to nothing
  SatelliteAntenna() noexcept
  {initialize();}

  /// @brief Constructor from a c-string containing the antenna type
  explicit
  SatelliteAntenna(const char* c) noexcept;

  /// @brief Compare satellite antenna types
  bool
  is_same(const char*) const noexcept;

  /// @brief Set model from a c-string (acts like a constructor)
  inline void
  set_from_cstr(const char* c) noexcept
  { copy_from_cstr(c); }

  /// @brief Get the underlying c-string
  const char*
  __underlying_char__() const noexcept
  { return __name; }

private:

  /// @brief set to no-string (aka '\0')
  void
  initialize() noexcept;

  /// @brief Copy model from an input string
  void
  copy_from_cstr(const char* ) noexcept;
  
  ///< A c-string containng the antenna model
  char __name[antenna_details::satellite_antenna_max_chars+1]; 
}; // SatelliteAntenna

} // ngpt

// TODO The following won't work !!!
/*

/// @brief Overload '<<' operator for ReceiverAntenna
std::ostream&
operator<<(std::ostream&, ngpt::ReceiverAntenna);

/// @brief Overload '<<' operator for SatelliteAntenna
std::ostream&
operator<<(std::ostream&, ngpt::SatelliteAntenna);

*/

#endif
