#ifndef __SP3C_IGS_FILE__
#define __SP3C_IGS_FILE__

#include <fstream>
#include "ggdatetime/dtcalendar.hpp"
#include "satsys.hpp"
#ifdef DEBUG
#include "ggdatetime/datetime_write.hpp"
#endif

namespace ngpt
{

class Sp3c
{
public:
  /// Let's not write this more than once.
  typedef std::ifstream::pos_type pos_type;
  
  /// @brief Constructor from filename
  explicit
  Sp3c(const char*);
  
  /// @brief Destructor (closing the file is not mandatory, but nevertheless)
  ~Sp3c() noexcept 
  { 
    if (__istream.is_open()) __istream.close();
  }
  
  /// @brief Copy not allowed !
  Sp3c(const Sp3c&) = delete;

  /// @brief Assignment not allowed !
  Sp3c& operator=(const Sp3c&) = delete;
  
  /// @brief Move Constructor.
  Sp3c(Sp3c&& a)
  noexcept(std::is_nothrow_move_constructible<std::ifstream>::value) = default;

  /// @brief Move assignment operator.
  Sp3c& operator=(Sp3c&& a) 
  noexcept(std::is_nothrow_move_assignable<std::ifstream>::value) = default;

  /// @brief Set the stream to end of header or anywhere else
  void
  rewind(pos_type pos=-1) noexcept;

#ifdef DEBUG
  void
  print_members() const noexcept
  {
    std::cout<<"\nfilename     :"<<__filename
             <<"\nVersion      :"<<version__
             <<"\nStart Epoch  :"<<ngpt::strftime_ymd_hms(start_epoch__)
             <<"\n# Epochs     :"<<num_epochs__
             <<"\nCoordinate S :"<<crd_sys__
             <<"\nOrbit Type   :"<<orb_type__
             <<"\nAgency       :"<<agency__
             <<"\nTime System  :"<<time_sys__
             <<"\nInterval     :"<<interval__.to_fractional_seconds();
   }
#endif

private:
  /// @brief Read sp3c header; assign info
  int
  read_header() noexcept;

  /// @brief clear the instance stream and return the exit_status
  int
  clear_stream(int exit_status) noexcept;

  std::string            __filename;    ///< The name of the file
  std::ifstream          __istream;     ///< The infput (file) stream
  char                   version__;     ///< the version 'c' or 'd'
  ngpt::datetime<ngpt::microseconds>
                         start_epoch__; ///< Start epoch
  int                    num_epochs__;  ///< Number of epochs in file
  std::string            crd_sys__,     ///< Coordinate system
                         orb_type__,    ///< Orbit type
                         agency__,      ///< Agency
                         time_sys__;    ///< Time system
  ngpt::microseconds     interval__;    ///< Epoch interval
  SATELLITE_SYSTEM       __satsys;      ///< satellite system
  pos_type               __end_of_head; ///< Mark the 'END OF HEADER' field
};// Sp3c

}// ngpt

#endif
