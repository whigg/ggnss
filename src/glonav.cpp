#include <iostream>
#include <stdexcept>
#include <cerrno>
#include "navrnx.hpp"
#ifdef DEBUG
#include "ggdatetime/datetime_write.hpp"
#endif

using ngpt::NavDataFrame;

/// GLONASS:    
///                        : Time of Clock in UTC time
///             data__[0]  : SV clock bias in sec, aka -TauN
///             data__[1]  : SV relative frequency bias, aka +GammaN
///             data__[2]  : Message frame time(tk+nd*86400) in seconds of UTC week
///             data__[3]  : Satellite position X (km)
///             data__[4]  : velocity X dot (km/sec)
///             data__[5]  : X acceleration (km/sec2)
///             data__[6]  : health(0=healthy, 1=unhealthy)(MSB of 3-bit Bn)
///             data__[7]  : Satellite position Y (km)
///             data__[8]  : velocity Y dot (km/sec)
///             data__[9]  : Y acceleration (km/sec2)
///             data__[10] : frequency number(-7...+13) (-7...+6 ICD 5.1)
///             data__[11] : Satellite position Z (km)
///             data__[12] : velocity Z dot (km/sec)
///             data__[13] : Z acceleration (km/sec2)
///             data__[14] : Age of oper. information (days) (E)
///             data__[15] : spare
///             .................................................
///             data__[30] : spare
///
/// @warning Note that all coordinate/velocity and acceleration components are
///          already converted to meters
///
/// @warning The following is quoted from Rinex v.304
///          GLONASS is basically running on UTC (or, more precisely, GLONASS 
///          system time linked to UTC(SU)), i.e. the time tags are given in 
///          UTC and not GPS time. It is not a continuous time, i.e. it 
///          introduces the same leap seconds as UTC. The reported GLONASS time 
///          has the same hours as UTC and not UTC+3 h as the original GLONASS 
///          System Time! I.e. the time tags in the GLONASS navigation files 
///          are given in UTC (i.e. not Moscow time or GPS time).

// integration step for glonass in seconds (h parameter for Runge-Kutta4)
constexpr double h_step = 60e0;

// (m3/s2) geocentric gravitational constant (mass of the Earth included)
constexpr double GM_GLO = 398600441.8e06;

// semi-major axis of the PZ-90 Earth’s ellipsoid
constexpr double AE_GLO = 6378136e0;

// second degree coefficient of normal potential, which describes the 
// Earth’s polar flattening
constexpr double J2_GLO = 1082625.75e-9;

// mean angular velocity of the Earth relative to vernal equinox
constexpr double OMEGA_GLO = 7.2921151467e-5; // rad/s

/// @brief Time of ephemeris as datetime<seconds> instance in UTC or MT
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
/// @param[in] to_MT If set to true, then the resulting datetime instance will
///                  be in Moscow UTC time, aka MT; if set to false then the
///                  resulting date will be in UTC.
/// @result message frame time as datetime<seconds> instance in UTC or MT
ngpt::datetime<ngpt::seconds>
NavDataFrame::glo_tb2date(bool to_MT) const noexcept
{
  // transform ToC to day_of_week
  auto toc = this->toc__;
  if (to_MT) toc.add_seconds(ngpt::seconds(10800L));
  long sow; 
  toc.as_gps_wsow(sow);
  int dow_toc = sow / 86400L; // day of week of toc
  // transform message frame time (tb) to day of week and seconds of day
  long sow_tb = ((to_MT)?
    (static_cast<long>(data__[2])+10800L):
    (static_cast<long>(data__[2])));
  int  dow_tb = sow_tb / 86400L;
  long sod_tb = sow_tb % 86400L;
  // return the date adding any days offset
  int  offset = dow_toc - dow_tb;
  ngpt::datetime<ngpt::seconds> tbdate(toc.mjd()-ngpt::modified_julian_day(offset), 
    ngpt::seconds(sod_tb));
  return tbdate;
}

