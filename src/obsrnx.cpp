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

/// Max satellites in eopch
constexpr int MAX_SAT_IN_EPOCH {80};

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
/// F14.3 -> the actual value of the observation (if empty use the 
///          RNXOBS_MISSING_VA Lvalue
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

/// Destructor; close the stream and delete the allocated buffer
ObservationRnx::~ObservationRnx() noexcept
{
  if (__istream.is_open()) __istream.close();
  if (__buf) {
    __buf_sz = 0;
    delete[] __buf;
  }
}

/// Loop through the observables of every satellite system, and return the
/// max number of observables any satellite system can have
/// Example:
///G   18 C1C L1C D1C S1C C1W S1W C2W L2W D2W S2W C2L L2L D2L  SYS / # / OBS TYPES
///       S2L C5Q L5Q D5Q S5Q                                  SYS / # / OBS TYPES
///E   20 C1C L1C D1C S1C C6C L6C D6C S6C C5Q L5Q D5Q S5Q C7Q  SYS / # / OBS TYPES
///       L7Q D7Q S7Q C8Q L8Q D8Q S8Q                          SYS / # / OBS TYPES
///S    8 C1C L1C D1C S1C C5I L5I D5I S5I                      SYS / # / OBS TYPES
///R   20 C1C L1C D1C S1C C1P L1P D1P S1P C2P L2P D2P S2P C2C  SYS / # / OBS TYPES
///       L2C D2C S2C C3Q L3Q D3Q S3Q                          SYS / # / OBS TYPES
///C   12 C2I L2I D2I S2I C7I L7I D7I S7I C6I L6I D6I S6I      SYS / # / OBS TYPES
///J   12 C1C L1C D1C S1C C2L L2L D2L S2L C5Q L5Q D5Q S5Q      SYS / # / OBS TYPES
///I    4 C5A L5A D5A S5A                                      SYS / # / OBS TYPES
/// In this case the function will return '20'
int
ObservationRnx::max_obs() const noexcept
{
  std::size_t sz, max=0;
  for (auto const& [key, val] : __obstmap) if ((sz=val.size())>max) max=sz;
  return max;
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
/*
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
*/

/// Given a map of SATSYS-GnssObservable(s), this function will search through
/// the observables in the RINEX files and construct a map to assist reading
/// of the input stream. The map will have as key the SATELLITE_SYSTEM and as
/// values a vector of vectors; For each observable (per sat.sys.), the vector
/// will hold a pair, of index (aka RINEX column) and coefficient.
/// Example:
/// Suppose the RINEX holds the following observables:
///G   18 C1C L1C D1C S1C C1W S1W C2W L2W D2W S2W C2L L2L D2L  SYS / # / OBS TYPES
///       S2L C5Q L5Q D5Q S5Q                                  SYS / # / OBS TYPES
///E   20 C1C L1C D1C S1C C6C L6C D6C S6C C5Q L5Q D5Q S5Q C7Q  SYS / # / OBS TYPES
///       L7Q D7Q S7Q C8Q L8Q D8Q S8Q                          SYS / # / OBS TYPES
///S    8 C1C L1C D1C S1C C5I L5I D5I S5I                      SYS / # / OBS TYPES
///R   20 C1C L1C D1C S1C C1P L1P D1P S1P C2P L2P D2P S2P C2C  SYS / # / OBS TYPES
///       L2C D2C S2C C3Q L3Q D3Q S3Q                          SYS / # / OBS TYPES
///C   12 C2I L2I D2I S2I C7I L7I D7I S7I C6I L6I D6I S6I      SYS / # / OBS TYPES
///J   12 C1C L1C D1C S1C C2L L2L D2L S2L C5Q L5Q D5Q S5Q      SYS / # / OBS TYPES
///I    4 C5A L5A D5A S5A                                      SYS / # / OBS TYPES
/// Further suppose the user want the following GnssObserbale(s):
/// input_map[GPS] = {G::C5Q*1.0, G::C1C*0.5+G::C2W*0.5}
/// input_map[GLO] = {R::C1P*1.0, R::C1C*0.3+R::C2P*0.30+R::C3Q*0.3}
/// input_map[GAL] = {E::C1C*1.0, E::C6C*0.3+E::C7Q*0.3+E::C8Q*0.3, E::C8Q*1.0}
/// The resulting map, will hold:
/// output_map[GPS] = {{(14, 1)}, {(0, 0.5), (6, 0.5)}}
/// output_map[GLO] = {{(4, 1)}, {(0, 0.3), (8, 0.3),(16, 0.3)}}
/// output_map[GAL] = {{(0, 1)}, {(4, 0.3), (12, 0.3), (16, 0.3)}, {(16, 1)}}
/// In this way we know that to form the GPS#2 observable (aka G::C1C*0.5+G::C2W*0.5)
/// we need the {(0, 0.5), (6, 0.5)} vector, which means COL#0*0.5 and 
/// COL#6*0.5
/// We actually have to columns (to read) and coefficients to formulate the
/// input GnssObservable(s).
///
/// @note Note that the input map (inmap) may change if some observables are
///       not found in the RINEX file (and the skip_missing flag is set to
///       true). For example, if we requested the folowing:
///       inmap[GPS]= {G::C5Q*1.000000, G::C1C*0.500000+G::C2W*0.500000}
///       but the observable C2W does not exist, then at output the vector will
///       be:
///       inmap[GPS]= {G::C5Q*1.000000}
///       If skip_missing is false then this will never happen as a missing 
///       observable will trigger an error and an empty map will be returned
/// 
/// In case of error, an empty map is returned (holding NO satellite systems)
///
/// @param[in] inmap A map with key SATELLITE_SYSTEM and values the respective
///                  vector of GnssObservables we want to read off the file
/// @param[in] skip_missing In case it is true, missing Observables and/or
///                  satellite systems will only produce a warning and will
///                  otherwise be ignored. If set to false, then they will
///                  trigger an error (and an empty map will be returned)
/// @reuturn A map with key SATELLITE_SYSTEM and values vector, of vectors of
///          <std::size_t, double> pairs; each pair denotes the index and 
///          coefficient of a RINEX observable (e.g. (13, 1.23) means read
///          column 14 and multiply value with 1.23). Inner vectors hold all
///          pairs for one GnssObservable (if a GnssObservable is a linear
///          combination, the respective vector's size will be > 1). Example:
///          map[GAL] = {{(0, 1)}, {(4, 0.3), (12, 0.3), (16, 0.3)}, {(16, 1)}}
std::map<ngpt::SATELLITE_SYSTEM, std::vector<ObservationRnx::vecof_idpair>>
ObservationRnx::set_read_map(std::map<SATELLITE_SYSTEM, std::vector<GnssObservable>>& inmap, bool skip_missing)
const noexcept
{
  typedef std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>> ResultType;
  ResultType resmap;

  // quick return if input map is empty
  if (inmap.empty()) return ResultType{};
  
  int status;
  // Loop through all elements of input map, aka:
  // inobs->first  : SATELLITE_SYSTEM
  // inobs->second : std::vector<GnssObservable>
  for (auto& inobs : inmap) {
    SATELLITE_SYSTEM s;
    // Loop through all elements of std::vector<GnssObservable> for a sat sys
    for (auto i=inobs.second.begin(); i!=inobs.second.end();) {
#ifdef DEBUG
      std::cout<<"\n[DEBUG] Resolving observable: "<<i->to_string();
      auto __it = resmap.find(inobs.first);
      if (__it==resmap.end()) {
        std::cout<<"\n[DEBUG(1)] Map empty for sat sys "<<satsys_to_char(inobs.first);
      } else {
        std::cout<<"\n[DEBUG(1)] Map["<<satsys_to_char(inobs.first)<<"] size is "<<__it->second.size();
      }
#endif
      // for every GnssObservable get a vector of vectors of pairs, e.g.
      // {(4, 0.3), (12, 0.3), (16, 0.3)}
      auto newvec = this->obs_getter(*i, s, status);
      // inapropriate sat sys or status>0
      if (s!=inobs.first || status>0) {
        std::cerr<<"\n[ERROR] ObservationRnx::set_read_map() Failed to set getter for"
          <<" observable: "<<i->to_string();
        return ResultType{};
      }
      // status is ok; add vector
      if (!status) {
        auto it = resmap.find(s);
        if (it!=resmap.end()) {
          it->second.emplace_back(newvec);
        } else {
          resmap[s].emplace_back(newvec);
        }
        ++i;
        std::cout<<"\n[DEBUG] Added vector for observable";
      // status is < 0 (aka something is missing, hence got an empty vector
      // if we don;t exit with error, then this GnssObservable must be removed
      // from the input map; otherwise the order of the input and output maps
      // will not be correct!
      } else {
        std::cerr<<"\n[WARNING] ObservationRnx::set_read_map() Cannot handle Observable:"
          <<i->to_string();
        if (skip_missing) {
          std::cerr<<"\n          Missing either Satellite System or Observation Type(s)";
          std::cerr<<"\n          Observable will be ignored!";
          i=inobs.second.erase(i);
          std::cout<<"\n[DEBUG] Did not add vector for observable";
        } else {
          return ResultType{};
        }
      }
#ifdef DEBUG
      __it = resmap.find(inobs.first);
      if (__it==resmap.end()) {
        std::cout<<"\n[DEBUG(2)] Map empty for sat sys "<<satsys_to_char(inobs.first);
      } else {
        std::cout<<"\n[DEBUG(2)] Map["<<satsys_to_char(inobs.first)<<"] size is "<<__it->second.size();
      }
#endif
      // have you augmented the itertaor ??
      // ++i;
    }
  }

  // before returning check for empty keys and erase them
  for (auto cit=resmap.cbegin(); cit!=resmap.cend();) {
    if (!cit->second.size()) {
      resmap.erase(cit++);
    } else {
      ++cit;
    }
  }
  return resmap;
}

/// Given a GnssObservable return a vector of <index coefficient> pairs so that
/// when reading a relevant satellite record line we know what columns to read
/// (and the respective coefficients) to formulate that GnssObservable.
/// E.g.
/// Suppose the RINEX holds the following observables:
///G   18 C1C L1C D1C S1C C1W S1W C2W L2W D2W S2W C2L L2L D2L  SYS / # / OBS TYPES
///       S2L C5Q L5Q D5Q S5Q                                  SYS / # / OBS TYPES
/// and we want to collect the GnssObservable "G::C1C*0.5+G::C2W*0.5"
/// the result will be the vector: {(0, 0.5), (6, 0.5)}. For each pair the
/// first element is the (relevant row's) column index and the second element
/// is the corresponding coefficient.
/// The function only accepts GnssObservable(s) of a single satellite system
///
/// @param[in]  obs  The GnssObservable to get info for
/// @param[out] sys  The satellite system of the observable
/// @param[out] status The return status as follows:
///                  -2 : Observable does not exist in RINEX file
///                  -1 : Satellite system (of observable) does not exist in RINEX
///                   0 : all ok
///                   1 : Observable is of mixed-satellite type
///                   If the status is not 0, then the returned vector is empty 
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
    std::cerr<<"\n[WARNING] ObservationRnx::obs_getter() Rinex file does not contain obsrvations for satellite system: "<<satsys_to_char(sys);
    status=-1; return vecof_idpair{};
  }
  const std::vector<ObservationCode>& satsys_codes = it->second;
  // ok, now: satsys_codes is the std::vector<ObservationCode> of the relevant satsys;
  // remember i is __ObsPart
  for (const auto& i : vec) {
    if (i.type().satsys() != sys) {
      std::cerr<<"\n[ERROR] ObservationRnx::obs_getter() Cannot handle mixed Satellite System observables";
      std::cerr<<"\n        Problem with GnssObservable: "<<obs.to_string();
      status=1; return vecof_idpair{};
    }
    // for every __ObsPart in correlate an ObservationCode
    auto j = std::find(satsys_codes.begin(), satsys_codes.end(), i.type().code());
    if (j==satsys_codes.end()) {
      std::cerr<<"\n[WARNING] Cannot find observable in RINEX ("
        <<i.type().code().to_string()<<")";
      status=-2; return vecof_idpair{};
    }
    std::size_t idx = std::distance(satsys_codes.begin(), j);
    double coef = i.__coef;
    ovec.emplace_back(idx, coef);
  }
  return ovec;
}

