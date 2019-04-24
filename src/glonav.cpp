#include <iostream>
#include <stdexcept>
#include <cerrno>
#include "navrnx.hpp"

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
  
// integration step for glonass in seconds (h parameter for Runge-Kutta4)
constexpr double h_step = 60e0;

// (m3/s2) geocentric gravitational constant (mass of the Earth included)
constexpr double GM_GLONASS = 398600441.8e06;

// semi-major axis of the PZ-90 Earth’s ellipsoid
constexpr double PZ90_SEMI_MAJOR = 6378136e0;

// second degree coefficient of normal potential, which describes the 
// Earth’s polar flattening
constexpr double J2_GLONASS = 1082625.75e-9;

/// @brief get SV coordinates from navigation block
/// see GLONASS-ICD, Appendix J
/// see https://gssc.esa.int/navipedia/index.php/GLONASS_Satellite_Coordinates_Computation
int
NavDataFrame::glo_ecef(double t, double& x, double& y, double& z)
const noexcept
{
  // the sidereal time in Greenwich at midnight GMT of a date at which the 
  // epoch te is specified. (Notice: GLONASS_time = UTC(SU) + 3 hours)
  double gmst = __gmst__(year, month, day);

  // sidereal time at epoch te, to which are referred the initial conditions,
  // in Greenwich meridian
  double Sti = gmst + OMEGA_E*(t-10800e0);

  double x[6], xdot[6], acc[3];
  // the initial conditions (x(te),y(te),z(te),vx(te),vy(te),vz(te)), as 
  // broadcast in the GLONASS navigation message, are in the ECEF Greenwich 
  // coordinate system PZ-90. Therefore, and previous to orbit integration, 
  // they must be transformed to an absolute (inertial) coordinate system
  double cosS {std::cos(Sti)};
  double sinS {std::sin(Sti)};
  x[0] = data__[3]*cosS - data__[7]*sinS;
  x[1] = data__[3]*sinS + data__[7]*cosS;
  x[2] = data__[11];
  x[3] = data__[4]*cosS - data__[8]*sinS - OMEGA_E*y_i;
  x[4] = data__[4]*sinS + data__[8]*cosS + OMEGA_E*x_i;
  x[5] = data__[12];

  // The (X′′(te),Y′′(te),Z′′(te)) acceleration components broadcast in the 
  // navigation message are the projections of luni-solar accelerations to axes 
  // of the ECEF Greenwich coordinate system. Thence, these accelerations must 
  // be transformed to the inertial system
  acc[0] = data__[5]*cosS - data__[9]*sinS;
  acc[1] = data__[5]*sinS + data__[9]*cosS;
  acc[2] = data__[13];
  
  // Runge-Kutta 4th with step = h_step
  double k[6*4], xinp[6];
  double h = h_step;
  while (std::abs(t-tb)>1e-9) {
    glo_state2deriv(x, acc, k); // compute k1 = F(t0, Y0)
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
  }
  return;
}

//
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
