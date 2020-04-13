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
constexpr double RNXOBS_MISSING_VAL = -999.99;
struct RawRnxObs__ {
  double __val;
  int __lli;
  int __ssi;
  int resolve(char *str) noexcept;
};

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
    if (__buf) {
      __buf_sz = 0;
      delete[] __buf;
    }
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

  int
  read_next_epoch();

private:
  
  /// @brief Read RINEX header; assign info
  int
  read_header() noexcept;

  int
  __resolve_epoch_304__(const char* cline, ngpt::modified_julian_day& mjd, 
    double& sec, int& flag, int& num_sats, double& rcvr_coff) noexcept;

  int
  collect_epoch(int numsats, const std::vector<SATELLITE_SYSTEM>& sysvec)
  noexcept;

  int
  __resolve_obstypes_304__(const char*) noexcept;

  std::string            __filename;        ///< The name of the file
  std::ifstream          __istream;         ///< The infput (file) stream
  SATELLITE_SYSTEM       __satsys;          ///< satellite system
  float                  __version;         ///< Rinex version (e.g. 3.4)
  pos_type               __end_of_head;     ///< Mark the 'END OF HEADER' field
  ReceiverAntenna        __antenna;         ///< Antenna
  std::string            __marker_name,     ///< Marker Name
                         __marker_number,   ///< Marker number
                         __receiver_number, ///< Receiver Serial
                         __receiver_type;   ///< Receiver Type
                        ///< Geocentric approximate marker position (meters)
  double                __approx[3] = {0e0, 0e0, 0e0},
                        ///< [0] Antenna height: Height of the antenna reference
                        ///<     point (ARP) above the marker
                        ///< [1] Horizontal eccentricity of ARP relative to the
                        ///<     marker - east
                        ///< [2] Horizontal eccentricity of ARP relative to the
                        ///<     marker - north
                        __eccentricity[3] = {0e0, 0e0, 0e0}; 
                        ///< If true, Epoch, code, and phase are corrected by
                        ///< applying the real-time-derived receiver clock
  bool                  __rcv_clk_offs_applied{false};
                        ///< Map of Observables per Satellite System; key is 
                        ///< satellite system, value is a vector of observation 
                        ///< descriptors (Type, Band, Attribute)
  std::map<SATELLITE_SYSTEM, std::vector<ObservationCode>> 
                         __obstmap;
                        ///< A char buffer which can hold an observation line 
                        ///< with maximum number of observables
  char*                 __buf{nullptr};
                        ///< Length of __buf, aka length of maximum observation 
                        ///< line in file (this->max_obs()*16+4)
  std::size_t           __buf_sz{0}; 

};// ObservationRnx

}// ngpt

#endif
