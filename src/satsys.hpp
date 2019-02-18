#ifndef __SATELLITE_SYSTEM_HPP__
#define __SATELLITE_SYSTEM_HPP__

/// @file
///
/// @version
///
/// @author    xanthos@mail.ntua.gr <br>
///           danast@mail.ntua.gr
///
/// @date      Sat 19 Dec 2015 02:23:35 PM EET 
///
/// @brief     Define the Global Navigation Satellite Systems (GNSS).
///
/// @details   GNSS are declared as enumerations; the template structure
///           ngpt::satellite_system_traits<>, specialized for each of the
///           systems, offers access to the characteristics of each satellite
///           system. Always use this structure to access sat. system traits,
///           e.g. don't automagically use 'G' to refer to GPS; use
///           satellite_system_traits<satellite_system::gps>::identifier.
///
/// @note      Each of the (enumerated) satellite systems, <em>should</em> have
///           a specialized satellite_system_traits<> class. In case a
///           satellite systems is added in the satellite_system enumeration
///           class, you should:
///           - add a specilized satellite_system_traits<> class
///           - modify satsys_identifier function
///           - modify char_to_satsys function
///           - modify nominal_frequency function
///           These are all dead simple modifications; one line is enough.
///
/// @copyright Copyright © 2015 Dionysos Satellite Observatory, <br>
///           National Technical University of Athens. <br>
///           This work is free. You can redistribute it and/or modify it under
///           the terms of the Do What The Fuck You Want To Public License,
///           Version 2, as published by Sam Hocevar. See http://www.wtfpl.net/
///           for more details.
///
/// <b><center><hr>
/// National Technical University of Athens <br>
///      Dionysos Satellite Observatory     <br>
///        Higher Geodesy Laboratory        <br>
///      http://dionysos.survey.ntua.gr
/// <hr></center></b>

#include <cstddef>
#include <stdexcept>
#include <map> 

namespace ngpt
{

/// Enumeration for known Satellite systems. This is extracted from 
/// \cite rnx303
///
/// \note Each of the enumerations recorded here, should have a specialized
///       ngpt::satellite_system_traits<> template.
///
enum class satellite_system
: char
{
    gps, glonass, sbas, galileo, beidou, qzss, irnss, mixed
};

/// Traits for Satellite Systems. A collection of satellite system - specific
/// "static" information for each system in ngpt::satellite_system.
///
/// \note We will have to specialize for each ngpt::satellite_system.
///
template<satellite_system S>
    struct satellite_system_traits
{};

/// Specialize traits for Satellite System Gps
template<>
    struct satellite_system_traits<satellite_system::gps>
{
    /// Identifier
    static constexpr char identifier { 'G' };

    /// Dictionary holding pairs of <frequency band, freq. value>.
    static const std::map<int, double> frequency_map;

    /// Dictionary holding pairs of <frequency band, std::string>. The
    /// string is a seqeuence of (the **only**) valid attributes for
    /// each frequency band.
    static const std::map<int, std::string> valid_atributes;
};

/// Specialize traits for Satellite System Glonass
template<>
    struct satellite_system_traits<satellite_system::glonass>
{
    /// Identifier
    static constexpr char identifier { 'R' };

    /// Dictionary holding pairs of <frequency band, freq. value>.
    static const std::map<int, double> frequency_map;
    
    /// Dictionary holding pairs of <frequency band, std::string>. The
    /// string is a seqeuence of (the **only**) valid attributes for
    /// each frequency band.
    static const std::map<int, std::string> valid_atributes;
};

/// Specialize traits for Satellite System Galileo
template<>
    struct satellite_system_traits<satellite_system::galileo>
{
    /// Identifier
    static constexpr char identifier { 'E' };

    /// Dictionary holding pairs of <frequency band, freq. value>.
    static const std::map<int, double> frequency_map;
    
    /// Dictionary holding pairs of <frequency band, std::string>. The
    /// string is a seqeuence of (the **only**) valid attributes for
    /// each frequency band.
    static const std::map<int, std::string> valid_atributes;
};

/// Specialize traits for Satellite System SBAS
template<>
    struct satellite_system_traits<satellite_system::sbas>
{
    /// Identifier
    static constexpr char identifier { 'S' };

    /// Dictionary holding pairs of <frequency band, freq. value>.
    static const std::map<int, double> frequency_map;

    /// Dictionary holding pairs of <frequency band, std::string>. The
    /// string is a seqeuence of (the **only**) valid attributes for
    /// each frequency band.
    static const std::map<int, std::string> valid_atributes;
};

/// Specialize traits for Satellite System QZSS
template<>
    struct satellite_system_traits<satellite_system::qzss>
{
    /// Identifier
    static constexpr char identifier { 'J' };

    /// Dictionary holding pairs of <frequency band, freq. value>.
    static const std::map<int, double> frequency_map;

    /// Dictionary holding pairs of <frequency band, std::string>. The
    /// string is a seqeuence of (the **only**) valid attributes for
    /// each frequency band.
    static const std::map<int, std::string> valid_atributes;
};

/// Specialize traits for Satellite System BDS
template<>
    struct satellite_system_traits<satellite_system::beidou>
{
    /// Identifier
    static constexpr char identifier { 'C' };

    /// Dictionary holding pairs of <frequency band, freq. value>.
    static const std::map<int, double> frequency_map;

    /// Dictionary holding pairs of <frequency band, std::string>. The
    /// string is a seqeuence of (the **only**) valid attributes for
    /// each frequency band.
    static const std::map<int, std::string> valid_atributes;
};

/// Specialize traits for Satellite System IRNSS
template<>
    struct satellite_system_traits<satellite_system::irnss>
{
    /// Identifier
    static constexpr char identifier { 'I' };

    /// Dictionary holding pairs of <frequency band, freq. value>.
    static const std::map<int, double> frequency_map;

    /// Dictionary holding pairs of <frequency band, std::string>. The
    /// string is a seqeuence of (the **only**) valid attributes for
    /// each frequency band.
    static const std::map<int, std::string> valid_atributes;
};

/// Specialize traits for Satellite System MIXED
template<>
    struct satellite_system_traits<satellite_system::mixed>
{
    /// Identifier
    static constexpr char identifier { 'M' };

    /// Number of frequency bands.
    static const std::size_t num_of_bands { 0 };

    /// Dictionary holding pairs of <frequency band, std::string>. The
    /// string is a seqeuence of (the **only**) valid attributes for
    /// each frequency band.
    static const std::map<int, std::string> valid_atributes;
};

/// Given a satellite system, return its identifier (i.e. the identifying
/// char).
char
satsys_identifier(satellite_system);

/// Given a char (i.e. the identifyingchar), return the corresponding 
/// satellite system.
satellite_system
char_to_satsys(char);

/// Given a frequency band (index) and a satellite system, return the nominal 
/// frequency value (Hz).
double
nominal_frequency(int band, satellite_system s);

} // end namespace

#endif
