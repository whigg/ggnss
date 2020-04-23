#ifndef __NAVIGATION_RINEX_HPP__
#define __NAVIGATION_RINEX_HPP__

#include <fstream>
#include "ggdatetime/dtcalendar.hpp"
#include "satsys.hpp"
#ifdef DEBUG
#include "ggdatetime/datetime_write.hpp"
#endif

namespace ngpt
{

class NavDataFrame
{
public:

  /// @brief Null constructor
  NavDataFrame() noexcept
  {};

  /// @brief Assignment operator
  // NavDataFrame&
  // operator=(const NavDataFrame& other) noexcept;
  
  /// @brief Set from a RINEX 3.x navigation data block
  int
  set_from_rnx3(std::ifstream& inp) noexcept;
  
  /// @brief get SV coordinates (WGS84) from navigation block
  /// see IS-GPS-200H, User Algorithm for Ephemeris Determination
  int
  gps_ecef(double t_sec, double* state, double* Ek=nullptr)
  const noexcept;
  /// @brief get SV coordinates (WGS84) from navigation block
  /// see IS-GPS-200H, User Algorithm for Ephemeris Determination
  int
  gal_ecef(double t_sec, double* state, double* Ek=nullptr)
  const noexcept;
  /// @brief get SV coordinates (WGS84) from navigation block
  /// see IS-GPS-200H, User Algorithm for Ephemeris Determination
  int
  bds_ecef(double t_sec, double* state, double* Ek=nullptr)
  const noexcept;
  
  int
  glo_ecef(double tb_sod, double* state)
  const noexcept;
  

  template<typename T,
    typename = std::enable_if_t<T::is_of_sec_type>
    >
    ngpt::datetime<T>
    toe2date() const
  {
    switch (this->sys__) {
      case (SATELLITE_SYSTEM::gps)    : return gps_toe2date<T>();
      case (SATELLITE_SYSTEM::glonass): return glo_toe2date<T>();
      case (SATELLITE_SYSTEM::galileo): return gal_toe2date<T>();
      case (SATELLITE_SYSTEM::beidou) : return bds_toe2date<T>();
      case (SATELLITE_SYSTEM::sbas)   :
      case (SATELLITE_SYSTEM::qzss)   :
      case (SATELLITE_SYSTEM::irnss)  :
      case (SATELLITE_SYSTEM::mixed)  :
        std::cerr<<"\n[ERROR] NavDataFrame::toe2date() Cannot handle satellite system: "<<satsys_to_char(sys__);
        throw std::runtime_error("ERROR] NavDataFrame::toe2date() Cannot handle satellite system");
    }
    return datetime<T>::min();
  }

  /// @brief GPS time of ephemeris to datetime<T> instance
  ///
  /// Transform Time_of_Ephemeris from gps_week and sec of gps week to a valid
  /// datetime<T> instance.
  /// TimeSystem is GPST
  template<typename T>
    ngpt::datetime<T>
    gps_toe2date() const noexcept
  {
    ngpt::gps_week wk (static_cast<int>(data__[21]));
    ngpt::seconds  sc (static_cast<long>(data__[11]));
    return ngpt::datetime<T>(wk, sc);
  }
  
  /// @brief GST time of ephemeris to datetime<T> instance
  ///
  /// Transform Time_of_Ephemeris from gst_week and sec of GST week to a valid
  /// datetime<T> instance.
  /// TimeSystem is Galileo System Time (GST)
  /// 
  /// @warning In RINEX v3.x, GAL Week number is actually GPS Week! In the
  ///          RINEX specifications, it is noted that:
  ///          "The GAL week number is a continuous number, aligned to (and 
  ///          hence identical to) the continuous GPS week number used in the 
  ///          RINEX navigation message files."
  template<typename T>
    ngpt::datetime<T>
    gal_toe2date() const noexcept
  {return this->gps_toe2date<T>();}

  /// @brief BDS time of ephemeris to datetime<T> instance
  ///
  /// Transform Time_of_Ephemeris from bdt_week and sec of BDT week to a valid
  /// datetime<T> instance. Note that BDT started at zero at 1-Jan-2006 (hence
  /// GPS Week 1356).
  /// TimeSystem is BDS Time (BDT) System
  template<typename T>
    ngpt::datetime<T>
    bds_toe2date() const noexcept
  {
    ngpt::gps_week wk (static_cast<int>(data__[21])+1356L);
    ngpt::seconds  sc (static_cast<long>(data__[11]));
    return ngpt::datetime<T>(wk, sc);
  }
  
