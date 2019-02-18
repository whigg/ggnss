#include <stdexcept>
#include "satsys.hpp"

/// Initialize the static frequency map for GPS. Values are {index, frequency}.
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::satellite_system::gps>
    ::frequency_map =
{
    { 1, 1575.42e0 },
    { 2, 1227.60e0 },
    { 5, 1176.45e0 }
};

/// Initialize the static frequency map for GPS.
const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::satellite_system::gps>
    ::valid_atributes =
{
    { 1, std::string("CSLXPWYMN?" ) },
    { 2, std::string("CDSLXPWYMN?") },
    { 5, std::string("IQX?"       ) }
};

/// Initialize the static frequency map for GLONASS.
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::satellite_system::glonass>
    ::frequency_map =
{
    { 1, 1602.000e0 },
    { 2, 1246.000e0 },
    { 3, 1202.025e0 }
};

const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::satellite_system::glonass>
    ::valid_atributes =
{
    { 1, std::string("CP?" ) },
    { 2, std::string("CP?" ) },
    { 3, std::string("IQX?") }
};

/// Initialize the static frequency map for GALILEO.
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::satellite_system::galileo>
    ::frequency_map =
{
    { 1, 1575.420e0 }, ///< E1
    { 5, 1176.450e0 }, ///< E5a
    { 7, 1207.140e0 }, ///< E5b
    { 8, 1191.795e0 }, ///< E5(E5a+E5b)
    { 6, 1278.750e0 }  ///< E6
};

const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::satellite_system::galileo>
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
ngpt::satellite_system_traits<ngpt::satellite_system::sbas>
    ::frequency_map =
{
    { 1, 1575.42e0 },
    { 5, 1176.45e0 }
};

const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::satellite_system::sbas>
    ::valid_atributes =
{
    { 1, std::string("C?"  ) },
    { 5, std::string("IQX?") }
};

/// Initialize the static frequency map for QZSS.
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::satellite_system::qzss>
    ::frequency_map =
{
    { 1, 1575.42e0 },
    { 2, 1227.60e0 },
    { 5, 1176.45e0 },
    { 6, 1278.75e0 } ///< LEX
};

const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::satellite_system::qzss>
    ::valid_atributes =
{
    { 1, std::string("CSLXZ?") },
    { 2, std::string("SLX?"  ) },
    { 5, std::string("IQX?"  ) },
    { 6, std::string("SLX?"  ) }
};

/// Initialize the static frequency map for BDS.
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::satellite_system::beidou>
    ::frequency_map =
{
    { 1, 1561.098e0 },
    { 2, 1207.140e0 },
    { 3, 1268.520e0 }
};

const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::satellite_system::beidou>
    ::valid_atributes =
{
    { 1, std::string("IQX?") },
    { 2, std::string("IQX?") },
    { 3, std::string("IQX?") }
};

/// Initialize the static frequency map for IRNSS.
/// \todo in \cite rnx303 the 2nd frequency band is denoted as 'S'
const std::map<int, double>
ngpt::satellite_system_traits<ngpt::satellite_system::irnss>
    ::frequency_map =
{
    { 5, 1176.450e0 },
    { 9, 2492.028e0 }
};

const std::map<int, std::string>
ngpt::satellite_system_traits<ngpt::satellite_system::irnss>
    ::valid_atributes =
{
    { 5, std::string("ABCX?") },
    { 9, std::string("ABCX?") }
};

