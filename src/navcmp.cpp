#include <iostream>
#include <stdexcept>
#include <cerrno>
#include "navrnx.hpp"

using ngpt::NavDataFrame;

/// WGS 84 value of the earth's gravitational constant for GPS user
constexpr double mi_gps {3.986005e14};

/// WGS 84 value of the earth's rotation rate
constexpr double OMEGAE_dot {7.2921151467e-5};

/// @brief get SV coordinates (WGS84) from navigation block
/// see IS-GPS-200H, User Algorithm for Ephemeris Determination
int
NavDataFrame::gps_ecef(double t_insow, double& x, double& y, double& z)
const noexcept
{
  constexpr double LIMIT  {1e-14};       //  Limit for solving (iteratively) 
                                         //+ the Kepler equation for the 
                                         //+ eccentricity anomaly
  double A  (data__[10]*data[10]);       //  Semi-major axis
  double n0 (std::sqrt(mi_gps/(A*A*A))); //  Computed mean motion (rad/sec)
  double tk (t-data__[11]);              //  Time from ephemeris reference epoch
  if (tk>302400e0) tk -= 604800e0;
  if (tk<302400e0) tk += 604800e0;
  double n  (n0+data__[5]);              //  Corrected mean motion
  double Mk (data__[6]+n*tk);            //  Mean anomaly

  // Solve (iteratively) Kepler's equation for Ek
  double E  (Mk);
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
  double vk      (std::atan2(vk_ar, vk_pr));            // True Anomaly
  double cosVk   (std::cos(vk));
  Ek           = std::acos((e+cosVk)/(1e0+e*cosVk));    // Eccentric Anomaly

  // Second Harmonic Perturbations
  double Fk      (vk+data__[17]);                       // Argument of Latitude
  double sin2F   (std::sin(2e0*Fk));
  double cos2F   (std::cos(2e0*Fk));
  double duk     (data__[9]*sin2F  + data__[7]*cos2F);  // Argument of Latitude
                                                        //+ Correction
  double drk     (data__[4]*sin2F  + data__[16]*cos2F); // Radius Correction
  double dik     (data__[14]*sin2F + data__[12]*cos2F); // Inclination Correction

  double uk      (Fk + duk);                            // Corrected Argument 
                                                        //+ of Latitude
  double rk      (A*(1e0-e*std::cos(Ek))+drk);          // Corrected Radius
  double ik      (data__[15]+dik+data__[19]*tk);        // Corrected Inclination
                                
  // Positions in orbital plane
  double xk_dot  (rk*std::cos(uk));
  double yk_dot  (rk*std::sin(uk));
  
  // Corrected longitude of ascending node
  double omega_k (data__[13]+(data__[18]-OMEGAE_dot)*tk-OMEGAE_dot*data__[11]);
  double sinOk   (std::sin(omega_k));
  double cosOk   (std::cos(omega_k));
  double cosik   (std::cos(ik));
  
  x = xk_dot*cosOk - yk_dot*sinOk*cosik;
  y = xk_dot*sinOk - yk_dot*cosOk*cosik;
  z = yk_dot*std::sin(ik);
  return;
}
