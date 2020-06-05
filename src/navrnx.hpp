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
  
  template<typename T,
    typename = std::enable_if_t<T::is_of_sec_type>
    >
    int
    stateNclock(const ngpt::datetime<T>& t, double* state, double& clock)
    const
  {
    switch (this->sys__) {
      case (SATELLITE_SYSTEM::gps)    : return gps_stateNclock(t, state, clock);
      case (SATELLITE_SYSTEM::glonass): return glo_stateNclock(t, state, clock);
      case (SATELLITE_SYSTEM::galileo): return gal_stateNclock(t, state, clock);
      case (SATELLITE_SYSTEM::beidou) : return bds_stateNclock(t, state, clock);
      case (SATELLITE_SYSTEM::sbas)   :
      case (SATELLITE_SYSTEM::qzss)   :
      case (SATELLITE_SYSTEM::irnss)  :
      case (SATELLITE_SYSTEM::mixed)  :
        std::cerr<<"\n[ERROR] NavDataFrame::stateNclock() Cannot handle satellite system: "<<satsys_to_char(sys__);
        throw std::runtime_error("ERROR] NavDataFrame::stateNclock) Cannot handle satellite system");
    }
    return 100;
  }
  
  int
  sv_health() const
  {
    switch (this->sys__) {
      case (SATELLITE_SYSTEM::glonass): return static_cast<int>(data__[6]);
      case (SATELLITE_SYSTEM::gps)    : 
      case (SATELLITE_SYSTEM::galileo): 
      case (SATELLITE_SYSTEM::beidou) : return static_cast<int>(data__[24]);
      case (SATELLITE_SYSTEM::sbas)   :
      case (SATELLITE_SYSTEM::qzss)   :
      case (SATELLITE_SYSTEM::irnss)  :
      case (SATELLITE_SYSTEM::mixed)  :
        std::cerr<<"\n[ERROR] NavDataFrame::sv_health() Cannot handle satellite system: "<<satsys_to_char(sys__);
        throw std::runtime_error("ERROR] NavDataFrame::sv_clock() Cannot handle satellite system");
    }
    return 100;
  }
  
  long 
  fit_interval() const
  {
    switch (this->sys__) {
      case (SATELLITE_SYSTEM::glonass): return 15*60L;
      case (SATELLITE_SYSTEM::gps)    : return gps_fit_interval();
      case (SATELLITE_SYSTEM::galileo): 
      case (SATELLITE_SYSTEM::beidou) : return 4L*60*60;
      case (SATELLITE_SYSTEM::sbas)   :
      case (SATELLITE_SYSTEM::qzss)   :
      case (SATELLITE_SYSTEM::irnss)  :
      case (SATELLITE_SYSTEM::mixed)  :
        std::cerr<<"\n[ERROR] NavDataFrame::fit_interval() Cannot handle satellite system: "<<satsys_to_char(sys__);
        throw std::runtime_error("ERROR] NavDataFrame::fit_interval() Cannot handle satellite system");
    }
    return 100;
  }

  /// @brief Reference t to the beggining of ToE (aka 00:00:00 of ToE)
  /// @param[in] t Datetime instance to reference to ToE
  /// @return Seconds of t since 00:00:00ToE
  template<typename T>
    inline double
    ref2toe(const ngpt::datetime<T>& t) const noexcept
  {
    double tsec = t.sec().to_fractional_seconds();
    int mjd_diff = t.mjd().as_underlying_type() - toe__.mjd().as_underlying_type();
    if (mjd_diff) tsec += 86400e0 * (double)mjd_diff;
    return tsec;
  }

  /// @brief Reference t to the beggining of ToE (aka 00:00:00 of ToC)
  /// @param[in] t Datetime instance to reference to ToC
  /// @return Seconds of t since 00:00:00ToC
  template<typename T>
    inline double
    ref2toc(const ngpt::datetime<T>& t) const noexcept
  {
    double tsec = t.sec().to_fractional_seconds();
    int mjd_diff = t.mjd().as_underlying_type() - toc__.mjd().as_underlying_type();
    if (mjd_diff) tsec += 86400e0 * (double)mjd_diff;
    return tsec;
  }
  
  template<typename T>
    int
    gps_stateNclock(ngpt::datetime<T> t, double* state, double& dt)
    const noexcept
  {
    int status = 0;
    double t_sec = this->ref2toe<T>(t);
    // if ((status=gps_ecef(t_sec, state))) return status;
    if ((status=this->kepler2state<SATELLITE_SYSTEM::gps>(t_sec, state))) return status;
    t_sec = this->ref2toc<T>(t);
    status = sv_clock<SATELLITE_SYSTEM::gps>(t_sec, dt);
    //status=gps_dtsv(t_sec, dt);
    return status;
  }
  
  template<typename T>
    int
    gal_stateNclock(ngpt::datetime<T> t, double* state, double& dt)
    const noexcept
  {
    int status = 0;
    double t_sec = this->ref2toe<T>(t);
    // if ((status=gal_ecef(t_sec, state))) return status;
    if ((status=this->kepler2state<SATELLITE_SYSTEM::galileo>(t_sec, state))) return status;
    t_sec = this->ref2toc<T>(t);
    status=sv_clock<SATELLITE_SYSTEM::galileo>(t_sec, dt);
    // status=gal_dtsv(t_sec, dt);
    return status;
  }
  
  template<typename T>
    int
    bds_stateNclock(ngpt::datetime<T> t, double* state, double& dt)
    const noexcept
  {
    int status = 0;
    double t_sec = this->ref2toe<T>(t);
    // if ((status=bds_ecef(t_sec, state))) return status;
    if ((status=this->kepler2state<SATELLITE_SYSTEM::beidou>(t_sec, state))) return status;
    t_sec = this->ref2toc<T>(t);
    status=sv_clock<SATELLITE_SYSTEM::beidou>(t_sec, dt);
    // status=bds_dtsv(t_sec, dt);
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
    double t_sec = this->ref2toe<T>(t);
    if ((status=glo_ecef(t_sec, state))) return status;
    if ((status=glo_clock(t_sec, dt))) return status;
    return 0;
  }
  
  double
  data(int idx) const noexcept {return data__[idx];}

  double&
  data(int idx) noexcept {return data__[idx];}

  SATELLITE_SYSTEM
  system() const noexcept {return sys__;}

  int
  prn() const noexcept {return prn__;}
  
  SATELLITE_SYSTEM&
  system() noexcept {return sys__;}

  int&
  prn() noexcept {return prn__;}

  ngpt::datetime<ngpt::seconds>
  toc() const noexcept {return toc__;}

  template<typename T,
    typename = std::enable_if_t<T::is_of_sec_type>
    >
    ngpt::datetime<T>
  toc() const noexcept {return toc__.cast_to<T>();}
  
  template<typename T,
    typename = std::enable_if_t<T::is_of_sec_type>
    >
    ngpt::datetime<T>
  toe() const noexcept {return toe__.cast_to<T>();}

  void
  set_toc(ngpt::datetime<ngpt::seconds> d) noexcept {toc__=d;}

  /* NEW FUNCTIONS */
  int gps_fit_interval() const noexcept;
  float gps_ura() const noexcept;
  float gal_sisa() const noexcept;
  int gal_iod_nav() const noexcept;

