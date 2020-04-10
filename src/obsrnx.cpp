#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <cerrno>
#include "obsrnx.hpp"
#include "nvarstr.hpp"
#include "ggdatetime/datetime_read.hpp"

using ngpt::ObservationRnx;

/// No header line can have more than 80 chars. However, there are cases when
/// they  exceed this limit, just a bit ...
constexpr int MAX_HEADER_CHARS { 85 };

/// Max header lines.
constexpr int MAX_HEADER_LINES { 1000 };

/// Max record characters (for a navigation data block)
// constexpr int MAX_RECORD_CHARS { 128 };

/// Size of 'END OF HEADER' C-string.
/// std::strlen is not 'constexr' so eoh_size can't be one either. Note however
/// that gcc has a builtin constexpr strlen function (if we want to disable this
/// we can do so with -fno-builtin).
#ifdef __clang__
  const     std::size_t eoh_size { std::strlen("END OF HEADER") };
#else
  constexpr std::size_t eoh_size { std::strlen("END OF HEADER") };
#endif

/// @details ObservationRnx constructor, using a filename. The constructor will
///          initialize (set) the _filename attribute and also (try to)
///          open the input stream (i.e. _istream).
///          If the file is successefuly opened, the constructor will read
///          the header and assign info.
/// @param[in] filename  The filename of the Rinex file
ObservationRnx::ObservationRnx(const char* filename)
  : __filename   (filename)
  , __istream    (filename, std::ios_base::in)
  , __satsys     (SATELLITE_SYSTEM::mixed)
  , __version    (0e0)
  , __end_of_head(0)
{
  int j;
  if ((j=read_header())) {
      if (__istream.is_open()) __istream.close();
      throw std::runtime_error("[ERROR] Failed to read (obs) RINEX header; Error Code: "+std::to_string(j));
  }
}

