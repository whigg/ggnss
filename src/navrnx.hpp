#ifndef __NAVIGATION_RINEX_HPP__
#define __NAVIGATION_RINEX_HPP__

#include <fstream>
#include "ggdatetime/dtcalendar.hpp"
#include "satsys.hpp"

namespace ngpt
{

/// GPS:                   : Time of Clock in GPS time
///             data__[0]  : SV clock bias in seconds
///             data__[1]  : SV clock drift in m/sec
///             data__[2]  : SV clock drift rate in m/sec^2
///             data__[3]  : IODE Issue of Data, Ephemeris
///             data__[4]  : Crs (meters)
///             data__[5]  : Deltan (radians/sec)
///             data__[6]  : M0 (radians)                                ---- 1
///             data__[7]  : Cuc (radians)
///             data__[8]  : e Eccentricity
///             data__[9]  : Cus (radians)
///             data__[10] : sqrt(A) (sqrt(m))                           ---- 2
///             data__[11] : Toe Time of Ephemeris (sec of GPS week)
///             data__[12] : Cic (radians)
///             data__[13] : OMEGA0 (radians)
///             data__[14] : Cis (radians)                               ---- 3
///             data__[15] : i0 (radians)
///             data__[16] : Crc (meters)
///             data__[17] : omega (radians)
///             data__[18] : OMEGADOT (radians/sec)                      ---- 4
///             data__[19] : IDOT (radians/sec)
///             data__[20] : Codes on L2 channel
///             data__[21] : GPS Week (to go with TOE)
///             data__[22] : L2 P data flag                              ---- 5
///             data__[23] : SV accuracy (meters)
///             data__[24] : SV health (bits 17-22 w 3 sf 1)
///             data__[25] : TGD (seconds)
///             data__[26] : IODC Issue of Data, Clock                   ---- 6
///             data__[27] : Transmission time of message
///             data__[28] : Fit Interval in hours
///             data__[29] : empty
///             data__[30] : empty                                       ---- 7
/*
  /// WGS 84 value of the earth's gravitational constant for GPS user
  constexpr double mi_gps {3.986005e14};
  /// WGS 84 value of the earth's rotation rate
  constexpr double OMEGAE_dot {7.2921151467e-5};
  int
  gps_ecef(double tin sow, double& x, double& y, double& z) const noexcept
  {
    status = 0;
    constexpr double LIMIT  {1e-14};         // Limit for solving (iteratively) the Kepler equation for the eccentricity anomaly
    double A  (data__[10]*data[10]);         // Semi-major axis
    double n0 (std::sqrt(mi_gps/(A*A*A)));   // Computed mean motion (rad/sec)
    double tk (t-data__[11]);                // Time from ephemeris reference epoch
    if (tk>302400e0) tk -= 604800e0;
    if (tk<302400e0) tk += 604800e0;
    double n  (n0+data__[5]);                // Corrected mean motion
    double Mk (data__[6]+n*tk);              // Mean anomaly
    double E  (Mk);                          // tmp E_k
    double Ek (0e0);
    double e  (data__[8]);
    for (int i=0; std::abs(E-Ek)>LIMIT && i<1001; i++) {
      Ek = E;
      E = std::sin(Ek)*e+Mk;
    }
    if (i>=1000) return 1;
    Ek = E;
    double sinE    (std::sin(E));
    double cosE    (std::cos(E));
    double ecosEm1 (1e0-e*cosE);
    double vk_ar   ((std::sqrt(1e0-e*e)*sinE)/ecosEm1);
    double vk_pr   ((cosE-e)/ecosEm1);
    double vk      (std::atan2(vk_ar, vk_pr)); // True Anomaly
    double cosVk   (std::cos(vk));
    Ek           = std::acos((e+cosVk)/(1e0+e*cosVk)); // Eccentric Anomaly
    double Fk      (vk+data__[17]);
    double sin2F   (std::sin(2e0*Fk));
    double cos2F   (std::cos(2e0*Fk));
    double duk     (data__[9]*sin2F  + data__[7]*cos2F);  // Argument of Latitude Correction
    double drk     (data__[4]*sin2F  + data__[16]*cos2F); // Radius Correction
    double dik     (data__[14]*sin2F + data__[12]*cos2F); // Inclination Correction
    double uk      (Fk + duk); // Corrected Argument of Latitude
    double rk      (A*(1e0-e*std::cos(Ek))+drk); // Corrected Radius
    double ik      (data__[15]+dik+data__[19]*tk); // Corrected Inclination
    // Positions in orbital plane
    double xk_dot  (rk*std::cos(uk));
    double yk_dot  (rk*std::sin(uk));
    double omega_k (data__[13]+(data__[18]-OMEGAE_dot)*tk-OMEGAE_dot*data__[11]); // Corrected longitude of ascending node
    double sinOk   (std::sin(omega_k));
    double cosOk   (std::cos(omega_k));
    double cosik   (std::cos(ik));
    x = xk_dot*cosOk - yk_dot*sinOk*cosik;
    y = xk_dot*sinOk - yk_dot*cosOk*cosik;
    z = yk_dot*std::sin(ik);
    return;
  }
*/
/// GALILEO:    data__[0]  : Time of Clock in GAL time
///             data__[0]  : SV clock bias in seconds, aka af0
///             data__[1]  : SV clock drift in m/sec, aka af1
///             data__[2]  : SV clock drift rate in m/sec^2, aka af2
///             data__[x]  : IODnav Issue of Data of the nav batch
///             data__[x]  : Crs(meters)
///             data__[x]  : Deltan (radians/sec)
///             data__[x]  : M0(radians)
/// GLONASS:    data__[0]  : Time of Clock in UTC time
///             data__[0]  : SV clock bias in sec, aka -TauN
///             data__[1]  : SV relative frequency bias, aka +GammaN
///             data__[2]  : Message frame time(tk+nd*86400) in seconds of UTC week
///             data__[x]  : Satellite position X (km)
///             data__[x]  : velocity X dot (km/sec)
///             data__[x]  : X acceleration (km/sec2)
///             data__[x]  : health(0=healthy, 1=unhealthy)(MSB of 3-bit Bn)
/// QZSS:       data__[0]  : Time of Clock
///             data__[0]  : SV clock bias in seconds
///             data__[1]  : SV clock drift in m/sec
///             data__[2]  : SV clock drift rate in m/sec^2
///             data__[x]  : IODE Issue of Data, Ephemeris
///             data__[x]  : Crs(meters)
///             data__[x]  : Delta n (radians/sec)
///             data__[x]  : M0 (radians)
/// BeiDou:     data__[0]  : Time of Clock in BDT time
///             data__[0]  : SV clock bias in seconds
///             data__[1]  : SV clock drift in m/sec
///             data__[2]  : SV clock drift rate in m/sec^2
///             data__[x]  : AODE Age of Data, Ephemeris (as specified in 
///                          BeiDou ICD Table Section 5.2.4.11 Table 5-8) and 
///                          field range is: 0-31
///             data__[x]  : Crs (meters)
///             data__[x]  : Delta n (radians/sec)
///             data__[x]  : M0 (radians) 
/// SBAS:       data__[0]  : Time of Clock in GPS time 
///             data__[0]  : SV clock bias in sec, aka aGf0
///             data__[1]  : SV relative frequency bias, aka aGf1
///             data__[2]  : Transmission time of message (start of the 
///                          message) in GPS seconds of the week
///             data__[x]  : Satellite position X (km)
///             data__[x]  : velocity X dot (km/sec)
///             data__[x]  : X acceleration (km/sec2)
///             data__[x]  : Health:SBAS:See Section 8.3.3 bit mask for:
///                          health, health availability and User Range Accuracy. 
/// IRNSS:      data__[0]  : Time of Clock in IRNSS time
///             data__[0]  : SV clock bias in seconds
///             data__[1]  : SV clock drift in m/sec
///             data__[2]  : SV clock drift rate in m/sec^2
///             data__[x]  : IODEC Issue of Data, Ephemeris and Clock
///             data__[x]  : Crs(meters)
///             data__[x]  : Deltan (radians/sec)
///             data__[x]  : M0(radians)
class NavDataFrame
{
public:

