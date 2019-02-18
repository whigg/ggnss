#include <stdexcept>
#include <cstring>
#include "gnssobs.hpp"

using ngpt::observable_type;
using ngpt::ObservationCode;

observable_type
ngpt::char_to_observabletype(char c)
{
    switch (c) {
        case 'C' : return observable_type::pseudorange;
        case 'L' : return observable_type::carrier_phase;
        case 'D' : return observable_type::doppler;
        case 'S' : return observable_type::signal_strength;
        case 'I' : return observable_type::ionosphere_phase_delay;
        case 'X' : return observable_type::receiver_channel_number;
        default  : throw std::runtime_error("[ERROR] Cannot match char to observable type");
    }
}

char
ngpt::observabletype_to_char(observable_type t)
{
    switch (t) {
        case observable_type::pseudorange : return 'C';
        case observable_type::carrier_phase : return 'L';
        case observable_type::doppler : return 'D';
        case observable_type::signal_strength : return 'S';
        case observable_type::ionosphere_phase_delay : return 'I';
        case observable_type::receiver_channel_number : return 'X';
        case observable_type::any : return '?';
        default : throw std::runtime_error("[ERROR] Cannot match observable type to char");
    }
}

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
