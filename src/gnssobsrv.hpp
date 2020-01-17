#ifndef __GNSS_OBSRV_HPP__
#define __GNSS_OBSRV_HPP__

/// @file      gnssobsrv.hpp
///
/// @version   0.10
///
/// @author    xanthos@mail.ntua.gr <br>
///            danast@mail.ntua.gr
///
/// @date      Mon 11 Feb 2019 01:08:33 PM EET 
///
/// @brief    Define the basics of GNSS observables
/// 
/// @see       RINEX v3.x
///
/// @copyright Copyright © 2019 Dionysos Satellite Observatory, <br>
///            National Technical University of Athens. <br>
///            This work is free. You can redistribute it and/or modify it under
///            the terms of the Do What The Fuck You Want To Public License,
///            Version 2, as published by Sam Hocevar. See http://www.wtfpl.net/
///            for more details.

#include "satsys.hpp"
#include "gnssobs.hpp"

namespace ngpt
{
class GnssRawObservable
{
public:
  explicit
  GnssRawObservable(ngpt::SATELLITE_SYSTEM sys, ngpt::ObservationCode code)
  noexcept
  : __sys(sys),
    __code(code)
  {};
private:
  ngpt::SATELLITE_SYSTEM __sys;
  ngpt::ObservationCode  __code;
}; // class GnssRawObservable

struct __ObsPart
{
  GnssRawObservable __type;
  double            __coef;
  __ObsPart(GnssRawObservable o, double c=1e0) noexcept
  : __type(o),
    __coef(c)
  {};
  __ObsPart(ngpt::SATELLITE_SYSTEM sys, ngpt::ObservationCode code, 
    double c=1e0)
  noexcept
  : __type(GnssRawObservable{sys, code}),
    __coef(c)
  {};
}; // __ObsPart

class GnssObservable
{
public:
  GnssObservable(ngpt::SATELLITE_SYSTEM sys, ngpt::ObservationCode code, 
    double coef=1e0)
  noexcept
  {__vec.emplace_back(sys, code, coef);}
  
  GnssObservable(GnssRawObservable obs, double coef=1e0)
  noexcept
  {__vec.emplace_back(obs, coef);}

  void
  add(GnssRawObservable obs, double coef=1e0)
  noexcept
  {__vec.emplace_back(obs, coef);}

  void
  add(ngpt::SATELLITE_SYSTEM sys, ngpt::ObservationCode code, 
    double coef=1e0)
  noexcept
  {__vec.emplace_back(sys, code, coef);}
private:
  std::vector<__ObsPart> __vec;
}; // class GnssObservable

} // namespace ngpt

#endif
