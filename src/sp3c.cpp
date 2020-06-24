#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <cerrno>
#include <algorithm>
#include "sp3c.hpp"
#include "ggdatetime/datetime_read.hpp"

using ngpt::Sp3c;

/// No header line can have more than 80 chars. However, there are cases when
/// they  exceed this limit, just a bit ...
constexpr int MAX_HEADER_CHARS { 85 };

/// Max header lines.
constexpr int MAX_HEADER_LINES { 1000 };

/// Max record characters (for a navigation data block)
constexpr int MAX_RECORD_CHARS { 128 };

/// @details Sp3c constructor, using a filename. The constructor will
///          initialize (set) the _filename attribute and also (try to)
///          open the input stream (i.e. _istream).
///          If the file is successefuly opened, the constructor will read
///          the header and assign info.
/// @param[in] filename  The filename of the Sp3 file
Sp3c::Sp3c(const char* filename)
  : __filename   (filename)
  , __istream    (filename, std::ios_base::in)
  , __satsys     (SATELLITE_SYSTEM::mixed)
  , __end_of_head(0)
{
  int j;
  if ((j=read_header())) {
      if (__istream.is_open()) __istream.close();
        throw std::runtime_error("[ERROR] Failed to read Sp3 header; Error Code: "+std::to_string(j));
  }
}

