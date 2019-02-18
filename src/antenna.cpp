#include <string>
#include <cstring>
#include <algorithm>
#include "antenna.hpp"

using ngpt::Antenna;
using ngpt::SatelliteAntenna;
using ngpt::antenna_details::antenna_model_max_chars;
using ngpt::antenna_details::antenna_radome_max_chars;
using ngpt::antenna_details::antenna_serial_max_chars;
using ngpt::antenna_details::antenna_full_max_chars;
using ngpt::antenna_details::satellite_antenna_max_chars;

/// The 'NONE' radome as a c-string.
constexpr char none_radome[] = "NONE";

/// Set all chars in __name to ' ' (i.e. whitespace). 
void
Antenna::set_to_wspace()
noexcept
{ std::memset(__name, ' ', antenna_full_max_chars * sizeof(char) ); }

/// Set radome to 'NONE'
void
Antenna::set_none_radome()
noexcept
{
    std::memcpy(__name+antenna_model_max_chars+1, none_radome, 
                antenna_radome_max_chars * sizeof(char));
}

/// @details      Set an antenna/radome pair from a c-string. The input 
///               antenna/radome pair, should follow the conventions in 
///               rcvr_ant. Note that the function will overwrite any previous
///               information stored in the __name array.
/// 
/// @param[in] c  A string representing a valid antenna model name (optionaly
///               including a serial number).
/// 
/// @note        At most antenna_full_max_chars - 1 characters will be copied;
///              any remaining characters will be ignored.
/// 
void
Antenna::copy_from_str(const std::string& s)
noexcept
{
    // initialize to spaces
    this->set_to_wspace();
  
    // size of input string
    std::size_t str_size { s.size() };
    std::size_t nrc      { std::min(str_size, antenna_full_max_chars-1) };
    std::copy(s.begin(), s.begin()+nrc, &__name[0]);
    if (radome_is_empty()) set_none_radome();
}

///  @details     Set an antenna/radome pair from a c-string. The input 
///               antenna/radome pair, should follow the conventions in 
///               rcvr_ant. Note that the function will overwrite any previous
///               information stored in the __name array.
/// 
///  @param[in] c A c-string representing a valid antenna model name (optionaly
///               including a serial number).
/// 
///  @note        At most antenna_full_max_chars - 1 characters will be copied;
///               any remaining characters will be ignored.
/// 
void
Antenna::copy_from_cstr(const char* c)
noexcept
{
    // initialize to spaces
    this->set_to_wspace();

    // get the size of the input string.
    std::size_t ant_size { std::strlen(c) };

    // copy at maximum antenna_full_max_chars - 1 characters
    std::memcpy(__name, c, sizeof(char) * 
                          std::min(ant_size, antenna_full_max_chars-1) );
    if (radome_is_empty()) set_none_radome();
}

/// Constructor from antenna type. At maximum antenna_full_max_char - 1 chars
/// are copied from the string.
///
/// @param[in] c A c-string representing an antenna as described in rcvr_ant
///              At most antenna_full_max_chars - 1 characters will be copied;
///              That means that the input string can hold the antenna model,
///              the radome and the serial number.
Antenna::Antenna(const char* c)
noexcept
{this->copy_from_cstr(c);}

/// Constructor from antenna type. At maximum antenna_full_max_char - 1 chars
/// are copied from the string.
///
/// @param[in] c A string representing an antenna as described in rcvr_ant
///              At most antenna_full_max_chars - 1 characters will be copied;
///              That means that the input string can hold the antenna model,
///              the radome and the serial number.
Antenna::Antenna(const std::string& s)
noexcept
{this->copy_from_str(s);}

/// This function compares two Antenna instances and return true if and only
/// if the Antenna is the same, i.e. they share the same model, radome and
/// serial number.
///
/// @param[in] rhs  Antenna instance to check against
/// @return         Returns true if the calling instance and the passed in
///                 instance are the same antenna, i.e. they share the same 
///                 model, radome and serial number.
bool
Antenna::is_same(const Antenna& rhs)
const noexcept
{ return ( !std::strncmp(__name ,rhs.__name, antenna_full_max_chars) ); }