/// Compute Greenwich Mean Sidereal Time given a Julian Date
///
/// @param[in] jd0 Julian Date for 00:00
/// @return Greenwich Mean Sidereal Time (GMST) for given Julian Date
/// @cite GLONASS-ICD, Appendix K, "Algorithm for calculation of the current 
///       Julian date JD0, Gregorian date, and GMST"
double
__gmst__(double jd0) noexcept
{
  // Earth’s rotation angle in radians
  constexpr double D2PI = 2e0*3.14159265358979323846e0;
  const double era = D2PI*(0.7790572732640+1.00273781191135448*(jd0-2451545.0e0));
  //  time from Epoch 2000, 1st January, 00:00 (UTC(SU)) till the current 
  // Epoch in Julian centuries, containing 36525 ephemeris days each
  double td = (jd0-2451545.0e0)/36525e0;
  double gmst = era + 0.0000000703270726e0 + (0.0223603658710194e0 +
                                (0.0000067465784654e0 -
                                (0.0000000000021332e0 -
                                (0.0000000001452308e0 -
                                (0.0000000000001784e0 )
                            *td) *td) *td) *td) *td;
  return gmst;
}

/// This is the computation of the system of ODE's described in J.2.1 in 
/// GLONASS ICD, par. J2 "Simplified algorithm for determination of position 
/// and velocity vector components for the SV’s center of mass for the given 
/// instant in MT"
/// 
/// Components for the SV’s center of mass for the given instant of MT
/// x    = [ x0, y0, z0, Vx0,    Vy0,    Vz0   ]
/// xdot = [ Vx, Vy, Vz, V_dotx, Vdot_y, Vdot_z]
/// acc is the vector of accelerations induced by gravitational perturbations of 
/// the Sun and Moon as given in the Navigation message.
/// The function will use x and acc to compute the xdot vector, using the
/// system of (differential) equations described in J.2.1
///
/// @param[in] x Array of length 6; state vector at instant t_MT, i.e.
///              the array [ x(ti), y(ti), z(ti), Vx(ti), Vy(ti), Vz(ti) ]
/// @param[in] acc Array of length 3; the acceleration components, i.e.
///              [acc_x(tb), acc_y(tb), acc_z(tb)] as given in the navigation
///              data frame
/// @param[out] Array of length 6; at return this is filled with the values of
///             the computed ODE system
/// 
/// @warning All units are in meters, meters/sec and meters2/sec
void
glo_state_deriv(const double *x, const double* acc, double *xdot)
noexcept
{
  const double r2 = (x[0]*x[0]+x[1]*x[1]+x[2]*x[2]);
  const double r3 = r2*std::sqrt(r2);
  const double omg2 = OMEGA_GLO*OMEGA_GLO;                    // ω2
  const double a = 1.5e0*J2_GLO*GM_GLO*(AE_GLO*AE_GLO)/r2/r3; // 3/2*J2*mu*Ae^2/r^5
  const double b = 5e0*x[2]*x[2]/r2;                          // 5*z^2/r^2
  const double c = -GM_GLO/r3-a*(1e0-b);                      // -mu/r^3-a(1-b)
  xdot[0] = x[3];
  xdot[1] = x[4];
  xdot[2] = x[5];
  xdot[3]=(c+omg2)*x[0]+2.0*OMEGA_GLO*x[4]+acc[0];
  xdot[4]=(c+omg2)*x[1]-2.0*OMEGA_GLO*x[3]+acc[1];
  xdot[5]=(c-2.0*a)*x[2]+acc[2];
  
  return;
}

