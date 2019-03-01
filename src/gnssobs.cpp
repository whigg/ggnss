#include <stdexcept>
#include <cstring>
#include "gnssobs.hpp"

using ngpt::OBSERVABLE_TYPE;
using ngpt::ObservationCode;

/// Cast a character/char to a valid OBSERVABLE_TYPE. The translation follows:
///   'C' pseudorange
///   'L' carrier_phase
///   'D' doppler
///   'S' signal_strength
///   'I' ionosphere_phase_delay
///   'X' receiver_channel_number
/// Any other character will produce a runtime_error
/// @param[in] c  A character representing an observable type
/// @return       An OBSERVABLE_TYPE
/// @throw        Will throw a runtime_error if the passed in char is not
///               translatable
/// @see          RINEX v3.x
/// @todo can we somehow not throw??
OBSERVABLE_TYPE
ngpt::char_to_observabletype(char c)
{
  switch (c) {
    case 'C' : return OBSERVABLE_TYPE::pseudorange;
    case 'L' : return OBSERVABLE_TYPE::carrier_phase;
    case 'D' : return OBSERVABLE_TYPE::doppler;
    case 'S' : return OBSERVABLE_TYPE::signal_strength;
    case 'I' : return OBSERVABLE_TYPE::ionosphere_phase_delay;
    case 'X' : return OBSERVABLE_TYPE::receiver_channel_number;
    default  : throw std::runtime_error("[ERROR] Cannot match char to observable type");
  }
}

/// Translate an OBSERVABLE_TYPE to a character. This follows:
///   'C' pseudorange
///   'L' carrier_phase
///   'D' doppler
///   'S' signal_strength
///   'I' ionosphere_phase_delay
///   'X' receiver_channel_number
///   '?' any
/// @param[in] t The OBSERVABLE_TYPE to translate
/// @return      The OBSERVABLE_TYPE as character
/// @see          RINEX v3.x
/// @warning This function should always include all OBSERVABLE_TYPE options.
char
ngpt::observabletype_to_char(OBSERVABLE_TYPE t) noexcept
{
  switch (t) {
    case OBSERVABLE_TYPE::pseudorange : return 'C';
    case OBSERVABLE_TYPE::carrier_phase : return 'L';
    case OBSERVABLE_TYPE::doppler : return 'D';
    case OBSERVABLE_TYPE::signal_strength : return 'S';
    case OBSERVABLE_TYPE::ionosphere_phase_delay : return 'I';
    case OBSERVABLE_TYPE::receiver_channel_number : return 'X';
    case OBSERVABLE_TYPE::any : return '?';
  }
  // should never reach this point
  return '?';
}

/// Constructor for an ObservationCode. An observation code should include
/// at least an OBSERVABLE_TYPE and a band; in RINEX v3.x, an attribute is
/// also included, which is missing in RINEX v2.x. Thus, if not present, the
/// attribute is set to '?' aka any.
/// @param[in] str  A c-string of length >=2. The first character is the
///                 OBSERVABLE_TYPE and the second should be the band. If a
///                 third character is present, it is treated as an attribute
///                 Any trailing characters are discarded.
/// @see RINEX v3.x, RINEX v.2x
ObservationCode::ObservationCode(const char* str)
{
  if (std::strlen(str) < 2) {
    throw std::runtime_error("[ERROR] Cannot convert string to ObservationCode");
  }
  ObservationAttribute at;
  auto ot = char_to_observabletype(*str);
  int  bn = str[1] - '0';
  if (std::strlen(str) > 2 && str[2] != ' ') {
    at = ObservationAttribute(str[2]);
  }
     
  __type = ot;
  __band = bn;
  __attr = at;
}

std::string
ObservationCode::to_string() const
{
  std::string str ("???");
  char c0 = ngpt::observabletype_to_char(__type);
  auto c1 = std::to_string(__band);
  char c2 = __attr.as_char();
  str[0] = c0;
  str[1] = c1[0];
  str[2] = c2;
  return str;
}
