#ifndef __ANTEXX_HPP__
#define __ANTEXX_HPP__

#include <fstream>
#include "satellite.hpp"
#include "antenna.hpp"
#include "antenna_pcv.hpp"
#include "ggdatetime/dtcalendar.hpp"

/// @file      antex.hpp
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

namespace ngpt
{

/// @class Antex
/// @see ftp://igs.org/pub/station/general/antex14.txt
class Antex
{
public:
    /// Let's not write this more than once.
    typedef std::ifstream::pos_type pos_type;

    /// Valid atx versions.
    enum class atx_version : char {
        v14, ///< Actually, the only valid !
        v13  ///< Used by EUREF but i can't find the dox
    };

    /// Constructor from filename.
    explicit
    Antex(const char*);

    /// Destructor (closing the file is not mandatory, but nevertheless)
    ~Antex() noexcept 
    { if (__istream.is_open()) __istream.close(); }
  
    /// Copy not allowed !
    Antex(const Antex&) = delete;

    /// Assignment not allowed !
    Antex& operator=(const Antex&) = delete;
  
    /// Move Constructor.
    /// TODO In gcc 4.8 this only compiles if the noexcept specification is
    ///      commented out
    Antex(Antex&& a)
        noexcept(std::is_nothrow_move_constructible<std::ifstream>::value) = default;

    /// Move assignment operator.
    Antex& operator=(Antex&& a) 
        noexcept(std::is_nothrow_move_assignable<std::ifstream>::value) = default;

    int
    get_antenna_pco(const Antenna& ant_in, AntennaPcoList& pco_list,
        bool must_match_serial=false);

    int
    find_satellite_antenna(int, satellite_system, const ngpt::datetime<ngpt::seconds>& at);
#ifdef DEBUG
    int read_headerD() {return this->read_header();}
    int read_next_antenna_typeD(ngpt::Antenna& antenna){return this->read_next_antenna_type(antenna);}
    int skip_rest_of_antennaD(){return this->skip_rest_of_antenna();}
#endif
private:
    /// Read the instance header, and assign (most of) the fields.
    int
    read_header();

    /// Read next antenna (from the stream)
    int
    read_next_antenna_type(Antenna& antenna, char* c=nullptr);
    
    /// Skip antenna info
    int
    skip_rest_of_antenna();
    
    /// Try to match a given antenna to a record in the atx file.
    int
    find_closest_antenna_match(const Antenna& ant_in,
        Antenna& ant_out, pos_type& ant_pos);


    std::string            __filename;    ///< The name of the antex file.
    std::ifstream          __istream;     ///< The infput (file) stream.
    satellite_system       __satsys;      ///< satellite system.
    atx_version            __version;     ///< Atx version (1.4).
    pos_type               __end_of_head; ///< Mark the 'END OF HEADER' field.
}; // Antex

} // ngpt

#endif
