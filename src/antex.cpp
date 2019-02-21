#include <stdexcept>
#include <cstring>
#include <cassert>
#include "antex.hpp"
#include "ggdatetime/datetime_read.hpp"
#ifdef DEBUG
#include <iostream>
#endif

using ngpt::Antex;
using ngpt::Antenna;
using ngpt::Satellite;

/// No header line can have more than 80 chars. However, there are cases when
/// they  exceed this limit, just a bit ...
constexpr int MAX_HEADER_CHARS { 85 };

/// Max header lines.
constexpr int MAX_HEADER_LINES { 1000 };

/// Size of 'END OF HEADER' C-string.
/// std::strlen is not 'constexr' so eoh_size can't be one either. Note however
/// that gcc has a builtin constexpr strlen function (if we want to disable this
/// we can do so with -fno-builtin).
#ifdef __clang__
    const     std::size_t eoh_size { std::strlen("END OF HEADER") };
#else
    constexpr std::size_t eoh_size { std::strlen("END OF HEADER") };
#endif

/// Max grid line chars; i.e. the maximum number of chars any pcv line can
/// hold. Typical 'ZEN1 / ZEN2 / DZEN' = '0.0  90.0   5.0' and the format is
/// mF8.2 which is 19*8 = 152 + (~8 starting chars) = 160.
/// Let's make enough space for DZEN = 3 -> max chars = 31*8 + 10 = 258
///
/// Update Feb.2019
/// Need to increase this limit cause founf field:
/// 'ZEN1 / ZEN2 / DZEN' = '0.0  20.0   0.5'
/// 20/0.5 = 40 numbers, 40*8 = 320 chars (at least)
/// setting MAX_GRID_CHARS to 512
///
/// @warning When using this constant always assert something like
///          8*(std::size_t)((zen2-zen1)/dzen) < MAX_GRID_CHARS-10
constexpr std::size_t MAX_GRID_CHARS { 512 };

// Forward declerationof non Antex:: functions;
int
__collect_pco__(std::ifstream&, ngpt::AntennaPcoList&);

/// @details Antex Constructor, using an antex filename. The constructor will
///          initialize (set) the _filename attribute and also (try to)
///          open the input stream (i.e. _istream).
///          If the file is successefuly opened, the constructor will read
///          the ANTEX header and assign info.
Antex::Antex(const char* filename)
: __filename   (filename),
  __istream    (filename, std::ios_base::in),
  __satsys     (satellite_system::mixed),
  __version    (Antex::atx_version::v14),
  __end_of_head(0)
{
    if (read_header()) {
          if (__istream.is_open()) __istream.close();
          throw std::runtime_error("[ERROR] Failed to read antex header");
    }
}