/// This is the computation of the system of ODE's described in J.1 in 
/// GLONASS ICD, par. J1 "Precise algorithm for determination of position and 
/// velocity vector components for the SV’s center of mass for the given 
/// instant of MT"
/// 
/// Components for the SV’s center of mass for the given instant of MT
/// x    = [ x0, y0, z0, Vx0,    Vy0,    Vz0   ]
/// xdot = [ Vx, Vy, Vz, V_dotx, Vdot_y, Vdot_z]
/// acc is the vector of accelerations induced by gravitational perturbations of 
/// the Sun and Moon. 
/// The function will use x and acc to compute the xdot vector, using the
/// system of (differential) equations described in J.1
///
/// @param[in] x Array of length 6; state vector at instant t_MT, i.e.
///              the array [ x(ti), y(ti), z(ti), Vx(ti), Vy(ti), Vz(ti) ]
/// @param[in] acc Array of length 3; the acceleration components, i.e.
///              [acc_x(tb), acc_y(tb), acc_z(tb)] as given in the navigation
///              data frame
/// @param[out] Array of length 6; at return this is filled with the values of
///             the computed ODE system
/// 
/// @warning All units are in meters, meters/sec and meters2/sec
void
glo_state_deriv_inertial(const double *x, const double* acc, double *xdot)
noexcept
{
  const double r2 = (x[0]*x[0]+x[1]*x[1]+x[2]*x[2]);
  const double r  = std::sqrt(r2);
  const double xhat = x[0] / r;
  const double yhat = x[1] / r;
  const double zhat = x[2] / r;
  const double zhat2= zhat*zhat;
  const double rho  = AE_GLO / r;
  const double GMhat= GM_GLO / r2;
  const double _32J2GMr = 1.5e0*J2_GLO*GMhat*rho*rho; // 3/2 * J2 *GM_hat *ρ2
  xdot[0] = x[3];
  xdot[1] = x[4];
  xdot[2] = x[5];
  xdot[3] = -GMhat*xhat - _32J2GMr*(1e0-5e0*zhat2)*xhat + acc[0];
  xdot[4] = -GMhat*yhat - _32J2GMr*(1e0-5e0*zhat2)*yhat + acc[1];
  xdot[5] = -GMhat*zhat - _32J2GMr*(3e0-5e0*zhat2)*zhat + acc[2];
  
  return;
}

/// @brief Transform ECEF (PZ90) coordinates to inertial
///
/// The transformation is used in the precise algorithm for calculating SV
/// centre of mass coordinates using the broadcast message. The system of 
/// equations is described in J.4
///
/// @param[in]  x_ecef      State vector in PZ90 reference frame (meters)
/// @param[in]  tb_MT       datetime<seconds> instance of the current epoch
/// @param[out] x_inertial  Resulting state vector in the inertial reference
///                         frame
/// @param[out] acc         Array of size 3; at input acceleration components
///                         in ECEF frame; at output acceleration components
///                         transformed to inertial frame
void
glo_ecef2inertial(const double *x_ecef, ngpt::datetime<ngpt::seconds> tb_MT, 
  double *x_inertial, double *acc=nullptr)
noexcept
{
  // compute GST of tb  -- actually GMST --
  double gmstb = __gmst__(tb_MT.mjd().as_underlying_type()+ngpt::mjd0_jd);
  double tb_sec = tb_MT.sec().to_fractional_seconds();
  gmstb += OMEGA_GLO*(tb_sec-10800e0);

  const double cosS = std::cos(gmstb);
  const double sinS = std::sin(gmstb);
  x_inertial[0] = x_ecef[0]*cosS - x_ecef[1]*sinS;
  x_inertial[1] = x_ecef[0]*sinS + x_ecef[1]*cosS;
  x_inertial[2] = x_ecef[2];
  x_inertial[3] = x_ecef[3]*cosS - x_ecef[4]*sinS - OMEGA_GLO*x_ecef[1];
  x_inertial[4] = x_ecef[3]*sinS + x_ecef[4]*cosS + OMEGA_GLO*x_ecef[0];
  x_inertial[5] = x_ecef[5];

  if (acc) {
    double Jx = acc[0]*cosS - acc[1]*sinS;
    double Jy = acc[0]*sinS + acc[1]*cosS;
    double Jz = acc[2];
    acc[0] = Jx; acc[1] = Jy; acc[2] = Jz;
  }

  return;
}

/// @brief Transform inertial coordinates to ECEF (PZ90)
///
/// The transformation is used in the precise algorithm for calculating SV
/// centre of mass coordinates using the broadcast message. The system of 
/// equations is described in J.5
///
/// @param[in]  x_inertial  State vector in inertial reference frame (meters)
/// @param[in]  tb_MT       datetime<seconds> instance of the current epoch
/// @param[out] x_ecef      Resulting state vector in the ECEF PZ90 reference
///                         frame
/// @param[out] acc         Array of size 3; at input acceleration components
///                         in inertial frame; at output acceleration components
///                         transformed to ECEF frame
void
glo_inertial2ecef(const double *x_inertial, ngpt::datetime<ngpt::seconds> ti_MT, 
  double *x_ecef)