  /// @brief Null constructor
  NavDataFrame() noexcept
  {};

  /// @brief Set from a RINEX 3.x navigation data block
  int
  set_from_rnx3(std::ifstream& inp) noexcept;

  /// @brief Return GPS satellite health. This is an integer, following Table 19,
  ///        Section 6.12 in RINEX 3.04
  /// @return An integer; if 0, satellite is healthy, else check Table 19
  int
  gps_SatHealth() const noexcept
  {return static_cast<int>(data__[24]);}
  /// @brief Get the GPS week
  int
  gps_GpsWeek() const noexcept
  {return static_cast<int>(data__[21]);}
  /// @brief Get the ToE (aka time of ephemeris) in (sec of GPS week)
  int
  gps_ToE_sec() const noexcept
  {return static_cast<int>(data__[11]);}
  /// @brief get SV coordinates (WGS84) from navigation block
  /// see IS-GPS-200H, User Algorithm for Ephemeris Determination
  
  double
  data(int idx) const noexcept { return data__[idx]; }
  
private:
  SATELLITE_SYSTEM              sys__{};     ///< Satellite system
  int                           prn__{};     ///< PRN as in Rinex 3x
  ngpt::datetime<ngpt::seconds> toc__{};     ///< Time of clock
  double                        data__[31]{};///< Data block
};

class NavigationRnx
{
public:
  /// Let's not write this more than once.
  typedef std::ifstream::pos_type pos_type;
  
  /// @brief Constructor from filename
  explicit
  NavigationRnx(const char*);
  
  /// @brief Destructor (closing the file is not mandatory, but nevertheless)
  ~NavigationRnx() noexcept 
  { 
    if (__istream.is_open()) __istream.close();
  }
  
  /// @brief Copy not allowed !
  NavigationRnx(const NavigationRnx&) = delete;

  /// @brief Assignment not allowed !
  NavigationRnx& operator=(const NavigationRnx&) = delete;
  
  /// @brief Move Constructor.
  NavigationRnx(NavigationRnx&& a)
  noexcept(std::is_nothrow_move_constructible<std::ifstream>::value) = default;

  /// @brief Move assignment operator.
  NavigationRnx& operator=(NavigationRnx&& a) 
  noexcept(std::is_nothrow_move_assignable<std::ifstream>::value) = default;

  /// @brief Read, resolved and store next navigation data block
  int
  read_next_record(NavDataFrame&) noexcept;
  
  /// @brief Check the first line of the following message to get the sat. sys
  ngpt::SATELLITE_SYSTEM
  peak_satsys(int&) noexcept;

  /// @brief Read and skip the next navigation message
  int
  ignore_next_block() noexcept;

  /// @brief Set the stream to end of header
  void
  rewind() noexcept;

private:
  
  /// @brief Read RINEX header; assign info
  int
  read_header() noexcept;

  std::string            __filename;    ///< The name of the file
  std::ifstream          __istream;     ///< The infput (file) stream
  SATELLITE_SYSTEM       __satsys;      ///< satellite system
  float                  __version;     ///< Rinex version (e.g. 3.4)
  pos_type               __end_of_head; ///< Mark the 'END OF HEADER' field
};// NavigationRnx

}// ngpt

#endif
