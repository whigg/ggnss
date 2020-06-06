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

#include <vector>
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

  ngpt::SATELLITE_SYSTEM&
  satsys() noexcept
  {return __sys;}
  
  ngpt::SATELLITE_SYSTEM
  satsys() const noexcept
  {return __sys;}
  
  int
  band() const noexcept
  {return __code.band();}

  ngpt::ObservationCode
  code() const noexcept {return __code;}

  bool
  operator==(const GnssRawObservable& o) const noexcept
  {return __sys==o.__sys && __code==o.__code;}

  bool
  operator!=(const GnssRawObservable& o) const noexcept
  {return !(*this == o);}

  std::string
  to_string() const noexcept;
  
  // @brief Template const getter function (aka instance.get<N>() with 0<=N<2)
  // @warning this is C++17
  template<std::size_t N>
    decltype(auto) get() const noexcept
  {
    static_assert(N<2);
    if constexpr (N==0) return __sys;
    else                return __code;
  }

  // @brief Template non-const getter function (aka instance.get<N>() with 0<=N<2)
  // Notice the parenthesis at the return statement; this function returns
  // references to the instance's members
  // @warning this is C++17
  template<std::size_t N>
    decltype(auto) get() noexcept
  {
    static_assert(N<2);
    if constexpr (N==0) return (__sys);
    else                return (__code);
  }

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

  bool
  operator==(const __ObsPart& o) const noexcept
  {return __type==o.__type && __coef==o.__coef;}
  
  bool
  operator!=(const __ObsPart& o) const noexcept
  {return !(*this == o);}

  /// nominal frequency multiplied by coefficient in MHz
  double
  frequency() const;

  /// return the type
  GnssRawObservable
  type() const noexcept {return __type;}
  
  std::string
  to_string() const noexcept;

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

  double
  frequency() const noexcept
  {
    double frequency = 0e0;
    for (const auto& v : __vec) frequency += v.frequency();
    return frequency;
  }
  
  std::vector<__ObsPart>
  underlying_vector() const noexcept {return __vec;}

  std::vector<__ObsPart>&
  underlying_vector() noexcept {return __vec;}

  bool
  operator==(const GnssObservable&) const noexcept;

  bool
  operator!=(const GnssObservable&) const noexcept
  {return !(*this == o);}

  bool
  is_of_mixed_satsys() const noexcept
  {
    if (!__vec.size()) return false;
    auto sys = __vec[0].__type.satsys();
    for (const auto& obspart : __vec) if (obspart.__type.satsys()!=sys) return true;
    return false;
  }
  
  std::string
  to_string() const noexcept;

private:
  std::vector<__ObsPart> __vec;
}; // class GnssObservable

} // namespace ngpt

namespace std {
// @brief Specialize tuple_size for ngpt::GnssRawObservable in std (used for
//        structured bindings)
template<>
  struct tuple_size<ngpt::GnssRawObservable> : integral_constant<size_t, 2> {};

// @brief Specialize tuple_element for ngpt::GnssRawObservable in std (used for
//        structured bindings). No default/empty co'tor, so use declval.
template<size_t N>
  struct tuple_element<N,ngpt::GnssRawObservable> {
    using type = decltype(declval<ngpt::GnssRawObservable>().get<N>());
};
} // std

#endif
