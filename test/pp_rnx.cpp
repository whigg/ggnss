#include <iostream>
#include <algorithm>
#include <cassert>
#include <array>
#include "obsrnx.hpp"
#include "navrnx.hpp"
#include "ggeodesy/car2ell.hpp"
#include "ggdatetime/datetime_write.hpp"
#include "gauss_newton.hpp"

using ngpt::ObservationRnx;
using ngpt::NavigationRnx;
using ngpt::NavDataFrame;
using ngpt::ObservationCode;
using ngpt::GnssObservable;
using ngpt::SATELLITE_SYSTEM;
using ngpt::Satellite;
using ngpt::datetime;
using ngpt::milliseconds;
  
typedef std::pair<std::size_t, double> id_pair;
typedef std::vector<id_pair>           vecof_idpair;
using svdit = std::vector<std::pair<Satellite, std::vector<double>>>::iterator;

constexpr int MAX_SATS = 30;
constexpr double MIN_ELEVATION = 10e0;

/* bernese manual, p. 188 */
class Saastamoinen {
public:
  Saastamoinen(double hgt) noexcept: h(hgt) {initialize();};
  double correction(double z) const noexcept {
    double cosz  = std::cos(z);
    double tanz2 = std::tan(z)*std::tan(z);
    double facT  = 0.05e0 + 1255e0/T;
    double faccz = 0.002277e0/cosz;
    return faccz*(P+facT*e-tanz2);
  }
  auto correction(const std::vector<double>& zangles) const noexcept {
    std::vector<double> dT;
    dT.reserve(zangles.size());
    std::transform(zangles.begin(), zangles.end(), std::back_inserter(dT),[this](double z)
                                                                          {return correction(z);}
    );
    return dT;
  }
private:
  double h,T,R,P,e;
  
  void
  initialize() noexcept {
    constexpr double pr = 1013.25e0;
    constexpr double hr = 0e0;
    constexpr double Tr = 18e0;
    constexpr double Rr = 50e0;
    T = Tr-0.0065*(h-hr);
    R = Rr*exp(-0.0006396*(h-hr));
    P = pr*std::pow(1e0-0.0000226*(h-hr), 5.225);
    e = R*exp(-37.2465e0+0.213166*T-std::pow(0.000256908,2e0));
  }
};

std::vector<double>
compute_zenith_angles(int numsats, const std::vector<std::array<double,4>>& states,
                    double x, double y, double z,
                    double& lat, double& lon, double& hgt) {
  std::vector<double> zangles;
  assert(numsats>0);
  zangles.reserve(numsats);
  double n,e,u,r,znth;

  ngpt::car2ell<ngpt::ellipsoid::grs80>(x,y,z,lat,lon,hgt);
  const double cosf = std::cos(lat);
  const double sinf = std::sin(lat);
  const double cosl = std::cos(lon);
  const double sinl = std::sin(lon);

  for (int i=0; i<numsats; i++) {
    const double xs=states[i][0];
    const double ys=states[i][1];
    const double zs=states[i][2];
    const double dx = xs-x;
    const double dy = ys-y;
    const double dz = zs-z;
    n = -sinf * cosl * dx - sinf * sinl * dy + cosf * dz;
    e = -sinl * dx        + cosl * dy;
    u =  cosf * cosl * dx + cosf * sinl * dy + sinf * dz;
    r = std::sqrt(n*n + e*e + u*u);
    znth = std::acos(u / r);
    zangles.push_back(znth);
    if (znth<0e0 || znth>=ngpt::DPI/2e0) {
      std::cerr<<"\n[ERROR] Zenith angle out of limits! Value is "<<znth;
      std::cerr<<"\n[ERROR] Occured at satellite #"<<i;
      throw i;
    }
  }

  return zangles;
}

/*   cos(z)^2  */
std::vector<double>
weighting_fun(std::vector<double>& zangles) {
  std::vector<double> v;
  v.reserve(zangles.size());
  std::transform(zangles.begin(), zangles.end(), std::back_inserter(v),[](double a) {
    // return (a>80e0*ngpt::DPI/180e0)?(200e0):(1e0/std::cos(a)*std::cos(a));});
    return (1e0/std::cos(a)*std::cos(a));});
  return v;
}

