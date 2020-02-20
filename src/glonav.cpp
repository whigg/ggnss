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
constexpr double GM_GLONASS = 398600441.8e06;

// semi-major axis of the PZ-90 Earth’s ellipsoid
constexpr double PZ90_SEMI_MAJOR = 6378136e0;

// second degree coefficient of normal potential, which describes the 
// Earth’s polar flattening
constexpr double J2_GLONASS = 1082625.75e-9;

// mean angular velocity of the Earth relative to vernal equinox
constexpr double OMEGA_E = 7.2921151467e-5; // rad/s

/// Time of ephemeris as datetime<seconds> instance in UTC
ngpt::datetime<ngpt::seconds>
NavDataFrame::glo_te2date() const noexcept
{
  long sow;
  auto wk = as_gps_wsow(toc__);

  /*
  // sec in day
  // ngpt::seconds te_sec ((static_cast<long>(data__[2]))%86400L);
  ngpt::seconds te_sec (std::fmod(static_cast<long>(data__[2]), 86400L));
  ngpt::datetime<ngpt::seconds> te (toc__.mjd(), te_sec);
  std::cout<<"\nTime of ephemeris: "<<ngpt::strftime_ymd_hms<seconds>(te);
  return te;
  */
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
double __sqr__(double x) noexcept {return x*x;}
void
glo_state2deriv(const double *x, const double* acc, double *xdot)
{
  const double r   = std::sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);
  const double rho = PZ90_SEMI_MAJOR / r;
  const double GM  = GM_GLONASS / (r*r);
  const double xsv = x[0] / r;
  const double ysv = x[1] / r;
  const double zsv = x[2] / r;
  const double fac = (3e0/2e0)*J2_GLONASS*GM*rho;

  xdot[0] = x[3];
  xdot[1] = x[4];
  xdot[2] = x[5];
  xdot[3] = -GM*xsv - fac*xsv*(1e0-5e0*zsv*zsv) + acc[0];
  xdot[4] = -GM*ysv - fac*ysv*(1e0-5e0*zsv*zsv) + acc[1];
  xdot[5] = -GM*zsv - fac*zsv*(3e0-5e0*zsv*zsv) + acc[2];

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
  long ltb = std::fmod(static_cast<long>(data__[2]), 86400L);
  double tb = static_cast<double>(ltb);
  double ti = t_sod; // (t_sod-10800e0); // t to MT
  printf("\ntb=%20.5f ti=%20.5f", tb, ti);
  
  // the sidereal time in Greenwich at midnight GMT of a date at which the 
  // epoch te is specified. (Notice: GLONASS_time = UTC(SU) + 3 hours)
  double mjd_te = static_cast<double>(glo_te2date().mjd().as_underlying_type());
  double gmst = __gmst__(mjd_te+ngpt::mjd0_jd);

  // sidereal time at epoch te, to which are referred the initial conditions,
  // in Greenwich meridian
  double Sti = gmst + OMEGA_E*(tb-10800e0);

  double x[6], /*xdot[6],*/ acc[3];
  // the initial conditions (x(te),y(te),z(te),vx(te),vy(te),vz(te)), as 
  // broadcast in the GLONASS navigation message, are in the ECEF Greenwich 
  // coordinate system PZ-90. Therefore, and previous to orbit integration, 
  // they must be transformed to an absolute (inertial) coordinate system
  double cosS {std::cos(Sti)};
  double sinS {std::sin(Sti)};
  x[0] = data__[3]*cosS - data__[7]*sinS;
  x[1] = data__[3]*sinS + data__[7]*cosS;
  x[2] = data__[11];
  x[3] = data__[4]*cosS - data__[8]*sinS - OMEGA_E*x[1];
  x[4] = data__[4]*sinS + data__[8]*cosS + OMEGA_E*x[0];
  x[5] = data__[12];
  for (int i=0; i<6; i++) x[i]*=1e3;

  // The (X′′(te),Y′′(te),Z′′(te)) acceleration components broadcast in the 
  // navigation message are the projections of luni-solar accelerations to axes 
  // of the ECEF Greenwich coordinate system. Thence, these accelerations must 
  // be transformed to the inertial system
  acc[0] = data__[5]*cosS - data__[9]*sinS;
  acc[1] = data__[5]*sinS + data__[9]*cosS;
  acc[2] = data__[13];
  for (int i=0; i<3; i++) acc[i]*=1e3;
  
  // Runge-Kutta 4th with step = h_step
  int it = 0;
  double k[6*4], xinp[6];
  double h = (ti>tb?h_step:-h_step);
  // while (std::abs(t-tb)>1e-9) {
  for (double t=tb; std::abs(t-ti)>1e-9; t+=h) {
    printf("\nRK4 iteration: %03d t=%20.5f ti=%20.5f",it,t,ti);
    if (it>1000) return 1;
    ++it;
    // compute k1 = F(t0, Y0)
    glo_state2deriv(x, acc, k); 
    //compute k2 = F(t0+h/2, Y0+h*k1/2)
    for (int i=0; i<6; i++) xinp[i] = x[i] + h*k[i] / 2e0;
    glo_state2deriv(xinp, acc, k+6);
    //compute k3 = F(t0+h/2, Y0+h*k2/2)
    for (int i=0; i<6; i++) xinp[i] = x[i] + h*k[6+i] / 2e0;
    glo_state2deriv(xinp, acc, k+12);
    // compute k4 = F(t0+h, Y0+h*k3)
    for (int i=0; i<6; i++) xinp[i] = x[i] + h*k[12+i];
    glo_state2deriv(xinp, acc, k+18);
    for (int i=0; i<6; i++) x[i] += (h/6e0)*(k[i] + 2*k[6+i] + 2*k[12+i] + k[18+i]);
    t+=h;
  }

  // Transform back to PZ90
  /*
  Sti = gmst + OMEGA_E*(ti-10800e0);
  cosS = std::cos(Sti);
  sinS = std::sin(Sti);
  xs = x[0]*cosS - x[1]*sinS;
  ys = -x[0]*sinS + x[1]*cosS;
  zs = x[2];
  */
  xs = x[0];
  ys = x[1];
  zs = x[2];
  return 0;
}