private:
  SATELLITE_SYSTEM              sys__{};     ///< Satellite system
  int                           prn__{};     ///< PRN as in Rinex 3x
  ngpt::datetime<ngpt::seconds> toc__{};     ///< Time of clock
  ngpt::datetime<ngpt::seconds> toe__{};     ///< Time of ephemeris
  double                        data__[31]{};///< Data block

  /// @brief get SV coordinates and velocity (PZ90) refferenced to SV mass centre
  int
  glo_ecef(double tb_sod, double* state)
  const noexcept;
  
  /// @brief Transform Keplerian elements to SV coordinates
  ///
  /// This function template replaces functions gps_ecef, gal_ecef and bds_ecef
  /// For these satellite systems (aka GPS, GALILEO and BeiDou) the Navigation
  /// message as guven in RINEX v3.x holds the same Keplerian elements at the
  /// same position (that is at the same indexes in data__ array). Hence, we only
  /// need one function to turn these elements to SV position.
  /// For more information, see the (now obsolete function gps_ecef, gal_ecef and
  /// bds_ecef).
  /// @tparam S            The satellite system of the NavDataFrame; only works
  ///                      for systems: GPS, GALILEO and BeiDou
  /// @param[in]  t_sec    Time (in seconds) from ToE in the same system as ToE
  /// @param[out] state    SV x,y,z -components of antenna phase center position
  ///                      in the ECEF coordinate system in meters; the 
  ///                      state array must have length >=3
  /// @param[out] Ek_ptr   If pointer is not null, it will hold (at output) the 
  ///                      value of the computed Ek aka Eccentric Anomaly
  /// @return Anything other than 0 denotes an error
  ///
  /// @note Input parameter t_sec should be referenced to the begining of ToE
  ///       (aka start of ToE day) at the same time-scale
  template<SATELLITE_SYSTEM S>
    int
    kepler2state(double t_sec, double* state, double* Ek_ptr=nullptr)
    const noexcept
  {
    int status = 0;

    constexpr double MI_SYS     = ngpt::satellite_system_traits<S>::mi();
    constexpr double OMEGAE_SYS = ngpt::satellite_system_traits<S>::omegae_dot();
    constexpr double LIMIT  {1e-14};       //  Limit for solving (iteratively) 
                                           //+ the Kepler equation for the 
                                           //+ eccentricity anomaly
    const double A  (data__[10]*data__[10]);     //  Semi-major axis
    const double n0 (std::sqrt(MI_SYS/(A*A*A))); //  Computed mean motion (rad/sec)
    const double toe_sec = toe__.sec().to_fractional_seconds();
    const double tk (t_sec-toe_sec);
#ifdef DEBUG
    if (tk<-302400e0 || tk>302400e0) {
      std::cerr<<"\n[ERROR] NavDataFrame::kepler2state Delta-seconds are off! WTF?";
      return -1;
    }
#endif
    const double n  (n0+data__[5]);              //  Corrected mean motion
    const double Mk (data__[6]+n*tk);            //  Mean anomaly

    // Solve (iteratively) Kepler's equation for Ek
    double E  (Mk);
    double Ek (0e0);
    const double e  (data__[8]);
    int i;
    for (i=0; std::abs(E-Ek)>LIMIT && i<1001; i++) {
      Ek = E;
      E = std::sin(Ek)*e+Mk;
    }
    if (i>=1000) return 1;
    Ek = E;

    if (Ek_ptr) *Ek_ptr = Ek;

    const double sinE    (std::sin(E));
    const double cosE    (std::cos(E));
    const double ecosEm1 (1e0-e*cosE);
    const double vk_ar   ((std::sqrt(1e0-e*e)*sinE)/ecosEm1);
    const double vk_pr   ((cosE-e)/ecosEm1);
    const double vk      (std::atan2(vk_ar, vk_pr));            // True Anomaly

    // Second Harmonic Perturbations
    const double Fk      (vk+data__[17]);                       // Argument of Latitude
    const double sin2F   (std::sin(2e0*Fk));
    const double cos2F   (std::cos(2e0*Fk));
    const double duk     (data__[9]*sin2F  + data__[7]*cos2F);  // Argument of Latitude
                                                                //+ Correction
    const double drk     (data__[4]*sin2F  + data__[16]*cos2F); // Radius Correction
    const double dik     (data__[14]*sin2F + data__[12]*cos2F); // Inclination Correction

    const double uk      (Fk + duk);                            // Corrected Argument 
                                                                //+ of Latitude
    const double rk      (A*(1e0-e*cosE)+drk);                  // Corrected Radius
    const double ik      (data__[15]+dik+data__[19]*tk);        // Corrected Inclination
                                  
    // Positions in orbital plane
    const double xk_dot  (rk*std::cos(uk));
    const double yk_dot  (rk*std::sin(uk));
    
    // Corrected longitude of ascending node
    const double omega_k (data__[13]+(data__[18]-OMEGAE_SYS)*tk-OMEGAE_SYS*data__[11]);
    const double sinOk   (std::sin(omega_k));
    const double cosOk   (std::cos(omega_k));
    const double cosik   (std::cos(ik));
    
    state[0] = xk_dot*cosOk - yk_dot*sinOk*cosik;
    state[1] = xk_dot*sinOk + yk_dot*cosOk*cosik;
    state[2] = yk_dot*std::sin(ik);

    // all done
    return status;
  }

  int
  glo_clock(double t_sec, double& dtsv) const noexcept;
  
  /// @brief Compute SV Clock Correction
  ///
  /// Determine the effective SV PRN code phase offset referenced to the phase 
  /// center of the antennas (∆tsv) with respect to system time (t) at the 
  /// time of data transmission. This estimated correction accounts for the 
  /// deterministic SV clock error characteristics of bias, drift and aging, as 
  /// well as for the SV implementation characteristics of group delay bias and 
  /// mean differential group delay. Since these coefficients do not include 
  /// corrections for relativistic effects, the user's equipment must determine 
  /// the requisite relativistic correction.
  /// The user shall correct the time received from the SV with the equation 
  /// (in seconds):
  /// t = t_sv - Δt_sv
  /// This function template replaces functions gps_dtsv, gal_dtsv and bds_dtsv
  /// For these satellite systems (aka GPS, GALILEO and BeiDou) the Navigation
  /// message as given in RINEX v3.x holds the same clock elements at the
  /// same position (that is at the same indexes in data__ array). Hence, we only
  /// need one function to turn these elements to SV clock correction.
  /// For more information, see the (now obsolete function gps_ecef, gal_ecef and
  /// bds_ecef).
  /// @tparam S         The satellite system of the NavDataFrame; only works
  /// @param[in]  t_sec Time (in seconds) from ToC
  /// @param[out] dt_sv SV Clock Correction in seconds; satellite clock bias 
  ///                   includes relativity correction without code bias (tgd or 
  ///                   bgd)
  /// @param[in]  Ein   If provided, the value to use for Eccentric Anomaly (to
  ///                   compute the relativistic error term). If not provided,
  ///                   then Kepler's equation will be used to compute it. If a
  ///                   user has already computed Ek (e.g. when computing SV
  ///                   coordinates), then this value could be used here with
  ///                   reduced accuracy
  /// @return Anything other than 0 denotes an error
  template<SATELLITE_SYSTEM S>
    int
    sv_clock(double t_sec, double& dt_sv, double* Ein=nullptr)
    const noexcept
  {
    constexpr double MI_SYS  = ngpt::satellite_system_traits<S>::mi();
    constexpr double F_CLOCK = ngpt::satellite_system_traits<S>::f_clock();
    constexpr double LIMIT  {1e-14};       //  Limit for solving (iteratively) 
                                           //+ the Kepler equation for the 
                                           //+ eccentricity anomaly
    double dt = t_sec - toc__.sec().to_fractional_seconds();
#ifdef DEBUG
    if (dt<-302400e0 || dt>302400e0) {
      std::cerr<<"\n[ERROR] NavDataFrame::gps_dtsv Delta-seconds are off! WTF?";
      return -1;
    }
    /*
    if (dt> 302400e0) dt -= 604800e0;
    if (dt<-302400e0) dt += 604800e0;
    */
#endif

    double Ek (0e0);
    if (!Ein) {
      // Solve (iteratively) Kepler's equation for Ek
      double A  (data__[10]*data__[10]);     //  Semi-major axis
      double n0 (std::sqrt(MI_SYS/(A*A*A))); //  Computed mean motion (rad/sec)
      double n  (n0+data__[5]);              //  Corrected mean motion
      double Mk (data__[6]+n*dt);            //  Mean anomaly
      double E  (Mk);
      double e  (data__[8]);
      int i;
      for (i=0; std::abs(E-Ek)>LIMIT && i<1001; i++) {
        Ek = E;
        E = std::sin(Ek)*e+Mk;
      }
      if (i>=1000) return 1;
      Ek = E;
    } else {
      Ek = *Ein;
    }

    // Compute Δtr relativistic correction term (seconds)
    const double Dtr = F_CLOCK * (data__[8]*data__[10]*std::sin(Ek));

    // Compute correction
    double Dtsv = data__[0] + data__[1]*dt + (data__[2]*dt)*dt;
    Dtsv += Dtr;
    dt_sv = Dtsv;

    return 0;
  }

  /// @brief get SV coordinates (WGS84) for SVs' antenna phase centre
  /* __OBSOLETE__ see kepler2state
  int
  gps_ecef(double t_sec, double* state, double* Ek=nullptr)
  const noexcept;
  */
  
  /// @brief get SV coordinates (GTRF) from navigation block
  /* __OBSOLETE__ see kepler2state
  int
  gal_ecef(double t_sec, double* state, double* Ek=nullptr)
  const noexcept;
  */
  
  /// @brief get SV coordinates (BDCS) from navigation block
  /* __OBSOLETE__ see kepler2state
  int
  bds_ecef(double t_sec, double* state, double* Ek=nullptr)
  const noexcept;
  */
  
  
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

  /* __OBSOLETE__ see sv_clock
  int
  gps_dtsv(double t_sec, double& dt_sv, double* Ek_in=nullptr) const noexcept;
  */

  /* __OBSOLETE__ see sv_clock
  int
  gal_dtsv(double t_sec, double& dt_sv, double* Ek_in=nullptr) const noexcept;
  */
  
  /* __OBSOLETE__ see sv_clock
  int
  bds_dtsv(double t_sec, double& dt_sv, double* Ek_in=nullptr) const noexcept;
  */
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

  /// @brief Read, resolve and store next navigation data block
  int
  read_next_record(NavDataFrame&) noexcept;
  
  /// @brief Check the first line of the following message to get the sat. sys
  ngpt::SATELLITE_SYSTEM
  peak_satsys(int&) noexcept;

  /// @brief Read and skip the next navigation message
  int
  ignore_next_block() noexcept;

  /// @brief Find next block belonging to a system (and satellite)
  int
  find_next(pos_type& curpos, NavDataFrame& frame, SATELLITE_SYSTEM sys, int prn=-1)
  noexcept;

  /// @brief Set the stream to end of header or anywhere else
  void
  rewind(pos_type pos=-1) noexcept;

  /// Navigation messages per SV should be sorted chronologicaly in the 
  /// RINEX file, because if we encounter a message of which the ToC is more
  /// than four hours ahead of the input date (t), then we will assume that
  /// no message is found suitable for the given epoch.
  /// @param[in]  t       The epoch for which we want the message
  /// @param[out] curpos  Current position (before starting the function) of the
  ///                     instane's stream; after the execution you can rewing back
  ///                     to curpos and pretend nothing changed
  /// @param[out] frame   NavDataFrame read (if found) that matches the requested
  ///                     satellite system and prn
  /// @param[in]  sys     Requested satellite system
  /// @param[in]  prn     Requested prn; if set to -1, it means any satellite of
  ///                     the requested satellite system 
  /// @return An integer denoting the following:
  ///                     * -1 EOF encountered before matching system and sv
  ///                     *  0 All ok! system and sv matched and frame resolved
  ///                     * >0 An error occured
  template<typename T,
    typename = std::enable_if_t<T::is_of_sec_type>
    >
  int
  find_next_valid(const ngpt::datetime<T>& t, pos_type& curpos, 
    NavDataFrame& frame, SATELLITE_SYSTEM sys, int prn) noexcept
  {
    // std::cout<<"\n$$$[DEBUG] Updating msg for t="<<ngpt::strftime_ymd_hms<ngpt::milliseconds>(t);
#ifdef DEBUG
    if (!__istream.good()) return 50;
#endif
    curpos = __istream.tellg();
    int status=0, dummy_it=0;
    SATELLITE_SYSTEM csys; 
    do {
      csys = this->peak_satsys(status);
      if (status) return status;
      if (csys==sys) {
        if ((status=this->read_next_record(frame))) return clear_stream(status);
        if (prn==frame.prn()) {
          auto toc = frame.toc<milliseconds>();
          // std::cout<<" found msg with ToC "<<ngpt::strftime_ymd_hms<ngpt::milliseconds>(toc);
          // are we too far ahead from the input date ??
          T limitT (3600L * 4 * T::template sec_factor<long>()); // 4 hour in T units
          // if (toc>t && ngpt::delta_sec(toc, t)>limitT) std::cout<<" t vs ToC too far away!";
          if (toc>t && ngpt::delta_sec(toc, t)>limitT) return -1;
          // check SV health
          if (!(frame.sv_health())) {
            // std::cout<<" SV health ok";
            // check if msg is ok for epoch
            if (sys!=SATELLITE_SYSTEM::glonass) {
              auto max_t(toc);
              max_t.add_seconds(ngpt::seconds(frame.fit_interval()));
              // std::cout<<" msg valid untill "<<ngpt::strftime_ymd_hms<ngpt::milliseconds>(max_t);
              // if (t>=toc && t<max_t) std::cout<<" msg OK!"; else std::cout<<" msg not ok!";
              if (t>=toc && t<max_t) return 0;
            } else {
              auto max_t(frame.toe<T>());
              auto min_t(frame.toe<T>());
              max_t.add_seconds(ngpt::seconds(frame.fit_interval()));
              min_t.remove_seconds(ngpt::seconds(frame.fit_interval()));
              if (t>=min_t && t<max_t) return 0;
            }
          }
        }
      } else {
        if ((status=this->ignore_next_block())) return clear_stream(status);
      }
    } while (dummy_it<5000);

    return 100;
  }

private:
  
  /// @brief Read RINEX header; assign info
  int
  read_header() noexcept;

  /// @brief clear the instance stream and return the exit_status
  int
  clear_stream(int exit_status) noexcept;

  std::string            __filename;    ///< The name of the file
  std::ifstream          __istream;     ///< The infput (file) stream
  SATELLITE_SYSTEM       __satsys;      ///< satellite system
  float                  __version;     ///< Rinex version (e.g. 3.4)
  pos_type               __end_of_head; ///< Mark the 'END OF HEADER' field
};// NavigationRnx

}// ngpt

#endif
