#include <iostream>
#include <stdexcept>
#include <cerrno>
#include "navrnx.hpp"

using ngpt::NavDataFrame;

/// GALILEO:               : Time of Clock in GAL time
///             data__[0]  : SV clock bias in seconds, aka af0
///             data__[1]  : SV clock drift in m/sec, aka af1
///             data__[2]  : SV clock drift rate in m/sec^2, aka af2
///             data__[3]  : IODnav Issue of Data of the nav batch
///             data__[4]  : Crs(meters)
///             data__[5]  : Deltan (radians/sec)
///             data__[6]  : M0(radians)
///             data__[7]  : Cuc (radians)
///             data__[8]  : e Eccentricity
///             data__[9]  : Cus (radians)
///             data__[10] : sqrt(a) (sqrt(m))
///             data__[11] : Toe Time of Ephemeris (sec of GAL week)
///             data__[12] : Cic (radians)
///             data__[13] : OMEGA0 (radians)
///             data__[14] : Cis (radians)
///             data__[14] : i0 (radians)
///             data__[16] : Crc (meters)
///             data__[17] : omega (radians)
///             data__[18] : OMEGA DOT (radians/sec)
///             data__[19] : IDOT (radians/sec)
///             data__[20] : Data sources (FLOAT --> INTEGER)
///             data__[21] : GAL Week # (to go with Toe)
///             data__[22] : Spare
///             data__[23] : SISA Signal in space accuracy (meters) 
///             data__[24] : SV health (FLOAT converted to INTEGER)
///             data__[25] : BGD E5a/E1 (seconds)
///             data__[26] : BGD E5b/E1 (seconds)
///             data__[27] : Transmission time of message
///             data__[28] : spare
///             data__[29] : spare
///             data__[30] : spare
///            
