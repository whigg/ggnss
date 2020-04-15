#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <cerrno>
#include <algorithm>
#include "obsrnx.hpp"
#include "nvarstr.hpp"
#include "ggdatetime/datetime_read.hpp"

using ngpt::ObservationRnx;
using ngpt::RawRnxObs__;

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

/// Resolve a RINEX observation using the relative string; this string, of type:
/// (F14.3 I1 I1) is resolved as:
/// F14.3 -> the actual value of the observation (if empty use the RNXOBS_MISSING_VAL
///          value
/// I1    -> Loss of lock indicator (LLI)
///          * 0 or blank: OK or not known
///          * Bit 0 set: Lost lock between previous and current observation: 
///            Cycle slip possible. For phase observations only.
///          * Bit 1 set: Half-cycle ambiguity/slip possible. Software not 
///            capable of handling half cycles should skip this observation. 
///            Valid for the current epoch only.
///          * Bit 2 set: Galileo BOC-tracking of an MBOC-modulated signal (may
///            suffer from increased noise)
/// I1    -> Signal Strength Indicator (SSI) projected into interval 1-9:
///          * 1: minimum possible signal strength
///          * 5: average/good S/N ratio
///          * 9: maximum possible signal strength
///          * 0 or blank: not known, don't care
/// @param[in] str A string of length (at least) 16 chars, following the format
///                F14.3I1I1; any -or all- of the three numbers (the float or any
///                of the ints can be blank)
int
RawRnxObs__::resolve(char *str) noexcept
{
  char* end;
  if (string_is_empty(str, 14)) {
    __val = RNXOBS_MISSING_VAL;
    return 0;
  }
  __val = std::strtod(str, &end);
  if (errno || str==end) {
    errno=0; return 1;
  }
  __lli = (str[14]==' ') ? (0) : (str[14]-'0');
  __ssi = (str[15]==' ') ? (0) : (str[15]-'0');

  return 0;
}

/// @details ObservationRnx constructor, using a filename. The constructor will
///          initialize (set) the _filename attribute and also (try to)
///          open the input stream (i.e. _istream).
///          If the file is successefuly opened, the constructor will read
///          the header and assign info.
///          It will also allocate memory for the __buf pointer (using the
///          collected head information).
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
  std::size_t maxobs = this->max_obs();
  __buf_sz = maxobs*16+4;
  __buf = new char[__buf_sz];
}

/// Read a RINEX Observation v3.x header and assign vital information.
/// The function will read all header lines, stoping after the line:
/// "END OF HEADER"
/// @return  Anything other than 0 denotes an error.
///
/// @todo Not handling yet:
///       SIGNAL STRENGTH UNIT
///       INTERVAL
///       TIME OF FIRST OBS
///       TIME OF LAST OBS
///       RCV CLOCK OFFS APPL
///       SYS / DCBS APPLIED
///       SYS / PCVS APPLIED
///       SYS / SCALE FACTOR
///       SYS / PHASE SHIFT
///       GLONASS SLOT / FRQ #
///       GLONASS COD/PHS/BIS
///       LEAP SECONDS
///       # OF SATELLITES
///       PRN / # OF OBS
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
/// dictionary by one (satellite system).
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
    return 1;
  }
  int obsnum = std::strtol(line+3, &end, 10);
  if (errno || (line+3)==end) {
    errno = 0;
    std::cerr<<"\n[ERROR] ObservationRnx::read_header() Failed to resolve field: \"SYS / # / OBS TYPES\"";
    std::cerr<<"\n        Fatal while interpreting Number of Obs Types";
    return 2;
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
      return 3;
    }
    ++resolved;
    ++vecsz;
    if (resolved==13) {
      __istream.getline(line, MAX_HEADER_CHARS);
      if (std::strncmp(line+60, "SYS / # / OBS TYPES", std::strlen("SYS / # / OBS TYPES"))) {
        std::cerr<<"\n[ERROR] ObservationRnx::read_header() Failed to resolve field: \"SYS / # / OBS TYPES\"";
        std::cerr<<"\n        Fatal; expected line \"SYS / # / OBS TYPES\" and got: "
                 <<"\n        \""<<(line+60)<<"\"";
        return 4;
      }
      resolved=0;
    }
  } while (vecsz<obsnum);

  if (obsvec.size()==(std::size_t)obsnum) {
    __obstmap[satsys] = std::move(obsvec);
    return 0;
  }

  return 9;
}