/// Search through the Navigation RInex file to find a valid message for
/// the given satellite (resolved from msg) at the given epoch (t).
/// If such a message is found, it is stored in msg and the function returns
/// 0. If -1 is returned, we reached EOF before finding such a message. Else,
/// if we return with >0, an error has occured.
int
update_msg(const datetime<milliseconds>& t, NavigationRnx& nav, 
  NavDataFrame& msg)
{
  SATELLITE_SYSTEM sys = msg.system();
  int prn = msg.prn();
  NavigationRnx::pos_type curpos;
  NavDataFrame nmsg;
  int j = nav.find_next_valid<milliseconds>(t, curpos, nmsg, sys, prn);
  if (!j) msg=nmsg;
  nav.rewind(curpos);
  return j;
}

/// Check if the navigation message is ok to use, for epoch t; aka, t is
/// inside the message's fit interval and the SV health is ok.
/// If everything is ok, 0 is returned.
/// If the satellite is unhealthy, -1 is returned.
/// If 1 is returned, t is outside the message's interval
/// if anything >1 is returned, an error has occured
int
check_nav_msg(const NavDataFrame& msg, const datetime<milliseconds>& ti)
{
  // first check health
  if (msg.sv_health()) return -1;

  // check fit interval
  ngpt::seconds fitsec (msg.fit_interval());
  if (msg.system()==SATELLITE_SYSTEM::glonass) {
    auto min_t = msg.toe<milliseconds>();
    auto max_t = msg.toe<milliseconds>();
    max_t.add_seconds(fitsec);
    min_t.remove_seconds(fitsec);
    if (ti>=min_t && ti<max_t) return 0;
    return 1;
  } else {
    if (ti>=msg.toc<milliseconds>()) {
      auto max_t = msg.toc<milliseconds>();
      max_t.add_seconds(fitsec);
      if (ti<max_t) return 0;
    }
  }

  return 1;
}

std::vector<NavDataFrame>::iterator
get_valid_msg(NavigationRnx& nav, const Satellite& sat, 
  const datetime<milliseconds>& t, std::vector<NavDataFrame>& sat_nav_vec,
  int& status)
{
  status=-200;
  auto nit = std::find_if(sat_nav_vec.begin(), sat_nav_vec.end(),
      [&sat, &t, &status](const NavDataFrame& p)
      {return (sat.system()==p.system() && sat.prn()==p.prn()) 
        && !(status=check_nav_msg(p, t));}
    );

  if (!status)  return nit;  // all ok, msg found!
    
  NavDataFrame msg; msg.system()=sat.system(); msg.prn()=sat.prn();
  int j=update_msg(t, nav, msg);
  if (j) {
    std::cerr<<" failed to valid new valid message for epoch: "
      <<ngpt::strftime_ymd_hms<milliseconds>(t);
    status=j;
    return sat_nav_vec.end();
  } else {
    if (status>=0) {
      nit = std::find_if(sat_nav_vec.begin(), sat_nav_vec.end(),
          [&sat](const NavDataFrame& p)
          {return sat.system()==p.system() && sat.prn()==p.prn();});
      assert(nit!=sat_nav_vec.end());
      *nit = std::move(msg);
    } else {
      sat_nav_vec.push_back(std::move(msg));
      nit = sat_nav_vec.end()-1;
    }
    status=0;
    return nit;
  }

  status=1000;
  return sat_nav_vec.end();
}