  /// @brief Time of ephemeris as datetime<T> instance in UTC or MT
  ///
  /// Transform the message frame time of the NavDataFrame instance to a
  /// datetime<seconds> instance. The message frame time is given as:
  /// data__[2]  : Message frame time(tk+nd*86400) in seconds of UTC week
  /// in RINEX v3.x This function will transform this to a datetime.
  ///
  /// The message frame time is given as seconds of UTC week, hence the function
  /// will use TimeOfClock (ToC) to resolve this to a date. The 
  /// pseudoalgorithm is something like:
  /// 1. resolve ToC to Week (ToC_wk) and DayOfWeek (ToC_dw)
  /// 2. resolve message frame time (Tb) to DayOfWeek (Tb_dw) and SecondsOfWeek
  ///    (Tb_sw)
  /// 3. resulting date is ToC.MJD - (ToC_dw - Tb_dw) and time Tb_sw
  /// 
  /// @result message frame time as datetime<seconds> instance in UTC(SU)
  template<typename T>
    ngpt::datetime<T>
    glo_toe2date() const noexcept
  {
    long sw_toc;
    // transform Time of Clock (UTC) to GPS Week and SoW
    toc__.as_gps_wsow(sw_toc);
    // day of week for ToC
    long dw_toc = sw_toc / 86400L;
    // day of week for ToE
    long sw_toe = static_cast<long>(data__[2]);
    long dw_toe = sw_toe / 86400L;
#ifdef DEBUG
    assert(std::abs(data__[2]-sw_toe)<1e-15);
#endif
    // days difference (if any)
    int offset = dw_toc - dw_toe;
    // seconds of day (for ToE)
    long sd_toe = sw_toe % 86400L;
#ifdef DEBUG
    double test = data__[2] - dw_toe*86400e0;
    bool health = (int)data__[6];
    if (!health) {
      assert(std::abs(test-(double)sd_toe)<1e-15);
      /*if (std::abs(test-(double)sd_toe)>1e-15) {
        std::cerr<<"\n[ERROR] ToE incorrect! PRN:"<<satsys_to_char(sys__)<<prn__;
        ngpt::datetime<T> tmp(toc__.mjd()-modified_julian_day(offset),
          ngpt::cast_to<seconds, T>(seconds(sd_toe)));
        std::cerr<<"\n        ToC "<<ngpt::strftime_ymd_hms<ngpt::seconds>(toc__);
        std::cerr<<"\n        ToE "<<ngpt::strftime_ymd_hms<T>(tmp)<<" from nav: "<<data__[2];
        std::cerr<<"\nSD: "<<sd_toe<<" Test: "<<test;
      }*/
    } else {
      if (std::abs(test-(double)sd_toe)<1e-15) {
        std::cerr<<"\n[WARNING] NavDataFrame::glo_toe2date() ToE not set correctly for unhealhty satellite: "<<satsys_to_char(sys__)<<prn__;
      }
    }
#endif
    // form the datetime instance
    return ngpt::datetime<T>(toc__.mjd()-modified_julian_day(offset),
      ngpt::cast_to<seconds, T>(seconds(sd_toe)));
  }

  int
  gps_dtsv(double t_sec, double& dt_sv, double* Ek_in=nullptr) const noexcept;
  
  int
  gal_dtsv(double t_sec, double& dt_sv, double* Ek_in=nullptr) const noexcept;
  
  int
  bds_dtsv(double t_sec, double& dt_sv, double* Ek_in=nullptr) const noexcept;
  
  int
  glo_dtsv(double t_sec, double& dtsv) const noexcept;

  template<typename T>
    inline double
    ref2toe(const ngpt::datetime<T>& t) const noexcept
  {
    double tsec = t.sec().to_fractional_seconds();
    int mjd_diff = t.mjd().as_underlying_type() - toe__.mjd().as_underlying_type();
    tsec += 86400e0 * (double)mjd_diff;
    return tsec;
  }
  template<typename T>
    inline double
    ref2toc(const ngpt::datetime<T>& t) const noexcept
  {
    double tsec = t.sec().to_fractional_seconds();
    int mjd_diff = t.mjd().as_underlying_type() - toc__.mjd().as_underlying_type();
    tsec += 86400e0 * (double)mjd_diff;
    return tsec;
  }
  
  template<typename T>
    int
    gps_stateNclock(ngpt::datetime<T> t, double* state, double& dt)
    const noexcept
  {
    int status = 0;
    // reference time for SV position computation, is ToE
    // datetime<T> toe (this->gps_toe2date<T>());
    // double toe_sec = toe.sec().to_fractional_seconds();
    // double t_sec   = t.sec().to_fractional_seconds();
    // reference t to ToE
    //if (t.mjd()>toe__.mjd()) {
    //  t_sec += 86400e0;
    //} else if (t.mjd()<toe__.mjd()) {
    //  t_sec = t_sec - 86400e0;
    // }
    double t_sec = this->ref2toe<T>(t);
    if ((status=gps_ecef(t_sec, state))) return status;
    // dt from ToC
    // auto   dt_ = ngpt::delta_sec(t, toc__);
    // double dti = dt_.to_fractional_seconds();
    t_sec = this->ref2toc<T>(t);
    status=gps_dtsv(t_sec, dt);
    return status;
  }
  
