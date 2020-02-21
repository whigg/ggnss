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

/// Time of ephemeris as datetime<seconds> instance in UTC
ngpt::datetime<ngpt::seconds>
NavDataFrame::glo_tb2date(bool to_MT) const noexcept
{
  auto toc = this->toc__;
  if (to_MT) toc.add_seconds(ngpt::seconds(10800L));
  long sow;
  toc.as_gps_wsow(sow);
  int  dow_toc = sow / 86400L;        // day of week of toc

  long sow_tb = ((to_MT)?(static_cast<long>(data__[2])):(static_cast<long>(data__[2])+10800L));
  int  dow_tob = sow_tb / 86400;
  long sod_tb = sow_tb % 86400L;
  int  offset = dow_toc - dow_tob;
  return ngpt::datetime<ngpt::seconds>(toc__.mjd()-ngpt::modified_julian_day(offset), 
    ngpt::seconds(sod_tb));
}

/// @brief Compute time arguments for the integration of GLONASS ephemerids
/// @param[in] jd0 Julian Date for 00:00 in MT
/// @return Greenwich Mean Sidereal Time (GMST) for dj0
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
  printf("\nGMST for jd0=%15.6f is %20.10f", jd0, gmst);
  return gmst;
}

// This is the computation of the system J.1 in GLONASS ICD, par.
// J.1 Precise algorithm for determination of position and velocity vector 
// components for the SV’s center of mass for the given instant of MT
// x    = [ x0, y0, z0, Vx0,    Vy0,    Vz0   ]
// xdot = [ Vx, Vy, Vz, V_dotx, Vdot_y, Vdot_z]
// acc is the vector of accelerations induced by gravitational perturbations of 
// the Sun and Moon (which  can be computed in different ways, see J.1)
// acc  = [j_x0s+j_x0m, j_y0s+j_y0m, j_z0s+j_z0m]
// The function will use x and acc to compute the xdot vector, using the
// system of (differential) equations described in J.1
void
glo_state_deriv(const double *x, const double* acc, double *xdot)
{
  double x_km = x[0]*1e-3;
  double y_km = x[1]*1e-3;
  double z_km = x[2]*1e-3;
  const double r2 = (x_km*x_km+y_km*y_km+z_km*z_km)*1e6;
  const double r = std::sqrt(x_km*x_km+y_km*y_km+z_km*z_km)*1e3;
  const double z2r25  = 5e0*x[2]*x[2]/r2;        // 5*z2/r2
  const double gmr3   = GM_GLO/r2/r;             // GM/r3
  const double ja2r2  = J2_GLO*AE_GLO*AE_GLO/r2; // J2*a2/r2
  const double omega2 = OMEGA_GLO*OMEGA_GLO;     // ω2
  // TODO simplify these shit below
  xdot[0] = x[3];
  xdot[1] = x[4];
  xdot[2] = x[5];
  xdot[3] = (-gmr3 - 1.5e0*gmr3*ja2r2*(1e0-z2r25) + omega2)*x[0] + 
            2e0*OMEGA_GLO*x[4] + acc[0];
  xdot[4] = (-gmr3 - 1.5e0*gmr3*ja2r2*(1e0-z2r25) + omega2)*x[1] + 
            2e0*OMEGA_GLO*x[3] + acc[1];
  xdot[5] = (-gmr3 - 1.5e0*gmr3*ja2r2*(3e0-z2r25))*x[2] + acc[2];

  
  /*
  double x0_km = x[0]*1e-3;
  double y0_km = x[1]*1e-3;
  double z0_km = x[2]*1e-3;
  const double r02 = (x0_km*x0_km+y0_km*y0_km+z0_km*z0_km)*1e6;
  const double r0 = std::sqrt(x0_km*x0_km+y0_km*y0_km+z0_km*z0_km)*1e3;
  const double rho2   = (AE_GLO*AE_GLO) / r02;
  const double x0hat  = x[0] / r0;
  const double y0hat  = x[1] / r0;
  const double z0hat  = x[2] / r0;
  const double GM_hat = GM_GLO / r02;
  const double J2rho2 = J2_GLO*;rho2                 // J2*ρ2
  const double a_fac  = 1.5e0*J2rho2;                // 1.5*J2*rho2
  const double b_fac  = (15e0/2)*J2rho2*z0hat*z0hat; // (15/2)J2*rho2*z0hat2

  xdot[0] = x[3];
  xdot[1] = x[4];
  xdot[2] = x[5];
  xdot[3] = (1e0 - a_fac + b_fac)*(GM_hat*x0hat) + acc[0];
  xdot[4] = (1e0 - a_fac + b_fac)*(GM_hat*y0hat) + acc[1];
  xdot[3] = (1e0 - a_fac + b_fac - 3e0*J2rho2)*(GM_hat*z0hat) + acc[2];
  */

  return;
}

