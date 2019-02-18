#ifndef __GNSS_OBS_HPP__
#define __GNSS_OBS_HPP__

/// @file      gnssobs.hpp
///
/// @version   0.10
///
/// @author    xanthos@mail.ntua.gr <br>
///            danast@mail.ntua.gr
///
/// @date      Mon 11 Feb 2019 01:08:33 PM EET 
///
/// @brief     
/// 
/// @see       
///
/// @copyright Copyright © 2019 Dionysos Satellite Observatory, <br>
///            National Technical University of Athens. <br>
///            This work is free. You can redistribute it and/or modify it under
///            the terms of the Do What The Fuck You Want To Public License,
///            Version 2, as published by Sam Hocevar. See http://www.wtfpl.net/
///            for more details.

// forward declare
namespace ngpt { class ObservationCode; }
std::ostream& operator<<(std::ostream&, const ngpt::ObservationCode&);

namespace ngpt
{

/// @enum observable_type
/// @see ftp://ftp.igs.org/pub/data/format/rinex304.pdf
enum class observable_type
: char
{
    pseudorange,
    carrier_phase,
    doppler,
    signal_strength,
    ionosphere_phase_delay,  ///< rnx304, sec. 5.12
    receiver_channel_number, ///< rnx304, sec. 5.13
    any                      ///< match anything
}; // observable_type

/// TODO: what char should represent 'any'? 'any' is translated to '?' but
/// should also work the other way around, or use e.g. ' '?
observable_type
char_to_observabletype(char);

char
observabletype_to_char(observable_type);

/// @class Attribute
///
/// @note The character '?', denotes any attribute
///
/// @see ftp://ftp.igs.org/pub/data/format/rinex304.pdf
class ObservationAttribute
{
public:
    explicit
    ObservationAttribute(char c='?')
    noexcept
    : __c(c)
    {}
    char
    as_char() const noexcept
    {return __c;}
private:
    char __c; ///< tracking mode or channel
}; // Attribute

/// @class ObservationCode
///
/// @see ftp://ftp.igs.org/pub/data/format/rinex304.pdf
class ObservationCode
{
public:
    explicit
    ObservationCode(observable_type otype=observable_type::any, int band=0,
        ObservationAttribute att=ObservationAttribute())
    noexcept
    : __type(otype),
      __band(band),
      __attr(att)
    {};

    explicit
    ObservationCode(const char* str);

    int&
    band() noexcept
    {return __band;}

    int
    band() const noexcept
    {return __band;}

    std::string
    to_string() const;

private:
    observable_type      __type;
    int                  __band;
    ObservationAttribute __attr;
}; // ObservationCode

} // ngpt

#endif