int main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cerr<<"\n[ERROR] Run as: $>pprnx [Obs. RINEX] [Nav. RINEX]\n";
    return 1;
  }

  // open Obs Rnx and printf info
  ObservationRnx obsrnx(argv[1]);
  obsrnx.print_members();
  
  // open Nav Rnx
  NavigationRnx navrnx(argv[2]);
  std::vector<NavDataFrame> sat_nav_vec; sat_nav_vec.reserve(50);
  
  // use GPS C1C
  SATELLITE_SYSTEM satsys = SATELLITE_SYSTEM::gps;
  GnssObservable gc1c(satsys, ObservationCode("C1C"), 2.5457277801631593e0);
                 gc1c.add(ngpt::SATELLITE_SYSTEM::gps, ObservationCode("C2W"), -1.5457277801631593e0);
  
  // make map to extract GnssObservables for Obs Rnx
  std::map<SATELLITE_SYSTEM, std::vector<GnssObservable>> map;
  map[satsys] = std::vector<GnssObservable>{gc1c};
  auto sat_obs_map = obsrnx.set_read_map(map);
  if (sat_obs_map.empty()) return 100;
  std::cout<<"\nList of observables to collect per satellite system:";
  for (const auto& m : sat_obs_map) {
    if (!m.second.size()) std::cerr<<"\nWarning empty vector for satellite sys. "<<ngpt::satsys_to_char(m.first);
  }
  for (auto const& m : map) {
    std::cout<<"\n\tSystem: "<<ngpt::satsys_to_char(m.first);
    for (const auto& v : m.second) std::cout<<" "<<v.to_string();
  }

  // inilialize vector to hold satellite/observation pairs for every epoch
  std::vector<std::pair<Satellite, std::vector<double>>>
    sat_obs_vec (obsrnx.initialize_epoch_vector(sat_obs_map));

  // go on and collect every epoch ....
  double lat, lon, hgt;
  ngpt::car2ell<ngpt::ellipsoid::grs80>(obsrnx.x_approx(), obsrnx.y_approx(), 
                                        obsrnx.z_approx(),lat,lon,hgt);
  Saastamoinen Trop(hgt);
  ngpt::Kalman<5> filter{{obsrnx.x_approx()+1.321, 
                          obsrnx.y_approx()-2.987, 
                          obsrnx.z_approx()-1.568, 0.5e6, .0e0}, };
  std::vector<double> Obs(MAX_SATS);
  std::vector<std::array<double,4>> States(MAX_SATS);
  int status, j, index(0);
  int epoch_counter=0, satsnum;
  ngpt::modified_julian_day mjd;
  double secday, clock, state[6];
  do {
    // get satellite-observations pairs, aka fill in sat_obs_vec
    status = obsrnx.read_next_epoch(sat_obs_map, sat_obs_vec, satsnum, mjd, secday);
    ngpt::datetime<milliseconds> epoch 
      (mjd, milliseconds(static_cast<long>(secday*milliseconds::sec_factor<double>())));
    if (satsnum>4) {
      // for every sat-obs pair
      for (int i=0; i<satsnum; i++) {
        svdit oit=sat_obs_vec.begin()+i; // iterator to sat_obs_vec
        if (std::abs(oit->second[0]-ngpt::RNXOBS_MISSING_VAL)>1e-3) {
          Satellite cursat(oit->first);    // current satellite
          // assert(cursat.system()==SATELLITE_SYSTEM::gps);
          // find satellite's navigation block or read rinex untill we find one
          auto nit = get_valid_msg(navrnx, cursat, epoch, sat_nav_vec, j);
          if (!j) {
            // get position and clock bias for satellite
            assert(nit!=sat_nav_vec.end());
            assert(cursat.system()==nit->system() && cursat.prn()==nit->prn());
            nit->stateNclock(epoch, state, clock);
            // std::cout<<"\n\tSV G"<<cursat.prn()<<" C1C: "<<oit->second[0]<<" X:"<< state[0]<<", Y:"<<state[1]<<", Z:"<<state[2]<<" dT:"<<clock;
            // assign for filter update
            Obs[index] = oit->second[0];
            for (int k=0; k<3; k++) {States[index][k]=state[k];} States[index][3]=clock;
            ++index;
          } else {
            std::cerr<<"\n*** Cannot find valid message for SV "
              <<satsys_to_char(cursat.system())<<cursat.prn()
              <<" Epoch is "<<ngpt::strftime_ymd_hms<milliseconds>(epoch)
              <<" status= "<<j;
          }
        }
      }
      // compute zenith angle per observation
      std::vector<double> zenith_angles;
      try {
        zenith_angles = compute_zenith_angles(index, States, obsrnx.x_approx(), 
            obsrnx.y_approx(), obsrnx.z_approx(), lat, lon, hgt);
      } catch (int ernum) {
        std::cerr<<"\n[ERROR] SV with bad azimouth had obs value "<<Obs[ernum];
        std::cerr<<"\n[ERROR] Num of sats was "<<index;
        zenith_angles.clear();
      }
      if (zenith_angles.size()) {
        // compute and apply Saastamoinen
        auto dT = Trop.correction(zenith_angles);
        std::transform(Obs.begin(), Obs.begin()+index, dT.begin(), Obs.begin(), std::minus<double>());
        // compute weight per observation
        auto W = weighting_fun(zenith_angles);
        // Kalman update
        // for (int i=0;i<index;i++) std::cout<<"\nPseudorange["<<i<<"] = "<<Obs[i];
        filter.update(index, &Obs, &States, secday, &W);
        std::cout<<"\n\""<<ngpt::strftime_ymd_hms<milliseconds>(epoch)<<"\" Sats: "<< index<<" ";
        //if (epoch_counter>2) return 80;
        filter.print_state();
      } else {
        std::cout<<"\n---- No filtering for this epoch!";
      }
    } else {
      std::cout<<"\n[DEBUG] Epoch with too few SVs ("<<satsnum<<") "<<ngpt::strftime_ymd_hms<milliseconds>(epoch);
      std::cout<<"\n---- No filtering for this epoch!";
    }
    ++epoch_counter;
    index=0;
  } while (!status);

  return status;
}