/// @brief Resolve an epoch header line for a RINEX v.3x files
///
/// @param[in] cline An EPOCH header line as described in RINEX v3.x
///                  specifications
/// @param[out] mjd  The Modified Julian Day of the reference epoch (resolved
///                  from Year, Month and DayOfMonth)
/// @param[out] sec  Seconds of day (to go with mjd), resolved from hours,
///                  minuts and seconds fields
/// @param[out] flag Epoch flag:
///                  * 0 -> OK
///                  * 1 -> power failure between previous and current epoch
///                  *>1 -> special event
/// @param[out] num_sats  Number of satellites observed in current epoch
/// @param[out] rcvr_coff Receiver clock offset (seconds, optional); if empty,
///                       it is set to 0
/// @warning error codes (at return) should be in range [0,10)
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

  if (*cline!='>' || lnlen<35) {
    std::cerr<<"\n[ERROR] ObservationRnx::__resolve_epoch_304__() Invalid epoch line";
    std::cerr<<"\n        Line was: \""<<cline<<"\" length: "<<lnlen;
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
      std::cerr<<"\n        Line was: \""<<cline<<"\"";
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
    std::cerr<<"\n        Line was: \""<<cline<<"\"";
    errno=0; return 3;
  }
  sec = (dints[3]*60e0 + dints[4])*60e0 + rsec;

  // resolve the epoch flag
  flag = cline[31] - '0';

  // resolve num of satellites in epoch
  num_sats = static_cast<int>(std::strtol(cline+32, &end, 10));
  if (errno || start==end) {
    std::cerr<<"\n[ERROR] ObservationRnx::__resolve_epoch_304__() failed to resolve num sats";
    std::cerr<<"\n        Line was: \""<<cline<<"\"";
    errno=0; return 4;
  }

  // resolve clock offset if any
  if (lnlen>41) {
    rcvr_coff = string_is_empty(cline+41) ? 0e0 : std::strtod(cline+41, &end);
    if (errno || start==end) {
      std::cerr<<"\n[ERROR] ObservationRnx::__resolve_epoch_304__() failed to resolve receiver clock offset";
      std::cerr<<"\n        Line was: \""<<cline<<"\"";
      errno=0; return 5;
    }
  }

  return 0;
}

