#ifndef __OBSERVATION_RINEX_HPP__
#define __OBSERVATION_RINEX_HPP__

#include <fstream>
#include <vector>
#include <map>
#include "satsys.hpp"
#include "antenna.hpp"
#include "gnssobsrv.hpp"
#include "satellite.hpp"
#include "ggdatetime/dtcalendar.hpp"
#ifdef DEBUG
#include "ggdatetime/datetime_write.hpp"
#endif

namespace ngpt
{

///< A missing (observable) value in a RINEX file, will result in this value:
constexpr double RNXOBS_MISSING_VAL = -999.99;

/// @brief A simple struct to hold a RINEX observation
struct RawRnxObs__ {
  double __val; ///< the actual value (observation); if empty in RINEX it will
                ///< default to RNXOBS_MISSING_VAL
  int __lli;    ///< Loss-Of-Lock indicator should only be associated with the 
                ///< phase observation
  int __ssi;    ///< Signal Strength Indicator (SSI)
  int resolve(char *str) noexcept;
};

class ObservationRnx
{
  typedef std::pair<std::size_t, double> id_pair;
  typedef std::vector<id_pair>           vecof_idpair;
  
public:
  /// Let's not write this more than once.
  typedef std::ifstream::pos_type pos_type;
  
  /// @brief Constructor from filename
  explicit
  ObservationRnx(const char*);
  
  /// @brief Destructor
  ~ObservationRnx() noexcept;
  
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
  rewind() noexcept {__istream.seekg(__end_of_head);}

  /// @brief Max observables of any satellite system
  int
  max_obs() const noexcept;

  /// @brief Get approximate coordinates
  double x_approx() const noexcept {return __approx[0];}
  double y_approx() const noexcept {return __approx[1];}
  double z_approx() const noexcept {return __approx[2];}

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

  /// @brief Set map for reading RINEX observations
  std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>
  set_read_map(/*const */std::map<SATELLITE_SYSTEM, std::vector<GnssObservable>>& inmap, 
    bool skip_missing=false)
  const noexcept;

  /// @brief Collect all satellite observation for next epoch based on input map
  int
  read_next_epoch(std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>& mmap, std::vector<std::pair<ngpt::Satellite, std::vector<double>>>& satobs, int& sats, ngpt::modified_julian_day& mjd, double& secofday) noexcept;
  
  /// @brief Initialize a big enough vector to hold any epoch in current instance
  std::vector<std::pair<ngpt::Satellite, std::vector<double>>>
  initialize_epoch_vector(std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>& mmap)
  const noexcept;

private:

  /// @brief Read RINEX header; assign info
  int
  read_header() noexcept;

  /// @brief Resolve an epoch header line for a RINEX v.3x files
  int
  __resolve_epoch_304__(const char* cline, ngpt::modified_julian_day& mjd, 
    double& sec, int& flag, int& num_sats, double& rcvr_coff)
  noexcept;

  /// @brief Collect values (actually GnssObservable values) from a satellite
  ///        record line
  int
  sat_epoch_collect(const std::vector<vecof_idpair>& sysobs, int& prn, 
    std::vector<double>& vals)
  const noexcept;
  
  /// @brief Collect  
  int
  collect_epoch(int numsats, int& satscollected, 
    std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>& mmap,
    std::vector<std::pair<ngpt::Satellite, std::vector<double>>>& satobs)
  noexcept;

  /// @brief Resolve line(s) of type "SYS / # / OBS TYPES" as RINEX v3.04
  int
  __resolve_obstypes_304__(const char*) noexcept;

  /// @brief Given a GnssObservable collect info on how to read and formulate it
  ///        (aka observable index column and coefficient)
  vecof_idpair
  obs_getter(const GnssObservable& obs, SATELLITE_SYSTEM& sys, int& status)
  const noexcept;

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
