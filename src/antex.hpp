#ifndef __ANTEXX_HPP__
#define __ANTEXX_HPP__

#include <fstream>
#include "satellite.hpp"
#include "antenna.hpp"
#include "antenna_pcv.hpp"
#include "ggdatetime/dtcalendar.hpp"

/// @file      antex.hpp
///
/// @version   0.10
///
/// @author    xanthos@mail.ntua.gr <br>
///            danast@mail.ntua.gr
///
/// @date      Mon 11 Feb 2019 01:08:33 PM EET 
///
/// @brief    Class and functions to handle ANTEX files and related information
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

/// @class Antex
/// @see ftp://igs.org/pub/station/general/antex14.txt
class Antex
{
public:
  /// Let's not write this more than once.
  typedef std::ifstream::pos_type pos_type;

  /// Valid atx versions.
  enum class ATX_VERSION : char {
    v14, ///< Actually, the only valid !
    v13  ///< Used by EUREF but i can't find the dox
  };

  /// @brief Constructor from filename.
  explicit
  Antex(const char*);

  /// @brief Destructor (closing the file is not mandatory, but nevertheless)
  ~Antex() noexcept 
  { 
    if (__istream.is_open()) __istream.close();
  }
  
  /// @brief Copy not allowed !
  Antex(const Antex&) = delete;

  /// @brief Assignment not allowed !
  Antex& operator=(const Antex&) = delete;
  
  /// @brief Move Constructor.
  Antex(Antex&& a)
  noexcept(std::is_nothrow_move_constructible<std::ifstream>::value) = default;

  /// @brief Move assignment operator.
  Antex& operator=(Antex&& a) 
  noexcept(std::is_nothrow_move_assignable<std::ifstream>::value) = default;

  int
  get_antenna_pco(const ReceiverAntenna& ant_in, AntennaPcoList& pco_list,
                  bool must_match_serial=false) noexcept;
  int
  get_antenna_pco(int prn, SATELLITE_SYSTEM ss, 
                  const ngpt::datetime<ngpt::seconds>& at,
                  AntennaPcoList& pco_list) noexcept;

private:

  /// @brief Read the instance header, and assign (most of) the fields.
  int
  read_header() noexcept;

  /// @brief Read next antenna (from the stream)
  int
  read_next_antenna_type(ReceiverAntenna& antenna, char* c=nullptr) noexcept;
    
  /// @brief Skip (current) antenna info block
  int
  skip_rest_of_antenna() noexcept;
    
  /// @brief Try to match a given receiver antenna to a record in the atx file.
  int
  find_closest_antenna_match(const ReceiverAntenna& ant_in,
                             ReceiverAntenna& ant_out,
                             pos_type& ant_pos) noexcept;

  /// @brief Try to match a given satellite antenna, for a given epoch.
  int
  find_satellite_antenna(int, SATELLITE_SYSTEM,
                         const ngpt::datetime<ngpt::seconds>& at,
                         pos_type&) noexcept;

  std::string            __filename;    ///< The name of the antex file.
  std::ifstream          __istream;     ///< The infput (file) stream.
  SATELLITE_SYSTEM       __satsys;      ///< satellite system.
  ATX_VERSION            __version;     ///< Atx version (1.4).
  pos_type               __end_of_head; ///< Mark the 'END OF HEADER' field.
}; // Antex

} // ngpt

#endif