/// Collect observation(s) values off from a satellite record line. The
/// function will not read all values in line; it will only read and resolve
/// the values in the indexes given in the input pairs. E.g., if we pass in
/// the vector: {(0, 0.5), (6, 0.5)}, the function will return two values:
/// 1st -> value at index 0 multiplied by 0.5
/// 2nd -> value at index 6 multiplied by 0.5
/// If any of the values to be collected hold RNXOBS_MISSING_VALUE, then the
/// respective GnssObservable will also have the same value (aka 
/// RNXOBS_MISSING_VALUE).
///
/// @param[in] sysobs A vector that contains info to collect, i.e. pairs of
///                   index and coefficients. E.g. {(0, 0.5), (6, 0.5)}
/// @param[out] prn   The PRN of the satellite
/// @param[out] vals  A vector containing the observation values collected;
///                   The size of this vector will be equal to the size of the 
///                   input vector. 
/// @return           An integer, denoting the following:
///                   -1 : some observable is missing
///                    0 : all ok
///                   >1 : error
/// @warning The function will not 'push_back' the collected values (in vector
///          vals) but access them (and alter them) via operator '[]'. Hence,
///          the vector vals should have enough size at input!
int
ObservationRnx::sat_epoch_collect(const std::vector<vecof_idpair>& sysobs, 
  int& prn, std::vector<double>& vals)