/// Read and resolve observations of a given epoch
///
/// @param[in]  numsat  Number of satellites in current epoch
/// @param[in]  sysvec  Vector of satellite systems to consider (observations that
///                     belong to systems not in sysvec are ignored).
/// @return Anything other than 0 denotes an error
int
ObservationRnx::collect_epoch(int numsats, const std::vector<SATELLITE_SYSTEM>& sysvec)
noexcept
{
  char* end;
  char tmp[] = "  ";
  char obf[17]; obf[16]='\0';
  SATELLITE_SYSTEM s;

  // The stream should be open by now!
  if (!__istream.is_open()) return 1;
  
  // vector to collect observations
  std::vector<RawRnxObs__> obs(this->max_obs());

  int sat_it=0, prn;

  while (sat_it<numsats) {
    // resolve satellite system
    __istream.getline(__buf, __buf_sz);
    try {
      s = char_to_satsys(*__buf);
    } catch (std::exception& e) {
      std::cerr<<"\n[ERROR] ObservationRnx::collect_epoch() Failed to resolve Satellite System";
      std::cerr<<"\n        Line was: \""<<__buf<<"\"";
      return 1;
    }

    // if satellite system is to be collected ....
    if (std::find(std::begin(sysvec), std::end(sysvec), s) != std::end(sysvec)) {
      // resolve satellite prn
      std::memcpy(tmp, __buf+1, 2);
      prn = std::strtol(tmp, &end, 10);
      if ((errno || tmp==end) || (prn<1 || prn>99)) {
        std::cerr<<"\n[ERROR] ObservationRnx::collect_epoch() Failed to resolve Satellite PRN";
        std::cerr<<"\n        Line was: \""<<__buf<<"\"";
        errno=0; return 2;
      }
      // lets see what we are going to read ....
      auto const& obsvec = __obstmap[s];
      // how many observable types ?
      std::size_t obsnum = obsvec.size();
      // go ahead and resolve the observations
      for (std::size_t i=0; i<obsnum; i++) {
        std::memcpy(obf, __buf+3+i*16, 16);
        if (obs[i].resolve(obf)) {
          std::cerr<<"\n[ERROR] ObservationRnx::collect_epoch() Failed to collect observation";
          std::cerr<<"\n        Line was: \""<<__buf<<"\"";
          return 3;
        }
      }
    }
    ++sat_it;
  }
  return 0;
}

/// @brief Set map for reading RINEX observations
///
/// Given a vector of GnssObservable(s), this function will search through
/// the observables in the RINEX files and construct a map to assist reading
/// of the input stream. The map will have as key the SATELLITE_SYSTEM and as
/// values a vector of vectors; For each observable (per sat.sys.), the vector
/// will hold a pair, of index (aka RINEX column) and coefficient. E.g, if
/// the user wants to collect (a) GPS L3 (that is let's say: a1*L1C + a2*L2P)
/// and GPS L1C, then the resulting map will be of type (also suppose L1C is
/// writen is n1 column and L2P in n2 column):
/// std::map<SATELLITE_SYSTEM::gps, {{(n1,a1),(n2,a2)}, {(n1,1e0)}}>
///
std::map<ngpt::SATELLITE_SYSTEM, std::vector<ObservationRnx::vecof_idpair>>
ObservationRnx::set_read_map(const std::map<SATELLITE_SYSTEM, std::vector<GnssObservable>>& inmap)
const noexcept
{
  typedef std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>> ResultType;
  ResultType resmap;
  int status;
  for (auto const& inobs : inmap) {
    SATELLITE_SYSTEM s;
    for (auto const& i : inobs.second) { // for every GnssObservation
      auto newvec = this->obs_getter(i, s, status);
#ifdef DEBUG
      assert(s==inobs.first);
#endif
      if (status) {
        std::cerr<<"\n[ERROR] ObservationRnx::set_read_map() Failed to set getter for"
          <<" observable";
        return ResultType{};
      }
      auto it = resmap.find(s);
      if (it!=resmap.end()) {
        it->second.emplace_back(newvec);
      } else {
        resmap[s].emplace_back(newvec);
      }
    }
  }
  return resmap;
}

