#ifndef __RINEX_GNSS_HPP__
#define __RINEX_GNSS_HPP__

namespace ngpt
{
namespace rinex
{
  /// @brief Make a two-digit year to a four digit year accorind to RINEX
  /// specifications
  /// @param[in] y A two-digit year (0-99)
  /// @return      A four-digit year, depending on y:
  ///              80-99: 1980-1999
  ///              00-79: 2000-2079
  inline int
  four2two_digit_year(int y) noexcept
  {
    return y<=79 ? 2000+y : 1900+y;
  }
}// rinex
}// ngpt

#endif