/// Read a RINEX Observation v3.x header and assign vital information.
/// The function will read all header lines, stoping after the line:
/// "END OF HEADER"
/// @return  Anything other than 0 denotes an error.
int
ObservationRnx::read_header() noexcept
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
  if (line[20] != 'O') return 11;
  try {
    __satsys = ngpt::char_to_satsys(line[40]);
  } catch (std::runtime_error& e) {
    return 12;
  }
  
  // Keep on readling and collecting info until 'END OF HEADER'.
  // ----------------------------------------------------
  int dummy_it = 0;
  std::size_t szt;
  char* end;
  __istream.getline(line, MAX_HEADER_CHARS);
  while (dummy_it < MAX_HEADER_LINES 
         && std::strncmp(line+60, "END OF HEADER", eoh_size) ) {
    __istream.getline(line, MAX_HEADER_CHARS);
    // check for marker name
    if (!std::strncmp(line+60, "MARKER NAME", std::strlen("MARKER NAME"))) {
      __marker_name = ngpt::rtrim(line, szt, 60);
    } else if (!std::strncmp(line+60, "MARKER NUMBER", std::strlen("MARKER NUMBER"))) {
      __marker_number = ngpt::rtrim(line, szt, 20);
    } else if (!std::strncmp(line+60, "REC # / TYPE / VERS", std::strlen("REC # / TYPE / VERS"))) {
      __receiver_number = ngpt::rtrim(line, szt, 20);
      __receiver_type   = ngpt::rtrim(line+20, szt, 20);
    } else if (!std::strncmp(line+60, "ANT # / TYPE", std::strlen("ANT # / TYPE"))) {
      ReceiverAntenna ant(ngpt::rtrim(line+20, szt, 20));
      if (!ngpt::string_is_empty(line, 20)) ant.set_serial_nr(line);
      __antenna = ant;
    } else if (!std::strncmp(line+60, "APPROX POSITION XYZ", std::strlen("APPROX POSITION XYZ"))) {
      if (!ngpt::__char2double__<3,14>(line, __approx)) {
        std::cerr<<"\n[ERROR] ObservationRnx::read_header() Failed to resolve field: \"APPROX POSITION XYZ\"";
        return 51;
      }
    } else if (!std::strncmp(line+60, "ANTENNA: DELTA H/E/N", std::strlen("ANTENNA: DELTA H/E/N"))) {
      if (!ngpt::__char2double__<3,14>(line, __eccentricity)) {
        std::cerr<<"\n[ERROR] ObservationRnx::read_header() Failed to resolve field: \"ANTENNA: DELTA H/E/N\"";
        return 52;
      }
    } else if (!std::strncmp(line+60, "SYS / # / OBS TYPES", std::strlen("SYS / # / OBS TYPES"))) {
      if ((szt=this->__resolve_obstypes_304__(line))) return szt;
    } else if (!std::strncmp(line+60, "RCV CLOCK OFFS APPL", std::strlen("RCV CLOCK OFFS APPL"))) {
        int i = std::strtol(line, &end, 10);
        if ((errno || (line+3)==end) || (i!=0 && i!=1)) {
          errno = 0;
          std::cerr<<"\n[ERROR] ObservationRnx::read_header() Failed to resolve field: \"RCV CLOCK OFFS APPL\"";
          std::cerr<<"\n        Fatal while resolving the answer (integer)";
          return 60;
        }
        __rcv_clk_offs_applied = i;
        if (__rcv_clk_offs_applied) {
          std::cout<<"\n[WARNING] Epoch, code, and phase are corrected by applying "
                   <<"          the real-time-derived receiver clock offset"
                   <<"\n        aka \"RCV CLOCK OFFS APPL\" is ON at RINEX";
        }
    }
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

/// @brief Resolve line(s) of type "SYS / # / OBS TYPES" as RINEX v3.04
///
/// This function will resolve a line (or more if needed) of type 
/// "SYS / # / OBS TYPES" for a given satellite system. The first line passed
/// in, should be the first line of type "SYS / # / OBS TYPES" for a given sat.
/// system. If more lines need to be read and resolved (for this sat. system),
/// then the function will read as many lines as needed. The function will
/// resolve all Observation Types of the given sat. system and create a
/// vector of such types (which is then pushed back to the instance's map).
/// Hence, at success, the function will augment the instance's __obstmap
/// dictionary by one.
/// @param[in] cline First line of type "SYS / # / OBS TYPES" for any satellite
///                  system
/// @return An integer value; anything other than 0, denotes an error
/// @note   The function is not const because it may need to read more lines
///         (except from the one passed in); hence it may change the state of
///         __istream.
int
ObservationRnx::__resolve_obstypes_304__(const char* cline) noexcept
{
  char line[MAX_HEADER_CHARS];
  std::memcpy(line, cline, sizeof line);

  char* end;
  std::vector<ngpt::ObservationCode> obsvec;
  auto satsys = ngpt::char_to_satsys(line[0]);
  if (__obstmap.find(satsys)!=__obstmap.end()) {
    std::cerr<<"\n[ERROR] ObservationRnx::read_header() Failed to resolve field: \"SYS / # / OBS TYPES\"";
    std::cerr<<"\n        Fatal Already resolved this satellite system!";
    return 56;
  }
  int obsnum = std::strtol(line+3, &end, 10);
  if (errno || (line+3)==end) {
    errno = 0;
    std::cerr<<"\n[ERROR] ObservationRnx::read_header() Failed to resolve field: \"SYS / # / OBS TYPES\"";
    std::cerr<<"\n        Fatal while interpreting Number of Obs Types";
    return 53;
  }
  // int lines_to_read = (obsnum-1)/13;
  int resolved=0, vecsz=0; 
  do {
    try {
      obsvec.emplace_back(line+7+resolved*4);
    } catch (std::exception& e) {
      std::cerr<<e.what();
      std::cerr<<"\n[ERROR] ObservationRnx::read_header() Failed to resolve field: \"SYS / # / OBS TYPES\"";
      std::cerr<<"\n        Fatal while resolving type: \""<<(line+6+resolved*3)<<"\"";
      return 54;
    }
    ++resolved;
    ++vecsz;
    if (resolved==13) {
      __istream.getline(line, MAX_HEADER_CHARS);
      if (std::strncmp(line+60, "SYS / # / OBS TYPES", std::strlen("SYS / # / OBS TYPES"))) {
        std::cerr<<"\n[ERROR] ObservationRnx::read_header() Failed to resolve field: \"SYS / # / OBS TYPES\"";
        std::cerr<<"\n        Fatal; expected line \"SYS / # / OBS TYPES\" and got: "
                 <<"\n        \""<<(line+60)<<"\"";
        return 55;
      }
      resolved=0;
    }
  } while (vecsz<obsnum);

  if (obsvec.size()==(std::size_t)obsnum) {
    __obstmap[satsys] = std::move(obsvec);
    return 0;
  }

  return 59;
}

int
ObservationRnx::__resolve_epoch_304__(const char* cline,
  ngpt::modified_julian_day& mjd, double& sec, int& flag, int& num_sats,
  double& rcvr_coff)
noexcept
{
  using ngpt::year;
  using ngpt::month;
  using ngpt::day_of_month;

  std::size_t lnlen = std::strlen(cline);

  if (*cline!='>' || lnlen<41) {
    std::cerr<<"\n[ERROR] ObservationRnx::__resolve_epoch_304__() Invalid epoch line";
    return 1;
  }

  // resolve the day as Modified Julian Day
  char* end;
  const char* start = cline+2;
  int dints[5];
  for (int i=0; i<5; i++) {
    dints[i] = static_cast<int>(std::strtol(start, &end, 10));
    if (errno || start==end) {
      std::cerr<<"\n[ERROR] ObservationRnx::__resolve_epoch_304__() failed to resolve epoch";
      errno=0; return 2;
    }
    start = ++end;
  }
#ifdef DEBUG
  day_of_month dom (dints[2]);
  assert(dom.is_valid(year(dints[0]), month(dints[1])));
#endif
  mjd = ngpt::modified_julian_day(year(dints[0]), month(dints[1]), day_of_month(dints[2]));

  // resolve seconds of day
  double rsec = std::strtod(start, &end);
  if (errno || start==end) {
    std::cerr<<"\n[ERROR] ObservationRnx::__resolve_epoch_304__() failed to resolve seconds";
    errno=0; return 3;
  }
  sec = (dints[3]*60e0 + dints[4])*60e0 + rsec;

  // resolve the epoch flag
  flag = cline[31] - '0';

  // resolve num of satellites in epoch
  num_sats = static_cast<int>(std::strtol(cline+32, &end, 10));
  if (errno || start==end) {
    std::cerr<<"\n[ERROR] ObservationRnx::__resolve_epoch_304__() failed to resolve num sats";
    errno=0; return 4;
  }

  // resolve clock offset if any
  if (lnlen>41) {
    rcvr_coff = string_is_empty(cline+41) ? 0e0 : std::strtod(cline+41, &end);
    if (errno || start==end) {
      std::cerr<<"\n[ERROR] ObservationRnx::__resolve_epoch_304__() failed to resolve receiver clock offset";
      errno=0; return 5;
    }
  }

  return 0;
}

char*
ObservationRnx::max_line() const noexcept
{
  std::size_t maxobs = this->max_obs();
  char* line = new char[maxobs*14+4];
  return line;
}

int
ObservationRnx::resolve_epoch_sat_line(const char* cline)
{
  char* end;

  // resolve satellite system
  ngpt::SATELLITE_SYSTEM s;
  try {
     s = ngpt::char_to_satsys(*line);
  } catch (std::exception& e) {
    std::cerr<<"\n[ERROR] ObservationRnx::__resolve_epoch_sat_line() failed to resolve satellite system";
    return 1;
  }

  // resolve PRN
  int prn = std::strtod(cline+1, &end);
  if ((errno || start==end) || (prn<1 || prn>99)) {
    std::cerr<<"\n[ERROR] ObservationRnx::__resolve_epoch_sat_line() failed to resolve PRN";
    errno=0; return 2;
  }

  // number of observations that follow
  int numobs = __obstmap[s];

}