noexcept
{
  // compute GST of ti  -- actually GMST --
  double gmsti = __gmst__(ti_MT.mjd().as_underlying_type()+ngpt::mjd0_jd);
  double ti_sec = ti_MT.sec().to_fractional_seconds();
  gmsti += OMEGA_GLO*(ti_sec-10800e0);

  const double cosS = std::cos(gmsti);
  const double sinS = std::sin(gmsti);
  x_ecef[0] = x_inertial[0]*cosS + x_inertial[1]*sinS;
  x_ecef[1] =-x_inertial[0]*sinS + x_inertial[1]*cosS;
  x_ecef[2] = x_inertial[2];
  x_ecef[3] = x_inertial[3]*cosS + x_inertial[4]*sinS + OMEGA_GLO*x_inertial[1];
  x_ecef[4] =-x_inertial[3]*sinS + x_inertial[4]*cosS - OMEGA_GLO*x_inertial[0];
  x_ecef[5] = x_inertial[5];

  return;
}

/// @brief Compute SV coordinates from navigation block
/// 
/// This function will compute the state vector for the SV, at time t_sod
/// (with reference time tb=tb_sec) in the ECEF PZ-90 reference frame, following
/// the "simplified" algorithm, aka "Simplified algorithm for determination of 
/// position and velocity vector components for the SV’s center of mass for the 
/// given instant in MT". Note that t_sod and tb_sec must not be more than
/// 15min apart.
///
/// @param[in]  t_sod  Seconds of day (as double) in MT
/// @param[in]  tb_sec Seconds of day (as double) in MT
/// @param[out] state  array of size 6; SV centre of mass state vector (aka
///                    [x,y,z,Vx,Vy,Vz]) at time t_sod in meters, meters/sec
/// @return an integer denoting the status: 0 means all ok, -1 means that the
///         computation is performed, but the time interval is more than 15min
///         apart; anything >0 denotes an error
///
/// @cite GLONASS-ICD, Appendix J, "Algorithms for determination of SV center of 
///       mass position and velocity vector components using ephemeris data"
int
NavDataFrame::glo_ecef(double t_sod, double tb_sec, double* state) 
const noexcept
{
  int status = 0;
  if (std::abs(tb_sec-t_sod)>15*60e0) {
    std::cerr<<"\n[WARNING] NavDataFrame::glo_ecef() Time interval too large!"
      <<"abs("<<tb_sec<<" - "<<t_sod<<") > "<<15*60e0<<" sec";
    status = -1;
  }

  double x[6], acc[3];
  // Initial conditions for ODE aka x = [x0, y0, z0, Vx0, Vy0, Vz0]
  x[0] = data__[3];
  x[1] = data__[7];
  x[2] = data__[11];
  x[3] = data__[4];
  x[4] = data__[8];
  x[5] = data__[12];
  // projections of accelerations
  acc[0] = data__[5];
  acc[1] = data__[9];
  acc[2] = data__[13];
  
  // quick return if tb == ti
  if (t_sod == tb_sec) {
    std::copy(x, x+6, state);
    return status;
  }

  double k1[6], k2[6], k3[6], k4[6];
  double ytmp[6], yti[6];
  double h = t_sod>tb_sec?h_step:-h_step;
  double ti= tb_sec;
  double t_lim = t_sod-std::round((t_sod-tb_sec)/86400)*86400;
  int    max_it=0;
  std::copy(x, x+6, yti);
  // Perform Runge-Kutta 4th 
  // while (std::abs(ti-t_lim)>1e-9 && ++max_it<1500) {
  while ( h>0?ti<t_lim:ti>t_lim && ++max_it<1500) {
    // compute k1
    glo_state_deriv(yti, acc, k1);
    // compute k2
    for (int i=0; i<6; i++) ytmp[i] = yti[i] + (h/2e0)*k1[i];
    glo_state_deriv(ytmp, acc, k2);
    // compute k3
    for (int i=0; i<6; i++) ytmp[i] = yti[i] + (h/2e0)*k2[i];
    glo_state_deriv(ytmp, acc, k3);
    // compute k4
    for (int i=0; i<6; i++) ytmp[i] = yti[i] + h*k3[i];
    glo_state_deriv(ytmp, acc, k4);
    // update y
    for (int i=0; i<6; i++) yti[i] += (h/6)*(k1[i]+2*k2[i]+2*k3[i]+k4[i]);
    // update ti
    ti += h;
  }
  if (max_it>=1500) {
    std::cerr<<"\n[ERROR] NavDataFrame::glo_ecef() h="<<h<<", from "
      <<tb_sec<<" to "<<t_lim<<" last t="<<ti;
    return 10;
  }

  // copy results
  std::copy(yti, yti+6, state);

  return status;
}