/// @details Read an Antex (instance) header. The format of the header should
///          closely follow the antex format specification (version 1.4).
///          This function will set the instance fields:
///          - __version (should be 1.4),
///          - __satsys,
///          - __type,
///          - __end_of_head
///          The function will exit after reading a header line, ending with
///          the (sub)string 'END OF HEADER'.
///
/// @warning 
///          - The instance's input steam (i.e. _istream) should be open and 
///          valid.
///          - Note that the function expects that no header line contains more
///          than MAX_HEADER_CHARS chars.
///          - If the header is not read correctly, then the (opened) file
///          buffer will be closed.
///
/// @return  Anything other than '0' is an error.
///
/// @see https://github.com/xanthospap/ngpt/blob/dev/src/antex.cpp
int
Antex::read_header()
{
    char line[MAX_HEADER_CHARS];

    // The stream should be open by now!
    if (!__istream.is_open()) return 1;

    // Go to the top of the file.
    __istream.seekg(0);

    // Read the first line. Get version and system.
    // ----------------------------------------------------
    __istream.getline(line, MAX_HEADER_CHARS);
    // strtod will keep on reading untill a non-valid
    // char is read. Fuck this, lets split the string
    // so that it only reads the valid chars (for version).
    *(line+15) = '\0';
    float fvers = std::strtod(line,nullptr);
    if (std::abs(fvers - 1.4) < .001) {
        __version = Antex::atx_version::v14;
    } else if (std::abs(fvers - 1.3) < .001) {
        __version = Antex::atx_version::v13;
    } else {
#ifdef DEBUG
        std::cerr<<"\n[ERROR] read_header() version";
#endif
        return 10;
    }
    // Resolve the satellite system.
    try {
        __satsys = ngpt::char_to_satsys(line[20]);
    } catch (std::runtime_error& e) {
#ifdef DEBUG
        std::cerr<<"\n[ERROR] read_header() satsys";
#endif
        return 11;
    }

    // Read the second line. Get PCV TYPE / REFANT.
    // If the atx is of type relative, then just return with error
    // ----------------------------------------------------
    __istream.getline(line, MAX_HEADER_CHARS);
    if (*line != 'A') {
#ifdef DEBUG
        std::cerr<<"\n[ERROR] read_header() type";
#endif
        return 12;
    }

    // Keep on readling lines until 'END OF HEADER'.
    // ----------------------------------------------------
    int dummy_it = 0;
    __istream.getline(line, MAX_HEADER_CHARS);
    while (dummy_it < MAX_HEADER_LINES && 
            strncmp(line+60, "END OF HEADER", eoh_size) ) {
        __istream.getline(line, MAX_HEADER_CHARS);
        dummy_it++;
    }
    if (dummy_it >= MAX_HEADER_LINES) {
#ifdef DEBUG
        std::cerr<<"\n[ERROR] read_header() no end of header";
#endif
        return 20;
    }

    // Mark the end of header
    __end_of_head = __istream.tellg();

    // All done !
    return 0;
}

/// Read the next antenna in the Antex file
///
/// Function assumes that that the stream is open and placed at a position
/// ready to read an antenna; this means, that the first line to be read is
/// "START OF ANTENNA" field.
/// The function will also read the field "TYPE / SERIAL NO" and return (this)
/// Antenna instance.
/// Then it will immidiately exit, leaving the stream at the field (aka next
/// line to be read will be) "METH / BY / # / DATE"
///
/// @return Anything other than 0, denotes an abnormal exit; An int < 0 denotes
///         EOF, while any integer > 0 denotes an error.
int
Antex::read_next_antenna_type(Antenna& antenna, char* c)
{
    char line[MAX_HEADER_CHARS];

    // The stream should be open by now!
    if (!__istream.is_open()) return 1;

    // Read the first line. Should be "START OF ANTENNA"
    // ----------------------------------------------------
    if (!__istream.getline(line, MAX_HEADER_CHARS) ||
            std::strncmp(line+60, "START OF ANTENNA", 16)) {
        if (__istream.eof()) {
            __istream.clear();
            return -1;
        }
#ifdef DEBUG
        std::cerr<<"\n[ERROR] read_next_antenna_type() no START OF ANTENNA";
#endif
        return 10;
    }

    // Read the next line. Should be "TYPE / SERIAL NO"
    // ----------------------------------------------------
    if (!__istream.getline(line, MAX_HEADER_CHARS) ||
            std::strncmp(line+60, "TYPE / SERIAL NO", 16)) {
#ifdef DEBUG
        std::cerr<<"\n[ERROR] read_next_antenna_type() no TYPE SERIAL NO";
#endif
        return 11;
    }
    // assign the antenna
    antenna = Antenna(line);
    
    //
    if (c) std::memcpy(c, line, sizeof(char)*60);

    // all done; return
    return 0;
}