const noexcept
{
  char tbuf[17];
  char* end;
  int status=0;

#ifdef DEBUG
  assert(vals.size()>=sysobs.size());
#endif

  std::memset(tbuf, '\0', 17);
  std::memcpy(tbuf, __buf+1, 2);
  prn = std::strtol(tbuf, &end, 10);
  if ((errno || tbuf==end) || (prn<1 || prn>99)) return 1;
  
  RawRnxObs__ raw_obs;
  int k=0;
  std::size_t len = std::strlen(__buf);
  for (const auto& obsrv : sysobs) { // For every observable in vector
    double obsval = 0e0;
    for (const auto& pr : obsrv) { // for every raw obs. in observable
      std::size_t idx = pr.first*16 +3;
      if (len>=idx+16) { // line ends before column of observable
        std::memcpy(tbuf, __buf+idx, 16);
        if (raw_obs.resolve(tbuf)) return 2;
        if (raw_obs.__val!=RNXOBS_MISSING_VAL) {
          obsval += raw_obs.__val * pr.second;
        } else {
          --status;
          obsval = RNXOBS_MISSING_VAL;
        }
      } else {
        raw_obs.__val = obsval = RNXOBS_MISSING_VAL;
      }
    }
    vals[k] = obsval; ++k;
  }

  return status;
}

