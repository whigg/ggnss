#ifndef __GNSS_ANTENNA_HPP__
#define __GNSS_ANTENNA_HPP__

/// @file      antenna.hpp
///
/// @version   0.10
///
/// @author    xanthos@mail.ntua.gr <br>
///            danast@mail.ntua.gr
///
/// @date      Mon 11 Feb 2019 01:08:33 PM EET 
///
/// @brief     GNSS Antenna (plus Radome) Class. Used both for stations and
///            satellites.
/// 
/// @see       http://www.epncb.oma.be/ftp/station/general/rcvr_ant.tab
///
/// @copyright Copyright © 2019 Dionysos Satellite Observatory, <br>
///            National Technical University of Athens. <br>
///            This work is free. You can redistribute it and/or modify it under
///            the terms of the Do What The Fuck You Want To Public License,
///            Version 2, as published by Sam Hocevar. See http://www.wtfpl.net/
///            for more details.

namespace ngpt
{

    /// Namespace to hide antenna specific details.
    namespace antenna_details
    {
        /// Maximum number of characters describing a GNSS antenna model 
        /// (no radome).
        constexpr std::size_t antenna_model_max_chars { 15 };

        /// Maximum number of characters describing a GNSS antenna radome type
        constexpr std::size_t antenna_radome_max_chars { 4 };

        /// Maximum number of characters describing a GNSS antenna serial number
        constexpr std::size_t antenna_serial_max_chars { 20 };

        /// Maximum size to represent all fields, including whitespaces and
        /// the (last) null terminating character.
        constexpr std::size_t antenna_full_max_chars
        {  antenna_model_max_chars  + 1 /* whitespace */
         + antenna_radome_max_chars
         + antenna_serial_max_chars + 1 /* null-reminating char */
        };

        constexpr std::size_t satellite_antenna_max_chars {20};
    }

/// @class    Antenna
///
/// @details  This class holds a GNSS Antenna either for a satellite or a 
///           receiver. Every antenna is represented by a specific Antenna model
///           name, a Radome model and a Serial Number. These are all concateneted
///           in a char array (but not exactly a c-string). See the note below
///           for how this character array is formed.
///
/// @see      rcvr_ant
///
/// @note     The c-string array allocated for each instance, looks like:
/*
@verbatim
  N = antenna_model_max_chars
  M = antenna_radome_max_chars
  K = antenna_serial_max_chars
  [0,     N)       antenna model name
  [N+1,   N+M)     antenna radome name
  [N+M+1, N+M+K+1) antenna serial number

  | model name      |   | radome      | serial number     
  v                 v   v             v    
  +---+---+-...-+---+---+---+-...-+---+-----+-----+-...-+-------+
  | 0 | 1 |     |N-1| N |N+1|     |N+M|N+M+1|N+M+2|     |N+M+K+1|
  +---+---+-...-+---+---+---+-...-+---+-----+-----+-...-+-------+
                      ^                                     ^       
                      |                                     |      
            whitespace character                           '\0'

@endverbatim
 */
///
/// @example  test_antenna.cpp
///           An example to show how to use the Antenna class.
///
class Antenna
{
public:

    /// Antenna Type: Receiver or Satellite.
    enum class Antenna_Type : char 
    { 
      Receiver_Antenna, 
      Satellite_Antenna 
    };

    /// @brief Default constructor; model, radome and serial are set to 
    ///        whitespace
    Antenna() noexcept
    { 
        set_to_wspace();
        set_none_radome();
    }

    /// @brief Constructor from a c-string (model+radome+serial if any)
    explicit Antenna
    (const char*) noexcept;

    /// @brief Constructor from a string (model+radome+serial if any)
    explicit Antenna
    (const std::string&) noexcept;

    /// @brief Equality operator (checks antenna type, radome and serial nr).
    bool
    is_same(const Antenna&) const noexcept;

    /// @brief Compare antenna's serial number to a c-string
    bool
    compare_serial(const char*) const noexcept;

    /// @brief Check if antenna ha serial number
    bool
    has_serial() const noexcept;

    /// @brief Compare model and radome (diregard serials if any)
    bool
    compare_model(const Antenna&) const noexcept;

    /// @brief Set antenna's serial number
    void
    set_serial_nr(const char*) noexcept;

    /// @brief Antenna model name as string.
    std::string
    model_str() const noexcept;

    /// @brief Antenna radome name as string.
    std::string
    radome_str() const noexcept;

    /// @brief
    const char*
    __underlying_char__() const noexcept
    { return __name; }

#ifdef DEBUG
    std::string
    to_string() const noexcept;
#endif

private:

    /// @brief Initialize antenna, aka set all chars to " ".
    void
    set_to_wspace() noexcept;

    /// @brief Set radome to 'NONE'
    void
    set_none_radome() noexcept;

    /// @brief Copy from an std::string
    void
    copy_from_str(const std::string&) noexcept;

    /// @brief Copy from a c-string
    void
    copy_from_cstr(const char*) noexcept;

    /// @brief check if radome is empty (aka only whitespaces)
    bool
    radome_is_empty() const noexcept;
    
    /// Combined antenna, radome and serial number (model+dome+serial).
    char __name[antenna_details::antenna_full_max_chars]; 

}; // Antenna

class SatelliteAntenna
{
public:
    explicit
    SatelliteAntenna(const char* c) noexcept;

    int
    compare(const char*) const noexcept;

    void
    set_from_cstr(const char* ) noexcept;
    
    /// @brief
    const char*
    __underlying_char__() const noexcept
    { return __name; }

private:
    char __name[antenna_details::satellite_antenna_max_chars+1]; 
}; // SatelliteAntenna

} // ngpt

#endif
