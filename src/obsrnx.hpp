#ifndef __OBSERVATION_RINEX_HPP__
#define __OBSERVATION_RINEX_HPP__

#include <fstream>
#include <vector>
#include <map>
#include "satsys.hpp"
#include "antenna.hpp"
#include "gnssobs.hpp"
#include "ggdatetime/dtcalendar.hpp"
#ifdef DEBUG
#include "ggdatetime/datetime_write.hpp"
#endif

namespace ngpt
{

class ObservationRnx
{
public:
  /// Let's not write this more than once.
  typedef std::ifstream::pos_type pos_type;
  
  /// @brief Constructor from filename
  explicit
  ObservationRnx(const char*);
  
  /// @brief Destructor (closing the file is not mandatory, but nevertheless)
  ~ObservationRnx() noexcept 
  { 
    if (__istream.is_open()) __istream.close();
  }
  
  /// @brief Copy not allowed !
  ObservationRnx(const ObservationRnx&) = delete;

  /// @brief Assignment not allowed !
  ObservationRnx& operator=(const ObservationRnx&) = delete;
  
  /// @brief Move Constructor.
  ObservationRnx(ObservationRnx&& a)
  noexcept(std::is_nothrow_move_constructible<std::ifstream>::value) = default;

  /// @brief Move assignment operator.
  ObservationRnx& operator=(ObservationRnx&& a) 
  noexcept(std::is_nothrow_move_assignable<std::ifstream>::value) = default;

  /// @brief Set the stream to end of header
  void
  rewind() noexcept;

  /// @brief Max observables of any satellite system
  /// Loop through the observables of every satellite system, and return the
  /// max number of observables any satellite system can have
  int
  max_obs() const noexcept
  {
    std::size_t sz, max=0;
    for (auto const& [key, val] : __obstmap) if ((sz=val.size())>max) max=sz;
    return max;
  }

  /// @brief allocate and return a string with enough capacity to hold all
  ///        observations (per epoch and satellite)
  char*
  max_line() const noexcept;

#ifdef DEBUG
  void
  print_members() const noexcept
  {
    std::cout<<"\nfilename     :"<<__filename
             <<"\nSatellite Sys:"<<satsys_to_char(__satsys)
             <<"\nVersion      :"<<__version
             <<"\nMarker Name  :"<<__marker_name
             <<"\nMarker Number:"<<__marker_number
             <<"\nReceiver Sn  :"<<__receiver_number
             <<"\nReceiver Type:"<<__receiver_type
             <<"\nAntenna Type :"<<__antenna.__underlying_char__()
             <<"\nAntenna Sn   :"<<__antenna.has_serial()
             <<"\nApprox. Pos. :"<<__approx[0]<<", "<<__approx[1]<<", "<<__approx[2]
             <<"\nEccentricity :"<<__eccentricity[0]<<", "<<__eccentricity[1]<<", "<<__eccentricity[2]
             <<"\nRcv Clk Off  :"<<(__rcv_clk_offs_applied?"Yes":"No");
    for (auto const& [key, val] : __obstmap) {
      std::cout<<"\n"<<satsys_to_char(key);
      for (auto const& o : val) std::cout<<" "<<o.to_string();
    }
   }
#endif

private:
  
  /// @brief Read RINEX header; assign info
  int
  read_header() noexcept;

  int
  __resolve_epoch_304__(const char* cline, ngpt::modified_julian_day& mjd, 
    double& sec, int& flag, int& num_sats, double& rcvr_coff) noexcept;

  int
  __resolve_obstypes_304__(const char*) noexcept;

  std::string            __filename;    ///< The name of the file
  std::ifstream          __istream;     ///< The infput (file) stream
  SATELLITE_SYSTEM       __satsys;      ///< satellite system
  float                  __version;     ///< Rinex version (e.g. 3.4)
  pos_type               __end_of_head; ///< Mark the 'END OF HEADER' field
  ReceiverAntenna        __antenna;
  std::string            __marker_name,
                         __marker_number,
                         __receiver_number,
                         __receiver_type;
  double                 __approx[3],
                         __eccentricity[3];
  bool                   __rcv_clk_offs_applied{false};
  std::map<SATELLITE_SYSTEM, std::vector<ObservationCode>> 
                         __obstmap;

};// ObservationRnx

}// ngpt

#endif
