#include "satellite.hpp"

using ngpt::Satellite;

std::string Satellite::to_string(bool compact) const noexcept {
  std::string sat;
  sat = ngpt::satsys_to_char(__system) +
        (__prn < 10 ? "0" + std::to_string(__prn) : std::to_string(__prn));
  if (compact)
    return sat;

  auto s = std::to_string(__svn);
  if (__svn < 10)
    sat += ("-00" + s);
  else if (__svn < 100)
    sat += ("-0" + s);
  else
    sat += ("-" + s);

  return sat +
         std::string("/" + std::string(__cospar, detail::COSPAR_ID_CHARS));
}