ObservationRnx::vecof_idpair
ObservationRnx::obs_getter(const GnssObservable& obs, SATELLITE_SYSTEM& sys, int& status)
const noexcept
{
  status = 0;
  // result vector
  vecof_idpair ovec;
  // the GnssObservable underlying vector
  auto vec = obs.underlying_vector();
  sys=vec[0].type().satsys();
  // vector of ObservationCodes for given sat. sys. in this RINEX
  auto it = __obstmap.find(sys);
  if (it==__obstmap.end()) {
    std::cerr<<"\n[ERROR] ObservationRnx::obs_getter() Rinex file does not contain obsrvations for satellite system: "<<satsys_to_char(sys);
    status=1; return vecof_idpair{};
  }
  const std::vector<ObservationCode>& satsys_codes = it->second;
  // ok, now: satsys_codes is the std::vector<ObservationCode> of the relevant satsys;
  // remember i is __ObsPart
  for (const auto& i : vec) {
    if (i.type().satsys() != sys) {
      std::cerr<<"\n[ERROR] ObservationRnx::obs_getter() Cannot handle mixed Satellite System observables";
      std::cerr<<"\n        Problem with GnssObservable: "<<obs.to_string();
      status=2; return vecof_idpair{};
    }
    // for every __ObsPart in correlate an ObservationCode
    auto j = std::find(satsys_codes.begin(), satsys_codes.end(), i.type().code());
    if (j==satsys_codes.end()) {
      std::cerr<<"\n[ERROR] Cannot find observable in RINEX ("
        <<i.type().code().to_string()<<")";
      status=3; return vecof_idpair{};
    }
    std::size_t idx = std::distance(satsys_codes.begin(), j);
    double coef = i.__coef;
    ovec.emplace_back(idx, coef);
  }
  return ovec;
}

/// param[in] sysobs A vector that contains info to 
int
ObservationRnx::sat_epoch_collect(const std::vector<vecof_idpair>& sysobs, 
  int& prn, std::vector<double>& vals)
const noexcept
{
  char tbuf[17];
  char* end;

  std::memset(tbuf, '\0', 17);
  std::memcpy(tbuf, __buf+1, 2);
  prn = std::strtol(tbuf, &end, 10);
  if ((errno || tbuf==end) || (prn<1 || prn>99)) return 1;
  
  RawRnxObs__ raw_obs;
  int k=0;
  for (const auto& obsrv : sysobs) {
    double obsval = 0e0;
    for (const auto& pr : obsrv) {
      std::size_t idx = pr.first;
      std::memcpy(tbuf, __buf+3+idx*16, 16);
      if (raw_obs.resolve(tbuf)) return 2;
      obsval += raw_obs.__val * pr.second;
    }
    vals[k] = obsval; ++k;
  }

  return 0;
}

int
ObservationRnx::collect_epoch(int numsats, std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>& mmap)
noexcept
{
  typedef typename std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>::iterator mmap_it;
  SATELLITE_SYSTEM s;
  int sat_it=0, prn;
  std::vector<double> vals(this->max_obs(), 0e0); 

  while (sat_it<numsats) {
    // resolve satellite system
    __istream.getline(__buf, __buf_sz);
    try {
      s = char_to_satsys(*__buf);
    } catch (std::exception& e) {
      std::cerr<<"\n[ERROR] ObservationRnx::collect_epoch() Failed to resolve Satellite System";
      std::cerr<<"\n        Line was: \""<<__buf<<"\"";
      return 1;
    }

    // if satellite system is to be collected ....
    mmap_it it = mmap.end();
    if ((it = mmap.find(s))!=mmap.end()) {
      if (sat_epoch_collect(mmap[s], prn, vals)) return 100;
    }
    ++sat_it;
  }
  return 0;
}

int
ObservationRnx::read_next_epoch()
{
  std::vector<SATELLITE_SYSTEM> ssvec = {SATELLITE_SYSTEM::gps, SATELLITE_SYSTEM::glonass, SATELLITE_SYSTEM::beidou};
  int c,j;
  if ( (c=__istream.peek()) != EOF ) {
    // if not EOF, read next line; it should be an epoche header line
    __istream.getline(__buf, __buf_sz);
    ngpt::modified_julian_day mjd;
    double sec, rcvr_coff;
    int flag, num_sats;
    // resolve epoch header
    if ((j=__resolve_epoch_304__(__buf, mjd, sec, flag, num_sats, rcvr_coff))) return 20+j;
    // resolve observation block
    if ((j=collect_epoch(num_sats, ssvec))) return 30+j;
  }
  if (__istream.eof()) {
    __istream.clear();
    return -1;
  }
  return 0;
}