/// Read and collect values from a satellite record block. The function will 
/// read all lines (for the given epoch) and collect the values (per satellite)
/// based on the input map.
/// The stream should be at the start of the end of the epoch header line; hence
/// the next line to read should be a satellite record line. We should already 
/// know how many satellites there are in the epoch.
/// What are we going to read? First of all, the function can read both GNSS
/// raw observables (e.g. GPS-C1C) as well as linear combinations of these;
/// actually the function reads GnssObservable(s) which is exactly that.
/// The input vector contains pairs of index values and coefficients to read off
/// observation values. For example, if the value of the input map for GPS is
/// output_map[GPS] = {{(14, 1)}, {(0, 0.5), (6, 0.5)}}, when encountering a
/// GPS satellite the function will read the respective index values (aka 14,
/// 0 and 6) and use the respective coefficient factors to formulate two
/// observation values.
/// To construct the input map, see the function @see ObservationRnx::set_read_map
///
/// @param[in] numsats  The number of satellites that follow
/// @param[out] satscollected Actual number of satellites for which we collected
///                     observation values (stored in satobs)
/// @param[in] mmap     Map where key is satellite system and values are a vector
///                     with elements one vector per GnssObservation, containing
///                     pairs of (col.index, factor).
/// @param[out] satobs  The collected results; that is a vector of pairs of
///                     type <Satellite, vector<double>> where for each satellite
///                     we have the observation values in one-to-one correspondance
///                     (in the same order) as in the mmap[SATSYS] vector. The
///                     elements in range [0, satscollected) hold the results 
///                     (indexes larger than satscollected hold garbage) 
/// @warning 
///        * The function will not 'push_back' the collected values (in vector
///          satobs) but access them (and alter them) via iterators. Hence,
///          the vector vals should have enough size at input!
///        * If an observable has an empty value in the RINEX files (that is
///          RNXOBS_MISSING_VAL) the observable will exist in the output
///          vector (vals) but its value will still be RNXOBS_MISSING_VAL
///        * The resulting vector may have size larger than the satellites
///          actually read (the function does not erase/delete on satobs). Hence
///          only use elements in range [0, satscollected)
int
ObservationRnx::collect_epoch(int numsats, int& satscollected, 
  std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>& mmap,
  std::vector<std::pair<ngpt::Satellite, std::vector<double>>>& satobs)
noexcept
{
  typedef typename std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>::iterator mmap_it;
  using OutputVecIt = std::vector<std::pair<ngpt::Satellite, std::vector<double>>>::iterator;

  satscollected=0;
#ifdef DEBUG
  assert(satobs.size()>=(std::size_t)numsats);
#endif
  SATELLITE_SYSTEM s;
  int sat_it=0, prn;
  OutputVecIt ovec_it = satobs.begin();

  // loop through every satellite in epoch
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
    int status=0;
    mmap_it it = mmap.end();
    if ((it = mmap.find(s))!=mmap.end()) {
      if ((status=sat_epoch_collect(mmap[s], prn, ovec_it->second))>0) return 1;
      Satellite sat(s, prn);
      ovec_it->first = sat;
      ++ovec_it;
      ++satscollected;
    }
    ++sat_it;
  }
  return 0;
}

