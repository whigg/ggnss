#ifndef __GAUSS_NEWTON_HPP__
#define __GAUSS_NEWTON_HPP__

#include <array>
#include <cmath>
#include <cstdio>
#include "eigen3/Eigen/Core"
#include "eigen3/Eigen/Sparse"
#include "eigen3/Eigen/Geometry"

namespace ngpt {

template<int Params>
class Kalman {
public:
  Kalman(std::initializer_list<double>&& l) {
    assert(l.size()==Params);
    int i=0;
    for (auto v : l) state_(i++) = v;
    initialize_F();
  }

  void
  print_state() {
    for (int i=0; i<5; i++) printf("%20.3f",state_(i));
  }

  void
  update(int nsats, const std::vector<double>* obs, 
    const std::vector<std::array<double,4>>* sv, double dt,
    const std::vector<double>* w=nullptr) {
    // set the vector pointrers
    nsats_ = nsats;
    obs_ = obs;
    sv_ = sv;
    // state prediction : x(1|0) = F(0)*x(0|0)
    // Wait!! update F(3,4) to dt
    F_.coeffRef(3,4) = dt;
    state_ = F_ * state_;
    // measurement prediction: p(1|0) = F(x(1|0)) = sqrt(....) + c*dt
    auto p = compute_pseudorange_vector();
    // measurement residuals: v(1) = p(1) - p(1|0)
    auto v = measurement_vector();
    v -= p;
    // evaluate Jacobian: H(1)
    evaluate_jacobian();
    // state prediction covariance: P(1|0) = F(0)*P(0|0)*F^T(0) + Q
    if (!update_idx) initialize_P();
    P_ = F_ * P_ * F_.transpose();
    P_(3,3)+=0.0114e0;
    P_(3,4)+=0.0019e0;
    P_(4,3)+=0.0019e0;
    P_(4,4)+=0.0039e0;
    // residual covariance: S = H(1)*P(1|0)*H^T(1) + R(1)
    Eigen::MatrixXd S (H_ * P_ * H_.transpose());
    if (update_idx && w) {
      for (int i=0; i<nsats_; i++) S(i,i) += 1e3/(*w)[i];
    } else {
      S.diagonal().array() += 10e0;
    }
    // filter gain W: W = P(1|0) * H^T(1) *S^-1(1)
    Eigen::MatrixXd W = P_ * H_.transpose() * S.inverse();
    // update state covariance
    P_ = P_ - W * S * W.transpose();
    // update state
    state_ = state_ + W * v;
    // update the index
    ++update_idx;
    return;
  }

private:

  void
  initialize_F(double dt=1e0) noexcept {
    Eigen::SparseMatrix<double> sm(Params,Params);
    sm.reserve(Params+1);
    std::vector<Eigen::Triplet<double>> tripletList;
    tripletList.emplace_back(0,0,1e0);
    tripletList.emplace_back(1,1,1e0);
    tripletList.emplace_back(2,2,1e0);
    tripletList.emplace_back(3,3,1e0);
    tripletList.emplace_back(3,4,dt);
    tripletList.emplace_back(4,4,1e0);
    sm.setFromTriplets(tripletList.begin(), tripletList.end());
    F_ = std::move(sm);
  }

  void
  initialize_P(/*double sigma=5e0*/) {
    /*
    double s2=sigma*sigma;
    P_ = Eigen::Matrix<double,Params,Params>::Identity();
    Eigen::DiagonalMatrix<double,Eigen::Dynamic> R(nsats_);
    R.setIdentity(); R = R * (1e0/s2);
    P_ = H_.transpose() * R * H_;
    P_ = P_.inverse();
    */
    P_ = Eigen::Matrix<double,Params,Params>::Identity();
    P_ *= 0.0000025;
    P_.row(4).setZero();
    P_.col(4).setZero();
    P_(4,4) = 0.0039*0.0039;
    return;
  }

  decltype(auto)
  compute_pseudorange_vector() {
    Eigen::Matrix<double,Eigen::Dynamic,1> p;
    p.resize(nsats_, 1);
    const double xr = state_(0);
    const double yr = state_(1);
    const double zr = state_(2);
    const double cdt= state_(3);
    for (int i=0; i<nsats_; i++) {
      const double xs = (*sv_)[i][0];
      const double ys = (*sv_)[i][1];
      const double zs = (*sv_)[i][2];
      p(i) = std::sqrt((xs-xr)*(xs-xr)+(ys-yr)*(ys-yr)+(zs-zr)*(zs-zr)) + cdt;
    }
    return p;
  }

  decltype(auto)
  measurement_vector(bool apply_sat_clock=true) {
    Eigen::Matrix<double,Eigen::Dynamic,1> p;
    p.resize(nsats_, 1);
    if (apply_sat_clock) {
      for (int i=0; i<nsats_; i++) p(i)=(*obs_)[i] + (*sv_)[i][3]*299792458e0;
    } else {
      for (int i=0; i<nsats_; i++) p(i)=(*obs_)[i];
    }
    return p;
  }

  decltype(auto)
  make_R_matrix() {
    
  }

  void
  evaluate_jacobian() {
    H_.resize(nsats_, Params);
    const double xr = state_(0);
    const double yr = state_(1);
    const double zr = state_(2);
    for (int i=0; i<nsats_; i++) {
      const double xs = (*sv_)[i][0];
      const double ys = (*sv_)[i][1];
      const double zs = (*sv_)[i][2];
      double r = std::sqrt((xs-xr)*(xs-xr)+(ys-yr)*(ys-yr)+(zs-zr)*(zs-zr));
      H_(i, 0) = -(xs-xr) / r;
      H_(i, 1) = -(ys-yr) / r;
      H_(i, 2) = -(zs-zr) / r;
      H_(i, 3) = 1e0;
      H_(i, 4) = 0e0;
    }
    return;
  }

  Eigen::Matrix<double,Params,1> state_;
  Eigen::Matrix<double,Params,Params> P_; // TODO this is actually symmetric
  Eigen::Matrix<double,Eigen::Dynamic,Params> H_;
  Eigen::SparseMatrix<double> F_;
  const std::vector<double>* obs_{nullptr};
  const std::vector<std::array<double,4>>* sv_{nullptr};
  int nsats_{0};
  int update_idx{0};
};

} // ngpt
#endif