/// @brief get SV coordinates from navigation block
///
/// @param[in] 
/// @cite GLONASS-ICD, Appendix J, "Algorithms for determination of SV center of 
///       mass position and velocity vector components using ephemeris data"
/// see https://gssc.esa.int/navipedia/index.php/GLONASS_Satellite_Coordinates_Computation
int
NavDataFrame::glo_ecef(double t_sod, double& xs, double& ys, double& zs)
const noexcept
{
  double x[6], acc[3];
  double k1[6], k2[6], k3[6], k4[6];
  double ytmp[6], yti[6];
  // tb as datetime instance in MT
  ngpt::datetime<seconds> tb = glo_tb2date(false);
  // tb as sec of day
  double tb_sec = tb.sec().to_fractional_seconds();
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

  std::copy(x, x+6, yti);
  double h = t_sod>tb_sec?h_step:-h_step;
  double ti= tb_sec;
  double t_lim = t_sod-std::round((t_sod-tb_sec)/86400)*86400;
  // Perform Runge-Kutta 4th 
  while (std::abs(ti-t_lim)>1e-9) {
    // compute k1
    glo_state_deriv(yti, acc, k1);
    // compute k2
    for (int i=0; i<6; i++) ytmp[i] = yti[i] + (h_step/2e0)*k1[i];
    glo_state_deriv(ytmp, acc, k2);
    // compute k3
    for (int i=0; i<6; i++) ytmp[i] = yti[i] + (h_step/2e0)*k2[i];
    glo_state_deriv(ytmp, acc, k3);
    // compute k4
    for (int i=0; i<6; i++) ytmp[i] = yti[i] + h_step*k3[i];
    glo_state_deriv(ytmp, acc, k4);
    // update y
    for (int i=0; i<6; i++) yti[i] += (h_step/6)*(k1[i]+2*k2[i]+2*k3[i]+k4[i]);
    // update ti
    ti += h;
  }

  // copy results
  xs = yti[0];
  ys = yti[1];
  zs = yti[2];

  return 0;

  /*
  // tb as datetime instance in MT
  ngpt::datetime<seconds> tb = glo_tb2date(true);
  // tb as sec of day
  double tb_sec = static_cast<double>(tb.sec());
  // compute GST of tb
  double gmstb = __gmst__(tb.mjd().as_underlying_type()+ngpt::mjd0_jd);
  gmstb += OMEGA_E*(tb_sec-10800e0);
  // state vector of SV
  double x[6], acc[3];
  // Initial conditions for ODE
  double cosS {std::cos(gmstb)};
  double sinS {std::sin(gmstb)};
  x[0] = data__[3]*cosS - data__[7]*sinS;
  x[1] = data__[3]*sinS + data__[7]*cosS;
  x[2] = data__[11];
  x[3] = data__[4]*cosS - data__[8]*sinS - OMEGA_E*x[1];
  x[4] = data__[4]*sinS + data__[8]*cosS + OMEGA_E*x[0];
  x[5] = data__[12];
  // projections of accelerations
  acc[0] = data__[5]*cosS - data[9]*sinS; // Jx0m + Jx0s
  acc[1] = data__[5]*sinS + data[9]*cosS; // Jy0m + Jy0s
  acc[2] = data__[13];                    // Jz0m + Jz0s

  double k1[6], k2[6], k3[6], k4[6];
  double ytmp[6], yti[6];
  std::copy(x, x+6, yti);
  double h = t_sod>tb_sec?h_step:-h_step;
  double ti= tb_sec;
  // Perform Runge-Kutta 4th for the interval t=tb_sec:t_sod with step=h_step
  while (std::abs(t_sod-ti)>1e-9) {
    // compute k1
    derivs(yti, acc, k1);
    // compute k2
    for (int i=0; i<6; i++) ytmp[i] = yti[i] + (h_step/2e0)*k1[i];
    derivs(ytmp, acc, k2);
    // compute k3
    for (int i=0; i<6; i++) ytmp[i] = yti[i] + (h_step/2e0)*k2[i];
    derivs(ytmp, acc, k3);
    // compute k4
    for (int i=0; i<6; i++) ytmp[i] = yti[i] + h_step*k3[i];
    derivs(ytmp, acc, k4);
    // update y
    for (int i=0; i<6; i++) yti[i] += (h_step/6)*(k1[i]+2*k2[i]+2*k3[i]+k4[i]);
    // update ti
    ti += h;
  }
  */
}