/// Skip the rest of the antenna information and go to next one.
/// 
/// It is expected that the antex is left at position "METH / BY / # / DATE",
/// (aka this is the first line to be read). The function will then continue
/// reading the rest of the fields/lines untill "END OF ANTENNA" is read.
int
Antex::skip_rest_of_antenna()
{
    char line[MAX_GRID_CHARS];

    // next field is 'METH / BY / # / DATE'
    if (!__istream.getline(line, MAX_GRID_CHARS)
        || std::strncmp(line+60, "METH / BY / # / DATE", 20)) {
#ifdef DEBUG
        std::cerr<<"\n[ERROR] skip_rest_of_antenna() no METH / BY / DATE";
#endif
        return 10;
    }

    int dummy_it = 0;
    do {
        __istream.getline(line, MAX_GRID_CHARS);
        dummy_it++;
    } while (std::strncmp(line+60, "END OF ANTENNA", 14) && dummy_it < 5000);

    if (dummy_it >= 5000) {
#ifdef DEBUG
        std::cerr<<"\n[ERROR] skip_rest_of_antenna() no END OF ANTENNA";
#endif
        return 1;
    }
    
    return 0;
}

/// Try to match a given antenna to a record in the antex file. The funtion
/// will try to match at least the model+radome and if possible also match
/// serial.
/// If the function does find a match (and returns <1), then it will leave the
/// stream at the field (aka next line to be read will be) 
/// "METH / BY / # / DATE".
/// @param[in] ant_in    Antenna to match in antex
/// @param[out] ant_out  If return value <= 0, then matched antenna as recorded
///                      in the antex file; else no valid antenna at all
/// @param[out] ant_pos  If return value <= 0, then matched antenna position
///                      in the input (antex) stream.
/// @return              -1 exact match (model+radome+serial)
///                       0 match model+radome
///                      >0 no match or error
int
Antex::find_closest_antenna_match(const Antenna& ant_in,
    Antenna& ant_out, pos_type& ant_pos)
{
    // go to the top of the file, after the header
    __istream.seekg(__end_of_head, std::ios_base::beg);

    Antenna cur_ant,
            best_match_ant;
    pos_type best_match_pos;
    int stat1,
        stat2;

    while (!(stat1 = read_next_antenna_type(cur_ant))) {
        if ( cur_ant.is_same(ant_in) ) {
            ant_out = cur_ant;
            ant_pos = __istream.tellg();
            return -1;
        }
        if ( cur_ant.compare_model(ant_in) && !cur_ant.has_serial() ) {
            best_match_ant = cur_ant;
            best_match_pos = __istream.tellg();
        }
        if ((stat2 = skip_rest_of_antenna())) break;
    }
    // some error status is set
    if (stat1 > 0 || stat2) {
        return 1;
    }
    ant_out = best_match_ant;
    ant_pos = best_match_pos;
    return 0;
}

int
__resolve_satellite_antenna_line__(const char* line, Satellite& sat)
{
    if (std::strlen(line) < 60) return 1;
    
    // first 20 chras are antenna type
    sat.antenna().set_from_cstr(line);

    // next twenty chars (aka line[20-40]) are:
    // Satellite code "sNN" (blank: all representatives of the specified
    // antenna type)
    // For the selection of single satellites the satellite system flag ('G','R',
    // 'E','C','J','S') together with the PRN number (GPS, Compass), the slot 
    // number (GLONASS), the SVID number (Galileo), the 'PRN number minus 192' 
    // (QZSS) or the 'PRN number minus 100' (SBAS) has to be specified.
    if (line[20] != ' ') {
        try {
            sat.system() = ngpt::char_to_satsys(line[20]);
            sat.prn()    = std::stoi(std::string(line+21, 5));
        } catch (std::exception&) {
            return 5;
        }
    }

    // next 10 fields (aka line[40-50]) are:
    // Satellite code "sNNN" (optional)
    // s    - satellite system flag
    // NNN  - SVN number (GPS), GLONASS
    //        number, GSAT number (Galileo)
    //        or SVN number (QZSS); blank
    //        (Compass, SBAS)
    if (line[40] != ' ') {
        if (line[40] != line[20]) {
            return 10;
        }
        if (line[40] != 'S' && line[40] != 'C') {
            sat.svn() = std::stoi(std::string(line+41, 5));
        }
    }
    return 0;
}