/// Read a, Sp3c file header and assign vital information.
/// The function will read all header lines, stoping after the line:
/// @return  Anything other than 0 denotes an error.
int
Sp3c::read_header() noexcept
{
  char line[MAX_HEADER_CHARS];
  char* str_end;
  int dummy_it=0;

  // The stream should be open by now!
  if (!__istream.is_open()) return 1;

  // Go to the top of the file.
  __istream.seekg(0);

  // Read the first line. Error codes 10-19
  // ------------------------------------------------------------
  __istream.getline(line, MAX_HEADER_CHARS);
  if (*line!='#') return 10;    // validate version
  version__ = *(line+1);
  if (version__!='c' && version__!='d') return 10;
  int year = std::strtol(line+3, &str_end, 10); // read year
  if (!year || errno == ERANGE) {
    errno = 0;
    return 11;
  }
  int month = std::strtol(line+8, &str_end, 10); // read month
  if (!month || errno == ERANGE) {
    errno = 0;
    return 12;
  }
  int dom = std::strtol(line+11, &str_end, 10); // read day of month
  if (!dom || errno == ERANGE) {
    errno = 0;
    return 13;
  }
  int hour = std::strtol(line+14, &str_end, 10); // read hour of day
  if (errno==ERANGE || str_end==line+14) {
    errno = 0;
    return 14;
  }
  int minute = std::strtol(line+17, &str_end, 10); // read hour of day
  if (errno==ERANGE || str_end==line+17) {
    errno = 0;
    return 15;
  }
  double sec = std::strtod(line+20, &str_end);    // read seconds
  
  num_epochs__ = std::strtol(line+32, &str_end, 10); // read number of epochs
  if (!num_epochs__ || errno == ERANGE) {
    errno = 0;
    return 16;
  }
  crd_sys__ = std::string(line+46, 5);
  orb_type__= std::string(line+52, 3);
  agency__  = std::string(line+56, 3);
  
  // all done for first line, construct the reference date
  start_epoch__ = ngpt::datetime<ngpt::microseconds>(ngpt::year(year), 
                                                   ngpt::month(month),
                                                   ngpt::day_of_month(dom),
                                                   ngpt::hours(hour),
                                                   ngpt::minutes(minute),
                                                   ngpt::microseconds(static_cast<long>(sec*1e6)));

  // Read the second line. Error codes [20,30)
  // ------------------------------------------------------------
  __istream.getline(line, MAX_HEADER_CHARS);
  if (*line!='#' || line[1]!='#') return 20;
  int gwk = std::strtol(line+3, &str_end, 10);
  if (!gwk || errno == ERANGE) {
    errno = 0;
    return 21;
  }
  sec = std::strtod(line+8, &str_end);
  // validate start epoch (#1)
  ngpt::microseconds sw;
  auto gwk1 = start_epoch__.as_gps_wsow(sw);
  if (gwk1.as_underlying_type()!=gwk || sw.to_fractional_seconds()!=sec) {
    std::cerr<<"\n[ERROR] Sp3c::read_header() Failed to validate start date";
    return 22;
  }
  sec = std::strtod(line+24, &str_end);
  interval__ = ngpt::microseconds(static_cast<long>(sec*1e6));
  int mjd = std::strtol(line+39, &str_end, 10);
  if (!mjd || errno == ERANGE) {
    errno = 0;
    return 23;
  }
  sec = std::strtod(line+45, &str_end);
  sec += mjd;
  if (sec!=start_epoch__.as_mjd()) {
    std::cerr<<"\n[ERROR] Sp3c::read_header() Failed to validate start date";
    return 24;
  }
  
  // Read the satellite ID lines; they must be at least 5, but there is no max
  // limitation for the Sp3d files. Each satellite accuracy line starts with 
  // '++'
  // Error code [10,40]
  // ------------------------------------------------------------
  __istream.getline(line, MAX_HEADER_CHARS);
  if (*line!='+' || line[1]!=' ') return 30;
  num_sats__ = std::strtol(line+3, &str_end, 10);
  if (!num_sats__ || errno == ERANGE) {
    errno = 0; return 31;
  }
  while (++dummy_it<MAX_HEADER_LINES && !std::strncmp(line, "+ ", 2) ) {
    __istream.getline(line, MAX_HEADER_CHARS);
  }
  if (dummy_it >= MAX_HEADER_LINES) {
    return 32;
  }
  
  // Read the satellite accuracy lines; they must be at least 5, but there is 
  // no max limitation for the Sp3d files. Each satellite id line starts with '+ '
  // Error code [40,50]
  // ------------------------------------------------------------
  // __istream.getline(line, MAX_HEADER_CHARS);
  if (*line!='+' || line[1]!='+') return 40;
  while (++dummy_it<MAX_HEADER_LINES && !std::strncmp(line, "++", 2) ) {
    __istream.getline(line, MAX_HEADER_CHARS);
    char c = __istream.peek();
    if (c!='+') break;
  }
  if (dummy_it >= MAX_HEADER_LINES) {
    return 41;
  }

  // two line follow, starting with '%c'; collect the system time
  // Error code [50,60]
  // ------------------------------------------------------------
  __istream.getline(line, MAX_HEADER_CHARS);
  if (*line!='%' || line[1]!='c') return 50;
  time_sys__ = std::string(line+9, 3);
  __istream.getline(line, MAX_HEADER_CHARS);
  if (*line!='%' || line[1]!='c') return 51;

  // two line follow, starting with '%f'
  // ------------------------------------------------------------
  for (int i=0; i<2; i++) {
    __istream.getline(line, MAX_HEADER_CHARS);
    if (*line!='%' || line[1]!='f') return 60;
  }
  
  // two line follow, starting with '%i'
  // ------------------------------------------------------------
  for (int i=0; i<2; i++) {
    __istream.getline(line, MAX_HEADER_CHARS);
    if (*line!='%' || line[1]!='i') return 70;
  }
  
  // read any remaining comment lines, starting with '%i'
  // ------------------------------------------------------------
  char c = __istream.peek();
  while (c=='/') {
    __istream.getline(line, MAX_HEADER_CHARS);
    if (line[1] != '*') return 80;
    c = __istream.peek();
    if (++dummy_it>MAX_HEADER_LINES) return 81;
  }
  
  // Mark the end of header
  __end_of_head = __istream.tellg();

  // All done !
  return 0;
}

