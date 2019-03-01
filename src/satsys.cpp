#include <stdexcept>
#include "satsys.hpp"

/// Initialize the static frequency map for GPS. Values are {index, frequency}.
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::gps>
    ::frequency_map =
{
  { 1, 1575.42e0 },
  { 2, 1227.60e0 },
  { 5, 1176.45e0 }
};

/// Initialize the static frequency map for GPS.
const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::gps>
    ::valid_atributes =
{
  { 1, std::string("CSLXPWYMN?" ) },
  { 2, std::string("CDSLXPWYMN?") },
  { 5, std::string("IQX?"       ) }
};

/// Initialize the static frequency map for GLONASS.
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::glonass>
    ::frequency_map =
{
  { 1, 1602.000e0 },
  { 2, 1246.000e0 },
  { 3, 1202.025e0 }
};

const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::glonass>
    ::valid_atributes =
{
  { 1, std::string("CP?" ) },
  { 2, std::string("CP?" ) },
  { 3, std::string("IQX?") }
};

/// Initialize the static frequency map for GALILEO.
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::galileo>
    ::frequency_map =
{
  { 1, 1575.420e0 }, ///< E1
  { 5, 1176.450e0 }, ///< E5a
  { 7, 1207.140e0 }, ///< E5b
  { 8, 1191.795e0 }, ///< E5(E5a+E5b)
  { 6, 1278.750e0 }  ///< E6
};

const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::galileo>
    ::valid_atributes =
{
  { 1, std::string("ABCXZ?") },
  { 5, std::string("IQX?"  ) },
  { 7, std::string("IQX?"  ) },
  { 8, std::string("IQX?"  ) },
  { 6, std::string("ABCXZ?") }
};

/// Initialize the static frequency map for SBAS.
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::sbas>
    ::frequency_map =
{
  { 1, 1575.42e0 },
  { 5, 1176.45e0 }
};

const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::sbas>
    ::valid_atributes =
{
  { 1, std::string("C?"  ) },
  { 5, std::string("IQX?") }
};

/// Initialize the static frequency map for QZSS.
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::qzss>
    ::frequency_map =
{
  { 1, 1575.42e0 },
  { 2, 1227.60e0 },
  { 5, 1176.45e0 },
  { 6, 1278.75e0 } ///< LEX
};

const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::qzss>
    ::valid_atributes =
{
  { 1, std::string("CSLXZ?") },
  { 2, std::string("SLX?"  ) },
  { 5, std::string("IQX?"  ) },
  { 6, std::string("SLX?"  ) }
};

/// Initialize the static frequency map for BDS.
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::beidou>
    ::frequency_map =
{
  { 1, 1561.098e0 },
  { 2, 1207.140e0 },
  { 3, 1268.520e0 }
};

const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::beidou>
    ::valid_atributes =
{
  { 1, std::string("IQX?") },
  { 2, std::string("IQX?") },
  { 3, std::string("IQX?") }
};

/// Initialize the static frequency map for IRNSS.
/// \todo in \cite rnx303 the 2nd frequency band is denoted as 'S'
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::irnss>
    ::frequency_map =
{
  { 5, 1176.450e0 },
  { 9, 2492.028e0 }
};

const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::SATELLITE_SYSTEM::irnss>
    ::valid_atributes =
{
  { 5, std::string("ABCX?") },
  { 9, std::string("ABCX?") }
};

/// @details  Given a satellite system enumerator, this function will return
///           it's identifier (e.g. given SatelliteSystem = GPS, the function
///           will return 'G'). The identifiers are taken from RINEX v3.02
///
/// @param[in] s Input Satellite System.
/// @return      A char, representing the satellite system
char
ngpt::satsys_to_char(ngpt::SATELLITE_SYSTEM s) noexcept
{
  using ngpt::SATELLITE_SYSTEM;

  switch (s)
  {
    case SATELLITE_SYSTEM::gps :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::gps>
        ::identifier;
    case SATELLITE_SYSTEM::glonass :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::glonass>
        ::identifier;
    case SATELLITE_SYSTEM::sbas :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::sbas>
        ::identifier;
    case SATELLITE_SYSTEM::galileo :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::galileo>
        ::identifier;
    case SATELLITE_SYSTEM::beidou :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::beidou>
        ::identifier;
    case SATELLITE_SYSTEM::qzss :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::qzss>
        ::identifier;
    case SATELLITE_SYSTEM::irnss :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::irnss>
        ::identifier;
    case SATELLITE_SYSTEM::mixed :
      return ngpt::satellite_system_traits<SATELLITE_SYSTEM::mixed>
        ::identifier;
  }
  // shoud never reach here!
  return ngpt::satellite_system_traits<SATELLITE_SYSTEM::mixed>
    ::identifier;
}

/// @details  Given a char, this function will return the corresponding
///           satellite system. The identifier are taken from RINEX v3.02
///
/// @param[in] c Input char.
/// @return      The corresponding satellite system
///
/// @throw       std::runtime_error if no matching satellite system is found.
ngpt::SATELLITE_SYSTEM
ngpt::char_to_satsys(char c)
{
  using ngpt::SATELLITE_SYSTEM;

  switch (c)
  {
    case ngpt::satellite_system_traits<SATELLITE_SYSTEM::gps>::identifier :
      return SATELLITE_SYSTEM::gps;
    case ngpt::satellite_system_traits<SATELLITE_SYSTEM::glonass>::identifier :
      return SATELLITE_SYSTEM::glonass;
    case ngpt::satellite_system_traits<SATELLITE_SYSTEM::galileo>::identifier :
      return SATELLITE_SYSTEM::galileo;
    case ngpt::satellite_system_traits<SATELLITE_SYSTEM::sbas>::identifier :
      return SATELLITE_SYSTEM::sbas;
    case ngpt::satellite_system_traits<SATELLITE_SYSTEM::qzss>::identifier :
      return SATELLITE_SYSTEM::qzss;
    case ngpt::satellite_system_traits<SATELLITE_SYSTEM::beidou>::identifier :
      return SATELLITE_SYSTEM::beidou;
    case ngpt::satellite_system_traits<SATELLITE_SYSTEM::irnss>::identifier :
      return SATELLITE_SYSTEM::irnss;
    case ngpt::satellite_system_traits<SATELLITE_SYSTEM::mixed>::identifier :
      return SATELLITE_SYSTEM::mixed;
    default :
      throw std::runtime_error
        ("ngpt::charToSatSys -> Invalid Satellite System Identifier!!");
  }
}