int
__find_time_interval__(std::ifstream& fin, const ngpt::datetime<ngpt::seconds>& at)
{
    char line[MAX_GRID_CHARS];

    // next field is 'METH / BY / # / DATE'
    if (!fin.getline(line, MAX_GRID_CHARS)
        || std::strncmp(line+60, "METH / BY / # / DATE", 20)) {
#ifdef DEBUG
        std::cerr<<"\n[ERROR] skip_rest_of_antenna() no METH / BY / DATE";
#endif
        return 10;
    }

    int  dummy_it = 0;
    bool from_ok = false,
         to_ok   = false;
    ngpt::datetime<ngpt::seconds> from,
                                  to;
    do {
        fin.getline(line, MAX_GRID_CHARS);
        dummy_it++;
        if (!std::strncmp(line+60, "VALID FROM", 10)) {
            from = ngpt::strptime_ymd_hms<ngpt::seconds>(line);
            from_ok = true;
        } else if (!std::strncmp(line+60, "VALID UNTIL", 11)) {
            to = ngpt::strptime_ymd_hms<ngpt::seconds>(line);
            to_ok = true;
        }
    } while (std::strncmp(line+60, "END OF ANTENNA", 14) && dummy_it < 5000);

    if (dummy_it >= 5000) {
#ifdef DEBUG
        std::cerr<<"\n[ERROR] skip_rest_of_antenna() no END OF ANTENNA";
#endif
        return 1;
    }
    
    if (!from_ok || !to_ok) {
        return 5;
    } else {
        return (from >= at && at <= to);
    }
}

int
Antex::find_satellite_antenna(int prn, satellite_system ss, const ngpt::datetime<ngpt::seconds>& at)
{
    char line[MAX_HEADER_CHARS];
    
    // go to the top of the file, after the header
    __istream.seekg(__end_of_head, std::ios_base::beg);

    Antenna   cur_ant;
    Satellite cur_sat;
    int stat1,
        stat2;

    while ( !(stat1 = read_next_antenna_type(cur_ant, line)) ) {
        cur_sat.system() = satellite_system::mixed;
        if (!__resolve_satellite_antenna_line__(line, cur_sat)) {
            if (cur_sat.prn() == prn && cur_sat.system() == ss) {
                std::cout<<"\nMatched satellite at: "<<line;
                if (!__find_time_interval__(__istream, at)) {
                    std::cout<<"\nFuck yeah, matched date!";
                }
            }
        } else {
            // std::cout<<"\nUnresolved antenna line: "<<line;
            ;
        }
        if ((stat2 = skip_rest_of_antenna())) break;
    }
    // some error status is set
    if (stat1 > 0 || stat2) {
        return 1;
    }

    return 0;
}

/// Get the list of PCO values for a given antenna (aka PCO values for each of
/// the observation+sats.sys codes in the ANTEX files).
/// @param[in]  ant_in    The antenna for which we want the PCO
/// @param[out] pco_list  The list of recorded PCO values for each satellite
///                       system and observation code. pco_list will be
///                       cleared at function entry (this any elements it
///                       holds at input will be removed).
/// @param[in] must_match_serial If set to true, then we will only match
///                       antennas that apart from same model+radome also have
///                       same serial (aka, we will be searching for ant_in
///                       exactly in the ANTEX file).
/// @return               0 : all ok, antenna matched
///                       1 : antenna was not found (model+radome)
///                       10: antenna (model+radome) found, but serial did not
///                           match
int
Antex::get_antenna_pco(const Antenna& ant_in, AntennaPcoList& pco_list,
    bool must_match_serial)
{
    pos_type ant_pos;
    Antenna  ant_out;

    // clean any entries in pco_list
    pco_list.__vecref__().clear();
    
    // match the antenna
    int ant_found = find_closest_antenna_match(ant_in, ant_out, ant_pos);
    if (ant_found>0) {
        return ant_found;
    } else if (!ant_found && must_match_serial) {
        return 10;
    }

    // go to the position where the antenna was found
    __istream.seekg(ant_pos, std::ios_base::beg);

    // we should now be ready to read "METH / BY / # / DATE"
    int status = __collect_pco__(__istream, pco_list);

    return status;
}