/// @return <0: EOF
///          0: ok
///         >0: error
int
Sp3c::get_next_epoch(ngpt::datetime<ngpt::microseconds>& t, std::vector<Sp3EpochSvRecord>& vec, int& sats_read)
noexcept
{
  char line[MAX_RECORD_CHARS];
  char* end, *start, c;
  int date[5];

  if (!__istream.good()) return 1;
  __istream.getline(line, MAX_RECORD_CHARS);
  
  if (line[0]!='*' || line[1]!=' ') {
    std::cerr<<"\n[ERROR] WTF?? Expected epoch header, got \""<<line<<"\"";
    return 2;
  }
  date[0] = std::strtol(line+3, &end, 10);
  if (!date[0] || errno==ERANGE) {
    errno = 0; return 5;
  }
  start = line+8;
  for (int i=1; i<5; i++) {
    date[i] = std::strtol(start, &end, 10);
    if (errno==ERANGE || end==start) {
      errno=0; return 5+i;
    }
    start+=3;
  }
  double fsec = std::strtod(start, &end);
  if (errno==ERANGE || end==start) {
    errno = 0; return 11;
  }

  t = ngpt::datetime<ngpt::microseconds>(ngpt::year(date[0]), 
      ngpt::month(date[1]),
      ngpt::day_of_month(date[2]),
      ngpt::hours(date[3]),
      ngpt::minutes(date[4]),
      ngpt::microseconds(static_cast<long>(fsec*1e6)));

  ngpt::SATELLITE_SYSTEM sys;
  int prn, j=0;
  std::array<double,4> state;
  Sp3Flag flag;
  bool keep_reading=true;
  auto it = vec.begin();
  /* possible following lines (three first chars):
   * 1. '*  ' i.e an epoch header
   * 2. 'PXX' i.e. a position & clock line, e.g. 'PG01 ....'
   * 3. 'EP ' i.e. position and clock correlation
   * 4. 'VXX' i.e. velocity line, e.g. 'VG01 ...'
   * 5. 'EV ' i.e. velocity correlation
   * 6. 'EOF' i.e. EOF
   */
  do {
    c = __istream.peek();
    if (c!='*') {
      __istream.getline(line, MAX_RECORD_CHARS);
      if (*line=='P') {/* position and clock line */
        if ((j=get_next_position(line, sys, prn, state, flag))) return j+20;
        *it = Sp3EpochSvRecord{sys, prn, state, flag};
        ++it;
      } else if (!std::strncmp(line, "EOF", 3)) {/* EOF */
        keep_reading=false;
        j=-1;
      } else {/* any other line is irrelevant */
        ;
      }
    } else {/* next line is header line */
      keep_reading=false;
    }
  } while(keep_reading);

  sats_read = std::distance(vec.begin(), it);
  return 0;
}

/// @brief
/// @param[out] s   The resolved satellite system
/// @param[out] prn The PRN of the satellite
/// @param[out] state The state vector, aka SV x,y,z and clock correction;
///                 Note that the components x,y and z are in meters and the
///                 clock correction is in microsec
/// @param[out] flag An Sp3Flag instance to hold all the flags for this record
int
Sp3c::get_next_position(char* line, ngpt::SATELLITE_SYSTEM& s, int& prn,
                        std::array<double,4>& state, Sp3Flag& flag)
noexcept
{
  char* end, *start;
  flag.reset();
  
  // __istream.getline(line, MAX_RECORD_CHARS);
  // if (!__istream.good()) return 1;
  if (*line!='P') return 2;
  
  try {
    s = ngpt::char_to_satsys(line[1]);
  } catch (std::runtime_error& e) {
    return 3;
  }
  prn = std::strtol(line+2, &end, 10);
  if (errno==ERANGE || end==line+2) {
    errno = 0;
    return 4;
  }

  start = line+4;
  for (int i=0; i<4; i++) {
    state[i] = std::strtod(start, &end);
    if (errno==ERANGE || end==start) {
      errno = 0;
      return 5+i;
    }
    start+=14;
  }

  state[0]*=1e3;
  state[1]*=1e3;
  state[2]*=1e3;

  if (std::any_of(state.begin(), state.begin()+3,
                  [](double d){
                    return d==SP3_MISSING_POS_VALUE;})) {
    flag.set(Sp3Event::bad_abscent_position);
  }
  if (state[3]>=SP3_MISSING_CLK_VALUE) flag.set(Sp3Event::bad_abscent_clock);
  if (line[74]=='E') flag.set(Sp3Event::clock_event);
  if (line[75]=='P') flag.set(Sp3Event::clock_prediction);
  if (line[78]=='M') flag.set(Sp3Event::maneuver);
  if (line[79]=='E') flag.set(Sp3Event::orbit_prediction);

  return 0;
}
