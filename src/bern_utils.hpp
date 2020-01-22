#ifndef __GNSS_BERN_UTILS_HPP__
#define __GNSS_BERN_UTILS_HPP__

#include <iostream>
#include <fstream>
#include "ggdatetime/dtcalendar.hpp"

namespace ngpt
{

/// @class BernSatellit
/// This class enables the reading and extraction of GNSS satellite information,
/// recorded in Bernese-specific files, named as 'SATELLIT.IXX'. For now, the
/// most important information concernes GLONASS satellites and is the
/// correspondance of GLONASS svn numbers to frequency channels.
/// An example of such a file, can be found at CODE's ftp repository, aka
/// ftp://ftp.aiub.unibe.ch/BSWUSER52/GEN/SATELLIT.I14
class BernSatellit
{
public:
  /// Let's not write this more than once.
  typedef std::ifstream::pos_type pos_type;

  /// @brief Constructor from filename.
  explicit
  BernSatellit(const char*);

  /// @brief Destructor (closing the file is not mandatory, but nevertheless)
  ~BernSatellit() noexcept 
  { 
    if (__istream.is_open()) __istream.close();
  }
  
  /// @brief Copy not allowed !
  BernSatellit(const BernSatellit&) = delete;

  /// @brief Assignment not allowed !
  BernSatellit& operator=(const BernSatellit&) = delete;
  
  /// @brief Move Constructor.
  BernSatellit(BernSatellit&& a)
  noexcept(std::is_nothrow_move_constructible<std::ifstream>::value) = default;

  /// @brief Move assignment operator.
  BernSatellit& operator=(BernSatellit&& a) 
  noexcept(std::is_nothrow_move_assignable<std::ifstream>::value) = default;

  /// @brief Get (GLONASS) satellite frequency channel, given svn
  int
  get_frequency_channel(int svn, const ngpt::datetime<ngpt::seconds>& eph,
    int& ifrqn, int& prn);

private:

  /// @brief Initialize the instance (open stream and validate format)
  int
  initialize() noexcept;

  std::string    __filename;    ///< The name of the file.
  std::ifstream  __istream;     ///< The infput (file) stream.
  pos_type       __part2;       ///< Mark the 'PART 2: ON-BOARD SENSORS' field.
}; // BernSatellit 

} // ngpt

#endif