/// Given an antex input stream (aka its __istream), placed just after the
/// field "TYPE / SERIAL NO" (i.e. next line to read should be the field
/// "METH / BY / # / DATE"), collect the list of PCO values (for every
/// satetllite-system and observation code recorded.
/// @param[in] fin       The antex input stream (aka instance's __istream)
/// @param[out] pco_list Collected PCO values
/// @return
int
__collect_pco__(std::ifstream& fin, ngpt::AntennaPcoList& pco_list)
{
    char hline[MAX_HEADER_CHARS];
    char gline[MAX_GRID_CHARS];
    char tmp[11];
    
    // next field is 'METH / BY / # / DATE'
    if (!fin.getline(hline, MAX_HEADER_CHARS)
            || strncmp(hline+60, "METH / BY / # / DATE", 20)) {
        return 1;
    }

    // next field is 'DAZI'
    if (!fin.getline(hline, MAX_HEADER_CHARS)
            || strncmp(hline+60, "DAZI", 4)) {
        // dazi = std::stof(hline+2, nullptr);
        return 2;
    }

    // next field is 'ZEN1 / ZEN2 / DZEN'
    if (!fin.getline(hline, MAX_HEADER_CHARS)
            || strncmp(hline+60, "ZEN1 / ZEN2 / DZEN", 18)) {
        return 3;
    }

    // next field is '# OF FREQUENCIES'
    int num_of_freqs = 0;
    if (!fin.getline(hline, MAX_HEADER_CHARS)
            || strncmp(hline+60, "# OF FREQUENCIES", 16)) {
        return 4;
    } else {
        num_of_freqs = std::stoi(std::string(hline, 6), nullptr);
    }

    // From here up untill the block 'START OF FREQUENCY' there can be a number
    // of optional fields.
    do {
        fin.getline(hline, MAX_HEADER_CHARS);
    } while (fin && strncmp(hline+60, "START OF FREQUENCY", 18));

    ngpt::satellite_system ss;
    ngpt::ObservationCode  obsc;
    int bn;
    double dn, de, du;
    for (int i=0; i<num_of_freqs; i++) {
        // make sure we are at the START OF FREQUENCY line
        if (!fin || strncmp(hline+60, "START OF FREQUENCY", 18) ) {
            // wait!! this might be a frequency rms from the previous frequency
            if (!strncmp(hline+60, "START OF FREQ RMS", 17)) {
                do {
                    fin.getline(gline, MAX_GRID_CHARS);
                } while (fin && strncmp(gline+60, "END OF FREQ RMS", 15));
                if (!fin.getline(hline, MAX_HEADER_CHARS) 
                        || strncmp(hline+60, "START OF FREQUENCY", 18) ) {
                    return 11;
                }
            } else {
                return 5;
            }
        }
        // resolve satellite system and observation code
        try {
            ss = ngpt::char_to_satsys(hline[3]);
            bn = std::stoi(std::string(hline+4, 2));
        } catch (std::exception &) {
            return 6;
        }
        obsc.band() = bn;
        // next field is 'NORTH / EAST / UP'
        if ( fin.getline(hline, MAX_HEADER_CHARS) 
                && !strncmp(hline+60, "NORTH / EAST / UP", 17)) {
            tmp[10] = '\0';
            std::memcpy(tmp, hline, 10);
            dn = std::stod(tmp);
            std::memcpy(tmp, hline+10, 10);
            de = std::stod(tmp);
            std::memcpy(tmp, hline+20, 10);
            du = std::stod(tmp);
        } else {
            return 7;
        }
        // assign to a new pco and add to list
        pco_list.__vecref__().emplace_back(obsc, ss, dn, de, du);
        // skip lines until "END OF FREQUENCY"
        do {
            fin.getline(gline, MAX_GRID_CHARS);
        } while (fin && strncmp(gline+60, "END OF FREQUENCY", 16));
        // read the following line into hline
        fin.getline(hline, MAX_HEADER_CHARS);
    }

    return 0;
}
