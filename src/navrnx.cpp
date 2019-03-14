#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <cerrno>
#include "navrnx.hpp"

using ngpt::NavigationRnx;

/// No header line can have more than 80 chars. However, there are cases when
/// they  exceed this limit, just a bit ...
constexpr int MAX_HEADER_CHARS { 85 };

/// Max header lines.
constexpr int MAX_HEADER_LINES { 1000 };

/// Max record characters
constexpr int MAX_RECORD_CHARS { };

/// Size of 'END OF HEADER' C-string.
/// std::strlen is not 'constexr' so eoh_size can't be one either. Note however
/// that gcc has a builtin constexpr strlen function (if we want to disable this
/// we can do so with -fno-builtin).
#ifdef __clang__
  const     std::size_t eoh_size { std::strlen("END OF HEADER") };
#else
  constexpr std::size_t eoh_size { std::strlen("END OF HEADER") };
#endif

/// @details NavigationRnx constructor, using a filename. The constructor will
///          initialize (set) the _filename attribute and also (try to)
///          open the input stream (i.e. _istream).
///          If the file is successefuly opened, the constructor will read
///          the header and assign info.
/// @param[in] filename  The filename of the Rinex file
NavigationRnx::NavigationRnx(const char* filename)
  : __filename   (filename)
  , __istream    (filename, std::ios_base::in)
  , __satsys     (SATELLITE_SYSTEM::mixed)
  , __version    (0e0)
  , __end_of_head(0)
{
  if (read_header()) {
      if (__istream.is_open()) __istream.close();
      throw std::runtime_error("[ERROR] Failed to read (nav) RINEX header");
  }
}

/// Read a RINEX Navigation v3.x header and assign vital information.
/// The function will read all header lines, stoping after the line:
/// "END OF HEADER"
/// @return  Anything other than 0 denotes an error.
int
NavigationRnx::read_header() noexcept
{
  char line[MAX_HEADER_CHARS];
  char* str_end;

  // The stream should be open by now!
  if (!__istream.is_open()) return 1;

  // Go to the top of the file.
  __istream.seekg(0);

  // Read the first line. Get version, data-type and sat. system.
  // ------------------------------------------------------------
  __istream.getline(line, MAX_HEADER_CHARS);
  __version = std::strtof(line, &str_end);
  if (str_end == line) return 10; // transformation to float has failed
  if (line[20] != 'N') return 11;
  try {
    __satsys = ngpt::char_to_satsys(line[40]);
  } catch (std::runtime_error& e) {
    return 12;
  }
  
  // Keep on readling lines until 'END OF HEADER'.
  // ----------------------------------------------------
  int dummy_it = 0;
  __istream.getline(line, MAX_HEADER_CHARS);
  while (dummy_it < MAX_HEADER_LINES 
         && std::strncmp(line+60, "END OF HEADER", eoh_size) ) {
    __istream.getline(line, MAX_HEADER_CHARS);
    dummy_it++;
  }
  if (dummy_it >= MAX_HEADER_LINES) {
    return 20;
  }

  // Mark the end of header
  __end_of_head = __istream.tellg();

  // All done !
  return 0;
}

int
NavigationRnx::read_nex_record() noexcept
{
  char line[MAX_RECORD_CHARS];
  char* str_end;

  // Read the first line.
  // ------------------------------------------------------------
  ngpt::SATELLITE_SYSTEM ss;
  int prn;
  ngpt::datetime<ngpt::seconds> toc;
  __istream.getline(line, MAX_RECORD_CHARS);
  try {
    ss  = ngpt::char_to_satsys(*line);
    toc = ngpt::strptime_ymd_hms<ngpt::seconds>(line+3);
  } catch (std::exception&) {
    return 10;
  }
  prn = std::strtol(line+1, &str_end, 10);
  if (!prn || errno == ERANGE) {
    errno = 0;
    return 11;
  }
  // replace 'D' or 'd' with 'e' in remaining floats
  str_end = line+23;
  while (str_end) {
    if (*str_end == 'D' || *str_end == 'd') *str_end = 'E';
    ++str_end;
  }
  const char* c = line+23;
  if (!(data[0] = std::strtod(c, &str_end)) || c == str_end) return 21;
  c += 19;
  if (!(data[1] = std::strtod(c, &str_end)) || c == str_end) return 22;
  c += 19;
  if (!(data[2] = std::strtod(c, &str_end)) || c == str_end) return 23;

}

int
NavigationRnx::skip_nav_record(ngpt::SATELLITE_SYSTEM s) noexcept
{
  using ngpt::SATELLITE_SYSTEM;

  int lines_to_skip = -10;
  switch (s) {
    case SATELLITE_SYSTEM::gps :
    case SATELLITE_SYSTEM::galileo :
    case SATELLITE_SYSTEM::qzss :
    case SATELLITE_SYSTEM::beidou :
    case SATELLITE_SYSTEM::irnss :
      lines_to_skip =  8;
      break;
    case SATELLITE_SYSTEM::glonass :
    case SATELLITE_SYSTEM::sbas :
      lines_to_skip = 4;
      break;
  }

  if (lines_to_skip < 0) return 10;

  char line[MAX_RECORD_CHARS];
  for (int i=0; i<lines_to_skip; i++) {
    if (!__istream.getline(line, MAX_RECORD_CHARS))
      return 15;
  }

  return 0;
}