  template<typename T>
    int
    gal_stateNclock(ngpt::datetime<T> t, double* state, double& dt)
    const noexcept
  {
    int status = 0;
    // reference time for SV position computation, is ToE
    // datetime<T> toe (this->gal_toe2date<T>());
    // double toe_sec = toe.sec().to_fractional_seconds();
    // double t_sec   = t.sec().to_fractional_seconds();
    // reference t to ToE
    // if (t.mjd()>toe__.mjd()) {
    //  t_sec += 86400e0;
    //} else if (t.mjd()<toe__.mjd()) {
    //  t_sec = t_sec - 86400e0;
    //}
    double t_sec = this->ref2toe<T>(t);
    if ((status=gal_ecef(t_sec, state))) return status;
    // dt from ToC
    // auto   dt_ = ngpt::delta_sec(t, toc__);
    // double dti = dt_.to_fractional_seconds();
    t_sec = this->ref2toc<T>(t);
    status=gal_dtsv(t_sec, dt);
    return status;
  }
  
  template<typename T>
    int
    bds_stateNclock(ngpt::datetime<T> t, double* state, double& dt)
    const noexcept
  {
    int status = 0;
    // reference time for SV position computation, is ToE
    // datetime<T> toe (this->bds_toe2date<T>());
    // double toe_sec = toe.sec().to_fractional_seconds();
    // double t_sec   = t.sec().to_fractional_seconds();
    // reference t to ToE
    // if (t.mjd()>toe__.mjd()) {
    //  t_sec += 86400e0;
    //} else if (t.mjd()<toe__.mjd()) {
    //  t_sec = t_sec - 86400e0;
    //}
    double t_sec = this->ref2toe<T>(t);
    if ((status=bds_ecef(t_sec, state))) return status;
    // dt from ToC
    //auto   dt_ = ngpt::delta_sec(t, toc__);
    //double dti = dt_.to_fractional_seconds();
    t_sec = this->ref2toc<T>(t);
    status=bds_dtsv(t_sec, dt);
    return status;
  }
  
  /// @brief Compute SV centre of mass state vector in ECEF PZ90 frame at
  ///        epoch epoch using the simplified algorithm
  /// @param[in] epoch The time in UTC for which we want the SV state
  /// @param[out] The SV centre of mass state vector in meters, meters/sec
  template<typename T>
    int
    glo_stateNclock(ngpt::datetime<T> t, double* state, double& dt)
    const noexcept
  {
    int status = 0;
    constexpr seconds secmt (10800L);
    // t_i and t_b to MT
    // t.add_seconds(secmt);
    // ngpt::datetime<T> tb = this->glo_toe2date<T>(true);
    // double t_sec = t.sec().to_fractional_seconds();
    // double tb_sec = tb.sec().to_fractional_seconds();
    // reference ti and tb to the same day (it may happen? that ti and tb are
    // in different days)
    //if (t.mjd()>toe__.mjd()) {
    //  t_sec += 86400e0;
    //} else if (t.mjd()<toe__.mjd()) {
    //  t_sec = t_sec - 86400e0;
    //}
    double t_sec = this->ref2toe<T>(t);
    if ( (status=glo_ecef(t_sec, state)) ) return status;
    if ( (status=glo_dtsv(t_sec, dt)) ) return status;
    return 0;
  }
  
  template<typename T>
    int
    gps_dtsv(const ngpt::datetime<T>& epoch, double& dtsv)
    const noexcept
  {
    T dsec = ngpt::delta_sec<T, ngpt::seconds>(epoch, toc__);
    double sec = static_cast<double>(dsec.as_underlying_type());
    return this->gps_dtsv(sec, dtsv);
  }

  double
  data(int idx) const noexcept { return data__[idx]; }

  double&
  data(int idx) noexcept { return data__[idx]; }

  SATELLITE_SYSTEM
  system() const noexcept { return sys__; }

  int
  prn() const noexcept { return prn__; }

  ngpt::datetime<ngpt::seconds>
  toc() const noexcept { return toc__; }

  template<typename T,
    typename = std::enable_if_t<T::is_of_sec_type>
    >
    ngpt::datetime<T>
  toc() const noexcept { return toc__.cast_to<T>(); }

  void
  set_toc(ngpt::datetime<ngpt::seconds> d) noexcept {toc__=d;}

  /* NEW FUNCTIONS */
  // URA for gps
  float ura() const noexcept;
  int sv_health() const noexcept;
  int fit_interval() const noexcept;

  float sisa() const noexcept;
  int iod_nav() const noexcept;

private:
  SATELLITE_SYSTEM              sys__{};     ///< Satellite system
  int                           prn__{};     ///< PRN as in Rinex 3x
  ngpt::datetime<ngpt::seconds> toc__{};     ///< Time of clock
  ngpt::datetime<ngpt::seconds> toe__{};     ///< Time of ephemeris
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
