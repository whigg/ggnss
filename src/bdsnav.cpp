#include <iostream>
#include <stdexcept>
#include <cerrno>
#include "navrnx.hpp"

using ngpt::NavDataFrame;

/// BeiDou:                : Time of Clock in BDT time
///             data__[0]  : SV clock bias in seconds
///             data__[1]  : SV clock drift in m/sec
///             data__[2]  : SV clock drift rate in m/sec^2
///             data__[3]  : AODE Age of Data, Ephemeris (as specified in 
///                          BeiDou ICD Table Section 5.2.4.11 Table 5-8) and 
///                          field range is: 0-31
///             data__[4]  : Crs (meters)
///             data__[5]  : Delta n (radians/sec)
///             data__[6]  : M0 (radians) 
///             data__[7]  : Cuc (radians)
///             data__[8]  : e Eccentricty
///             data__[9]  : Cus (radians)
///             data__[10] : sqrt(A) (sqrt(m))
///             data__[11] : Toe Time of ephemeris (sec of BDT Week)
///             data__[12] : Cic (radians)
///             data__[13] : OMEGA0 (radians)
///             data__[14] : Cis (radians)
///             data__[15] : i0 (radians)
///             data__[16] : Crc (meters)
///             data__[17] : omega (radians)
///             data__[18] : OMEGA_DOT (radians/sec)
///             data__[19] : IDOT (radians/sec)
///             data__[20] : Spare
///             data__[21] : BDT Week
///             data__[22] : Spare
///             data__[23] : SV accuracy (meters)
///             data__[24] : SatH1
///             data__[25] : TGD1 B1/B3 (seconds)
///             data__[26] : TGD2 B2/B3 (seconds)
///             data__[27] : Transmission time of message (sec of BDT Week)
///             data__[28] : AODC Age of Data Clock
///             data__[29] : Spare
///             data__[30] : Spare

/// Ratio of a circle’s circumference to its diameter
constexpr double PI_BDS = 3.1415926535898e0;

/// Geocentric gravitational constant
constexpr double MI_BDS = 3.986004418e14;

/// Mean angular velocity of the Earth
constexpr double OMEGA_BDS = 7.2921150e-5;

/// Speed of light
constexpr double C_BDS = 2.99792458e8;

///
constexpr double F_CLOCK = -2e0 * std::sqrt(MI_BDS) / (C_BDS*C_BDS);

int
NavDataFrame::bds_dtsv(double dt, double& dt_sv, double* Ein)
const noexcept
{
  constexpr double LIMIT  {1e-14};       //  Limit for solving (iteratively) 
                                         //+ the Kepler equation for the 
                                         //+ eccentricity anomaly
#ifdef DEBUG
  if (dt<-302400e0 || dt>302400e0) {
    std::cerr<<"\n[ERROR] NavDataFrame::gal_dtsv Delta-seconds are off! WTF?";
    return -1;
  }
  if (dt> 302400e0) dt -= 604800e0;
  if (dt<-302400e0) dt += 604800e0;
#endif

  double Ek (0e0);
  if (!Ein) {
    // Solve (iteratively) Kepler's equation for Ek
    double A  (data__[10]*data__[10]);     //  Semi-major axis
    double n0 (std::sqrt(MI_BDS/(A*A*A))); //  Computed mean motion (rad/sec)
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

/// @brief get SV coordinates (WGS84) from navigation block
/// 
/// Compute the ECEF coordinates of position for the phase center of the SVs' 
/// antennas. The time parameter should be given in GPS Time
/// @param[in]  toe_sec  Time of Ephemeris as seconds in day
/// @param[in]  t_sec    Epoch as seconds in day
/// @param[out] state    SV x,y,z -components of antenna phase center position
///                      in the WGS84 ECEF coordinate system in meters; the 
///                      state array must have length >=3
/// @param[out] Ek_ptr   If pointer is not null, it will hold (at output) the 
///                      value of the computed Ek aka Eccentric Anomaly
/// @return Anything other than 0 denotes an error
///
/// @note Input parameters toe_sec and t_sec should be referenced to the same
///       start (aka start of day) at the same time-scale
///
/// @see IS-GPS-200H, User Algorithm for Ephemeris Determination
int
NavDataFrame::bds_ecef(double toe_sec, double t_sec, double* state, 
  double* Ek_ptr)
const noexcept
{
  int status = 0;
  constexpr double LIMIT  {1e-14};       //  Limit for solving (iteratively) 
                                         //+ the Kepler equation for the 
                                         //+ eccentricity anomaly
  const double A  (data__[10]*data__[10]);     //  Semi-major axis
  const double n0 (std::sqrt(MI_BDS/(A*A*A))); //  Computed mean motion (rad/sec)
  double tk (t_sec-toe_sec);
#ifdef DEBUG
  if (tk<-302400e0 || tk>302400e0) {
    std::cerr<<"\n[ERROR] NavDataFrame::gal_ecef Delta-seconds are off! WTF?";
    return -1;
  }
  if (tk> 302400e0) tk -= 604800e0;
  if (tk<-302400e0) tk += 604800e0;
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
  const double rk      (A*(1e0-e*cosE)+drk);          // Corrected Radius
  const double ik      (data__[15]+dik+data__[19]*tk);        // Corrected Inclination
                                
  // Positions in orbital plane
  const double xk_dot  (rk*std::cos(uk));
  const double yk_dot  (rk*std::sin(uk));
  
  // Corrected longitude of ascending node
  const double omega_k (data__[13]+(data__[18]-OMEGA_BDS)*tk-OMEGA_BDS*data__[11]);
  const double sinOk   (std::sin(omega_k));
  const double cosOk   (std::cos(omega_k));
  const double cosik   (std::cos(ik));
  
  state[0] = xk_dot*cosOk - yk_dot*sinOk*cosik;
  state[1] = xk_dot*sinOk + yk_dot*cosOk*cosik;
  state[2] = yk_dot*std::sin(ik);

  // all done
  return status;
}