/// \details  Given a satellite system enumerator, this function will return
///           it's identifier (e.g. given SatelliteSystem = GPS, the function
///           will return 'G'). The identifiers are taken from RINEX v3.02
///
/// \param[in] s Input Satellite System.
/// \return      A char, representing the satellite system
///
/// \throw       std::runtime_error if no matching satellite system is found.
char
ngpt::satsys_identifier(ngpt::satellite_system s)
{
    using ngpt::satellite_system;

    switch ( s )
    {
        case satellite_system::gps :
            return ngpt::satellite_system_traits<satellite_system::gps>
                   ::identifier;
        case satellite_system::glonass :
            return ngpt::satellite_system_traits<satellite_system::glonass>
                   ::identifier;
        case satellite_system::sbas :
            return ngpt::satellite_system_traits<satellite_system::sbas>
                   ::identifier;
        case satellite_system::galileo :
            return ngpt::satellite_system_traits<satellite_system::galileo>
                   ::identifier;
        case satellite_system::beidou :
            return ngpt::satellite_system_traits<satellite_system::beidou>
                   ::identifier;
        case satellite_system::qzss :
            return ngpt::satellite_system_traits<satellite_system::qzss>
                   ::identifier;
        case satellite_system::irnss :
            return ngpt::satellite_system_traits<satellite_system::irnss>
                   ::identifier;
        case satellite_system::mixed :
            return ngpt::satellite_system_traits<satellite_system::mixed>
                   ::identifier;
        default:
            throw std::runtime_error
                ("ngpt::SatSysIdentifier -> Invalid Satellite System !!");
    }
}

/// \details  Given a char, this function will return the corresponding
///           satellite system. The identifier are taken from RINEX v3.02
///
/// \param[in] c Input char.
/// \return      The corresponding satellite system
///
/// \throw       std::runtime_error if no matching satellite system is found.
ngpt::satellite_system
ngpt::char_to_satsys(char c)
{
    using ngpt::satellite_system;

    switch ( c )
    {
        case (ngpt::satellite_system_traits<satellite_system::gps>::identifier) :
            return satellite_system::gps;
        case (ngpt::satellite_system_traits<satellite_system::glonass>::identifier) :
            return satellite_system::glonass;
        case (ngpt::satellite_system_traits<satellite_system::galileo>::identifier) :
            return satellite_system::galileo;
        case (ngpt::satellite_system_traits<satellite_system::sbas>::identifier) :
            return satellite_system::sbas;
        case (ngpt::satellite_system_traits<satellite_system::qzss>::identifier) :
            return satellite_system::qzss;
        case (ngpt::satellite_system_traits<satellite_system::beidou>::identifier) :
            return satellite_system::beidou;
        case (ngpt::satellite_system_traits<satellite_system::irnss>::identifier) :
            return satellite_system::irnss;
        case (ngpt::satellite_system_traits<satellite_system::mixed>::identifier) :
            return satellite_system::mixed;
        default :
            throw std::runtime_error
                ("ngpt::charToSatSys -> Invalid Satellite System Identifier!!");
    }
}

/// \details     Given a frequency band (index) and a satellite system,
///              return the nominal frequency value.
///
/// \param[in]   band The frequency band.
/// \param[in]   s    The satellite system.
/// \return           The corresponding (band/system) nominal frequency in Hz.
///
/// \throw       std::runtime_error if no matching satellite system or 
///              satellite system / frequency band pair is not found.
///
double
ngpt::nominal_frequency(int band, ngpt::satellite_system s)
{
    using ngpt::satellite_system;

    try { // std::map.at() might throw !
        switch ( s )
        {
            case satellite_system::gps :
                return ngpt::satellite_system_traits<satellite_system::gps>
                    ::frequency_map.at(band);
            case satellite_system::glonass :
                return ngpt::satellite_system_traits<satellite_system::glonass>
                    ::frequency_map.at(band);
            case satellite_system::sbas :
                return ngpt::satellite_system_traits<satellite_system::sbas>
                    ::frequency_map.at(band);
            case satellite_system::galileo :
                return ngpt::satellite_system_traits<satellite_system::galileo>
                    ::frequency_map.at(band);
            case satellite_system::beidou :
                return ngpt::satellite_system_traits<satellite_system::beidou>
                    ::frequency_map.at(band);
            case satellite_system::qzss :
                return ngpt::satellite_system_traits<satellite_system::qzss>
                    ::frequency_map.at(band);
            case satellite_system::irnss :
                return ngpt::satellite_system_traits<satellite_system::irnss>
                    ::frequency_map.at(band);
            default:
                // std::map.at() will throw an std::out_of_range; let's do the
                // same so we won;t have to catch more than one exception types.
                throw std::out_of_range
                    ("ngpt::nominal_frequency -> Invalid Satellite System!!");
        }
    } catch (std::out_of_range& e) {
        throw std::runtime_error
            ("ngpt::nominal_frequency -> Invalid Satellite System / Frequency Band pair");
    }
}