/// This function compares two Antenna instances and return true if and only
/// if the Antenna is of the same model+radome, i.e. they share the same model
/// and radome disregarding any serial numbers.
///
/// @param[in] rhs  Antenna instance to check against
/// @return         Returns true if the calling instance and the passed in
///                 instance are identical antennas, i.e. they share the same 
///                 model and radome.
bool
Antenna::compare_model(const Antenna& rhs)
const noexcept
{
    auto sz = antenna_model_max_chars + 1 + antenna_radome_max_chars;
    return ( !std::strncmp(__name, rhs.__name, sz) );
}

/// Compare the antenna's serial number to the given c-string
/// @param[in] c The serial to check against
/// @return      True if the serials match exactly; false otherwise
bool
Antenna::compare_serial(const char* c)
const noexcept
{
    return !std::strncmp(__name + antenna_model_max_chars 
                                 + 1
                                 + antenna_radome_max_chars,
                          c,
                          antenna_serial_max_chars);
}

/// Check if the Antenna instance has a serial number; that is, any of the
/// characters at the indexes of the serial (typically between 20 and 40) is
/// not a whistespace.
/// @return true if the antenna has a serial number (can be just one character)
///         or false otherwise.
bool
Antenna::has_serial()
const noexcept
{
    auto idx = antenna_model_max_chars + 1 + antenna_radome_max_chars;
    auto asz = std::strlen(__name);
    for (; idx < asz; idx++) {
        if (__name[idx] != ' ') {
            return true;
        }
    }
    return false;
}

/// Check if the instance's radome type is empty, aka is "    "
/// @return true if radome is empty; false otherwise
bool
Antenna::radome_is_empty()
const noexcept
{
    for (std::size_t i = antenna_model_max_chars;
                     i < antenna_model_max_chars+antenna_radome_max_chars;
                     i++) {
        if ( __name[i] != ' ') return false;
    }
    return true;
}

/// Set the antenna's serial number from a c-string
/// @param[in] c A c-string containing a serial number; note that the serial
///              of the calling instance will be set to exactly c; i.e. any
///              leading or trailing whitespace characters will be copied. 
void
Antenna::set_serial_nr(const char* c)
noexcept
{
    constexpr std::size_t start_idx { antenna_model_max_chars  + 1 /* whitespace */
                                    + antenna_radome_max_chars };
    std::memset(__name + start_idx, '\0', antenna_serial_max_chars);
    std::memcpy(__name + start_idx, c, sizeof(char) * 
                        std::min(std::strlen(c), antenna_serial_max_chars) );

    return;
}

/// Antenna model name as string. This will always produce a string of 15 chars
/// in length (aka trailing whitespaces are also copied to the result)
/// @return The model (only) of the instance as an std::string of length
///         antenna_model_max_chars
std::string
Antenna::model_str() const noexcept
{return std::string(__name, antenna_model_max_chars);}

/// Antenna radome name as string. This will always produce a string of 4 chars
/// in length (aka trailing whitespaces are also copied to the result)
/// @return The radome (only) of the instance as an std::string of length
///         antenna_radome_max_chars
std::string
Antenna::radome_str() const noexcept
{return std::string(__name+antenna_model_max_chars+1, antenna_radome_max_chars);}

#ifdef DEBUG
std::string
Antenna::to_string()
const noexcept
{return std::string(__name, std::strlen(__name));}
#endif

void
SatelliteAntenna::set_from_cstr(const char* c)
noexcept
{
    std::memset(__name, ' ', satellite_antenna_max_chars*sizeof(char));
    
    // get the size of the input string.
    std::size_t ant_size (std::min(std::strlen(c), satellite_antenna_max_chars));

    // copy at maximum antenna_full_max_chars - 1 characters
    std::memcpy(__name, c, sizeof(char)*ant_size);

    __name[satellite_antenna_max_chars] = '\0';
}

SatelliteAntenna::SatelliteAntenna(const char* c) noexcept
{ this->set_from_cstr(c); }

int
SatelliteAntenna::compare(const char* c) const noexcept
{
    return std::strncmp(__name, c, satellite_antenna_max_chars);
}