/// Read the next (following) satellite record block off from the RINEX stream.
/// Here 'next' just means the next after the current stream's get position.
/// So, the function will read in a block of satellite records, see what
/// it is supposed to extract (this info is contained in the input map) and
/// return the resolved observations in a vector.
/// The info on what to actually read (per satellite system) is contained in the
/// input map; this holds (per sat, system) a vector of vectors (one per 
/// GnssObservable); each such vector contains one or more pairs of <index, coefs>
/// to formulate the GnssObservable.
/// So the function will first check the satellite and then check the mmap to
/// see which columns we need to collect. It will formulate all GnssObservabe(s)
/// for this sat. system and store the result in the output vector satobs;
/// So, satobs will have REAL size equal to the number of satellites read and 
/// resolved (which can obviously be smaller that number of satellites in epoch).
/// Each entry will be a pair of <Satellite, vector<double>>, where the second 
/// element is actually the GnssObservables collected; this internal vector will 
/// have REAL size equal to the element of the corresponding sat. system in the
/// input map. That is if satobs[i]=<G01, {20.e0, 21,e0 ,....}> the REAL size
/// of the vector is equal to mmap[s].size()
/// If any GnssObservable is absent in RINEX file or one of its elements is absent
/// then it will hold the value RNXOBS_MISSING_VAL
/// REAL size here means the ACTUAL SIZE OF RELEVANT DATA in a vector. E.g.,
/// normally satobs will have a size larger than that (so that we don;t have to
/// allocate). So, it size will be satobs.size() but it's REAL size will be
/// 'sats'.
///
/// @param[in] mmap     Map where key is satellite system and values are a vector
///                     with elements one vector per GnssObservation, containing
///                     pairs of (col.index, factor).
/// @param[out] satobs  Vector of results; aka pairs of Satellite and vector<double>
///                     holding the observable values for each of the input
///                     GnssObservables (as in mmap[satsys]). Only use the
///                     values in range [0, sats). To create this vector, see
///                     ObservationRnx::initialize_epoch_vector()
/// @param[out] sats    Actual number of satellites written in satobs vector
/// @return An integer as:
///                     * -1 : EOF encountered
///                     *  0 : All ok
///                     * >0 : ERROR do not use the output vector satobs
///
/// @warning
///      * Please use the function ObservationRnx::initialize_epoch_vector to
///        initialize a long enough output vector to pass in this function (as
///        satobs). If this vector is not long enough it can cause problems.
int
ObservationRnx::read_next_epoch(std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>& mmap, std::vector<std::pair<ngpt::Satellite, std::vector<double>>>& satobs, int& sats, ngpt::modified_julian_day& mjd, double& secofday) noexcept
{
  int c,j;
  sats=0;
  if ((c=__istream.peek()) != EOF) {
    // if not EOF, read next line; it should be an epoche header line
    __istream.getline(__buf, __buf_sz);
    double rcvr_coff;
    int flag, num_sats;
    // resolve epoch header
    if ((j=__resolve_epoch_304__(__buf, mjd, secofday, flag, num_sats, rcvr_coff))) return 20+j;
    // resolve observation block
    if ((j=collect_epoch(num_sats, sats, mmap, satobs))) return 30+j;
  }
  if (__istream.eof()) {
    __istream.clear();
    return -1;
  }
  return 0;
}

/// This function will return a vector with a big enough size to read all epochs
/// in the RINEX file. Before start reading epochs in a RINEX, call this function
/// with the input map that will be used for reading, and get the output vector
/// to then use in the function ObservationRnx::read_next_epoch()
/// @param[in] mmap  The map to use for reading this instance
/// @return a vector that can hold any epoch's satellite records.
std::vector<std::pair<ngpt::Satellite, std::vector<double>>>
ObservationRnx::initialize_epoch_vector(std::map<SATELLITE_SYSTEM, std::vector<vecof_idpair>>& mmap)
const noexcept
{
  std::size_t max_obs=0, tmp;
  for (const auto& it : mmap) if ((tmp=it.second.size())>max_obs) max_obs=tmp;
  std::pair<Satellite, std::vector<double>> emptyp
    {Satellite(), std::vector<double>(max_obs, RNXOBS_MISSING_VAL)};
  std::vector<std::pair<ngpt::Satellite, std::vector<double>>> vec(MAX_SAT_IN_EPOCH, emptyp);
  return vec;
}