int
NavDataFrame::glo_ecef2(double t_sod, double tb_sec, double& xs, double& ys, double& zs, double* vel)
const noexcept
{
  double x[6], acc[3];
  double k1[6], k2[6], k3[6], k4[6];
  double ytmp[6], yti[6];
  
  // tb as datetime instance in MT
  // ngpt::datetime<seconds> tb = glo_tb2date(true);
  if (std::abs(tb_sec-t_sod)>15*60e0) {
    std::cerr<<"\n[ERROR] NavDataFrame::glo_ecef() Time interval too large! "
      <<"abs("<<tb_sec<<" - "<<t_sod<<") > "<<15*60e0<<" sec";
    return 9;
  }
  
  // Initial conditions for ODE aka x = [x0, y0, z0, Vx0, Vy0, Vz0]
  x[0] = data__[3];
  x[1] = data__[7];
  x[2] = data__[11];
  x[3] = data__[4];
  x[4] = data__[8];
  x[5] = data__[12];

  // projections of accelerations
  acc[0] = data__[5];
  acc[1] = data__[9];
  acc[2] = data__[13];
  
  // quick return if tb == ti
  if (t_sod == tb_sec) {
    xs = x[0];
    ys = x[1];
    zs = x[2];
    if (vel!=nullptr) std::copy(x+3, x+6, vel);
    return 0;
  }

  // transform state vector to inertial frame
  // tb as datetime instance in MT
  ngpt::datetime<seconds> tb_dt = glo_tb2date(true);
  glo_ecef2inertial(x, tb_dt, ytmp, acc);
  std::copy(ytmp, ytmp+6, x);

  // Perform Runge-Kutta 4th 
  std::copy(x, x+6, yti);
  double h = t_sod>tb_sec?h_step:-h_step;
  double ti= tb_sec;
  double t_lim = t_sod-std::round((t_sod-tb_sec)/86400)*86400;
  int max_it=0;
  // Perform Runge-Kutta 4th 
  while (std::abs(ti-t_lim)>1e-9 && ++max_it<1500) {
    // printf("\nt=%10.5f x=%+20.5f y=%+20.5f z=%+20.5f", ti, yti[0], yti[1], yti[2]);
    // compute k1
    glo_state_deriv_inertial(yti, acc, k1);
    // compute k2
    for (int i=0; i<6; i++) ytmp[i] = yti[i] + (h/2e0)*k1[i];
    glo_state_deriv_inertial(ytmp, acc, k2);
    // compute k3
    for (int i=0; i<6; i++) ytmp[i] = yti[i] + (h/2e0)*k2[i];
    glo_state_deriv_inertial(ytmp, acc, k3);
    // compute k4
    for (int i=0; i<6; i++) ytmp[i] = yti[i] + h*k3[i];
    glo_state_deriv_inertial(ytmp, acc, k4);
    // update y
    for (int i=0; i<6; i++) yti[i] += (h/6)*(k1[i]+2*k2[i]+2*k3[i]+k4[i]);
    // update ti
    ti += h;
  }

  // all done! result state vector is at yti in an inertial RF. convert to PZ90
  // ti as datetime instance in MT
  ngpt::datetime<ngpt::seconds> tidt (tb_dt.mjd(), ngpt::seconds(t_sod));
  glo_inertial2ecef(yti, tidt, x);

  // copy results
  xs = x[0];
  ys = x[1];
  zs = x[2];

  // copy velocities if needed
  if (vel!=nullptr) std::copy(x+3, x+6, vel);

  return 0;
}

int
NavDataFrame::glo_dtsv(double t_mt, double toe_mt, double& dtsv)
const noexcept
{
  double deltat = t_mt - toe_mt - std::round((t_mt-toe_mt)/86400e0)*86400e0;
  dtsv = data__[0] + data__[1]*deltat;
  return 0;
}
